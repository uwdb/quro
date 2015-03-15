import os

filename = "/tmp/results5/bh/connection_98.log"

fp = open(filename, "r")

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
cnt = 0
exec_time = 0

for line in fp:
		chs = line.split(" ")
		if chs[0] == "start=(":
				exec_time = exec_time + float(chs[8][0:len(chs[8])-1])
				cnt = cnt + 1

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

print "-----------max------"
for i in range(6):
		print "frame %d: "%(i+1),
		for j in range(23):
				if(_bin[i][j]>0):
						print "query %d(%f)"%(j+1, _max[i][j]),
		print ""


print "--------"
print "avg txn time: %f"%(exec_time/float(cnt))
