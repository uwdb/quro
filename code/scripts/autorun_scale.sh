#!/bin/bash
#REORDERN="reorder"
#ORIGN="orig"
TXN="tpcc"
TXNNAME="payment"
TYPEN="reorder"

#TXN="tpce"
#TXNNAME="tradeupdate_1024t"

#TXN="bid"
#TXNNAME="1item"

USER=32
#WH=4

run_benchmark()
{
	#USERS="8 16 32 64 128"
	WH="2 8 16 32"
	#USERS="128"

		for WHN in $WH ; do
				echo "start warehouse ${WHN}"


				./tpcc-run-workload -d 300 -u ${USER} -n tpcc -f ${WHN} -o ~/results_link/DBT5_results/${TXN}_${TYPEN}_${TXNNAME}_${WHN}wh_${USER}conn_300sec		
				#./dbt5-run-workload -c 1000 -t 1000 -d 300 -u ${USER} -f 100 -w 20 -n dbt5 -o ~/results_link/DBT5_results/${TXN}_${REORDERN}_${TXNNAME}_${USER}conn_300sec
				#./bid-run-workload -d 200 -u ${USER} -n bid -o ~/results_link/DBT5_results/${TXN}_${ORIGN}_${TXNNAME}_${USER}conn_200sec
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
