#include "fstream"
#include "rdma_driver.h"

int rdma_client::read_remote_node(void *addr, uint64_t fbt_key = 0) {
  struct ibv_wc wc;
  int ret = -1;

  /* Now we prepare a READ using same variables but for destination */
  client_send_sge.addr = (uint64_t)remote_node_mr->addr;
  client_send_sge.length = (uint64_t)remote_node_mr->length;
  client_send_sge.lkey = remote_node_mr->lkey;

  /* now we link to the send work request */
  bzero(&client_send_wr, sizeof(client_send_wr));
  client_send_wr.sg_list = &client_send_sge;
  client_send_wr.num_sge = 1;
  client_send_wr.opcode = IBV_WR_RDMA_READ;
  client_send_wr.send_flags = IBV_SEND_SIGNALED;

  /* we have to tell server side info for RDMA */
  if (addr == NULL) {
    /* first packet, inject fbt_key as remote_addr */
    client_send_wr.wr.rdma.remote_addr = fbt_key;
    client_send_wr.wr.rdma.rkey = 0;
  } else {
    client_send_wr.wr.rdma.remote_addr = addr;
    client_send_wr.wr.rdma.rkey = server_metadata_attr.stag.remote_stag;
  }

  /* Now we post it */
  ret = ibv_post_send(client_qp, &client_send_wr, &bad_client_send_wr);
  if (ret) {
    rdma_error("Failed to read client dst buffer from the master, errno: %d \n",
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

int rdma_client::rdma_client_setup(char *address, void *addr, uint64_t length) {
  struct sockaddr_in server_sockaddr;
  int ret, option;
  bzero(&server_sockaddr, sizeof server_sockaddr);
  server_sockaddr.sin_family = AF_INET;
  server_sockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  // /* buffers are NULL */
  // src = dst = NULL;

  // printf("Passed string is : %s , with count %u \n", test_string,
  //        (unsigned int)strlen(test_string));
  // src = calloc(strlen(test_string), 1);
  // if (!src) {
  //   rdma_error("Failed to allocate memory : -ENOMEM\n");
  //   return -ENOMEM;
  // }
  // /* Copy the passes arguments */
  // strncpy(src, test_string, strlen(test_string));
  // dst = calloc(strlen(test_string), 1);
  // if (!dst) {
  //   rdma_error("Failed to allocate destination memory, -ENOMEM\n");
  //   free(src);
  //   return -ENOMEM;
  // }

  /* remember, this overwrites the port info */
  ret = get_addr(address, (struct sockaddr *)&server_sockaddr);
  if (ret) {
    rdma_error("Invalid IP \n");
    return ret;
  }

  if (!server_sockaddr.sin_port) {
    /* no port provided, use the default port */
    server_sockaddr.sin_port = htons(DEFAULT_RDMA_PORT);
  }
  // if (src == NULL) {
  //   printf("Please provide a string to copy \n");
  //   exit(0);
  // }

  ret = client_prepare_connection(&server_sockaddr);
  if (ret) {
    rdma_error("Failed to setup client connection , ret = %d \n", ret);
    return ret;
  }
  ret = client_pre_post_recv_buffer();
  if (ret) {
    rdma_error("Failed to setup client connection , ret = %d \n", ret);
    return ret;
  }
  ret = client_connect_to_server();
  if (ret) {
    rdma_error("Failed to setup client connection , ret = %d \n", ret);
    return ret;
  }
  ret = client_xchange_metadata_with_server();
  if (ret) {
    rdma_error("Failed to setup client connection , ret = %d \n", ret);
    return ret;
  }

  remote_node_mr =
      rdma_buffer_register(pd, addr, length,
                           (IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
                            IBV_ACCESS_REMOTE_READ));
  if (!remote_node_mr) {
    rdma_error("We failed to create the destination buffer, -ENOMEM\n");
    return -ENOMEM;
  }

  cout << "rdma::rkey " << server_metadata_attr.stag.remote_stag << endl;
  cout << "rdma::remote_addr " << server_metadata_attr.address << endl;

  // ret = client_remote_memory_ops();
  // if (ret) {
  //   rdma_error("Failed to finish remote memory ops, ret = %d \n", ret);
  //   return ret;
  // }
  // if (check_src_dst()) {
  //   rdma_error("src and dst buffers do not match \n");
  // } else {
  //   printf("...\nSUCCESS, source: %s and destination: %s buffers match \n",
  //   src,
  //          dst);
  // }
  return ret;
}

int rdma_server::rdma_server_setup(void *addr, uint64_t length,
                                   string filename) {
  int ret, option;
  struct sockaddr_in server_sockaddr;
  bzero(&server_sockaddr, sizeof server_sockaddr);
  server_sockaddr.sin_family = AF_INET; /* standard IP NET address */
  server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY); /* passed address */

  if (!server_sockaddr.sin_port) {
    /* If still zero, that mean no port info provided */
    server_sockaddr.sin_port = htons(DEFAULT_RDMA_PORT); /* use default port
                                                          */
  }
  ret = start_rdma_server(&server_sockaddr);
  if (ret) {
    rdma_error("RDMA server failed to start cleanly, ret = %d \n", ret);
    return ret;
  }
  ret = setup_client_resources();
  if (ret) {
    rdma_error("Failed to setup client resources, ret = %d \n", ret);
    return ret;
  }
  ret = accept_client_connection();
  if (ret) {
    rdma_error("Failed to handle client cleanly, ret = %d \n", ret);
    return ret;
  }
  ret = send_server_metadata_to_client(addr, length);
  if (ret) {
    rdma_error("Failed to send server metadata to the client, ret = %d \n",
               ret);
    return ret;
  }

  std::ofstream fout(filename);

  fout << "remote_addr:" << server_metadata_attr.address << std::endl;
  fout << "remote_stag:" << server_metadata_attr.stag.local_stag << std::endl;
  fout << std::endl;

  fout.close();

  // cout << "remote_addr:" << (uint64_t)server_buffer_mr->addr << endl;
  // cout << "remote_stag:" << (uint32_t)server_buffer_mr->lkey << endl;

  return ret;
}
