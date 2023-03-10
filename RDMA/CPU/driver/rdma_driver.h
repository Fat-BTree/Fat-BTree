#include "rdma_common.h"
#include <bits/stdc++.h>

using namespace std;

class rdma_client {
public:
  int rdma_client_setup(char *, void *, uint64_t);
  int read_remote_node(void *, uint64_t = 0);

private:
  /* These are basic RDMA resources */
  /* These are RDMA connection related resources */
  struct rdma_event_channel *cm_event_channel = NULL;
  struct rdma_cm_id *cm_client_id = NULL;
  struct ibv_pd *pd = NULL;
  struct ibv_comp_channel *io_completion_channel = NULL;
  struct ibv_cq *client_cq = NULL;
  struct ibv_qp_init_attr qp_init_attr;
  struct ibv_qp *client_qp;
  /* These are memory buffers related resources */
  struct ibv_mr *client_metadata_mr = NULL, *client_src_mr = NULL,
                *client_dst_mr = NULL, *server_metadata_mr = NULL;

  struct ibv_mr *remote_node_mr = NULL;

  struct rdma_buffer_attr client_metadata_attr, server_metadata_attr;
  struct ibv_send_wr client_send_wr, *bad_client_send_wr = NULL;
  struct ibv_recv_wr server_recv_wr, *bad_server_recv_wr = NULL;
  struct ibv_sge client_send_sge, server_recv_sge;
  /* Source and Destination buffers, where RDMA operations source and sink */
  char *src = NULL, *dst = NULL;

  /* This is our testing function */
  int check_src_dst() { return memcmp((void *)src, (void *)dst, strlen(src)); }

