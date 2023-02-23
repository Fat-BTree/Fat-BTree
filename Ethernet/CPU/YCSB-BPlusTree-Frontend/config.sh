echo "config switch 42"
ssh root@172.16.6.42 "cd /root/FBT_TCAM/config && ./config.sh" > /dev/null
echo "config switch 46"
ssh root@172.16.6.46 "cd /root/FBT_SRAM && ./config.sh" > /dev/null
echo "config switch 50"
ssh root@172.16.6.50 "cd /mnt/onl/data/FBT_SRAM && ./config.sh" > /dev/null
echo "config switch 51"
ssh root@172.16.6.51 "cd /mnt/onl/data/FBT_SRAM && ./config.sh" > /dev/null
