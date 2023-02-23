import re
txt = open('./node_info_greedy.txt','r')
txt.readline()
txt.readline()
txt.readline()
selfaddrline = txt.readline()
selfkeysline = txt.readline()
childrenaddrline = txt.readline()
selfaddr = selfaddrline[6:].strip().split()
selfaddr = int(selfaddr[0], 16)
selfkeys = re.findall(r'\d+', selfkeysline)
keys = []
for i in range(0,len(selfkeys),2):
    keys.append((int(selfkeys[i]) << 32) + int(selfkeys[i + 1]))
childrenaddr = childrenaddrline[7:]
childrenaddr = childrenaddr.split()
childrenaddr = [int(x, 16) for x in childrenaddr]
for k in keys[1:]:
    if k == keys[-1]:
        print(k, end = '')
    else:
        print(k, end = ' ')
print('')
for a in childrenaddr[::]:
    print(a, end = ' ')
