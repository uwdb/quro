#!/bin/bash

#USERS="1 2 4 8 16 32 64"
#METHODS="DL_DETECT OCC MVCC NO_WAIT WAIT_DIE"

USERS="1 2"
METHODS="DL_DETECT OCC"

cp config_files/tpcc_txn.cpp benchmarks/

for METHOD in $METHODS; do
	for USER in $USERS; do
		echo "running ${METHOD} with ${USER} user"
		cp config_files/config_${METHOD}_${USER}.h config.h
		make
		sleep 120
		./rundb > log_files/mix_orig_${METHOD}_${USER}.log 2>&1 &
		sleep 300
		killall ./rundb 2> /dev/null
	done
done

cp config_files/reorder_tpcc_txn.cpp benchmarks/tpcc_txn.cpp
METHODS="DL_DETECT NO_WAIT WAIT_DIE"
for METHOD in $METHODS; do
	for USER in $USERS; do
		echo "running ${METHOD} with ${USER} user"
		cp config_files/config_${METHOD}_${USER}.h config.h
		make
		sleep 120
		./rundb > log_files/mix_reorder_${METHOD}_${USER}.log 2>&1 &
		sleep 300
		killall ./rundb 2> /dev/null
	done
done



#./rundb > temp.log 2>&1 &
#sleep 20
#killall ./rundb 2> /dev/null
