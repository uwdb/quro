#!/bin/bash
TXN="bid"
TYPEN="reorder"

DURATION=300
#DURATION=1200
ITEM=4

run_benchmark()
{
	USERS="1 2 4 8 16 32 64"

		for USER in $USERS ; do

				./bid-mysql-build-db
				./bid-run-workload -d ${DURATION} -u ${USER} -n bid -f ${ITEM} -o ~/results/${TXN}_${TYPEN}_${ITEM}item_${USER}conn_${DURATION}sec
				sleep 120

				echo "======finish threads $USER ====="

		done
}

run_benchmark
