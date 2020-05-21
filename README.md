# DPDKD-IPHASH-WRKR-THREADS
Spread the incoming flow to worker threads using simple hash buckets built from IP SRC-DST pair

Main Functions:

 - lcore_testCuckoohash: `recieves traffic from Ports, does basic validation and stats updates. Lookups up for HASH entry, if found send to FIFO pipe with a sequentally added TX port as metadata.`
 - lcore_fifoProcess: `user thread to process the pkt as required. Once successfull; create multiple pkts with same payload (zero copy) and transmit the pkt through TX ports.`
 
## Purpose: 
to have simple frame work for multi port traffic processing. Can be used glue layer for work distribution.

## options:
```
 a. r : sets number of RX ports
 b. f : sets number of worker threds
 c. 4/6/all : choose whehter IPv4, IPv6 or both has to be processed
 
 note: currently implemeted for IPv4 only
 
example: ./test-cuckoo-hash -c 0xfe -n 3 -- -r 2 -f 4 
```
