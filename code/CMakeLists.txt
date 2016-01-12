SET(WORKLOAD_TPCC 1)
#CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
#PROJECT(dbt5 CXX)
#SET(WORKLOAD_TPCE 1)
#SET(WORKLOAD_SEATS 1)
#SET(WORKLOAD_BID 1)
#SET(WORKLOAD_TPCC 1)
#SET(DEF_REORDER 1)
#
# Defining binaries so to minimized typoes.
#

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-rdynamic -lpthread -lmysqlclient")

SET(MYSQL_LIB /usr/lib/x86_64-linux-gnu)
SET(MYSQL_INCLUDE /usr/include/mysql)
#SET(HOME /afs/csail.mit.edu/u/c/quro)
SET(DBT5_SEATS bin/SeatsMain)
SET(DBT5_TPCC bin/TPCCMain)
SET(DBT5_BID bin/BidMain)
SET(DBT5_BROKERAGE_HOUSE bin/BrokerageHouseMain)
SET(DBT5_DRIVER bin/DriverMain)
SET(DBT5_MARKET_EXCHANGE bin/MarketExchangeMain)
#SET(DBT5_TEST_TXN bin/TestTxn)
#
# Check for large file support by using 'getconf'.
#

FIND_PROGRAM(GETCONF getconf)
IF (GETCONF)
    SET(GETCONF_FOUND TRUE)
ENDIF (GETCONF)