  /* This function prepares client side connection resources for an RDMA
   * connection */
  int client_prepare_connection(struct sockaddr_in *s_addr) {
    struct rdma_cm_event *cm_event = NULL;
    int ret = -1;
    /*  Open a channel used to report asynchronous communication event */
    cm_event_channel = rdma_create_event_channel();
    if (!cm_event_channel) {
      rdma_error("Creating cm event channel failed, errno: %d \n", -errno);
      return -errno;
    }
    debug("RDMA CM event channel is created at : %p \n", cm_event_channel);
    /* rdma_cm_id is the connection identifier (like socket) which is used
     * to define an RDMA connection.
     */
    ret = rdma_create_id(cm_event_channel, &cm_client_id, NULL, RDMA_PS_TCP);
    if (ret) {
      rdma_error("Creating cm id failed with errno: %d \n", -errno);
      return -errno;
    }
    /* Resolve destination and optional source addresses from IP addresses to
     * an RDMA address.  If successful, the specified rdma_cm_id will be
     bound
     * to a local device. */
    ret =
        rdma_resolve_addr(cm_client_id, NULL, (struct sockaddr *)s_addr, 2000);
    if (ret) {
      rdma_error("Failed to resolve address, errno: %d \n", -errno);
      return -errno;
    }
    debug("waiting for cm event: RDMA_CM_EVENT_ADDR_RESOLVED\n");
    ret = process_rdma_cm_event(cm_event_channel, RDMA_CM_EVENT_ADDR_RESOLVED,
                                &cm_event);
    if (ret) {
      rdma_error("Failed to receive a valid event, ret = %d \n", ret);
      return ret;
    }
    /* we ack the event */
    ret = rdma_ack_cm_event(cm_event);
    if (ret) {
      rdma_error("Failed to acknowledge the CM event, errno: %d \n", -errno);
      return -errno;
    }
    debug("RDMA address is resolved \n");

    /* Resolves an RDMA route to the destination address in order to
     * establish a connection */
    ret = rdma_resolve_route(cm_client_id, 2000);
    if (ret) {
      rdma_error("Failed to resolve route, erno: %d \n", -errno);
      return -errno;
    }
    debug("waiting for cm event: RDMA_CM_EVENT_ROUTE_RESOLVED\n");
    ret = process_rdma_cm_event(cm_event_channel, RDMA_CM_EVENT_ROUTE_RESOLVED,
                                &cm_event);
    if (ret) {
      rdma_error("Failed to receive a valid event, ret = %d \n", ret);
      return ret;
    }
    /* we ack the event */
    ret = rdma_ack_cm_event(cm_event);
    if (ret) {
      rdma_error("Failed to acknowledge the CM event, errno: %d \n", -errno);
      return -errno;
    }
    printf("Trying to connect to server at : %s port: %d \n",
           inet_ntoa(s_addr->sin_addr), ntohs(s_addr->sin_port));
    /* Protection Domain (PD) is similar to a "process abstraction"
     * in the operating system. All resources are tied to a particular PD.
     * And accessing recourses across PD will result in a protection fault.
     */
    pd = ibv_alloc_pd(cm_client_id->verbs);
    if (!pd) {
      rdma_error("Failed to alloc pd, errno: %d \n", -errno);
      return -errno;
    }
    debug("pd allocated at %p \n", pd);
    /* Now we need a completion channel, were the I/O completion
     * notifications are sent. Remember, this is different from connection
     * management (CM) event notifications.
     * A completion channel is also tied to an RDMA device, hence we will
     * use cm_client_id->verbs.
     */
    io_completion_channel = ibv_create_comp_channel(cm_client_id->verbs);
    if (!io_completion_channel) {
      rdma_error("Failed to create IO completion event channel, errno: %d \n",
                 -errno);
      return -errno;
    }
    debug("completion event channel created at : %p \n", io_completion_channel);
    /* Now we create a completion queue (CQ) where actual I/O
     * completion metadata is placed. The metadata is packed into a structure
     * called struct ibv_wc (wc = work completion). ibv_wc has detailed
     * information about the work completion. An I/O request in RDMA world
     * is called "work" ;)
     */
    client_cq =
        ibv_create_cq(cm_client_id->verbs /* which device*/,
                      CQ_CAPACITY /* maximum capacity*/,
                      NULL /* user context, not used here */,
                      io_completion_channel /* which IO completion channel
                                             */
                      ,
                      0 /* signaling vector, not used here*/);
    if (!client_cq) {
      rdma_error("Failed to create CQ, errno: %d \n", -errno);
      return -errno;
    }
    debug("CQ created at %p with %d elements \n", client_cq, client_cq->cqe);
    ret = ibv_req_notify_cq(client_cq, 0);
    if (ret) {
      rdma_error("Failed to request notifications, errno: %d \n", -errno);
      return -errno;
    }
    /* Now the last step, set up the queue pair (send, recv) queues and their
     * capacity. The capacity here is define statically but this can be
     probed
     * from the device. We just use a small number as defined in
     rdma_common.h
     */
    bzero(&qp_init_attr, sizeof qp_init_attr);
    qp_init_attr.cap.max_recv_sge =
        MAX_SGE; /* Maximum SGE per receive posting */
    qp_init_attr.cap.max_recv_wr =
        MAX_WR; /* Maximum receive posting capacity */
    qp_init_attr.cap.max_send_sge = MAX_SGE; /* Maximum SGE per send posting
                                              */
    qp_init_attr.cap.max_send_wr = MAX_WR;   /* Maximum send posting
 capacity */
    qp_init_attr.qp_type = IBV_QPT_RC;       /* QP type, RC = Reliable
connection */
    /* We use same completion queue, but one can use different queues */
    qp_init_attr.recv_cq =
        client_cq; /* Where should I notify for receive completion operations
                    */
    qp_init_attr.send_cq =
        client_cq; /* Where should I notify for send completion operations */
    /*Lets create a QP */
    ret = rdma_create_qp(cm_client_id /* which connection id */,
                         pd /* which protection domain*/,
                         &qp_init_attr /* Initial attributes */);
    if (ret) {
      rdma_error("Failed to create QP, errno: %d \n", -errno);
      return -errno;
    }
    client_qp = cm_client_id->qp;
    debug("QP created at %p \n", client_qp);

    printf("Source Queue Pair: %x\n", client_qp->qp_num);

    return 0;
  }

  /* Pre-posts a receive buffer before calling rdma_connect () */
  int client_pre_post_recv_buffer() {
    int ret = -1;
    server_metadata_mr = rdma_buffer_register(pd, &server_metadata_attr,
                                              sizeof(server_metadata_attr),
                                              (IBV_ACCESS_LOCAL_WRITE));
    if (!server_metadata_mr) {
      rdma_error("Failed to setup the server metadata mr , -ENOMEM\n");
      return -ENOMEM;
    }
    server_recv_sge.addr = (uint64_t)server_metadata_mr->addr;
    server_recv_sge.length = (uint64_t)server_metadata_mr->length;
    server_recv_sge.lkey = (uint32_t)server_metadata_mr->lkey;
    /* now we link it to the request */
    bzero(&server_recv_wr, sizeof(server_recv_wr));
    server_recv_wr.sg_list = &server_recv_sge;
    server_recv_wr.num_sge = 1;
    ret = ibv_post_recv(client_qp /* which QP */,
                        &server_recv_wr /* receive work request*/,
                        &bad_server_recv_wr /* error WRs */);
    if (ret) {
      rdma_error("Failed to pre-post the receive buffer, errno: %d \n", ret);
      return ret;
    }
    debug("Receive buffer pre-posting is successful \n");
    return 0;
  }

