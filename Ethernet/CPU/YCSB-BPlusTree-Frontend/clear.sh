echo "clear switch 42"
ssh root@172.16.6.42 "cd /root/FBT_TCAM/config && ./clear.sh" > /dev/null
echo "clear switch 46"
ssh root@172.16.6.46 "cd /root/FBT_SRAM && ./clear.sh" > /dev/null
echo "clear switch 50"
ssh root@172.16.6.50 "cd /mnt/onl/data/FBT_SRAM && ./clear.sh" > /dev/null
echo "clear switch 51"
ssh root@172.16.6.51 "cd /mnt/onl/data/FBT_SRAM && ./clear.sh" > /dev/null

