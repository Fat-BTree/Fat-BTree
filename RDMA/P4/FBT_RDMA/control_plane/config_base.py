import re

def readNode(txt):
    selfaddrline = txt.readline()
    selfkeysline = txt.readline()
    childrenaddrline = txt.readline()

    if not selfaddrline: 
        return None

    selfaddrline = selfaddrline[6:].strip().split()
    selfaddr = int(selfaddrline[0], 16)
    selfdepth = int(re.findall(r"\d+",selfaddrline[-1])[0])
    selfkeys = re.findall(r'\d+', selfkeysline)[2:]

    childrenaddr = childrenaddrline[7:]
    childrenaddr = childrenaddr.split()
    childrenaddr = [int(x, 16) for x in childrenaddr]

    keys = [{'hi' : 0, 'lo' : 0}]
    for i in range(0, len(selfkeys), 2):
        keys.append({'hi':selfkeys[i], 'lo':selfkeys[i + 1]})

    while len(childrenaddr) < 16:
        childrenaddr.append(childrenaddr[-1])
    while len(keys) < 16:
        keys.append(keys[-1])

    class FBTNode:
        def __init__(self, addr, children, keys, depth):
            self.children = children
            self.keys = keys
            self.addr = addr
            self.depth = depth
        def print(self):
            print('address: ', self.addr)
            print('depth: ', self.depth)
            print('keys: ', self.keys)
            print('children: ',self.children)

        def init_table(self, bfrt, gress, layer):
            if layer == 1:
                #######################################################################################
                table = gress.set_comp_table_0_0_hi
                table.add_with_set_comp_action_0_0_hi_s(0)
                table.add_with_set_comp_action_0_0_hi_k(1)

                table = gress.set_comp_table_0_0_lo
                table.add_with_set_comp_action_0_0_lo_s(0)
                table.add_with_set_comp_action_0_0_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_0_1_hi
                table.add_with_set_comp_action_0_1_hi_s(0)
                table.add_with_set_comp_action_0_1_hi_k(1)

                table = gress.set_comp_table_0_1_lo
                table.add_with_set_comp_action_0_1_lo_s(0)
                table.add_with_set_comp_action_0_1_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_0_2_hi
                table.add_with_set_comp_action_0_2_hi_s(0)
                table.add_with_set_comp_action_0_2_hi_k(1)

                table = gress.set_comp_table_0_2_lo
                table.add_with_set_comp_action_0_2_lo_s(0)
                table.add_with_set_comp_action_0_2_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_0_3_hi
                table.add_with_set_comp_action_0_3_hi_s(0)
                table.add_with_set_comp_action_0_3_hi_k(1)

                table = gress.set_comp_table_0_3_lo
                table.add_with_set_comp_action_0_3_lo_s(0)
                table.add_with_set_comp_action_0_3_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_1_0_hi
                table.add_with_set_comp_action_1_0_hi_s(0)
                table.add_with_set_comp_action_1_0_hi_k(1)

                table = gress.set_comp_table_1_0_lo
                table.add_with_set_comp_action_1_0_lo_s(0)
                table.add_with_set_comp_action_1_0_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_1_1_hi
                table.add_with_set_comp_action_1_1_hi_s(0)
                table.add_with_set_comp_action_1_1_hi_k(1)

                table = gress.set_comp_table_1_1_lo
                table.add_with_set_comp_action_1_1_lo_s(0)
                table.add_with_set_comp_action_1_1_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_1_2_hi
                table.add_with_set_comp_action_1_2_hi_s(0)
                table.add_with_set_comp_action_1_2_hi_k(1)

                table = gress.set_comp_table_1_2_lo
                table.add_with_set_comp_action_1_2_lo_s(0)
                table.add_with_set_comp_action_1_2_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_1_3_hi
                table.add_with_set_comp_action_1_3_hi_s(0)
                table.add_with_set_comp_action_1_3_hi_k(1)

                table = gress.set_comp_table_1_3_lo
                table.add_with_set_comp_action_1_3_lo_s(0)
                table.add_with_set_comp_action_1_3_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_2_0_hi
                table.add_with_set_comp_action_2_0_hi_s(0)
                table.add_with_set_comp_action_2_0_hi_k(1)

                table = gress.set_comp_table_2_0_lo
                table.add_with_set_comp_action_2_0_lo_s(0)
                table.add_with_set_comp_action_2_0_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_2_1_hi
                table.add_with_set_comp_action_2_1_hi_s(0)
                table.add_with_set_comp_action_2_1_hi_k(1)

                table = gress.set_comp_table_2_1_lo
                table.add_with_set_comp_action_2_1_lo_s(0)
                table.add_with_set_comp_action_2_1_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_2_2_hi
                table.add_with_set_comp_action_2_2_hi_s(0)
                table.add_with_set_comp_action_2_2_hi_k(1)

                table = gress.set_comp_table_2_2_lo
                table.add_with_set_comp_action_2_2_lo_s(0)
                table.add_with_set_comp_action_2_2_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_2_3_hi
                table.add_with_set_comp_action_2_3_hi_s(0)
                table.add_with_set_comp_action_2_3_hi_k(1)

                table = gress.set_comp_table_2_3_lo
                table.add_with_set_comp_action_2_3_lo_s(0)
                table.add_with_set_comp_action_2_3_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_3_0_hi
                table.add_with_set_comp_action_3_0_hi_s(0)
                table.add_with_set_comp_action_3_0_hi_k(1)

                table = gress.set_comp_table_3_0_lo
                table.add_with_set_comp_action_3_0_lo_s(0)
                table.add_with_set_comp_action_3_0_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_3_1_hi
                table.add_with_set_comp_action_3_1_hi_s(0)
                table.add_with_set_comp_action_3_1_hi_k(1)

                table = gress.set_comp_table_3_1_lo
                table.add_with_set_comp_action_3_1_lo_s(0)
                table.add_with_set_comp_action_3_1_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_3_2_hi
                table.add_with_set_comp_action_3_2_hi_s(0)
                table.add_with_set_comp_action_3_2_hi_k(1)

                table = gress.set_comp_table_3_2_lo
                table.add_with_set_comp_action_3_2_lo_s(0)
                table.add_with_set_comp_action_3_2_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_3_3_hi
                table.add_with_set_comp_action_3_3_hi_s(0)
                table.add_with_set_comp_action_3_3_hi_k(1)

                table = gress.set_comp_table_3_3_lo
                table.add_with_set_comp_action_3_3_lo_s(0)
                table.add_with_set_comp_action_3_3_lo_k(1)
            elif layer == 2:
                #######################################################################################
                table = gress.set_comp_table_0_0_hi
                table.add_with_set_comp_action_0_0_hi_s(0)
                table.add_with_set_comp_action_0_0_hi_k(1)

                table = gress.set_comp_table_0_0_lo
                table.add_with_set_comp_action_0_0_lo_s(0)
                table.add_with_set_comp_action_0_0_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_0_1_hi
                table.add_with_set_comp_action_0_1_hi_s(0)
                table.add_with_set_comp_action_0_1_hi_k(1)

                table = gress.set_comp_table_0_1_lo
                table.add_with_set_comp_action_0_1_lo_s(0)
                table.add_with_set_comp_action_0_1_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_0_2_hi
                table.add_with_set_comp_action_0_2_hi_s(0)
                table.add_with_set_comp_action_0_2_hi_k(1)

                table = gress.set_comp_table_0_2_lo
                table.add_with_set_comp_action_0_2_lo_s(0)
                table.add_with_set_comp_action_0_2_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_0_3_hi
                table.add_with_set_comp_action_0_3_hi_s(0)
                table.add_with_set_comp_action_0_3_hi_k(1)

                table = gress.set_comp_table_0_3_lo
                table.add_with_set_comp_action_0_3_lo_s(0)
                table.add_with_set_comp_action_0_3_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_1_0_hi
                table.add_with_set_comp_action_1_0_hi_s(0)
                table.add_with_set_comp_action_1_0_hi_k(1)

                table = gress.set_comp_table_1_0_lo
                table.add_with_set_comp_action_1_0_lo_s(0)
                table.add_with_set_comp_action_1_0_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_1_1_hi
                table.add_with_set_comp_action_1_1_hi_s(0)
                table.add_with_set_comp_action_1_1_hi_k(1)

                table = gress.set_comp_table_1_1_lo
                table.add_with_set_comp_action_1_1_lo_s(0)
                table.add_with_set_comp_action_1_1_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_1_2_hi
                table.add_with_set_comp_action_1_2_hi_s(0)
                table.add_with_set_comp_action_1_2_hi_k(1)

                table = gress.set_comp_table_1_2_lo
                table.add_with_set_comp_action_1_2_lo_s(0)
                table.add_with_set_comp_action_1_2_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_1_3_hi
                table.add_with_set_comp_action_1_3_hi_s(0)
                table.add_with_set_comp_action_1_3_hi_k(1)

                table = gress.set_comp_table_1_3_lo
                table.add_with_set_comp_action_1_3_lo_s(0)
                table.add_with_set_comp_action_1_3_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_2_0_hi
                table.add_with_set_comp_action_2_0_hi_s(0)
                table.add_with_set_comp_action_2_0_hi_k(1)

                table = gress.set_comp_table_2_0_lo
                table.add_with_set_comp_action_2_0_lo_s(0)
                table.add_with_set_comp_action_2_0_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_2_1_hi
                table.add_with_set_comp_action_2_1_hi_s(0)
                table.add_with_set_comp_action_2_1_hi_k(1)

                table = gress.set_comp_table_2_1_lo
                table.add_with_set_comp_action_2_1_lo_s(0)
                table.add_with_set_comp_action_2_1_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_2_2_hi
                table.add_with_set_comp_action_2_2_hi_s(0)
                table.add_with_set_comp_action_2_2_hi_k(1)

                table = gress.set_comp_table_2_2_lo
                table.add_with_set_comp_action_2_2_lo_s(0)
                table.add_with_set_comp_action_2_2_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_2_3_hi
                table.add_with_set_comp_action_2_3_hi_s(0)
                table.add_with_set_comp_action_2_3_hi_k(1)

                table = gress.set_comp_table_2_3_lo
                table.add_with_set_comp_action_2_3_lo_s(0)
                table.add_with_set_comp_action_2_3_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_3_0_hi
                table.add_with_set_comp_action_3_0_hi_s(0)
                table.add_with_set_comp_action_3_0_hi_k(1)

                table = gress.set_comp_table_3_0_lo
                table.add_with_set_comp_action_3_0_lo_s(0)
                table.add_with_set_comp_action_3_0_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_3_1_hi
                table.add_with_set_comp_action_3_1_hi_s(0)
                table.add_with_set_comp_action_3_1_hi_k(1)

                table = gress.set_comp_table_3_1_lo
                table.add_with_set_comp_action_3_1_lo_s(0)
                table.add_with_set_comp_action_3_1_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_3_2_hi
                table.add_with_set_comp_action_3_2_hi_s(0)
                table.add_with_set_comp_action_3_2_hi_k(1)

                table = gress.set_comp_table_3_2_lo
                table.add_with_set_comp_action_3_2_lo_s(0)
                table.add_with_set_comp_action_3_2_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_3_3_hi
                table.add_with_set_comp_action_3_3_hi_s(0)
                table.add_with_set_comp_action_3_3_hi_k(1)

                table = gress.set_comp_table_3_3_lo
                table.add_with_set_comp_action_3_3_lo_s(0)
                table.add_with_set_comp_action_3_3_lo_k(1)
            elif layer == 3:
                #######################################################################################
                table = gress.set_comp_table_4_0_hi
                table.add_with_set_comp_action_4_0_hi_s(0)
                table.add_with_set_comp_action_4_0_hi_k(1)

                table = gress.set_comp_table_4_0_lo
                table.add_with_set_comp_action_4_0_lo_s(0)
                table.add_with_set_comp_action_4_0_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_4_1_hi
                table.add_with_set_comp_action_4_1_hi_s(0)
                table.add_with_set_comp_action_4_1_hi_k(1)

                table = gress.set_comp_table_4_1_lo
                table.add_with_set_comp_action_4_1_lo_s(0)
                table.add_with_set_comp_action_4_1_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_4_2_hi
                table.add_with_set_comp_action_4_2_hi_s(0)
                table.add_with_set_comp_action_4_2_hi_k(1)

                table = gress.set_comp_table_4_2_lo
                table.add_with_set_comp_action_4_2_lo_s(0)
                table.add_with_set_comp_action_4_2_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_4_3_hi
                table.add_with_set_comp_action_4_3_hi_s(0)
                table.add_with_set_comp_action_4_3_hi_k(1)

                table = gress.set_comp_table_4_3_lo
                table.add_with_set_comp_action_4_3_lo_s(0)
                table.add_with_set_comp_action_4_3_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_5_0_hi
                table.add_with_set_comp_action_5_0_hi_s(0)
                table.add_with_set_comp_action_5_0_hi_k(1)

                table = gress.set_comp_table_5_0_lo
                table.add_with_set_comp_action_5_0_lo_s(0)
                table.add_with_set_comp_action_5_0_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_5_1_hi
                table.add_with_set_comp_action_5_1_hi_s(0)
                table.add_with_set_comp_action_5_1_hi_k(1)

                table = gress.set_comp_table_5_1_lo
                table.add_with_set_comp_action_5_1_lo_s(0)
                table.add_with_set_comp_action_5_1_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_5_2_hi
                table.add_with_set_comp_action_5_2_hi_s(0)
                table.add_with_set_comp_action_5_2_hi_k(1)

                table = gress.set_comp_table_5_2_lo
                table.add_with_set_comp_action_5_2_lo_s(0)
                table.add_with_set_comp_action_5_2_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_5_3_hi
                table.add_with_set_comp_action_5_3_hi_s(0)
                table.add_with_set_comp_action_5_3_hi_k(1)

                table = gress.set_comp_table_5_3_lo
                table.add_with_set_comp_action_5_3_lo_s(0)
                table.add_with_set_comp_action_5_3_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_6_0_hi
                table.add_with_set_comp_action_6_0_hi_s(0)
                table.add_with_set_comp_action_6_0_hi_k(1)

                table = gress.set_comp_table_6_0_lo
                table.add_with_set_comp_action_6_0_lo_s(0)
                table.add_with_set_comp_action_6_0_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_6_1_hi
                table.add_with_set_comp_action_6_1_hi_s(0)
                table.add_with_set_comp_action_6_1_hi_k(1)

                table = gress.set_comp_table_6_1_lo
                table.add_with_set_comp_action_6_1_lo_s(0)
                table.add_with_set_comp_action_6_1_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_6_2_hi
                table.add_with_set_comp_action_6_2_hi_s(0)
                table.add_with_set_comp_action_6_2_hi_k(1)

                table = gress.set_comp_table_6_2_lo
                table.add_with_set_comp_action_6_2_lo_s(0)
                table.add_with_set_comp_action_6_2_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_6_3_hi
                table.add_with_set_comp_action_6_3_hi_s(0)
                table.add_with_set_comp_action_6_3_hi_k(1)

                table = gress.set_comp_table_6_3_lo
                table.add_with_set_comp_action_6_3_lo_s(0)
                table.add_with_set_comp_action_6_3_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_7_0_hi
                table.add_with_set_comp_action_7_0_hi_s(0)
                table.add_with_set_comp_action_7_0_hi_k(1)

                table = gress.set_comp_table_7_0_lo
                table.add_with_set_comp_action_7_0_lo_s(0)
                table.add_with_set_comp_action_7_0_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_7_1_hi
                table.add_with_set_comp_action_7_1_hi_s(0)
                table.add_with_set_comp_action_7_1_hi_k(1)

                table = gress.set_comp_table_7_1_lo
                table.add_with_set_comp_action_7_1_lo_s(0)
                table.add_with_set_comp_action_7_1_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_7_2_hi
                table.add_with_set_comp_action_7_2_hi_s(0)
                table.add_with_set_comp_action_7_2_hi_k(1)

                table = gress.set_comp_table_7_2_lo
                table.add_with_set_comp_action_7_2_lo_s(0)
                table.add_with_set_comp_action_7_2_lo_k(1)
                #######################################################################################
                table = gress.set_comp_table_7_3_hi
                table.add_with_set_comp_action_7_3_hi_s(0)
                table.add_with_set_comp_action_7_3_hi_k(1)

                table = gress.set_comp_table_7_3_lo
                table.add_with_set_comp_action_7_3_lo_s(0)
                table.add_with_set_comp_action_7_3_lo_k(1)
            else:
                assert(0)
            
        def set_table(self, bfrt, gress, layer):
            if layer == 1:
                #######################################################################################
                table = gress.set_thresh_table_0_0
                table.add_with_set_thresh_action_0_0(self.addr, self.keys[0]['hi'], self.keys[0]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_0_1
                table.add_with_set_thresh_action_0_1(self.addr, self.keys[1]['hi'], self.keys[1]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_0_2
                table.add_with_set_thresh_action_0_2(self.addr, self.keys[2]['hi'], self.keys[2]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_0_3
                table.add_with_set_thresh_action_0_3(self.addr, self.keys[3]['hi'], self.keys[3]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_1_0
                table.add_with_set_thresh_action_1_0(self.addr, self.keys[4]['hi'], self.keys[4]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_1_1
                table.add_with_set_thresh_action_1_1(self.addr, self.keys[5]['hi'], self.keys[5]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_1_2
                table.add_with_set_thresh_action_1_2(self.addr, self.keys[6]['hi'], self.keys[6]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_1_3
                table.add_with_set_thresh_action_1_3(self.addr, self.keys[7]['hi'], self.keys[7]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_2_0
                table.add_with_set_thresh_action_2_0(self.addr, self.keys[8]['hi'], self.keys[8]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_2_1
                table.add_with_set_thresh_action_2_1(self.addr, self.keys[9]['hi'], self.keys[9]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_2_2
                table.add_with_set_thresh_action_2_2(self.addr, self.keys[10]['hi'], self.keys[10]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_2_3
                table.add_with_set_thresh_action_2_3(self.addr, self.keys[11]['hi'], self.keys[11]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_3_0
                table.add_with_set_thresh_action_3_0(self.addr, self.keys[12]['hi'], self.keys[12]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_3_1
                table.add_with_set_thresh_action_3_1(self.addr, self.keys[13]['hi'], self.keys[13]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_3_2
                table.add_with_set_thresh_action_3_2(self.addr, self.keys[14]['hi'], self.keys[14]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_3_3
                table.add_with_set_thresh_action_3_3(self.addr, self.keys[15]['hi'], self.keys[15]['lo'])
                #######################################################################################
                table = gress.set_result_table_0
                table.add_with_set_result_action_0(self.addr, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[0])
                table.add_with_set_result_action_0(self.addr, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[1])
                table.add_with_set_result_action_0(self.addr, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[2])
                table.add_with_set_result_action_0(self.addr, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[3])
                table.add_with_set_result_action_0(self.addr, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[4])
                table.add_with_set_result_action_0(self.addr, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[5])
                table.add_with_set_result_action_0(self.addr, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[6])
                table.add_with_set_result_action_0(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, self.children[7])
                table.add_with_set_result_action_0(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, self.children[8])
                table.add_with_set_result_action_0(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, self.children[9])
                table.add_with_set_result_action_0(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, self.children[10])
                table.add_with_set_result_action_0(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, self.children[11])
                table.add_with_set_result_action_0(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, self.children[12])
                table.add_with_set_result_action_0(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, self.children[13])
                table.add_with_set_result_action_0(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, self.children[14])
                table.add_with_set_result_action_0(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, self.children[15])
            elif layer == 2:
                #######################################################################################
                table = gress.set_thresh_table_0_0
                table.add_with_set_thresh_action_0_0(self.addr, self.keys[0]['hi'], self.keys[0]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_0_1
                table.add_with_set_thresh_action_0_1(self.addr, self.keys[1]['hi'], self.keys[1]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_0_2
                table.add_with_set_thresh_action_0_2(self.addr, self.keys[2]['hi'], self.keys[2]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_0_3
                table.add_with_set_thresh_action_0_3(self.addr, self.keys[3]['hi'], self.keys[3]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_1_0
                table.add_with_set_thresh_action_1_0(self.addr, self.keys[4]['hi'], self.keys[4]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_1_1
                table.add_with_set_thresh_action_1_1(self.addr, self.keys[5]['hi'], self.keys[5]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_1_2
                table.add_with_set_thresh_action_1_2(self.addr, self.keys[6]['hi'], self.keys[6]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_1_3
                table.add_with_set_thresh_action_1_3(self.addr, self.keys[7]['hi'], self.keys[7]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_2_0
                table.add_with_set_thresh_action_2_0(self.addr, self.keys[8]['hi'], self.keys[8]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_2_1
                table.add_with_set_thresh_action_2_1(self.addr, self.keys[9]['hi'], self.keys[9]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_2_2
                table.add_with_set_thresh_action_2_2(self.addr, self.keys[10]['hi'], self.keys[10]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_2_3
                table.add_with_set_thresh_action_2_3(self.addr, self.keys[11]['hi'], self.keys[11]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_3_0
                table.add_with_set_thresh_action_3_0(self.addr, self.keys[12]['hi'], self.keys[12]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_3_1
                table.add_with_set_thresh_action_3_1(self.addr, self.keys[13]['hi'], self.keys[13]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_3_2
                table.add_with_set_thresh_action_3_2(self.addr, self.keys[14]['hi'], self.keys[14]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_3_3
                table.add_with_set_thresh_action_3_3(self.addr, self.keys[15]['hi'], self.keys[15]['lo'])
                #######################################################################################
                table = gress.set_result_table_1
                table.add_with_set_result_action_1(self.addr, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[0])
                table.add_with_set_result_action_1(self.addr, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[1])
                table.add_with_set_result_action_1(self.addr, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[2])
                table.add_with_set_result_action_1(self.addr, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[3])
                table.add_with_set_result_action_1(self.addr, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[4])
                table.add_with_set_result_action_1(self.addr, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[5])
                table.add_with_set_result_action_1(self.addr, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[6])
                table.add_with_set_result_action_1(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, self.children[7])
                table.add_with_set_result_action_1(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, self.children[8])
                table.add_with_set_result_action_1(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, self.children[9])
                table.add_with_set_result_action_1(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, self.children[10])
                table.add_with_set_result_action_1(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, self.children[11])
                table.add_with_set_result_action_1(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, self.children[12])
                table.add_with_set_result_action_1(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, self.children[13])
                table.add_with_set_result_action_1(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, self.children[14])
                table.add_with_set_result_action_1(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, self.children[15])
            elif layer == 3:
                #######################################################################################
                table = gress.set_thresh_table_4_0
                table.add_with_set_thresh_action_4_0(self.addr, self.keys[0]['hi'], self.keys[0]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_4_1
                table.add_with_set_thresh_action_4_1(self.addr, self.keys[1]['hi'], self.keys[1]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_4_2
                table.add_with_set_thresh_action_4_2(self.addr, self.keys[2]['hi'], self.keys[2]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_4_3
                table.add_with_set_thresh_action_4_3(self.addr, self.keys[3]['hi'], self.keys[3]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_5_0
                table.add_with_set_thresh_action_5_0(self.addr, self.keys[4]['hi'], self.keys[4]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_5_1
                table.add_with_set_thresh_action_5_1(self.addr, self.keys[5]['hi'], self.keys[5]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_5_2
                table.add_with_set_thresh_action_5_2(self.addr, self.keys[6]['hi'], self.keys[6]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_5_3
                table.add_with_set_thresh_action_5_3(self.addr, self.keys[7]['hi'], self.keys[7]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_6_0
                table.add_with_set_thresh_action_6_0(self.addr, self.keys[8]['hi'], self.keys[8]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_6_1
                table.add_with_set_thresh_action_6_1(self.addr, self.keys[9]['hi'], self.keys[9]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_6_2
                table.add_with_set_thresh_action_6_2(self.addr, self.keys[10]['hi'], self.keys[10]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_6_3
                table.add_with_set_thresh_action_6_3(self.addr, self.keys[11]['hi'], self.keys[11]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_7_0
                table.add_with_set_thresh_action_7_0(self.addr, self.keys[12]['hi'], self.keys[12]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_7_1
                table.add_with_set_thresh_action_7_1(self.addr, self.keys[13]['hi'], self.keys[13]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_7_2
                table.add_with_set_thresh_action_7_2(self.addr, self.keys[14]['hi'], self.keys[14]['lo'])
                #######################################################################################
                table = gress.set_thresh_table_7_3
                table.add_with_set_thresh_action_7_3(self.addr, self.keys[15]['hi'], self.keys[15]['lo'])
                #######################################################################################
                table = gress.set_result_table_2
                table.add_with_set_result_action_2(self.addr, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[0])
                table.add_with_set_result_action_2(self.addr, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[1])
                table.add_with_set_result_action_2(self.addr, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[2])
                table.add_with_set_result_action_2(self.addr, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[3])
                table.add_with_set_result_action_2(self.addr, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[4])
                table.add_with_set_result_action_2(self.addr, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[5])
                table.add_with_set_result_action_2(self.addr, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[6])
                table.add_with_set_result_action_2(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, self.children[7])
                table.add_with_set_result_action_2(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, self.children[8])
                table.add_with_set_result_action_2(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, self.children[9])
                table.add_with_set_result_action_2(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, self.children[10])
                table.add_with_set_result_action_2(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, self.children[11])
                table.add_with_set_result_action_2(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, self.children[12])
                table.add_with_set_result_action_2(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, self.children[13])
                table.add_with_set_result_action_2(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, self.children[14])
                table.add_with_set_result_action_2(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, self.children[15])
            else:
                assert(0)

    Node = FBTNode(selfaddr, childrenaddr, keys, selfdepth)
    return Node

f = open("./table_info.txt")
table_idx = 0
while True:
    line = f.readline()
    if line:
        if line[0] == "#":
            if line[2] == "1":
                table = bfrt.FBT_RDMA_BASE.pipe0.Ingress0.set_result_table_1
                table_idx = 1
            elif line[2] == "2":
                table = bfrt.FBT_RDMA_BASE.pipe0.Ingress0.set_result_table_2
                table_idx = 2
            elif line[2] == "3":
                table = bfrt.FBT_RDMA_BASE.pipe0.Ingress0.set_result_table_3
                table_idx = 3
            elif line[2] == "4":
                table = bfrt.FBT_RDMA_BASE.pipe0.Ingress0.set_result_table_4
                table_idx = 4
        else:
            if table_idx == 0:
                continue
            args = [int(i, base=16) for i in line.strip().split()]
            if len(args) < 2:
                continue
            if table_idx == 1:
                print("add_1", args[0], args[1], args[2], args[3])
                table.add_with_set_result_action_1(args[0], args[1], None, args[3], args[2])
            elif table_idx == 2:
                print("add_2", args[0], args[1], args[2], args[3], args[4])
                table.add_with_set_result_action_2(args[0], args[1], args[2], None, args[4], args[3])
            elif table_idx == 3:
                print("add_3", args[0], args[1], args[2], args[3], args[4])
                table.add_with_set_result_action_3(args[0], args[1], args[2], None, args[4], args[3])
            elif table_idx == 4:
                print("add_4", args[0], args[1], args[2], args[4])
                table.add_with_set_result_action_4(args[0], args[1], args[2], None, args[4])
            else:
                assert(0)
    else:
        break

g = open("./keys_info.txt")
table = bfrt.FBT_RDMA_BASE.pipe0.Ingress0.set_addr_table
line = g.readline()
addrs = [int(i) for i in g.readline().strip().split()]
for i in range(len(addrs)):
    print("add_addr", i, hex(addrs[i]))
    table.add_with_set_addr(i, addrs[i])

file = open('./node_info_greedy.txt')
nodes = []

remote_addr = int(file.readline()[12:])
remote_stag = int(file.readline()[12:])
file.readline()

print(remote_addr)
print(remote_stag)

table = bfrt.FBT_RDMA_BASE.pipe0.Egress0.fetch_addr_table
table.set_default_with_fetch_addr_action(remote_addr)

table = bfrt.FBT_RDMA_BASE.pipe0.Egress0.fetch_stag_table
table.set_default_with_fetch_stag_action(remote_stag)

while True:
    node = readNode(file)
    if node == None:
        break

    while len(nodes) < node.depth + 1:
        nodes.append([])
    nodes[node.depth].append(node)

for layer in range(1, 4):
    print(layer, len(nodes[layer]))
    if layer == 1:
        nodes[layer][0].init_table(bfrt, bfrt.FBT_RDMA_BASE.pipe0.Ingress0, layer)
    elif layer == 2:
        nodes[layer][0].init_table(bfrt, bfrt.FBT_RDMA_BASE.pipe1.Ingress1, layer)
    elif layer == 3:
        nodes[layer][0].init_table(bfrt, bfrt.FBT_RDMA_BASE.pipe1.Ingress1, layer)
    else:
        assert(0)

    nb_load = 0
    for node in nodes[layer]:
        nb_load += 1
# print("prev-load: ", nb_load)

        if layer == 1:
            node.set_table(bfrt, bfrt.FBT_RDMA_BASE.pipe0.Ingress0, layer)
        elif layer == 2:
            node.set_table(bfrt, bfrt.FBT_RDMA_BASE.pipe1.Ingress1, layer)
        elif layer == 3:
            node.set_table(bfrt, bfrt.FBT_RDMA_BASE.pipe1.Ingress1, layer)
        else:
            assert(0)

# print("post-load: ", nb_load)
# print("")