  /* Connects to the RDMA server */
  int client_connect_to_server() {
    struct rdma_conn_param conn_param;
    struct rdma_cm_event *cm_event = NULL;
    int ret = -1;
    bzero(&conn_param, sizeof(conn_param));
    conn_param.initiator_depth = 3;
    conn_param.responder_resources = 3;
    conn_param.retry_count = 3; // if fail, then how many times to retry
    ret = rdma_connect(cm_client_id, &conn_param);
    if (ret) {
      rdma_error("Failed to connect to remote host , errno: %d \n", -errno);
      return -errno;
    }
    debug("waiting for cm event: RDMA_CM_EVENT_ESTABLISHED\n");
    ret = process_rdma_cm_event(cm_event_channel, RDMA_CM_EVENT_ESTABLISHED,
                                &cm_event);
    if (ret) {
      rdma_error("Failed to get cm event, ret = %d \n", ret);
      return ret;
    }
    ret = rdma_ack_cm_event(cm_event);
    if (ret) {
      rdma_error("Failed to acknowledge cm event, errno: %d \n", -errno);
      return -errno;
    }
    printf("The client is connected successfully \n");
    return 0;
  }

  /* Exchange buffer metadata with the server. The client sends its, and then
   * receives from the server. The client-side metadata on the server is
   _not_
   * used because this program is client driven. But it shown here how to do
   it
   * for the illustration purposes
   */
  int client_xchange_metadata_with_server() {
    struct ibv_wc wc[2];
    int ret = -1;
    // client_src_mr =
    //     rdma_buffer_register(pd, src, strlen(src),
    //                          (IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ
    //                          |
    //                           IBV_ACCESS_REMOTE_WRITE));
    // if (!client_src_mr) {
    //   rdma_error("Failed to register the first buffer, ret = %d \n", ret);
    //   return ret;
    // }
    // /* we prepare metadata for the first buffer */
    // client_metadata_attr.address = (uint64_t)client_src_mr->addr;
    // client_metadata_attr.length = client_src_mr->length;
    // client_metadata_attr.stag.local_stag = client_src_mr->lkey;
    // /* now we register the metadata memory */
    // client_metadata_mr = rdma_buffer_register(pd, &client_metadata_attr,
    //                                           sizeof(client_metadata_attr),
    //                                           IBV_ACCESS_LOCAL_WRITE);
    // if (!client_metadata_mr) {
    //   rdma_error("Failed to register the client metadata buffer, ret = %d
    //   \n",
    //              ret);
    //   return ret;
    // }
    // /* now we fill up SGE */
    // client_send_sge.addr = (uint64_t)client_metadata_mr->addr;
    // client_send_sge.length = (uint64_t)client_metadata_mr->length;
    // client_send_sge.lkey = client_metadata_mr->lkey;
    // /* now we link to the send work request */
    // bzero(&client_send_wr, sizeof(client_send_wr));
    // client_send_wr.sg_list = &client_send_sge;
    // client_send_wr.num_sge = 1;
    // client_send_wr.opcode = IBV_WR_SEND;
    // client_send_wr.send_flags = IBV_SEND_SIGNALED;
    // /* Now we post it */
    // ret = ibv_post_send(client_qp, &client_send_wr, &bad_client_send_wr);
    // if (ret) {
    //   rdma_error("Failed to send client metadata, errno: %d \n", -errno);
    //   return -errno;
    // }

    /* at this point we are expecting 2 work completion. One for our
     * send and one for recv that we will get from the server for
     * its buffer information */
    // ret = process_work_completion_events(io_completion_channel, wc, 2);
    // if (ret != 2) {
    //   rdma_error("We failed to get 2 work completions , ret = %d \n", ret);
    //   return ret;
    // }
    ret = process_work_completion_events(io_completion_channel, wc, 1);
    if (ret != 1) {
      rdma_error("We failed to get 1 work completions , ret = %d \n", ret);
      return ret;
    }
    debug("Server sent us its buffer location and credentials, showing \n");
    show_rdma_buffer_attr(&server_metadata_attr);
    return 0;
  }

