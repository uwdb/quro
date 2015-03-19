import os

filename = "/tmp/64_users_mix_reorder/bh/connection_75.log"

fp = open(filename, "r")

def cal_time(start_s, start_u):
		return float(start_s) + float(start_u)/1000000.0

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
commit_cnt = 0
abort_cnt = 0
commit_exec_time = 0
abort_exec_time = 0
waiting_time = 0
last_time = 0

for line in fp:
		chs = line.split(" ")
		if len(chs)>1 and chs[1] == "start=(":
				if chs[0] == "0":
						abort_exec_time = abort_exec_time + float(chs[9][0:len(chs[9])-1])
						abort_cnt = abort_cnt + 1
				else:
						commit_exec_time = commit_exec_time +  float(chs[9][0:len(chs[9])-1])
						commit_cnt = commit_cnt + 1
				cur_time = cal_time(int(chs[2], 10), int(chs[3], 10))
				if last_time > 0:
						waiting_time = waiting_time + cur_time-last_time;
				last_time = cal_time(int(chs[6], 10), int(chs[7], 10))

		elif len(chs)>10 and ('_' in chs[0]):
				for i in range(len(chs)):
						if '_' in chs[i]:
								q_id = chs[i].split('_')
								frame = int(q_id[0], 10)
								query = int(q_id[1], 10)
								_time = float(chs[i+1][1:len(chs[i+1])-1])
								_bin[frame-1][query-1] = _bin[frame-1][query-1] + _time
								_cnt[frame-1][query-1] = _cnt[frame-1][query-1] + 1
								if(_max[frame-1][query-1]<_time):
										_max[frame-1][query-1] = _time
fp.close()
fp = open(filename, "r")
for line in fp:
		chs = line.split(" ")
		if chs[0]!="start=(" and len(chs)>10 and ('_' in chs[0]):
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