IF (GETCONF_FOUND)
    EXECUTE_PROCESS(COMMAND ${GETCONF} LFS_CFLAGS COMMAND ${GETCONF}
            LFS64_CFLAGS OUTPUT_VARIABLE LFS_CFLAGS
            OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDIF (GETCONF_FOUND)

#IF (DBMS STREQUAL "pgsql")
#    #
#    # Create PostgreSQL specific configuration.
#    #
#
#    FIND_PROGRAM(PG_CONFIG pg_config)
#
#    IF (PG_CONFIG)
#       SET(PG_CONFIG_FOUND TRUE)
#    ENDIF (PG_CONFIG)
#
#    IF (PG_CONFIG_FOUND)
#        MESSAGE(STATUS "Found pg_config for PostgreSQL: ${PG_CONFIG}")
#
#        EXECUTE_PROCESS(COMMAND ${PG_CONFIG} --includedir OUTPUT_VARIABLE
#                PG_INCLUDEDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
#        EXECUTE_PROCESS(COMMAND ${PG_CONFIG} --libdir OUTPUT_VARIABLE
#                PG_LIBDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
#
#        SET(DBCFLAGS "-DLIBPQ -I${PG_INCLUDEDIR}")
#
#        SET(DB_LIBS "-L${PG_LIBDIR} -lpqxx -lpq")
#
#        SET(DB_SCRIPTS scripts/pgsql/dbt5-pgsql-start-db
#                       scripts/pgsql/dbt5-pgsql-create-indexes
#                       scripts/pgsql/dbt5-pgsql-create-tables
#                       scripts/pgsql/dbt5-pgsql-drop-tables
#                       scripts/pgsql/dbt5-pgsql-restore-db
#                       scripts/pgsql/dbt5-pgsql-load-stored-procs
#                       scripts/pgsql/dbt5-pgsql-backup-db
#                       scripts/pgsql/dbt5-pgsql-db-plans
#                       scripts/pgsql/dbt5-pgsql-drop-db
#                       scripts/pgsql/dbt5-pgsql-check-db
#                       scripts/pgsql/dbt5-pgsql-db-stat
#                       scripts/pgsql/dbt5-pgsql-stop-db
#                       scripts/pgsql/dbt5-pgsql-create-db
#                       scripts/pgsql/dbt5-pgsql-build-db)
#    ENDIF (PG_CONFIG_FOUND)
#ELSE (NOT DEFINED DBMS)
#    MESSAGE(FATAL_ERROR "Need to specify -DDBMS=pgsql")
#
#ENDIF (DBMS STREQUAL "pgsql")
SET(DBCFLAGS "-DLIBPQ -I${PG_INCLUDEDIR} -I${MYSQL_INCLUDE} -lpthread -lmysqlclient")
SET(DB_LIBS "-L${MYSQL_LIB} -lmysqlclient -lz -lm -lrt -ldl -lpthread")
IF (WORKLOAD_TPCE)
	SET(DB_SCRIPTS scripts/mysql/dbt5-mysql-start-db
                       scripts/mysql/dbt5-mysql-create-indexes
                       scripts/mysql/dbt5-mysql-create-tables
											 scripts/mysql/dbt5-mysql-create-db
                       scripts/mysql/dbt5-mysql-build-db)
ELSEIF (WORKLOAD_SEATS)
	SET(DB_SCRIPTS scripts/mysql/seats-mysql-create-tables
										scripts/mysql/seats-mysql-create-db
										scripts/mysql/seats-mysql-build-db)
ELSEIF (WORKLOAD_BID)
	SET(DB_SCRIPTS scripts/mysql/bid-mysql-create-tables
										scripts/mysql/bid-mysql-create-db
										scripts/mysql/bid-mysql-build-db)
ELSEIF (WORKLOAD_TPCC)
	SET(DB_SCRIPTS scripts/mysql/tpcc-mysql-create-tables
										scripts/mysql/tpcc-mysql-create-db
										scripts/mysql/tpcc-mysql-create-db)
ENDIF (WORKLOAD_TPCE)
#
# Defining commonly used paths.
#
SET(EGENINC egen/inc)
SET(EGENSRC egen/src)
SET(TXN src/transactions)
SET(INTF src/interfaces)


SET(BH src/BrokerageHouse)
SET(CE src/Driver)
SET(ME src/MarketExchange)

#
# Defining common groups of source files.
#
IF (WORKLOAD_TPCE)
	IF (DEF_REORDER)
			SET(TXN_SOURCE ${TXN}/pgsql/DBConnection.cpp
               ${TXN}/TxnBaseDB.cpp
               ${TXN}/MarketFeedDB.cpp
               ${TXN}/TradeCleanupDB.cpp
               ${TXN}/DataMaintenanceDB.cpp
               ${TXN}/MarketWatchDB.cpp
               ${TXN}/SecurityDetailDB.cpp
               ${TXN}/BrokerVolumeDB.cpp
               ${TXN}/CustomerPositionDB.cpp
               ${TXN}/TradeUpdateDB.cpp
               ${TXN}/TradeLookupDB.cpp
               ${TXN}/TradeResultDB.cpp
               ${TXN}/TradeOrderDB.cpp
							 ${TXN}/TradeOrderReorderTxn.cpp
							 ${TXN}/TradeResultReorderTxn.cpp
							 ${TXN}/TradeUpdateTxn.cpp
               ${TXN}/TradeStatusDB.cpp
							 ${TXN}/TradeStatusTxn.cpp)
	ELSE (NOT_DEFINED_REORDER)
				SET(TXN_SOURCE ${TXN}/pgsql/DBConnection.cpp
               ${TXN}/TxnBaseDB.cpp
               ${TXN}/MarketFeedDB.cpp
               ${TXN}/TradeCleanupDB.cpp
               ${TXN}/DataMaintenanceDB.cpp
               ${TXN}/MarketWatchDB.cpp
               ${TXN}/SecurityDetailDB.cpp
               ${TXN}/BrokerVolumeDB.cpp
               ${TXN}/CustomerPositionDB.cpp
               ${TXN}/TradeUpdateDB.cpp
               ${TXN}/TradeLookupDB.cpp
               ${TXN}/TradeResultDB.cpp
               ${TXN}/TradeOrderDB.cpp
							 ${TXN}/TradeOrderTxn.cpp
							 ${TXN}/TradeResultTxn.cpp
							 ${TXN}/TradeUpdateTxn.cpp
               ${TXN}/TradeStatusDB.cpp
							 ${TXN}/TradeStatusTxn.cpp)
	ENDIF (DEF_REORDER)
ELSEIF (WORKLOAD_SEATS)
			SET(TXN_SOURCE ${TXN}/pgsql/DBConnection.cpp
									${TXN}/TxnBaseDB.cpp
									${TXN}/Seats/NewReservationTxn.cpp
									${TXN}/Seats/FindFlightTxn.cpp)
ELSEIF (WORKLOAD_BID)
			SET(TXN_SOURCE ${TXN}/pgsql/DBConnection.cpp
									${TXN}/TxnBaseDB.cpp
									${TXN}/BiddingTxn.cpp)
ELSEIF (WORKLOAD_TPCC)
			SET(TXN_SOURCE ${TXN}/pgsql/DBConnection.cpp
									${TXN}/TxnBaseDB.cpp
									${TXN}/StocklevelTxn.cpp
									${TXN}/DeliveryTxn.cpp
									${TXN}/OrderstatusTxn.cpp
									${TXN}/NewOrderTxn.cpp
									${TXN}/PaymentTxn.cpp)
#									${TXN}/Storeproc_stocklevel.cpp
#									${TXN}/Storeproc_orderstatus.cpp
#									${TXN}/Storeproc_delivery.cpp
#									${TXN}/Storeproc_neworder.cpp
#									${TXN}/Storeproc_payment.cpp)
ENDIF (WORKLOAD_TPCE)
#
# Files to build.
#
IF (WORKLOAD_TPCE)
	ADD_EXECUTABLE(${DBT5_BROKERAGE_HOUSE}
               ${EGENSRC}/DateTime
               ${EGENSRC}/locking.cpp
               ${INTF}/CSocket.cpp
               ${INTF}/TxnHarnessSendToMarket.cpp
	       ${EGENSRC}/ReadRowFunctions.cpp
               ${EGENSRC}/BaseLogger.cpp
               ${EGENSRC}/EGenLogFormatterTab.cpp
               ${EGENSRC}/EGenVersion.cpp
               ${EGENSRC}/error.cpp
               ${EGENSRC}/Random.cpp
               ${EGENSRC}/DateTime.cpp
               ${EGENSRC}/WheelTime.cpp
               ${EGENSRC}/MEESecurity.cpp
               ${EGENSRC}/MEEPriceBoard.cpp
               ${EGENSRC}/MEETickerTape.cpp
               ${EGENSRC}/MEETradingFloor.cpp
               ${EGENSRC}/MEE.cpp
               ${EGENSRC}/locking.cpp
               ${EGENSRC}/InputFlatFilesStructure.cpp
	       ${INTF}/MEESUT.cpp
               ${INTF}/BaseInterface.cpp
               ${TXN_SOURCE}
               ${BH}/BrokerageHouseMain.cpp
               ${BH}/BrokerageHouse.cpp)

	ADD_EXECUTABLE(${DBT5_DRIVER}
               ${CE}/Driver.cpp
               ${CE}/DriverMain.cpp
               src/Customer/Customer.cpp
               ${EGENSRC}/BaseLogger.cpp
               ${EGENSRC}/EGenLogFormatterTab.cpp
               ${EGENSRC}/CETxnMixGenerator.cpp
               ${EGENSRC}/CETxnInputGenerator.cpp
               ${EGENSRC}/ReadRowFunctions.cpp
               ${EGENSRC}/InputFlatFilesStructure.cpp
               ${EGENSRC}/error.cpp
               ${EGENSRC}/AddressTable.cpp
               ${EGENSRC}/EGenVersion.cpp
               ${EGENSRC}/CustomerTable.cpp
               ${EGENSRC}/CustomerSelection.cpp
               ${EGENSRC}/Random.cpp
               ${EGENSRC}/Person.cpp
               ${EGENSRC}/DateTime.cpp
               ${EGENSRC}/CE.cpp
               ${EGENSRC}/DM.cpp
               ${EGENSRC}/locking.cpp
               ${EGENSRC}/MEE.cpp
               ${EGENSRC}/MEETradingFloor.cpp
               ${EGENSRC}/MEETickerTape.cpp
               ${EGENSRC}/MEEPriceBoard.cpp
               ${EGENSRC}/WheelTime.cpp
               ${EGENSRC}/MEESecurity.cpp
               ${INTF}/CESUT.cpp
               ${INTF}/CSocket.cpp
               ${INTF}/DMSUT.cpp
               ${INTF}/BaseInterface.cpp)

	ADD_EXECUTABLE(${DBT5_MARKET_EXCHANGE}
               ${EGENSRC}/ReadRowFunctions.cpp
               ${EGENSRC}/BaseLogger.cpp
               ${EGENSRC}/EGenLogFormatterTab.cpp
               ${EGENSRC}/EGenVersion.cpp
               ${EGENSRC}/error.cpp
               ${INTF}/CSocket.cpp
               ${EGENSRC}/Random.cpp
               ${EGENSRC}/DateTime.cpp
               ${EGENSRC}/WheelTime.cpp
               ${EGENSRC}/MEESecurity.cpp
               ${EGENSRC}/MEEPriceBoard.cpp
               ${EGENSRC}/MEETickerTape.cpp
               ${EGENSRC}/MEETradingFloor.cpp
               ${EGENSRC}/MEE.cpp
               ${EGENSRC}/locking.cpp
               ${EGENSRC}/InputFlatFilesStructure.cpp
               ${INTF}/MEESUT.cpp
               ${INTF}/BaseInterface.cpp
               ${ME}/MarketExchange.cpp
               ${ME}/MarketExchangeMain.cpp)

ELSEIF (WORKLOAD_SEATS)
	ADD_EXECUTABLE(${DBT5_SEATS}
							 ${EGENSRC}/DateTime.cpp
               ${EGENSRC}/locking.cpp
               ${INTF}/CSocket.cpp
							 ${INTF}/util.cpp
							 ${INTF}/CSeats.cpp
               ${TXN_SOURCE}
							 ${INTF}/BaseInterface.cpp
							 src/Seats/SeatsMain.cpp
							 src/Seats/Seats.cpp)
			 	ADD_EXECUTABLE(${DBT5_DRIVER}
			                src/Customer/Customer.cpp
			                ${CE}/Driver.cpp
			                ${CE}/DriverMain.cpp
			 							  ${EGENSRC}/DateTime.cpp
			 							  ${EGENSRC}/locking.cpp
			 							  ${INTF}/CSocket.cpp
											${INTF}/util.cpp
			 						    ${INTF}/CSeats.cpp
			                ${INTF}/BaseInterface.cpp)
ELSEIF (WORKLOAD_BID)
				ADD_EXECUTABLE(${DBT5_BID}
							 ${EGENSRC}/DateTime.cpp
               ${EGENSRC}/locking.cpp
               ${INTF}/CSocket.cpp
							 ${INTF}/util.cpp
							 ${INTF}/CBid.cpp
               ${TXN_SOURCE}
							 ${INTF}/BaseInterface.cpp
							 src/Bid/BidMain.cpp
							 src/Bid/Bid.cpp)
			 	ADD_EXECUTABLE(${DBT5_DRIVER}
			                src/Customer/Customer.cpp
			                ${CE}/Driver.cpp
			                ${CE}/DriverMain.cpp
			 							  ${EGENSRC}/DateTime.cpp
			 							  ${EGENSRC}/locking.cpp
			 							  ${INTF}/CSocket.cpp
											${INTF}/util.cpp
			 						    ${INTF}/CBid.cpp
			                ${INTF}/BaseInterface.cpp)
ELSEIF (WORKLOAD_TPCC)
				ADD_EXECUTABLE(${DBT5_TPCC}
							 ${EGENSRC}/DateTime.cpp
               ${EGENSRC}/locking.cpp
               ${INTF}/CSocket.cpp
							 ${INTF}/util.cpp
							 ${INTF}/CTPCC.cpp
               ${TXN_SOURCE}
							 ${INTF}/BaseInterface.cpp
							 src/TPCC/TPCCMain.cpp
							 src/TPCC/TPCC.cpp)
			 	ADD_EXECUTABLE(${DBT5_DRIVER}
			                src/Customer/Customer.cpp
			                ${CE}/Driver.cpp
			                ${CE}/DriverMain.cpp
			 							  ${EGENSRC}/DateTime.cpp
			 							  ${EGENSRC}/locking.cpp
			 							  ${INTF}/CSocket.cpp
											${INTF}/util.cpp
											${INTF}/CTPCC.cpp
			                ${INTF}/BaseInterface.cpp)
ENDIF (WORKLOAD_TPCE)

set(GLOBAL_LIBRARIES "mysqlclient.so" "pthread.so")
target_link_libraries($(GLOBAL_LIBRARIES))

#
# Set compiler flags.
#
IF (WORKLOAD_TPCE)
	IF (DEF_REORDER)
			SET(CMAKE_CXX_FLAGS "-g -Wall -I${EGENINC} -Isrc/include ${LFS_CFLAGS} -I${MYSQL_INCLUDE} -DREORDER -DWORKLOAD_TPCE -lpthread -lmysqlclient")
	ELSE (NOT DEFINED DEF_REORDER)
			SET(CMAKE_CXX_FLAGS "-g -Wall -I${EGENINC} -Isrc/include ${LFS_CFLAGS} -I${MYSQL_INCLUDE} -DREORDER -DWORKLOAD_TPCE -lpthread -lmysqlclient")
	ENDIF (DEF_REORDER)
ELSEIF (WORKLOAD_SEATS)
	SET(CMAKE_CXX_FLAGS "-g -Wall -I${EGENINC} -Isrc/include ${LFS_CFLAGS} -I${MYSQL_INCLUDE} -DWORKLOAD_SEATS -std=c++11")
ELSEIF (WORKLOAD_BID)
	SET(CMAKE_CXX_FLAGS "-g -Wall -I${EGENINC} -Isrc/include ${LFS_CFLAGS} -I${MYSQL_INCLUDE} -DWORKLOAD_BID -std=c++11")
ELSEIF (WORKLOAD_TPCC)
	SET(CMAKE_CXX_FLAGS "-g -Wall -I${EGENINC} -Isrc/include ${LFS_CFLAGS} -I${MYSQL_INCLUDE} -DWORKLOAD_TPCC -std=c++11")
ENDIF (WORKLOAD_TPCE)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_FORMAT_MACROS")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_CONSTANT_MACROS")