  /* This function does :
   * 1) Prepare memory buffers for RDMA operations
   * 1) RDMA write from src -> remote buffer
   * 2) RDMA read from remote bufer -> dst
   */
  int client_remote_memory_ops() {
    struct ibv_wc wc;
    int ret = -1;
    client_dst_mr =
        rdma_buffer_register(pd, dst, strlen(src),
                             (IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
                              IBV_ACCESS_REMOTE_READ));
    if (!client_dst_mr) {
      rdma_error("We failed to create the destination buffer, -ENOMEM\n");
      return -ENOMEM;
    }
    /* Step 1: is to copy the local buffer into the remote buffer. We will
     * reuse the previous variables. */
    /* now we fill up SGE */
    client_send_sge.addr = (uint64_t)client_src_mr->addr;
    client_send_sge.length = (uint64_t)client_src_mr->length;
    client_send_sge.lkey = client_src_mr->lkey;
    /* now we link to the send work request */
    bzero(&client_send_wr, sizeof(client_send_wr));
    client_send_wr.sg_list = &client_send_sge;
    client_send_wr.num_sge = 1;
    client_send_wr.opcode = IBV_WR_RDMA_WRITE;
    client_send_wr.send_flags = IBV_SEND_SIGNALED;
    /* we have to tell server side info for RDMA */
    client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
    client_send_wr.wr.rdma.remote_addr = server_metadata_attr.address;
    /* Now we post it */
    ret = ibv_post_send(client_qp, &client_send_wr, &bad_client_send_wr);
    if (ret) {
      rdma_error("Failed to write client src buffer, errno: %d \n", -errno);
      return -errno;
    }
    /* at this point we are expecting 1 work completion for the write */
    ret = process_work_completion_events(io_completion_channel, &wc, 1);
    if (ret != 1) {
      rdma_error("We failed to get 1 work completions , ret = %d \n", ret);
      return ret;
    }
    debug("Client side WRITE is complete \n");
    /* Now we prepare a READ using same variables but for destination */
    client_send_sge.addr = (uint64_t)client_dst_mr->addr;
    client_send_sge.length = (uint64_t)client_dst_mr->length;
    client_send_sge.lkey = client_dst_mr->lkey;
    /* now we link to the send work request */
    bzero(&client_send_wr, sizeof(client_send_wr));
    client_send_wr.sg_list = &client_send_sge;
    client_send_wr.num_sge = 1;
    client_send_wr.opcode = IBV_WR_RDMA_READ;
    client_send_wr.send_flags = IBV_SEND_SIGNALED;
    /* we have to tell server side info for RDMA */
    client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
    client_send_wr.wr.rdma.remote_addr = server_metadata_attr.address;
    /* Now we post it */
    ret = ibv_post_send(client_qp, &client_send_wr, &bad_client_send_wr);
    if (ret) {
      rdma_error(
          "Failed to read client dst buffer from the master, errno: %d \n",
          -errno);
      return -errno;
    }
    /* at this point we are expecting 1 work completion for the write */
    ret = process_work_completion_events(io_completion_channel, &wc, 1);
    if (ret != 1) {
      rdma_error("We failed to get 1 work completions , ret = %d \n", ret);
      return ret;
    }
    debug("Client side READ is complete \n");
    return 0;
  }

  /* This function disconnects the RDMA connection from the server and cleans
  up
   * all the resources.
   */
  int client_disconnect_and_clean() {
    struct rdma_cm_event *cm_event = NULL;
    int ret = -1;
    /* active disconnect from the client side */
    ret = rdma_disconnect(cm_client_id);
    if (ret) {
      rdma_error("Failed to disconnect, errno: %d \n", -errno);
      // continuing anyways
    }
    ret = process_rdma_cm_event(cm_event_channel, RDMA_CM_EVENT_DISCONNECTED,
                                &cm_event);
    if (ret) {
      rdma_error("Failed to get RDMA_CM_EVENT_DISCONNECTED event, ret = %d \n",
                 ret);
      // continuing anyways
    }
    ret = rdma_ack_cm_event(cm_event);
    if (ret) {
      rdma_error("Failed to acknowledge cm event, errno: %d \n", -errno);
      // continuing anyways
    }
    /* Destroy QP */
    rdma_destroy_qp(cm_client_id);
    /* Destroy client cm id */
    ret = rdma_destroy_id(cm_client_id);
    if (ret) {
      rdma_error("Failed to destroy client id cleanly, %d \n", -errno);
      // we continue anyways;
    }
    /* Destroy CQ */
    ret = ibv_destroy_cq(client_cq);
    if (ret) {
      rdma_error("Failed to destroy completion queue cleanly, %d \n", -errno);
      // we continue anyways;
    }
    /* Destroy completion channel */
    ret = ibv_destroy_comp_channel(io_completion_channel);
    if (ret) {
      rdma_error("Failed to destroy completion channel cleanly, %d \n", -errno);
      // we continue anyways;
    }
    /* Destroy memory buffers */
    rdma_buffer_deregister(server_metadata_mr);
    rdma_buffer_deregister(client_metadata_mr);
    rdma_buffer_deregister(client_src_mr);
    rdma_buffer_deregister(client_dst_mr);
    /* We free the buffers */
    free(src);
    free(dst);
    /* Destroy protection domain */
    ret = ibv_dealloc_pd(pd);
    if (ret) {
      rdma_error("Failed to destroy client protection domain cleanly, %d \n",
                 -errno);
      // we continue anyways;
    }
    rdma_destroy_event_channel(cm_event_channel);
    printf("Client resource clean up is complete \n");
    return 0;
  }
};

