import os

filename = "/tmp/results/bh/connection_0.log"

fp = open(filename, "r")
_bin = [[0 for x in range(23)] for x in range(6)]

for i in range(6):
		for j in range(23):
				_bin[i][j] = 0

for line in fp:
		chs = line.split(" ")
		if chs[0] == "error:":
				frame = int(chs[3][5], 10)
				query = int(chs[5], 10)
				_bin[frame-1][query-1] = _bin[frame-1][query-1] + 1

print "show error bin:"

for i in range(6):
		print "frame %d: "%(i+1),
		for j in range(23):
				if _bin[i][j]>0:
						print "%d(%d), "%(j+1, _bin[i][j]),
		print ""
