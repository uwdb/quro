import os
import sys
import random
args = sys.argv
fileid = sys.argv[1]
bhcnt = int(sys.argv[2], 10)
mecnt = bhcnt/4
#file [0, bhcnt/4-1], [bhcnt/4, bhcnt+bhcnt/4-1]
def cal_time(start_s, start_u):
		return float(start_s) + float(start_u)/1000000.0

commit_cnt_total = 1
abort_cnt_total = 1
commit_exec_time_total = 0
abort_exec_time_total = 0
expan = 0
startt = 0
total_exec_time = 0

for i in range(mecnt):
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

TR_commit_total = commit_cnt_total;
TR_abort_total = abort_cnt_total;
TR_commit_avg = float(commit_exec_time_total)/float(commit_cnt_total)
TR_abort_avg = float(abort_exec_time_total)/float(abort_cnt_total)
TR_txn_avg = float(commit_exec_time_total + abort_exec_time_total)/float(abort_cnt_total+commit_cnt_total)
TR_exec_total = total_exec_time

commit_cnt_total = 1
abort_cnt_total = 1
commit_exec_time_total = 0
abort_exec_time_total = 0
total_exec_time = 0

for i in range(bhcnt):
		fname = "%s/bh/connection_%d.log"%(fileid, i+mecnt+1)
		fp = open(fname, "r")
		startt = 0
		expan = 0
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
								commit_cnt_total = commit_cnt_total + 1
						if startt==0:
								startt = cal_time(int(chs[2], 10), int(chs[3], 10))
						expan = cal_time(int(chs[6], 10), int(chs[7], 10)) - startt
		total_exec_time = total_exec_time+expan
#		print "file %d, expan = %f, total_exec_time = %f"%(i, expan, total_exec_time)
		fp.close()
TO_commit_total = commit_cnt_total;
TO_abort_total = abort_cnt_total;
TO_commit_avg = float(commit_exec_time_total)/float(commit_cnt_total)
TO_abort_avg = float(abort_exec_time_total)/float(abort_cnt_total)
TO_txn_avg = float(commit_exec_time_total + abort_exec_time_total)/float(abort_cnt_total+commit_cnt_total)
TO_exec_total = total_exec_time

def readSingleFile(fp):
	_bin = [[0 for x in range(23)] for x in range(6)]
	_cnt = [[0 for x in range(23)] for x in range(6)]
	_max = [[0 for x in range(23)] for x in range(6)]
	_var = [[0 for x in range(23)] for x in range(6)]
	for i in range(6):
			for j in range(23):
					_bin[i][j] = 0.0
					_cnt[i][j] = 0
					_max[i][j] = 0.0
					_var[i][j] = 0.0
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
									_bin[frame-1][query-1] = _bin[frame-1][query-1] + _time
									_add = _add + _time
									_cnt[frame-1][query-1] = _cnt[frame-1][query-1] + 1
									if(_max[frame-1][query-1]<_time):
											_max[frame-1][query-1] = _time
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
									avg = _bin[frame-1][query-1]/float(_cnt[frame-1][query-1])
									_var[frame-1][query-1] = _var[frame-1][query-1] + (_time-avg)*(_time-avg)
	
	for i in range(6):
			print "frame %d: "%(i+1),
			for j in range(23):
					if(_bin[i][j]>0):
							print "query %d(%f)"%(j+1, _bin[i][j]/float(_cnt[i][j])),
			print ""
	
	print "------add-----"
	print "%f"%(_add/float(commit_cnt + abort_cnt))
	print "-----------variance------"
	for i in range(6):
			print "frame %d: "%(i+1),
			for j in range(23):
					if(_bin[i][j]>0):
							print "query %d(%f)"%(j+1, _var[i][j]/float(_cnt[i][j])),
			print ""
	
	#print "-----------max------"
	#for i in range(6):
	#		print "frame %d: "%(i+1),
	#		for j in range(23):
	#				if(_bin[i][j]>0):
	#						print "query %d(%f)"%(j+1, _max[i][j]),
	#		print ""
	
	print "-----------cnt------"
	for i in range(6):
			print "frame %d: "%(i+1),
			for j in range(23):
					if(_bin[i][j]>0):
							print "query %d(%d)"%(j+1, _cnt[i][j]),
			print ""
	
	print "--------"
	print "abort cnt: %d"%abort_cnt
	print "avg abort txn time: %f"%(abort_exec_time/float(abort_cnt))
	print "commit cnt: %d"%commit_cnt
	print "avg commit txn time: %f"%(commit_exec_time/float(commit_cnt))
	print "waiting time: %f"%(waiting_time)

print "==================TradeResult================="
id1 = int(mecnt/2)
filename = "%s/bh/connection_%d.log"%(fileid, id1)
print "filename = %s"%filename
fp = open(filename, "r")
print "id1 = %d"%(id1)
readSingleFile(fp)
fp.close()

print ""
print "==================TradeOrder================="
id2 = int(mecnt + bhcnt/2+1)
filename = "%s/bh/connection_%d.log"%(fileid, id2)
fp = open(filename, "r")
print "id1 = %d"%(id2)
readSingleFile(fp)
fp.close()

print ""
print "=================Final================="
print "TR total_time = %f, TO total_time = %f (per conn)"%(float(TR_exec_total)/float(mecnt), float(TO_exec_total)/float(bhcnt))
print "TR total cnt = %d (%d), TO total_cnt = %d (%d)"%(TR_commit_total, TR_commit_total+TR_abort_total, TO_commit_total, TO_commit_total+TO_abort_total)
print "%f\t%f\t\t%f\t%f\t%f"%(float(TR_commit_total)/TR_exec_total, TR_txn_avg, TR_abort_avg, TR_commit_avg, float(TR_abort_total)/float(TR_abort_total+TR_commit_total))
print "%f\t%f\t\t%f\t%f\t%f"%(float(TO_commit_total)/TO_exec_total, TO_txn_avg, TO_abort_avg, TO_commit_avg, float(TO_abort_total)/float(TO_abort_total+TO_commit_total))

