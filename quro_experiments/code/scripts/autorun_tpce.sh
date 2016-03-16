#!/bin/bash

TXN="tpce"
TXNNAME="tradeupdate"
TYPEN="orig"
DURATION=300
#DURATION=1200

CONNS="1 2 4 8 16 32 64"
if [ $# -lt 2 ]
  then
  echo "At least 2 arguments [transaction_name] [implementation_type] are required"
  echo "  For example: ./autorun_tpce.sh tradeupdate orig"
  echo "  or           ./autorun_tpce.sh tradeorder reorder"
  exit
fi

TXNNAME=$1
shift
TYPEN=$1
shift

if [ $# -gt 0 ]
  then
  CONNS=""
  for var in "$@"
    do
    echo "$var"
    CONNS="${CONNS} $var"
  done
fi

echo "Running ${TXN} ${TXNNAME} (${TYPEN} implementation) on ${CONNS} number of threads"


run_benchmark()
{

		for CONN in $CONNS ; do


				./dbt5-run-workload -c 1000 -t 1000 -d ${DURATION} -u ${CONN} -f 500 -w 5 -n dbt5 -o ~/results/${TXN}_${TYPEN}_${TXNNAME}_${CONN}conn_${DURATION}sec
				#In the paper, we use larger settings as following. But the vm seems to be too small to run efficiently on this data size, so we use a smaller size data here
				#./dbt5-run-workload -c 1000 -t 1000 -d ${DURATION} -u ${CONN} -f 100 -w 20 -n dbt5 -o ~/results/${TXN}_${TYPEN}_${TXNNAME}_${CONN}conn_${DURATION}sec
				sleep 120

				echo "======finish threads $CONN ====="

		done
}

run_benchmark
