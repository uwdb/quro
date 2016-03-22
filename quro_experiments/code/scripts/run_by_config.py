import os
import sys

config_file=sys.argv[1]
fp = open(config_file, "r")

benchmark = "tpcc"
transaction = "payment"
duration = 300
impl = "original"
connections = "4 8"
warehouses = 1
trades = 1024
items = 1

for line in fp:
  line = line.replace("\n","")
  if len(line) > 0:
    if line.startswith("#") == False:
      chs = line.split(" ")
      if chs[0].startswith("BENCHMARK"):
        benchmark = chs[1]
      elif chs[0].startswith("TRANSACTION"):
        transaction = chs[1]
      elif chs[0].startswith("DURATION"):
        duration = int(chs[1], 10)
      elif chs[0].startswith("IMPL"):
        impl = chs[1]
      elif chs[0].startswith("CONNECTIONS"):
        connections = line.replace("CONNECTIONS: ","")
      elif chs[0].startswith("WAREHOUSES"):
        warehouses = int(chs[1], 10)
      elif chs[0].startswith("TRADES"):
        trades = int(chs[1], 10)
      elif chs[0].startswith("ITEMS"):
        items = int(chs[1], 10)
      else:
        print "Unrecognized configuration %s"%line
        exit(0)

if benchmark == "TPCE":
  header_file_string = ""
  if transaction == "tradeupdate":
    header_file_string += "#define TRADEUPDATE\n"
  elif transaction == "tradeorder":
    header_file_string += "#define TRADEORDER\n"
  elif transaction == "trademix":
    header_file_string += "#define TRADEMIX\n"
  elif transaction == "tradestatus":
    header_file_string += "#define TRADESTATUS\n"
  else:
    print "Unrecognied transaction type %s!"%transaction
    exit(0)
  
  if impl == "reorder":
    header_file_string += "\n#define QURO\n"

  header_file_string += "\n#define TRADEUPDATE_TRADE_NUM %d\n"%(trades)
  
  header_file = open("../src/include/TPCE_const.h","w")
  header_file.write(header_file_string)
  header_file.close()

  os.system("./call_make_command.sh TPCE")
  os.system("./autorun_tpce.sh %s %s %d %s"%(transaction, impl, duration, connections))

elif benchmark == "BID":
  header_file_string = ""
  header_file_string += "#define TOTAL_USER_NUM 4000\n"
  header_file_string += "#define TOTAL_ITEM_NUM %d\n"%(items)
  header_file_string += "#define PRICE_FACTOR 1000.0\n"
  header_file_string += "#define HIGHER_BID_THRESHOLD 0.8\n"
  
  if impl == "reorder":
    header_file_string += "\n#define QURO\n"
  
  header_file = open("../src/include/Bid_const.h","w")
  header_file.write(header_file_string)
  header_file.close()

  os.system("./call_make_command.sh BID")
  os.system("./autorun_bid.sh %s %s %d %s"%(transaction, impl, duration, connections))

else: #benchmark == "TPCC"
  header_file_string = ""
  if transaction == "neworder":
    header_file_string += "#define NEW_ORDER_PERC 1.0\n"
    header_file_string += "#define PAYMENT_PERC 0.0\n"
    header_file_string += "#define DELIVERY_PERC 0.0\n"
    header_file_string += "#define STOCKLEVEL_PERC 0.0\n"
    header_file_string += "#define ORDERSTATUS_PERC 0.0\n"
  elif transaction == "payment":
    header_file_string += "#define NEW_ORDER_PERC 0.0\n"
    header_file_string += "#define PAYMENT_PERC 1.0\n"
    header_file_string += "#define DELIVERY_PERC 0.0\n"
    header_file_string += "#define STOCKLEVEL_PERC 0.0\n"
    header_file_string += "#define ORDERSTATUS_PERC 0.0\n"
  elif transaction == "mix_of_payment_neworder" or transaction == "mix_of_neworder_payment":
    header_file_string += "#define NEW_ORDER_PERC 0.5\n"
    header_file_string += "#define PAYMENT_PERC 0.5\n"
    header_file_string += "#define DELIVERY_PERC 0.0\n"
    header_file_string += "#define STOCKLEVEL_PERC 0.0\n"
    header_file_string += "#define ORDERSTATUS_PERC 0.0\n"
  elif transaction == "mix_of_all":
    header_file_string += "#define NEW_ORDER_PERC 0.45\n"
    header_file_string += "#define PAYMENT_PERC 0.43\n"
    header_file_string += "#define DELIVERY_PERC 0.04\n"
    header_file_string += "#define STOCKLEVEL_PERC 0.04\n"
    header_file_string += "#define ORDERSTATUS_PERC 0.04\n"
  else:
    print "Unrecognied transaction type %s!"%transaction
    exit(0)
  
  if impl == "reorder":
    header_file_string += "\n#define QURO\n\n"

  header_file_string += "#define D_ID_MAX 10\n"
  header_file_string += "#define C_ID_UNKNOWN 0\n"
  header_file_string += "#define O_CARRIER_ID_MAX 10\n"
  header_file_string += "#define C_LAST_LEN 12\n"
  header_file_string += "#define C_LAST_SYL_MAX 10\n"

  header_file = open("../src/include/TPCC_const.h","w")
  header_file.write(header_file_string)
  header_file.close()

  os.system("./call_make_command.sh TPCC")
  os.system("./autorun_tpcc.sh %d %s %s %d %s"%(warehouses, transaction, impl, duration, connections))