class rdma_server {
public:
  int rdma_server_setup(void *, uint64_t, string);

  struct ibv_mr *client_metadata_mr = NULL, *server_buffer_mr = NULL,
                *server_metadata_mr = NULL;
  struct rdma_buffer_attr client_metadata_attr, server_metadata_attr;

private:
  /* These are the RDMA resources needed to setup an RDMA connection */
  /* Event channel, where connection management (cm) related events are
  relayed
   */
  struct rdma_event_channel *cm_event_channel = NULL;
  struct rdma_cm_id *cm_server_id = NULL, *cm_client_id = NULL;
  struct ibv_pd *pd = NULL;
  struct ibv_comp_channel *io_completion_channel = NULL;
  struct ibv_cq *cq = NULL;
  struct ibv_qp_init_attr qp_init_attr;
  struct ibv_qp *client_qp = NULL;
  /* RDMA memory resources */
  struct ibv_recv_wr client_recv_wr, *bad_client_recv_wr = NULL;
  struct ibv_send_wr server_send_wr, *bad_server_send_wr = NULL;
  struct ibv_sge client_recv_sge, server_send_sge;

  /* When we call this function cm_client_id must be set to a valid
  identifier.
   * This is where, we prepare client connection before we accept it. This
   * mainly involve pre-posting a receive buffer to receive client side
   * RDMA credentials
   */
  int setup_client_resources() {
    int ret = -1;
    if (!cm_client_id) {
      rdma_error("Client id is still NULL \n");
      return -EINVAL;
    }
    /* We have a valid connection identifier, lets start to allocate
     * resources. We need:
     * 1. Protection Domains (PD)
     * 2. Memory Buffers
     * 3. Completion Queues (CQ)
     * 4. Queue Pair (QP)
     * Protection Domain (PD) is similar to a "process abstraction"
     * in the operating system. All resources are tied to a particular PD.
     * And accessing recourses across PD will result in a protection fault.
     */
    pd = ibv_alloc_pd(cm_client_id->verbs
  		/* verbs defines a verb's provider,
  		 * i.e an RDMA device where the incoming
  		 * client connection came */);
    if (!pd) {
      rdma_error("Failed to allocate a protection domain errno: %d \n", -errno);
      return -errno;
    }
    debug("A new protection domain is allocated at %p \n", pd);
    /* Now we need a completion channel, were the I/O completion
     * notifications are sent. Remember, this is different from connection
     * management (CM) event notifications.
     * A completion channel is also tied to an RDMA device, hence we will
     * use cm_client_id->verbs.
     */
    io_completion_channel = ibv_create_comp_channel(cm_client_id->verbs);
    if (!io_completion_channel) {
      rdma_error("Failed to create an I/O completion event channel, %d \n",
                 -errno);
      return -errno;
    }
    debug("An I/O completion event channel is created at %p \n",
          io_completion_channel);
    /* Now we create a completion queue (CQ) where actual I/O
     * completion metadata is placed. The metadata is packed into a structure
     * called struct ibv_wc (wc = work completion). ibv_wc has detailed
     * information about the work completion. An I/O request in RDMA world
     * is called "work" ;)
     */
    cq = ibv_create_cq(cm_client_id->verbs /* which device*/,
                       CQ_CAPACITY /* maximum capacity*/,
                       NULL /* user context, not used here */,
                       io_completion_channel /* which IO completion channel
                                              */
                       ,
                       0 /* signaling vector, not used here*/);
    if (!cq) {
      rdma_error("Failed to create a completion queue (cq), errno: %d \n",
                 -errno);
      return -errno;
    }
    debug("Completion queue (CQ) is created at %p with %d elements \n", cq,
          cq->cqe);
    /* Ask for the event for all activities in the completion queue*/
    ret = ibv_req_notify_cq(cq /* on which CQ */,
                            0 /* 0 = all event type, no filter*/);
    if (ret) {
      rdma_error("Failed to request notifications on CQ errno: %d \n", -errno);
      return -errno;
    }
    /* Now the last step, set up the queue pair (send, recv) queues and their
     * capacity. The capacity here is define statically but this can be
     probed
     * from the device. We just use a small number as defined in
     rdma_common.h
     */
    bzero(&qp_init_attr, sizeof qp_init_attr);
    qp_init_attr.cap.max_recv_sge =
        MAX_SGE; /* Maximum SGE per receive posting */
    qp_init_attr.cap.max_recv_wr =
        MAX_WR; /* Maximum receive posting capacity */
    qp_init_attr.cap.max_send_sge = MAX_SGE; /* Maximum SGE per send posting
                                              */
    qp_init_attr.cap.max_send_wr = MAX_WR;   /* Maximum send posting
 capacity */
    qp_init_attr.qp_type = IBV_QPT_RC;       /* QP type, RC = Reliable
connection */
    /* We use same completion queue, but one can use different queues */
    qp_init_attr.recv_cq =
        cq; /* Where should I notify for receive completion operations */
    qp_init_attr.send_cq =
        cq; /* Where should I notify for send completion operations */
    /*Lets create a QP */
    ret = rdma_create_qp(cm_client_id /* which connection id */,
                         pd /* which protection domain*/,
                         &qp_init_attr /* Initial attributes */);
    if (ret) {
      rdma_error("Failed to create QP due to errno: %d \n", -errno);
      return -errno;
    }
    /* Save the reference for handy typing but is not required */
    client_qp = cm_client_id->qp;
    debug("Client QP created at %p\n", client_qp);
    return ret;
  }