SET_SOURCE_FILES_PROPERTIES(${TXN}/pgsql/DBConnection.cpp COMPILE_FLAGS
        "${DBCFLAGS}")

#
# Set linker flags.
#
IF (WORKLOAD_TPCE)
		SET_TARGET_PROPERTIES(${DBT5_BROKERAGE_HOUSE} PROPERTIES LINK_FLAGS
				"-L${MYSQL_LIB} -lmysqlclient ${DB_LIBS} -lpthread")
		SET_TARGET_PROPERTIES(${DBT5_DRIVER} PROPERTIES LINK_FLAGS "-lpthread -lmysqlclient -L${MYSQL_LIB}")
		SET_TARGET_PROPERTIES(${DBT5_MARKET_EXCHANGE} PROPERTIES LINK_FLAGS "-lpthread -lmysqlclient -L${MYSQL_LIB}")
ELSEIF (WORKLOAD_SEATS)
		SET_TARGET_PROPERTIES(${DBT5_SEATS} PROPERTIES LINK_FLAGS
				"-L${MYSQL_LIB} ${DB_LIBS} -lpthread")
		SET_TARGET_PROPERTIES(${DBT5_DRIVER} PROPERTIES LINK_FLAGS "-lpthread -L${MYSQL_LIB}")
ELSEIF (WORKLOAD_BID)
		SET_TARGET_PROPERTIES(${DBT5_BID} PROPERTIES LINK_FLAGS
				"-L${MYSQL_LIB} ${DB_LIBS} -lpthread")
		SET_TARGET_PROPERTIES(${DBT5_DRIVER} PROPERTIES LINK_FLAGS "-lpthread -L${MYSQL_LIB}")
