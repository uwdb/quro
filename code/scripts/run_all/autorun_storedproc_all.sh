#!/bin/bash
#REORDERN="reorder"
#ORIGN="orig"
TXN="tpcc"
TXNNAME="all"
TYPEN="storedproc"


#TXN="1GBP_bid"
#TYPEN="orig"
#ITEM=4
#TXNNAME="1item"

#USER=32
WHN=4

run_benchmark()
{
	USERS="1 2 4 8 16 32 64 128"
	#USERS="1 2 4"
	#USERS="32"

		for USER in $USERS ; do
				echo "start warehouse ${WHN}"


				./tpcc-run-workload -d 300 -u ${USER} -n tpcc -f ${WHN} -o ~/results_link/DBT5_results/${TXN}_${TYPEN}_${TXNNAME}_${WHN}wh_${USER}conn_300sec		
				sleep 120

#				cd ..
#				cp ./src/transactions/orig_${TXNNAME}Txn.cpp ./src/transactions/${TXNNAME}Txn.cpp
#				make clean;
#				make;
#				make install;
#				cd scripts/
#
#				./tpcc-run-workload -d 300 -u ${USER} -n tpcc -f ${WHN} -o ~/results_link/DBT5_results/${TXN}_${ORIGN}_${WHN}wh_${USERS}conn_300sec		
#
#				sleep 120

				echo "======finish user $USER ====="

		done
}

run_benchmark
