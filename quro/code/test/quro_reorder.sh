#!/bin/bash


echo "If the script can safely generate output.cpp, then some errors can be safely ignored..."

 if [ $# -lt 1 ]
  then
  echo "Arrgument [benchmark] is required"
  echo "For example: ./quro_reorder payment"
fi

BENCHMARK=$1

echo "${BENCHMARK}" &> dbname.txt

if [ -f "simple_${BENCHMARK}.cpp" ] && [ -f "${BENCHMARK}_freq.txt" ]
then
        rm output.cpp
	quro simple_${BENCHMARK}.cpp -- &> temp.log
else
	echo "${BENCHMARK}_freq.txt is not found. To try existing benchmarks, options for ${BENCHMARK} are payment, neworder, bid"
fi

if [ -f "output.cpp" ]
then
	echo "Thank god quro finished, checkout output.cpp for reordering result"
else
 	echo "Oop there must be some bugs. Please contact congy@cs.washington.edu to complain about this issue..."
fi