  /* Starts an RDMA server by allocating basic connection resources */
  int start_rdma_server(struct sockaddr_in *server_addr) {
    struct rdma_cm_event *cm_event = NULL;
    int ret = -1;
    /*  Open a channel used to report asynchronous communication event */
    cm_event_channel = rdma_create_event_channel();
    if (!cm_event_channel) {
      rdma_error("Creating cm event channel failed with errno : (%d)", -errno);
      return -errno;
    }
    debug("RDMA CM event channel is created successfully at %p \n",
          cm_event_channel);
    /* rdma_cm_id is the connection identifier (like socket) which is used
     * to define an RDMA connection.
     */
    ret = rdma_create_id(cm_event_channel, &cm_server_id, NULL, RDMA_PS_TCP);
    if (ret) {
      rdma_error("Creating server cm id failed with errno: %d ", -errno);
      return -errno;
    }
    debug("A RDMA connection id for the server is created \n");
    /* Explicit binding of rdma cm id to the socket credentials */
    ret = rdma_bind_addr(cm_server_id, (struct sockaddr *)server_addr);
    if (ret) {
      rdma_error("Failed to bind server address, errno: %d \n", -errno);
      return -errno;
    }
    debug("Server RDMA CM id is successfully binded \n");
    /* Now we start to listen on the passed IP and port. However unlike
     * normal TCP listen, this is a non-blocking call. When a new client is
     * connected, a new connection management (CM) event is generated on the
     * RDMA CM event channel from where the listening id was created. Here we
     * have only one channel, so it is easy. */
    ret = rdma_listen(cm_server_id, 8); /* backlog = 8 clients, same as TCP, see
                                    man listen*/
    if (ret) {
      rdma_error("rdma_listen failed to listen on server address, errno: %d ",
                 -errno);
      return -errno;
    }
    printf("Server is listening successfully at: %s , port: %d \n",
           inet_ntoa(server_addr->sin_addr), ntohs(server_addr->sin_port));
    /* now, we expect a client to connect and generate a
     * RDMA_CM_EVNET_CONNECT_REQUEST We wait (block) on the connection
     * management event channel for the connect event.
     */
    ret = process_rdma_cm_event(cm_event_channel, RDMA_CM_EVENT_CONNECT_REQUEST,
                                &cm_event);
    if (ret) {
      rdma_error("Failed to get cm event, ret = %d \n", ret);
      return ret;
    }
    /* Much like TCP connection, listening returns a new connection
    identifier
     * for newly connected client. In the case of RDMA, this is stored in id
     * field. For more details: man rdma_get_cm_event
     */
    cm_client_id = cm_event->id;
    /* now we acknowledge the event. Acknowledging the event free the
    resources
     * associated with the event structure. Hence any reference to the event
     * must be made before acknowledgment. Like, we have already saved the
     * client id from "id" field before acknowledging the event.
     */
    ret = rdma_ack_cm_event(cm_event);
    if (ret) {
      rdma_error("Failed to acknowledge the cm event errno: %d \n", -errno);
      return -errno;
    }
    debug("A new RDMA client connection id is stored at %p\n", cm_client_id);
    return ret;
  }

