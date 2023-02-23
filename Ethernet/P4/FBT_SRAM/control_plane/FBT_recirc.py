import re

def readNode(txt):
    #读取报文中的信息，返回一个FBTNode
    #报文格式：
    #Node: 0x******** (depth = *)
    #keys: <0,0><hi,lo><hi,lo><hi,lo> ...
    #child: 0x******** 0x******** 0x******** ...
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
        #FBTNode: 包含自身地址信息，子节点地址信息，和自身所有keys，这里默认有16个子节点，若不足需要用最后一个子节点地址补全，
        #keys也有十六个，每个以一个字典{'hi':***,'lo':***}表示，第一个key是{'hi':0, 'lo':0}，不足的需要用最后一组keys补全
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
            
        def set_table(self, bfrt, pipe, layer, clear):
        #pipe:指定的table所在流水线
        #layer:节点在该流水线中的层数，如果是第0层，占用的stage为0，1，2，3，4；如果是第1层，占用的stage为6，7，8，9，10层
            assert layer == 0 or layer == 1
            if layer == 0:
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_0_0
                if clear: table.clear()
                table.add_with_set_thresh_action_0_0(self.addr, self.keys[0]['hi'], self.keys[0]['lo'])

                table = pipe.Ingress.set_comp_table_0_0_hi
                table.clear()
                table.add_with_set_comp_action_0_0_hi_s(0)
                table.add_with_set_comp_action_0_0_hi_k(1)

                table = pipe.Ingress.set_comp_table_0_0_lo
                table.clear()
                table.add_with_set_comp_action_0_0_lo_s(0)
                table.add_with_set_comp_action_0_0_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_0_1
                if clear: table.clear()
                table.add_with_set_thresh_action_0_1(self.addr, self.keys[1]['hi'], self.keys[1]['lo'])

                table = pipe.Ingress.set_comp_table_0_1_hi
                table.clear()
                table.add_with_set_comp_action_0_1_hi_s(0)
                table.add_with_set_comp_action_0_1_hi_k(1)

                table = pipe.Ingress.set_comp_table_0_1_lo
                table.clear()
                table.add_with_set_comp_action_0_1_lo_s(0)
                table.add_with_set_comp_action_0_1_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_0_2
                if clear: table.clear()
                table.add_with_set_thresh_action_0_2(self.addr, self.keys[2]['hi'], self.keys[2]['lo'])

                table = pipe.Ingress.set_comp_table_0_2_hi
                table.clear()
                table.add_with_set_comp_action_0_2_hi_s(0)
                table.add_with_set_comp_action_0_2_hi_k(1)

                table = pipe.Ingress.set_comp_table_0_2_lo
                table.clear()
                table.add_with_set_comp_action_0_2_lo_s(0)
                table.add_with_set_comp_action_0_2_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_0_3
                if clear: table.clear()
                table.add_with_set_thresh_action_0_3(self.addr, self.keys[3]['hi'], self.keys[3]['lo'])

                table = pipe.Ingress.set_comp_table_0_3_hi
                table.clear()
                table.add_with_set_comp_action_0_3_hi_s(0)
                table.add_with_set_comp_action_0_3_hi_k(1)

                table = pipe.Ingress.set_comp_table_0_3_lo
                table.clear()
                table.add_with_set_comp_action_0_3_lo_s(0)
                table.add_with_set_comp_action_0_3_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_1_0
                if clear: table.clear()
                table.add_with_set_thresh_action_1_0(self.addr, self.keys[4]['hi'], self.keys[4]['lo'])

                table = pipe.Ingress.set_comp_table_1_0_hi
                table.clear()
                table.add_with_set_comp_action_1_0_hi_s(0)
                table.add_with_set_comp_action_1_0_hi_k(1)

                table = pipe.Ingress.set_comp_table_1_0_lo
                table.clear()
                table.add_with_set_comp_action_1_0_lo_s(0)
                table.add_with_set_comp_action_1_0_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_1_1
                if clear: table.clear()
                table.add_with_set_thresh_action_1_1(self.addr, self.keys[5]['hi'], self.keys[5]['lo'])

                table = pipe.Ingress.set_comp_table_1_1_hi
                table.clear()
                table.add_with_set_comp_action_1_1_hi_s(0)
                table.add_with_set_comp_action_1_1_hi_k(1)

                table = pipe.Ingress.set_comp_table_1_1_lo
                table.clear()
                table.add_with_set_comp_action_1_1_lo_s(0)
                table.add_with_set_comp_action_1_1_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_1_2
                if clear: table.clear()
                table.add_with_set_thresh_action_1_2(self.addr, self.keys[6]['hi'], self.keys[6]['lo'])

                table = pipe.Ingress.set_comp_table_1_2_hi
                table.clear()
                table.add_with_set_comp_action_1_2_hi_s(0)
                table.add_with_set_comp_action_1_2_hi_k(1)

                table = pipe.Ingress.set_comp_table_1_2_lo
                table.clear()
                table.add_with_set_comp_action_1_2_lo_s(0)
                table.add_with_set_comp_action_1_2_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_1_3
                if clear: table.clear()
                table.add_with_set_thresh_action_1_3(self.addr, self.keys[7]['hi'], self.keys[7]['lo'])

                table = pipe.Ingress.set_comp_table_1_3_hi
                table.clear()
                table.add_with_set_comp_action_1_3_hi_s(0)
                table.add_with_set_comp_action_1_3_hi_k(1)

                table = pipe.Ingress.set_comp_table_1_3_lo
                table.clear()
                table.add_with_set_comp_action_1_3_lo_s(0)
                table.add_with_set_comp_action_1_3_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_2_0
                if clear: table.clear()
                table.add_with_set_thresh_action_2_0(self.addr, self.keys[8]['hi'], self.keys[8]['lo'])

                table = pipe.Ingress.set_comp_table_2_0_hi
                table.clear()
                table.add_with_set_comp_action_2_0_hi_s(0)
                table.add_with_set_comp_action_2_0_hi_k(1)

                table = pipe.Ingress.set_comp_table_2_0_lo
                table.clear()
                table.add_with_set_comp_action_2_0_lo_s(0)
                table.add_with_set_comp_action_2_0_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_2_1
                if clear: table.clear()
                table.add_with_set_thresh_action_2_1(self.addr, self.keys[9]['hi'], self.keys[9]['lo'])

                table = pipe.Ingress.set_comp_table_2_1_hi
                table.clear()
                table.add_with_set_comp_action_2_1_hi_s(0)
                table.add_with_set_comp_action_2_1_hi_k(1)

                table = pipe.Ingress.set_comp_table_2_1_lo
                table.clear()
                table.add_with_set_comp_action_2_1_lo_s(0)
                table.add_with_set_comp_action_2_1_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_2_2
                if clear: table.clear()
                table.add_with_set_thresh_action_2_2(self.addr, self.keys[10]['hi'], self.keys[10]['lo'])

                table = pipe.Ingress.set_comp_table_2_2_hi
                table.clear()
                table.add_with_set_comp_action_2_2_hi_s(0)
                table.add_with_set_comp_action_2_2_hi_k(1)

                table = pipe.Ingress.set_comp_table_2_2_lo
                table.clear()
                table.add_with_set_comp_action_2_2_lo_s(0)
                table.add_with_set_comp_action_2_2_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_2_3
                if clear: table.clear()
                table.add_with_set_thresh_action_2_3(self.addr, self.keys[11]['hi'], self.keys[11]['lo'])

                table = pipe.Ingress.set_comp_table_2_3_hi
                table.clear()
                table.add_with_set_comp_action_2_3_hi_s(0)
                table.add_with_set_comp_action_2_3_hi_k(1)

                table = pipe.Ingress.set_comp_table_2_3_lo
                table.clear()
                table.add_with_set_comp_action_2_3_lo_s(0)
                table.add_with_set_comp_action_2_3_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_3_0
                if clear: table.clear()
                table.add_with_set_thresh_action_3_0(self.addr, self.keys[12]['hi'], self.keys[12]['lo'])

                table = pipe.Ingress.set_comp_table_3_0_hi
                table.clear()
                table.add_with_set_comp_action_3_0_hi_s(0)
                table.add_with_set_comp_action_3_0_hi_k(1)

                table = pipe.Ingress.set_comp_table_3_0_lo
                table.clear()
                table.add_with_set_comp_action_3_0_lo_s(0)
                table.add_with_set_comp_action_3_0_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_3_1
                if clear: table.clear()
                table.add_with_set_thresh_action_3_1(self.addr, self.keys[13]['hi'], self.keys[13]['lo'])

                table = pipe.Ingress.set_comp_table_3_1_hi
                table.clear()
                table.add_with_set_comp_action_3_1_hi_s(0)
                table.add_with_set_comp_action_3_1_hi_k(1)

                table = pipe.Ingress.set_comp_table_3_1_lo
                table.clear()
                table.add_with_set_comp_action_3_1_lo_s(0)
                table.add_with_set_comp_action_3_1_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_3_2
                if clear: table.clear()
                table.add_with_set_thresh_action_3_2(self.addr, self.keys[14]['hi'], self.keys[14]['lo'])

                table = pipe.Ingress.set_comp_table_3_2_hi
                table.clear()
                table.add_with_set_comp_action_3_2_hi_s(0)
                table.add_with_set_comp_action_3_2_hi_k(1)

                table = pipe.Ingress.set_comp_table_3_2_lo
                table.clear()
                table.add_with_set_comp_action_3_2_lo_s(0)
                table.add_with_set_comp_action_3_2_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_3_3
                if clear: table.clear()
                table.add_with_set_thresh_action_3_3(self.addr, self.keys[15]['hi'], self.keys[15]['lo'])

                table = pipe.Ingress.set_comp_table_3_3_hi
                table.clear()
                table.add_with_set_comp_action_3_3_hi_s(0)
                table.add_with_set_comp_action_3_3_hi_k(1)

                table = pipe.Ingress.set_comp_table_3_3_lo
                table.clear()
                table.add_with_set_comp_action_3_3_lo_s(0)
                table.add_with_set_comp_action_3_3_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_result_table
                if clear: table.clear()
                table.add_with_set_result_action(self.addr, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[0])
                table.add_with_set_result_action(self.addr, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[1])
                table.add_with_set_result_action(self.addr, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[2])
                table.add_with_set_result_action(self.addr, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[3])
                table.add_with_set_result_action(self.addr, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[4])
                table.add_with_set_result_action(self.addr, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[5])
                table.add_with_set_result_action(self.addr, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.children[6])
                table.add_with_set_result_action(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, self.children[7])
                table.add_with_set_result_action(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, self.children[8])
                table.add_with_set_result_action(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, self.children[9])
                table.add_with_set_result_action(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, self.children[10])
                table.add_with_set_result_action(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, self.children[11])
                table.add_with_set_result_action(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, self.children[12])
                table.add_with_set_result_action(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, self.children[13])
                table.add_with_set_result_action(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, self.children[14])
                table.add_with_set_result_action(self.addr, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, self.children[15])
            if layer == 1:
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_4_0
                if clear: table.clear()
                table.add_with_set_thresh_action_4_0(self.addr, self.keys[0]['hi'], self.keys[0]['lo'])

                table = pipe.Ingress.set_comp_table_4_0_hi
                table.clear()
                table.add_with_set_comp_action_4_0_hi_s(0)
                table.add_with_set_comp_action_4_0_hi_k(1)

                table = pipe.Ingress.set_comp_table_4_0_lo
                table.clear()
                table.add_with_set_comp_action_4_0_lo_s(0)
                table.add_with_set_comp_action_4_0_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_4_1
                if clear: table.clear()
                table.add_with_set_thresh_action_4_1(self.addr, self.keys[1]['hi'], self.keys[1]['lo'])

                table = pipe.Ingress.set_comp_table_4_1_hi
                table.clear()
                table.add_with_set_comp_action_4_1_hi_s(0)
                table.add_with_set_comp_action_4_1_hi_k(1)

                table = pipe.Ingress.set_comp_table_4_1_lo
                table.clear()
                table.add_with_set_comp_action_4_1_lo_s(0)
                table.add_with_set_comp_action_4_1_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_4_2
                if clear: table.clear()
                table.add_with_set_thresh_action_4_2(self.addr, self.keys[2]['hi'], self.keys[2]['lo'])

                table = pipe.Ingress.set_comp_table_4_2_hi
                table.clear()
                table.add_with_set_comp_action_4_2_hi_s(0)
                table.add_with_set_comp_action_4_2_hi_k(1)

                table = pipe.Ingress.set_comp_table_4_2_lo
                table.clear()
                table.add_with_set_comp_action_4_2_lo_s(0)
                table.add_with_set_comp_action_4_2_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_4_3
                if clear: table.clear()
                table.add_with_set_thresh_action_4_3(self.addr, self.keys[3]['hi'], self.keys[3]['lo'])

                table = pipe.Ingress.set_comp_table_4_3_hi
                table.clear()
                table.add_with_set_comp_action_4_3_hi_s(0)
                table.add_with_set_comp_action_4_3_hi_k(1)

                table = pipe.Ingress.set_comp_table_4_3_lo
                table.clear()
                table.add_with_set_comp_action_4_3_lo_s(0)
                table.add_with_set_comp_action_4_3_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_5_0
                if clear: table.clear()
                table.add_with_set_thresh_action_5_0(self.addr, self.keys[4]['hi'], self.keys[4]['lo'])

                table = pipe.Ingress.set_comp_table_5_0_hi
                table.clear()
                table.add_with_set_comp_action_5_0_hi_s(0)
                table.add_with_set_comp_action_5_0_hi_k(1)

                table = pipe.Ingress.set_comp_table_5_0_lo
                table.clear()
                table.add_with_set_comp_action_5_0_lo_s(0)
                table.add_with_set_comp_action_5_0_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_5_1
                if clear: table.clear()
                table.add_with_set_thresh_action_5_1(self.addr, self.keys[5]['hi'], self.keys[5]['lo'])

                table = pipe.Ingress.set_comp_table_5_1_hi
                table.clear()
                table.add_with_set_comp_action_5_1_hi_s(0)
                table.add_with_set_comp_action_5_1_hi_k(1)

                table = pipe.Ingress.set_comp_table_5_1_lo
                table.clear()
                table.add_with_set_comp_action_5_1_lo_s(0)
                table.add_with_set_comp_action_5_1_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_5_2
                if clear: table.clear()
                table.add_with_set_thresh_action_5_2(self.addr, self.keys[6]['hi'], self.keys[6]['lo'])

                table = pipe.Ingress.set_comp_table_5_2_hi
                table.clear()
                table.add_with_set_comp_action_5_2_hi_s(0)
                table.add_with_set_comp_action_5_2_hi_k(1)

                table = pipe.Ingress.set_comp_table_5_2_lo
                table.clear()
                table.add_with_set_comp_action_5_2_lo_s(0)
                table.add_with_set_comp_action_5_2_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_5_3
                if clear: table.clear()
                table.add_with_set_thresh_action_5_3(self.addr, self.keys[7]['hi'], self.keys[7]['lo'])

                table = pipe.Ingress.set_comp_table_5_3_hi
                table.clear()
                table.add_with_set_comp_action_5_3_hi_s(0)
                table.add_with_set_comp_action_5_3_hi_k(1)

                table = pipe.Ingress.set_comp_table_5_3_lo
                table.clear()
                table.add_with_set_comp_action_5_3_lo_s(0)
                table.add_with_set_comp_action_5_3_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_6_0
                if clear: table.clear()
                table.add_with_set_thresh_action_6_0(self.addr, self.keys[8]['hi'], self.keys[8]['lo'])

                table = pipe.Ingress.set_comp_table_6_0_hi
                table.clear()
                table.add_with_set_comp_action_6_0_hi_s(0)
                table.add_with_set_comp_action_6_0_hi_k(1)

                table = pipe.Ingress.set_comp_table_6_0_lo
                table.clear()
                table.add_with_set_comp_action_6_0_lo_s(0)
                table.add_with_set_comp_action_6_0_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_6_1
                if clear: table.clear()
                table.add_with_set_thresh_action_6_1(self.addr, self.keys[9]['hi'], self.keys[9]['lo'])

                table = pipe.Ingress.set_comp_table_6_1_hi
                table.clear()
                table.add_with_set_comp_action_6_1_hi_s(0)
                table.add_with_set_comp_action_6_1_hi_k(1)

                table = pipe.Ingress.set_comp_table_6_1_lo
                table.clear()
                table.add_with_set_comp_action_6_1_lo_s(0)
                table.add_with_set_comp_action_6_1_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_6_2
                if clear: table.clear()
                table.add_with_set_thresh_action_6_2(self.addr, self.keys[10]['hi'], self.keys[10]['lo'])

                table = pipe.Ingress.set_comp_table_6_2_hi
                table.clear()
                table.add_with_set_comp_action_6_2_hi_s(0)
                table.add_with_set_comp_action_6_2_hi_k(1)

                table = pipe.Ingress.set_comp_table_6_2_lo
                table.clear()
                table.add_with_set_comp_action_6_2_lo_s(0)
                table.add_with_set_comp_action_6_2_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_6_3
                if clear: table.clear()
                table.add_with_set_thresh_action_6_3(self.addr, self.keys[11]['hi'], self.keys[11]['lo'])

                table = pipe.Ingress.set_comp_table_6_3_hi
                table.clear()
                table.add_with_set_comp_action_6_3_hi_s(0)
                table.add_with_set_comp_action_6_3_hi_k(1)

                table = pipe.Ingress.set_comp_table_6_3_lo
                table.clear()
                table.add_with_set_comp_action_6_3_lo_s(0)
                table.add_with_set_comp_action_6_3_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_7_0
                if clear: table.clear()
                table.add_with_set_thresh_action_7_0(self.addr, self.keys[12]['hi'], self.keys[12]['lo'])

                table = pipe.Ingress.set_comp_table_7_0_hi
                table.clear()
                table.add_with_set_comp_action_7_0_hi_s(0)
                table.add_with_set_comp_action_7_0_hi_k(1)

                table = pipe.Ingress.set_comp_table_7_0_lo
                table.clear()
                table.add_with_set_comp_action_7_0_lo_s(0)
                table.add_with_set_comp_action_7_0_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_7_1
                if clear: table.clear()
                table.add_with_set_thresh_action_7_1(self.addr, self.keys[13]['hi'], self.keys[13]['lo'])

                table = pipe.Ingress.set_comp_table_7_1_hi
                table.clear()
                table.add_with_set_comp_action_7_1_hi_s(0)
                table.add_with_set_comp_action_7_1_hi_k(1)

                table = pipe.Ingress.set_comp_table_7_1_lo
                table.clear()
                table.add_with_set_comp_action_7_1_lo_s(0)
                table.add_with_set_comp_action_7_1_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_7_2
                if clear: table.clear()
                table.add_with_set_thresh_action_7_2(self.addr, self.keys[14]['hi'], self.keys[14]['lo'])

                table = pipe.Ingress.set_comp_table_7_2_hi
                table.clear()
                table.add_with_set_comp_action_7_2_hi_s(0)
                table.add_with_set_comp_action_7_2_hi_k(1)

                table = pipe.Ingress.set_comp_table_7_2_lo
                table.clear()
                table.add_with_set_comp_action_7_2_lo_s(0)
                table.add_with_set_comp_action_7_2_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_thresh_table_7_3
                if clear: table.clear()
                table.add_with_set_thresh_action_7_3(self.addr, self.keys[15]['hi'], self.keys[15]['lo'])

                table = pipe.Ingress.set_comp_table_7_3_hi
                table.clear()
                table.add_with_set_comp_action_7_3_hi_s(0)
                table.add_with_set_comp_action_7_3_hi_k(1)

                table = pipe.Ingress.set_comp_table_7_3_lo
                table.clear()
                table.add_with_set_comp_action_7_3_lo_s(0)
                table.add_with_set_comp_action_7_3_lo_k(1)
                #######################################################################################
                table = pipe.Ingress.set_result_table_2
                if clear: table.clear()
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
    Node = FBTNode(selfaddr, childrenaddr, keys, selfdepth)
    return Node

