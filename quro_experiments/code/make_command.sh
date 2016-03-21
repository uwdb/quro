if [ $1 = "TPCC" ]; then
  make clean
  echo "$(tail -n +2 CMakeLists.txt)" > CMakeLists.txt
  sed -i '1s/^/SET(WORKLOAD_TPCC 1)\n/' CMakeLists.txt
  cmake ./
  make
  sed -i.bck '$s/$/ -lpthread -lmysqlclient/' CMakeFiles/bin/TPCCMain.dir/link.txt
  sed -i.bck '$s/$/ -lpthread -lmysqlclient/' CMakeFiles/bin/DriverMain.dir/link.txt
  make
  make install

elif [ $1 = "TPCE" ]; then
  make clean
  echo "$(tail -n +2 CMakeLists.txt)" > CMakeLists.txt
  sed -i '1s/^/SET(WORKLOAD_TPCE 1)\n/' CMakeLists.txt
  cmake ./
  cd egen/prj
  make clean
  make
  cd ../..
  make
  sed -i.bck '$s/$/ -lpthread -lmysqlclient/' CMakeFiles/bin/BrokerageHouseMain.dir/link.txt
  sed -i.bck '$s/$/ -lpthread -lmysqlclient/' CMakeFiles/bin/MarketExchangeMain.dir/link.txt
  sed -i.bck '$s/$/ -lpthread -lmysqlclient/' CMakeFiles/bin/DriverMain.dir/link.txt
  make
  make install

elif [ $1 = "BID" ]; then
  make clean
  echo "$(tail -n +2 CMakeLists.txt)" > CMakeLists.txt
  sed -i '1s/^/SET(WORKLOAD_BID 1)\n/' CMakeLists.txt
  cmake ./
  make
  sed -i.bck '$s/$/ -lpthread -lmysqlclient/' CMakeFiles/bin/BidMain.dir/link.txt
  sed -i.bck '$s/$/ -lpthread -lmysqlclient/' CMakeFiles/bin/DriverMain.dir/link.txt
  make
  make install

else
  echo "Please specify a benchmark: TPCC / TPCE / BID"
fi