  /* Pre-posts a receive buffer and accepts an RDMA client connection */
  int accept_client_connection() {
    struct rdma_conn_param conn_param;
    struct rdma_cm_event *cm_event = NULL;
    struct sockaddr_in remote_sockaddr;
    int ret = -1;
    if (!cm_client_id || !client_qp) {
      rdma_error("Client resources are not properly setup\n");
      return -EINVAL;
    }
    // /* we prepare the receive buffer in which we will receive the client
    //  * metadata*/
    // client_metadata_mr =
    //     rdma_buffer_register(pd /* which protection domain */,
    //                          &client_metadata_attr /* what memory */,
    //                          sizeof(client_metadata_attr) /* what length */,
    //                          (IBV_ACCESS_LOCAL_WRITE) /* access permissions
    //                          */);
    // if (!client_metadata_mr) {
    //   rdma_error("Failed to register client attr buffer\n");
    //   // we assume ENOMEM
    //   return -ENOMEM;
    // }
    // /* We pre-post this receive buffer on the QP. SGE credentials is where we
    //  * receive the metadata from the client */
    // client_recv_sge.addr =
    //     (uint64_t)client_metadata_mr->addr; // same as &client_buffer_attr
    // client_recv_sge.length = client_metadata_mr->length;
    // client_recv_sge.lkey = client_metadata_mr->lkey;
    // /* Now we link this SGE to the work request (WR) */
    // bzero(&client_recv_wr, sizeof(client_recv_wr));
    // client_recv_wr.sg_list = &client_recv_sge;
    // client_recv_wr.num_sge = 1; // only one SGE
    // ret = ibv_post_recv(client_qp /* which QP */,
    //                     &client_recv_wr /* receive work request*/,
    //                     &bad_client_recv_wr /* error WRs */);
    // if (ret) {
    //   rdma_error("Failed to pre-post the receive buffer, errno: %d \n", ret);
    //   return ret;
    // }
    // debug("Receive buffer pre-posting is successful \n");
    /* Now we accept the connection. Recall we have not accepted the
    connection
     * yet because we have to do lots of resource pre-allocation */
    memset(&conn_param, 0, sizeof(conn_param));
    /* this tell how many outstanding requests can we handle */
    conn_param.initiator_depth =
        3; /* For this exercise, we put a small number here */
    /* This tell how many outstanding requests we expect other side to handle
     */
    conn_param.responder_resources =
        3; /* For this exercise, we put a small number */
    ret = rdma_accept(cm_client_id, &conn_param);
    if (ret) {
      rdma_error("Failed to accept the connection, errno: %d \n", -errno);
      return -errno;
    }
    /* We expect an RDMA_CM_EVNET_ESTABLISHED to indicate that the RDMA
     * connection has been established and everything is fine on both, server
     * as well as the client sides.
     */
    debug("Going to wait for : RDMA_CM_EVENT_ESTABLISHED event \n");
    ret = process_rdma_cm_event(cm_event_channel, RDMA_CM_EVENT_ESTABLISHED,
                                &cm_event);
    if (ret) {
      rdma_error("Failed to get the cm event, errnp: %d \n", -errno);
      return -errno;
    }
    /* We acknowledge the event */
    ret = rdma_ack_cm_event(cm_event);
    if (ret) {
      rdma_error("Failed to acknowledge the cm event %d \n", -errno);
      return -errno;
    }
    /* Just FYI: How to extract connection information */
    memcpy(&remote_sockaddr /* where to save */,
           rdma_get_peer_addr(cm_client_id) /* gives you remote sockaddr */,
           sizeof(struct sockaddr_in) /* max size */);
    printf("A new connection is accepted from %s \n",
           inet_ntoa(remote_sockaddr.sin_addr));
    return ret;
  }

