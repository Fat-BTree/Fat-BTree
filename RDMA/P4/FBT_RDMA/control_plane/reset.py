pipe = bfrt.FBT_RDMA.pipe0
for table in pipe.Ingress0._children:
    table.clear()

pipe = bfrt.FBT_RDMA.pipe1
for table in pipe.Ingress1._children:
    table.clear()

table = bfrt.FBT_RDMA.pipe0.Ingress0.set_port_table
table.add_with_set_port_action(136, 28)
table.add_with_set_port_action(128, 36)

table = bfrt.FBT_RDMA.pipe1.Ingress1.set_port_table
table.add_with_set_port_action(36, 136)
table.add_with_set_port_action(28, 128)
