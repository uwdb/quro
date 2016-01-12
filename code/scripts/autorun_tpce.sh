#!/bin/bash

TXN="tpce"
TXNNAME="tradeupdate"
TYPEN="reorder"
DURATION=300
#DURATION=1200

run_benchmark()
{
	USERS="1 2 4 8 16 32 64"

		for USER in $USERS ; do


				./dbt5-run-workload -c 1000 -t 1000 -d ${DURATION} -u ${USER} -f 500 -w 5 -n dbt5 -o ~/results/${TXN}_${TYPEN}_${TXNNAME}_${USER}conn_${DURATION}sec
				#In the paper, we use larger settings as following. But the vm seems to be too small to run efficiently on this data size, so we use a smaller size data here
				#./dbt5-run-workload -c 1000 -t 1000 -d ${DURATION} -u ${USER} -f 100 -w 20 -n dbt5 -o ~/results/${TXN}_${TYPEN}_${TXNNAME}_${USER}conn_${DURATION}sec
				sleep 120

				echo "======finish threads $USER ====="

		done
}

run_benchmark
