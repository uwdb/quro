#!/bin/bash
REORDERN="2item_orig"
TXN="bid"

USERS=1
./bid-run-workload  -d 60 -u ${USERS}  -n bid -o ~/results_link/DBT5_results/${TXN}_${REORDERN}_${USERS}conn

echo"========finish user 1==========="
sleep 120


#USERS=2
#./bid-run-workload  -d 60 -u ${USERS}  -n bid -o ~/results_link/DBT5_results/${TXN}_${REORDERN}_${USERS}conn
#
#echo"========finish user 2==========="
#sleep 120

#USERS=4
#./bid-run-workload  -d 60 -u ${USERS}  -n bid -o ~/results_link/DBT5_results/${TXN}_${REORDERN}_${USERS}conn
#
#echo"========finish user 4==========="
#sleep 120

USERS=8
./bid-run-workload  -d 60 -u ${USERS}  -n bid -o ~/results_link/DBT5_results/${TXN}_${REORDERN}_${USERS}conn

echo"========finish user 8==========="
sleep 120

USERS=16
./bid-run-workload  -d 60 -u ${USERS}  -n bid -o ~/results_link/DBT5_results/${TXN}_${REORDERN}_${USERS}conn

echo"========finish user 16==========="
sleep 120

USERS=32
./bid-run-workload  -d 60 -u ${USERS}  -n bid -o ~/results_link/DBT5_results/${TXN}_${REORDERN}_${USERS}conn

echo"========finish user 32==========="
sleep 120

USERS=64
./bid-run-workload  -d 60 -u ${USERS}  -n bid -o ~/results_link/DBT5_results/${TXN}_${REORDERN}_${USERS}conn

echo"========finish user 64==========="
sleep 120

USERS=128
./bid-run-workload  -d 60 -u ${USERS}  -n bid -o ~/results_link/DBT5_results/${TXN}_${REORDERN}_${USERS}conn

echo"========finish user 128==========="