ELSEIF (WORKLOAD_TPCC)
		SET_TARGET_PROPERTIES(${DBT5_TPCC} PROPERTIES LINK_FLAGS
				"-L${MYSQL_LIB} ${DB_LIBS} -lpthread -lmysqlclient")
		SET_TARGET_PROPERTIES(${DBT5_DRIVER} PROPERTIES LINK_FLAGS "-L${MYSQL_LIB} -lpthread -lmysqlclient")
ENDIF (WORKLOAD_TPCE)
#SET_TARGET_PROPERTIES(${DBT5_TEST_TXN} PROPERTIES LINK_FLAGS
#        "${DB_LIBS} -lpthread")

#
# Install rules.
#
IF (WORKLOAD_TPCE)
		INSTALL(PROGRAMS ${DBT5_BROKERAGE_HOUSE}
                 ${DBT5_DRIVER}
                 ${DBT5_MARKET_EXCHANGE}
                 ${DBT5_TEST_TXN}
                 scripts/dbt5-post-process
                 scripts/dbt5-sysstats
                 scripts/dbt5-get-os-info
                 scripts/dbt5-run-workload
                 scripts/dbt5-plot-transaction-rate
                 scripts/dbt5-generate-report
                 ${DB_SCRIPTS}
        DESTINATION "/home/quro/bin")
ELSEIF (WORKLOAD_SEATS)		
		INSTALL(PROGRAMS ${DBT5_SEATS}
                 ${DBT5_DRIVER}
                 scripts/seats-run-workload
                 ${DB_SCRIPTS}
        DESTINATION "/home/quro/bin")
ELSEIF (WORKLOAD_BID)		
		INSTALL(PROGRAMS ${DBT5_BID}
                 ${DBT5_DRIVER}
                 scripts/bid-run-workload
                 ${DB_SCRIPTS}
        DESTINATION "/home/quro/bin")
ELSEIF (WORKLOAD_TPCC)		
		INSTALL(PROGRAMS ${DBT5_TPCC}
                 ${DBT5_DRIVER}
                 scripts/tpcc-run-workload
                 ${DB_SCRIPTS}
        DESTINATION "/home/quro/bin")

ENDIF (WORKLOAD_TPCE)