file1 = open('/root/FBT_SRAM/node_info_1_1.txt')
file2 = open('/root/FBT_SRAM/node_info_1_2.txt')
file3 = open('/root/FBT_SRAM/node_info_1_3.txt')
file4 = open('/root/FBT_SRAM/node_info_1_4.txt')
is_root = True
cnt = 0
maxlayer = 0
nodes = [[],[],[],[],[],[],[],[]]

while True:
    node = readNode(file1)
    if node == None:
        break
    cnt+=1
    if node.depth == 0:
        maxlayer = 1
        nodes[0].append(node)
    elif node.depth == 1:
        maxlayer = 2
        nodes[1].append(node)
    elif node.depth == 2:
        maxlayer = 3
        nodes[2].append(node)
    elif node.depth == 3:
        maxlayer = 4
        nodes[3].append(node)
    elif node.depth == 4:
        maxlayer = 5
        nodes[4].append(node)
    else:
        maxlayer = 6
        nodes[5].append(node)

while True:
    node = readNode(file2)
    if node == None:
        break
    cnt+=1
    if node.depth == 0:
        maxlayer = 1
        nodes[0].append(node)
    elif node.depth == 1:
        maxlayer = 2
        nodes[1].append(node)
    elif node.depth == 2:
        maxlayer = 3
        nodes[2].append(node)
    elif node.depth == 3:
        maxlayer = 4
        nodes[3].append(node)
    elif node.depth == 4:
        maxlayer = 5
        nodes[4].append(node)
    else:
        maxlayer = 6
        nodes[5].append(node)

