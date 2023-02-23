f = open('./table_info.txt')
table_idx = 0
table1_len = 0
table2_len = 0
table3_len = 0
table4_len = 0
while True:
    line = f.readline()
    if line:
        if line[0] == '#':
            if line[2] == '1':
                table = bfrt.FBT_TCAM.pipe.Ingress.set_result_table_1
                table_idx = 1
            elif line[2] == '2':
                table = bfrt.FBT_TCAM.pipe.Ingress.set_result_table_2
                table_idx = 2
            elif line[2] == '3':
                table = bfrt.FBT_TCAM.pipe.Ingress.set_result_table_3
                table_idx = 3
            elif line[2] == '4':
                table = bfrt.FBT_TCAM.pipe.Ingress.set_result_table_4
                table_idx = 4
        else:
            if table_idx == 0:
                continue
            args = [int(i, base=16) for i in line.strip().split()]
            if len(args) < 2:
                continue
            if table_idx == 1:
                #print('add_1', args[0], args[1], args[2], args[3])
                table1_len += 1
                #print(table1_len)
                table.add_with_set_result_action_1(args[0], args[1], None, args[3], args[2])
            elif table_idx == 2:
                #print('add_2', args[0], args[1], args[2], args[3], args[4])
                table2_len += 1
                #print(table2_len)
                table.add_with_set_result_action_2(args[0], args[1], args[2], None, args[4], args[3])
            elif table_idx == 3:
                #print('add_3', args[0], args[1], args[2], args[3], args[4])
                table3_len += 1
                #print(table3_len)
                table.add_with_set_result_action_3(args[0], args[1], args[2], None, args[4], args[3])
            elif table_idx == 4:
                #print('add_4', args[0], args[1], args[2], args[4])
                table4_len += 1
                #print(table4_len)
                table.add_with_set_result_action_4(args[0], args[1], args[2], None, args[4])
            else:
                continue
    else:
        break

print(table1_len, table2_len, table3_len, table4_len)
g = open('./keyinfo.txt')
table = bfrt.FBT_TCAM.pipe.Ingress.set_addr_table
line = g.readline()
addrs = [int(i) for i in g.readline().strip().split()]
for i in range(len(addrs)):
    table.add_with_set_addr(i, addrs[i])