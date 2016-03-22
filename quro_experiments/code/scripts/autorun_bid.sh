#!/bin/bash
TXN="bid"
TYPEN="orig"

DURATION=300
#DURATION=1200
ITEM=1

CONNS="1 2 4 8 16 32 64"

if [ $# -lt 2 ]
  then
  echo "At least 2 arguments [transaction_name] [implementation_type] are required"
  echo "  For example: ./autorun_bid.sh bid orig"
  echo "  or           ./autorun_bid.sh bid reorder"
  exit
fi

TXNNAME=$1
shift
TYPEN=$1
shift
DURATION=$1
shift

if [ $# -gt 0 ]
  then
  CONNS=""
  for var in "$@"
    do
    CONNS="${CONNS} $var"
  done
fi

echo "Running ${TXN} ${TXNNAME} (${TYPEN} implementation on ${ITEM} items) with ${CONNS} number of threads"

run_benchmark()
{

		for CONN in $CONNS ; do
				echo "start running bid on ${ITEM} items, with $CONN threads"
				./bid-mysql-build-db
				./bid-run-workload -d ${DURATION} -u ${CONN} -n bid -f ${SCALEFACTOR} -o ~/results/${TXN}_${TYPEN}_${TXNNAME}_${ITEM}item_${CONN}conn_${DURATION}sec
				sleep 120

				echo "======finish running with $CONN threads====="

		done
}

run_benchmark
