#!/bin/bash

TXN="tpcc"
TXNNAME="payment"
TYPEN="orig"

DURATION=300
#DURATION=1200
WHN=4

run_benchmark()
{
	USERS="1 2 4 8 16 32 64 128"

		for USER in $USERS ; do
				echo "start warehouse ${WHN}"


				./tpcc-run-workload -d ${DURATION} -u ${USER} -n tpcc -f ${WHN} -o ~/results/${TXN}_${TYPEN}_${TXNNAME}_${WHN}wh_${USER}conn_${DURATION}sec		
				sleep 120

				echo "======finish user $USER ====="

		done
}

run_benchmark
