pipe = bfrt.FBT_TCAM.pipe
for table in pipe.Ingress._children:
    table.clear()

table = bfrt.FBT_TCAM.pipe.Ingress.set_port_table
table.add_with_set_port_action(0,404,384)
table.add_with_set_port_action(0,384,404)
table.add_with_set_port_action(1,384,404)
table.add_with_set_port_action(2,384,404)
table.add_with_set_port_action(3,384,404)
