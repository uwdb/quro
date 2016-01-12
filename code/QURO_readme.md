This folder includes the code used for the experiments in the paper: Leveraging Lock Contention to Improve OLTP Application Performance (http://homes.cs.washington.edu/~congy/vldb16_reorder.pdf).
Benchmarks include TPCC, TPCE, and Bidding.


1. To compile:
Use the script make_command.sh [BENCHMARK_NAME]. Options for BENCHMARK_NAME include: TPCC, TPCE, BID.
For example:
$ ./make_command.sh TPCC


2. To build the database:
In this VM, all database have already been created and data is loaded.
Due to the limited space of this VM, the configurations of these databases are as follows:
TPCC: #warehouses=4
TPCE: #transaction_days=5 #customers_total=1000 scale-factor=500
BID: #users=6400000 #items=64 #bid_rec=1921082

To create a database of your own:
*TPCC:
$ cd scripts/mysql
Generate your own TPCC data (examples are: ~/dbt2-w4 [4 warehouse] and ~/dbt2-w32 [32 warehouses])
Modify the datapath in tpcc-mysql-load-data
$ ./tpcc-mysql-build-db

*TPCE:
$ cd scripts/mysql
$ ./dbt5-mysql-build-db -c [#CUSTOMERS] -t [#CUSTOMERS_TOTAL] -s [#SCALE_FACTOR] -w [#TRANSACTION_DAYS]
(This may take a while. For the meaning of each parameter, please refer to TPC-E documentation: http://www.tpc.org/tpce/spec/TPCEDetailed.doc)

*BID:
$ cd scripts/mysql
Modify the configurations in bid_datagen.cpp
$ g++ bid_datagen.cpp -o bid_datagen
$ ./bid_datagen


3. To configure the experiment:
The configuration for each experiment, such as the mix of different types of transactions, are set in header files like src/include/BENCHMARK_const.h.
To use the reordered implementation, add
#define QURO
to the BENCHMARK_const.h file.
Then recompile.


4. To run the experiments:
Scripts to run each application are in the directory ./scripts.
Script like "BENCHMARK_run_workload" runs application once with specific settings.
Script like "autorun_BENCHMARK.sh" runs the application with scaling number of threads.
By default the results are stored in the directory of $HOME/results. Modifying the above scripts can change the output directory (change the value for -o option).


5. To run the stored-procedure implementation of TPC-C benchmark:
Comment out CMakeLists.txt line 170-175 and uncomment line 176-180 (to use source code Storeproc_TXNNAME.cpp instead of TXNNAMETxn.cpp), recompile and rerun the application.


NOTICE THAT:
Since the VM has different hardware settings as in the paper, the results may be different from the paper. Feel free to copy the files to local machines and test the performance.
