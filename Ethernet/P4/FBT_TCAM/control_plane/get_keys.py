import re

txt = open('./node_info_0_1.txt','r')
selfaddrline = txt.readline()
selfkeysline = txt.readline()
childrenaddrline = txt.readline()
selfaddr = selfaddrline[6:].strip().split()
selfaddr = int(selfaddr[0], 16)
selfkeys = re.findall(r'\d+', selfkeysline)
keys_1 = []
for i in range(0,len(selfkeys),2):
    keys_1.append((int(selfkeys[i]) << 32) + int(selfkeys[i + 1]))
childrenaddr_1 = childrenaddrline[7:]
childrenaddr_1 = childrenaddr_1.split()
childrenaddr_1 = [int(x, 16) for x in childrenaddr_1]

txt = open('./node_info_0_2.txt','r')
selfaddrline = txt.readline()
selfkeysline = txt.readline()
childrenaddrline = txt.readline()
selfaddr = selfaddrline[6:].strip().split()
selfaddr = int(selfaddr[0], 16)
selfkeys = re.findall(r'\d+', selfkeysline)
keys_2 = []
for i in range(0,len(selfkeys),2):
    keys_2.append((int(selfkeys[i]) << 32) + int(selfkeys[i + 1]))
childrenaddr_2 = childrenaddrline[7:]
childrenaddr_2 = childrenaddr_2.split()
childrenaddr_2 = [int(x, 16) for x in childrenaddr_2]

txt = open('./node_info_0_3.txt','r')
selfaddrline = txt.readline()
selfkeysline = txt.readline()
childrenaddrline = txt.readline()
selfaddr = selfaddrline[6:].strip().split()
selfaddr = int(selfaddr[0], 16)
selfkeys = re.findall(r'\d+', selfkeysline)
keys_3 = []
for i in range(0,len(selfkeys),2):
    keys_3.append((int(selfkeys[i]) << 32) + int(selfkeys[i + 1]))
childrenaddr_3 = childrenaddrline[7:]
childrenaddr_3 = childrenaddr_3.split()
childrenaddr_3 = [int(x, 16) for x in childrenaddr_3]

txt = open('./node_info_0_4.txt','r')
selfaddrline = txt.readline()
selfkeysline = txt.readline()
childrenaddrline = txt.readline()
selfaddr = selfaddrline[6:].strip().split()
selfaddr = int(selfaddr[0], 16)
selfkeys = re.findall(r'\d+', selfkeysline)
keys_4 = []
for i in range(0,len(selfkeys),2):
    keys_4.append((int(selfkeys[i]) << 32) + int(selfkeys[i + 1]))
childrenaddr_4 = childrenaddrline[7:]
childrenaddr_4 = childrenaddr_4.split()
childrenaddr_4 = [int(x, 16) for x in childrenaddr_4]

g = open("keyinfo.txt", "w")

for k in keys_1[1:]:
    print(k, end = ' ', file=g)
print(1 * 2 ** 62, end = ' ', file=g)
for k in keys_2[1:]:
    print(k, end = ' ', file=g)
print(2 * 2 ** 62, end = ' ', file=g)
for k in keys_3[1:]:
    print(k, end = ' ', file=g)
print(3 * 2 ** 62, end = ' ', file=g)
for k in keys_4[1:]:
    if k == keys_4[-1]:
        print(k, end = '\n', file=g)
    else:
        print(k, end = ' ', file=g)

for a in childrenaddr_1[::]:
    print(a, end = ' ', file=g)
for a in childrenaddr_2[::]:
    print(a, end = ' ', file=g)
for a in childrenaddr_3[::]:
    print(a, end = ' ', file=g)
for a in childrenaddr_4[::]:
    print(a, end = ' ', file=g)
