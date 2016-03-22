#!/bin/bash

TXN="tpcc"
TXNNAME="payment"
TYPEN="orig"

DURATION=300
WHN=1

CONNS="1 2 4 8 16 32 64"

if [ $# -lt 2 ]
  then
  echo "At least 4 arguments [number of warehouses] [transaction_name] [implementation_type] [running duration] are required"
  echo "  For example: ./autorun_tpcc.sh 1 payment original 300"
  echo "  or           ./autorun_tpcc.sh 4 mix reorder 100"
  exit
fi

WHN=$1
shift
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

echo "Running ${TXN} ${TXNNAME} (${TYPEN} implementation on ${WHN} warehouses) with ${CONNS} number of threads"

run_benchmark()
{

		for CONN in $CONNS ; do
				echo "start tpcc on ${WHN} warehouses, with $CONN threads"


				./tpcc-run-workload -d ${DURATION} -u ${CONN} -n tpcc -f ${WHN} -o ~/results/${TXN}_${TYPEN}_${TXNNAME}_${WHN}wh_${CONN}conn_${DURATION}sec		
				sleep 120

				echo "======finish running $CONN threads====="

		done
}

run_benchmark
