#To build:
a. Clone the repo to $LLVM_DIR/llvm/tools/clang/tools

b. Update $LLVM_DIR/llvm/tools/clang/tools/CMakeList.txt

+add_subdirectory(QURO)

c. >> cd $LLVM_DIR/build

cmake $LLVM_DIR/llvm
make

d. Make sure $LLVM_DIR/build/bin is in your PATH

Library dependencies:
a. MySQL (sudo apt-get install libmysqlclient-dev)

#To run QURO:
Simple examples are included in test/.

a. In the directory where you wish to run quro, following files are needed:

a.1. dbname.txt: Including only one line of your database name.

a.2. #{db_name}\_freq.txt: Including the size of each table.

The data comes from profiling. We use the number of records being read/written in application runs to indicate conflict index. More records being read/written indicates less data conflict.

b. Original transaction file. In one of the examples, the transaction is written in tests/simple_bidding.cpp. The other .cpp/.h files (mysql_helper.h, myql_helper.cpp) in the tests/ are just making sure the compiler doesn't report any annoying "class not found" errors.

c. quro #{transaction_source_file} --

The new version of transaction code will be in output.cpp under the same directory of the transaction source file.


#To use external ILP solver: (to be done)

The connection with ILP is still under construction. 
Currently QURO uses a simple heuristic to reorder all statements instead of using ILP.
But it can generate the input for ILP, and different ILP solvers (gurobi, lpsolver) are available in this VM to produce the final order of queries/units.

To use the ILP solver to give the order of only queries: 

>> add "#define GENERATE_ILP_INPUT" to the QURO source code (for example, in QueryBlock.h)

To use the ILP solver to give the order of all reorder units:

>> add "#define GENERATE_LARGE_ILP_INPUT" to the QURO source code

Then

$ cp $HOME/llvm/test/queryinfo.txt lpsolver/lp_solve_5.5/quro/  (for lpsolver)

or

$ cp $HOME/llvm/test/queryinfo.txt gurobi/ (for gurobi)
Then run the ILP solver.


For more information, go to the paper http://homes.cs.washington.edu/~congy/vldb16_reorder.pdf or email congy@cs.washington.edu.