  /* This function sends server side buffer metadata to the connected client
   */
  int send_server_metadata_to_client(void *addr, uint64_t length) {
    struct ibv_wc wc;
    int ret = -1;
    // /* Now, we first wait for the client to start the communication by
    //  * sending the server its metadata info. The server does not use it
    //  * in our example. We will receive a work completion notification for
    //  * our pre-posted receive request.
    //  */
    // ret = process_work_completion_events(io_completion_channel, &wc, 1);
    // if (ret != 1) {
    //   rdma_error("Failed to receive , ret = %d \n", ret);
    //   return ret;
    // }
    // /* if all good, then we should have client's buffer information, lets see
    //  */
    // printf("Client side buffer information is received...\n");
    // show_rdma_buffer_attr(&client_metadata_attr);
    // printf("The client has requested buffer length of : %u bytes \n",
    //        client_metadata_attr.length);
    // /* We need to setup requested memory buffer. This is where the client
    // will
    //  * do RDMA READs and WRITEs. */
    // server_buffer_mr = rdma_buffer_alloc(
    //     pd /* which protection domain */,
    //     client_metadata_attr.length /* what size to allocate */,
    //     (IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ |
    //      IBV_ACCESS_REMOTE_WRITE) /* access permissions */);
    server_buffer_mr =
        rdma_buffer_register(pd, addr, length,
                             (IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
                              IBV_ACCESS_REMOTE_READ));
    if (!server_buffer_mr) {
      rdma_error("Server failed to create a buffer \n");
      /* we assume that it is due to out of memory error */
      return -ENOMEM;
    }
    /* This buffer is used to transmit information about the above
     * buffer to the client. So this contains the metadata about the server
     * buffer. Hence this is called metadata buffer. Since this is already
     * on allocated, we just register it.
     * We need to prepare a send I/O operation that will tell the
     * client the address of the server buffer.
     */
    server_metadata_attr.address = (uint64_t)server_buffer_mr->addr;
    server_metadata_attr.length = (uint64_t)server_buffer_mr->length;
    server_metadata_attr.stag.local_stag = (uint32_t)server_buffer_mr->lkey;
    server_metadata_mr = rdma_buffer_register(
        pd /* which protection domain*/,
        &server_metadata_attr /* which memory to register */,
        sizeof(server_metadata_attr) /* what is the size of memory */,
        IBV_ACCESS_LOCAL_WRITE /* what access permission */);
    if (!server_metadata_mr) {
      rdma_error("Server failed to create to hold server metadata \n");
      /* we assume that this is due to out of memory error */
      return -ENOMEM;
    }
    /* We need to transmit this buffer. So we create a send request.
     * A send request consists of multiple SGE elements. In our case, we only
     * have one
     */
    server_send_sge.addr = (uint64_t)&server_metadata_attr;
    server_send_sge.length = sizeof(server_metadata_attr);
    server_send_sge.lkey = server_metadata_mr->lkey;
    /* now we link this sge to the send request */
    bzero(&server_send_wr, sizeof(server_send_wr));
    server_send_wr.sg_list = &server_send_sge;
    server_send_wr.num_sge = 1;          // only 1 SGE element in the array
    server_send_wr.opcode = IBV_WR_SEND; // This is a send request
    server_send_wr.send_flags =
        IBV_SEND_SIGNALED; // We want to get notification
    /* This is a fast data path operation. Posting an I/O request */
    ret = ibv_post_send(
        client_qp /* which QP */,
        &server_send_wr /* Send request that we prepared before */,
        &bad_server_send_wr /* In case of error, this will contain failed
        requests */);
    if (ret) {
      rdma_error("Posting of server metdata failed, errno: %d \n", -errno);
      return -errno;
    }
    /* We check for completion notification */
    ret = process_work_completion_events(io_completion_channel, &wc, 1);
    if (ret != 1) {
      rdma_error("Failed to send server metadata, ret = %d \n", ret);
      return ret;
    }
    debug("Local buffer metadata has been sent to the client \n");
    return 0;
  }

  /* This is server side logic. Server passively waits for the client to call
   * rdma_disconnect() and then it will clean up its resources */
  int disconnect_and_cleanup() {
    struct rdma_cm_event *cm_event = NULL;
    int ret = -1;
    /* Now we wait for the client to send us disconnect event */
    debug("Waiting for cm event: RDMA_CM_EVENT_DISCONNECTED\n");
    ret = process_rdma_cm_event(cm_event_channel, RDMA_CM_EVENT_DISCONNECTED,
                                &cm_event);
    if (ret) {
      rdma_error("Failed to get disconnect event, ret = %d \n", ret);
      return ret;
    }
    /* We acknowledge the event */
    ret = rdma_ack_cm_event(cm_event);
    if (ret) {
      rdma_error("Failed to acknowledge the cm event %d \n", -errno);
      return -errno;
    }
    printf("A disconnect event is received from the client...\n");
    /* We free all the resources */
    /* Destroy QP */
    rdma_destroy_qp(cm_client_id);
    /* Destroy client cm id */
    ret = rdma_destroy_id(cm_client_id);
    if (ret) {
      rdma_error("Failed to destroy client id cleanly, %d \n", -errno);
      // we continue anyways;
    }
    /* Destroy CQ */
    ret = ibv_destroy_cq(cq);
    if (ret) {
      rdma_error("Failed to destroy completion queue cleanly, %d \n", -errno);
      // we continue anyways;
    }
    /* Destroy completion channel */
    ret = ibv_destroy_comp_channel(io_completion_channel);
    if (ret) {
      rdma_error("Failed to destroy completion channel cleanly, %d \n", -errno);
      // we continue anyways;
    }
    /* Destroy memory buffers */
    rdma_buffer_free(server_buffer_mr);
    rdma_buffer_deregister(server_metadata_mr);
    rdma_buffer_deregister(client_metadata_mr);
    /* Destroy protection domain */
    ret = ibv_dealloc_pd(pd);
    if (ret) {
      rdma_error("Failed to destroy client protection domain cleanly, %d \n",
                 -errno);
      // we continue anyways;
    }
    /* Destroy rdma server id */
    ret = rdma_destroy_id(cm_server_id);
    if (ret) {
      rdma_error("Failed to destroy server id cleanly, %d \n", -errno);
      // we continue anyways;
    }
    rdma_destroy_event_channel(cm_event_channel);
    printf("Server shut-down is complete \n");
    return 0;
  }
};