while True:
    node = readNode(file3)
    if node == None:
        break
    cnt+=1
    if node.depth == 0:
        maxlayer = 1
        nodes[0].append(node)
    elif node.depth == 1:
        maxlayer = 2
        nodes[1].append(node)
    elif node.depth == 2:
        maxlayer = 3
        nodes[2].append(node)
    elif node.depth == 3:
        maxlayer = 4
        nodes[3].append(node)
    elif node.depth == 4:
        maxlayer = 5
        nodes[4].append(node)
    else:
        maxlayer = 6
        nodes[5].append(node)

while True:
    node = readNode(file4)
    if node == None:
        break
    cnt+=1
    if node.depth == 0:
        maxlayer = 1
        nodes[0].append(node)
    elif node.depth == 1:
        maxlayer = 2
        nodes[1].append(node)
    elif node.depth == 2:
        maxlayer = 3
        nodes[2].append(node)
    elif node.depth == 3:
        maxlayer = 4
        nodes[3].append(node)
    elif node.depth == 4:
        maxlayer = 5
        nodes[4].append(node)
    else:
        maxlayer = 6
        nodes[5].append(node)

for layer in range(maxlayer):
    for node in nodes[layer]:
        if layer  == 1:
            node.set_table(bfrt, bfrt.FBT_SRAM.pipe, 0, 0)
        else:
            node.set_table(bfrt, bfrt.FBT_SRAM.pipe, 1, 0)


# table = bfrt.FBT_SRAM.pipe.Ingress.set_port_table
# table.clear()
# table.add_with_set_port_action(0, 168, 128)
# table.add_with_set_port_action(0, 184, 128)
# table.add_with_set_port_action(0, 128, 168)
# table.add_with_set_port_action(1, 128, 168)
# table.add_with_set_port_action(2, 128, 184)
# table.add_with_set_port_action(3, 128, 184)
