import os
import sys
import random
args = sys.argv
fileid = sys.argv[1]
cnt = int(sys.argv[2], 10)

def cal_time(start_s, start_u):
		return float(start_s) + float(start_u)/1000000.0

commit_cnt_total = 1
abort_cnt_total = 1
commit_exec_time_total = 0
abort_exec_time_total = 0
expan = 0
startt = 0
total_exec_time = 0


for i in range(cnt):
		fname = "%s/bh/connection_%d.log"%(fileid, i)
		fp = open(fname, "r")
		startt = 0
		expan = 0
		endt = 0
		for line in fp:
				chs = line.strip().split(" ")
				#if len(chs)==1 and chs[0] == "END":
				#		break
				if len(chs)>1 and chs[1] == "start=(":
						if chs[0] == "0":
								abort_exec_time_total = abort_exec_time_total + float(chs[9][0:len(chs[9])-1])
								abort_cnt_total = abort_cnt_total + 1
						else:
								commit_exec_time_total = commit_exec_time_total +  float(chs[9][0:len(chs[9])-1])
								if float(chs[9][0:len(chs[9])-1]) < 0.00001:
									continue;
								else:
									commit_cnt_total = commit_cnt_total + 1
						if startt==0:
								startt = cal_time(int(chs[2], 10), int(chs[3], 10))
						expan = cal_time(int(chs[6], 10), int(chs[7], 10)) - startt
						endt = cal_time(int(chs[6], 10), int(chs[7], 10))
		total_exec_time = total_exec_time+expan
		fp.close()

TU_commit_total = commit_cnt_total;
TU_abort_total = abort_cnt_total;
TU_commit_avg = float(commit_exec_time_total)/float(commit_cnt_total)
TU_abort_avg = float(abort_exec_time_total)/float(abort_cnt_total)
TU_txn_avg = float(commit_exec_time_total + abort_exec_time_total)/float(abort_cnt_total+commit_cnt_total)
TU_exec_total = total_exec_time



def readSingleFile(fp):
	_bin = [0 for x in range(24)]
	_cnt = [0 for x in range(24)]
	_max = [0 for x in range(24)]
	_var = [0 for x in range(24)]
	for i in range(8):
		_bin[i] = 0.0
		_cnt[i] = 0
		_max[i] = 0.0
		_var[i] = 0.0
	avg = 0.0
	commit_cnt = 1
	abort_cnt = 1
	commit_exec_time = 0
	abort_exec_time = 0
	waiting_time = 0
	last_time = 0
	_add = 0
	for line in fp:
			chs = line.strip().split(" ")
			#if chs[0] == "END":
			#		print "FIND END"
			#		break
			if len(chs)>1 and chs[1] == "start=(":
					if chs[0] == "0":
							abort_exec_time = abort_exec_time + float(chs[9][0:len(chs[9])-1])
							abort_cnt = abort_cnt + 1
					else:
							commit_exec_time = commit_exec_time +  float(chs[9][0:len(chs[9])-1])
							if float(chs[9][0:len(chs[9])-1]) < 0.00001:
									continue;
							else:
									commit_cnt = commit_cnt + 1
					cur_time = cal_time(int(chs[2], 10), int(chs[3], 10))
					if last_time > 0:
							waiting_time = waiting_time + cur_time-last_time;
					last_time = cal_time(int(chs[6], 10), int(chs[7], 10))
	
			elif len(chs)>2 and ('_' in chs[0]):
					for i in range(len(chs)):
							if '_' in chs[i]:
									q_id = chs[i].split('_')
									frame = int(q_id[0], 10)
									query = int(q_id[1], 10)
									_time = float(chs[i+1][1:len(chs[i+1])-1])
									_bin[query-1] = _bin[query-1] + _time
									_add = _add + _time
									_cnt[query-1] = _cnt[query-1] + 1
									if(_max[query-1]<_time):
											_max[query-1] = _time
	fp.close()
	fp = open(filename, "r")
	for line in fp:
			chs = line.strip().split(" ")
			#if len(chs)==1 and chs[0] == "END":
			#			break
			if chs[0]!="start=(" and len(chs)>2 and ('_' in chs[0]):
					for i in range(len(chs)):
							if '_' in chs[i]:
									q_id = chs[i].split('_')
									frame = int(q_id[0], 10)
									query = int(q_id[1], 10)
									_time = float(chs[i+1][1:len(chs[i+1])-1])
									avg = _bin[query-1]/float(_cnt[query-1])
									_var[query-1] = _var[query-1] + (_time-avg)*(_time-avg)
	
	for i in range(8):
		if(_bin[i]>0):
			print "query %d(%f)"%(i+1, _bin[i]/float(_cnt[i])),
	print ""
	
	print "------add-----"
	print "%f"%(_add/float(commit_cnt + abort_cnt))
	print "-----------variance------"
	for i in range(8):
		if(_bin[i]>0):
			print "query %d(%f)"%(i+1, _var[i]/float(_cnt[i])),
	print ""
	
	#print "-----------max------"
	#for i in range(6):
	#		print "frame %d: "%(i+1),
	#		for j in range(24):
	#				if(_bin[i][j]>0):
	#						print "query %d(%f)"%(j+1, _max[i][j]),
	#		print ""
	
	print "-----------cnt------"
	for i in range(8):
		if(_bin[i]>0):
				print "query %d(%d)"%(i+1, _cnt[i]),
	print ""
	
	print "--------"
	print "abort cnt: %d"%abort_cnt
	print "avg abort txn time: %f"%(abort_exec_time/float(abort_cnt))
	print "commit cnt: %d"%commit_cnt
	print "avg commit txn time: %f"%(commit_exec_time/float(commit_cnt))
	print "waiting time: %f"%(waiting_time)
        return commit_exec_time/float(commit_cnt)


print "==================TradeUpdate======================"
id1 = int(cnt/2+1)
filename = "%s/bh/connection_%d.log"%(fileid, id1)
print "filename = %s"%filename
fp = open(filename, "r")
print "id1 = %d"%(id1)
latency = readSingleFile(fp)
fp.close()

print "===================Final================="
print "TU total time = %f"%(float(TU_exec_total)/float(cnt))
print "TU total cnt = %d ( %d )"%(TU_commit_total, TU_commit_total + TU_abort_total)

print "%f\t%f\t%f\t%f\t%f"%(float(TU_commit_total)/160.0,  float(TU_commit_total)/(float(TU_exec_total)/float(cnt)), latency, float(TU_exec_total)/float(TU_commit_total+TU_abort_total), float(TU_abort_total)/float(TU_abort_total+TU_commit_total)) 

