scp node_info_0.txt root@172.16.6.51:~
scp node_info_1.txt root@172.16.6.47:~
scp node_info_2.txt root@172.16.6.50:~

ssh root@172.16.6.51 "(cd /mnt/onl/data/FatBTree/FBT_TCAM/config && pwd && bash config_table.sh)"
ssh root@172.16.6.47 "(cd /root/FatBTree/FBT_SRAM/FBT_recirculate && pwd && bash config_table.sh)"
ssh root@172.16.6.50 "(cd /mnt/onl/data/FatBTree/FBT_SRAM/FBT_recirculate && pwd && bash config_table.sh)"
