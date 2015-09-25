#!/bin/bash
#REORDERN="reorder"
#ORIGN="orig"
#TXN="256MBP_tpcc"
#TXNNAME="mix"
#TYPEN="reorder"

TXN="tpce"
TXNNAME="tradeorder_allcus"
TYPEN="reorder"

#TXN="1GBP_bid"
#TYPEN="orig"
#ITEM=4
#TXNNAME="1item"

#USER=32
WHN=4

run_benchmark()
{
	USERS="1 2 4 8 16 32"
	#USERS="1 2 4"
	#USERS="32"

		for USER in $USERS ; do
				echo "start warehouse ${WHN}"


				#./tpcc-run-workload -d 300 -u ${USER} -n tpcc -f ${WHN} -o ~/results_link/DBT5_results/${TXN}_${TYPEN}_${TXNNAME}_${WHN}wh_${USER}conn_300sec		
				./dbt5-run-workload -c 1000 -t 1000 -d 300 -u ${USER} -f 100 -w 20 -n dbt5 -o ~/results_link/DBT5_results/${TXN}_${TYPEN}_${TXNNAME}_${USER}conn_300sec
				#./bid-mysql-build-db
				#./bid-run-workload -d 200 -u ${USER} -n bid -f ${ITEM} -o ~/results_link/DBT5_results/${TXN}_${TYPEN}_${ITEM}item_${USER}conn_200sec
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
