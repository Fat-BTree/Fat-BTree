# Yahoo! Cloud System Benchmark
# Workload C: Read only
#   Application example: user profile cache, where profiles are constructed elsewhere (e.g., Hadoop)
#                        
#   Read/update ratio: 100/0
#   Default data size: 1 KB records (10 fields, 100 bytes each, plus key)
#   Request distribution: zipfian

recordcount=100000000
operationcount=100000000
workload=com.yahoo.ycsb.workloads.CoreWorkload

fieldcount=1
fieldlength=46

readallfields=true

readproportion=0.95
updateproportion=0
scanproportion=0
insertproportion=0.05
zeropadding=20

requestdistribution=zipfian
