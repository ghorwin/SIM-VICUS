import os
import re
import sys
import bisect     # for binary search algorithm
import optparse   # for command line parameters

from CCDFile import CCDFile

# current dir climate_dir is assumed to be the climate root dir
# first check if we have enough arguments
if (len(sys.argv) != 2):
	print("Syntax: cc2hourly.py <ccd-file>")
	exit()

input = sys.argv[1]
output = input[0:-4] + "-hourly.ccd"

ccd = CCDFile()
ccd.read("", input)

neu_tp = []
neu_val = []

for i in range(1, 8761):
	# time point in seconds
	tp_sec = i*3600
	# store time point
	neu_tp.append(tp_sec)
	# calculate values by linear interpolation
	index = bisect.bisect_right(ccd.tp, tp_sec)
	if index == 0:
		val = ccd.values[0]
	elif index >=  len(ccd.tp):
		val = ccd.values[-1]
	else:
		tp1 = ccd.tp[index-1]
		tp2 = ccd.tp[index]
		v1 = ccd.values[index-1]
		v2 = ccd.values[index]
		val = v1 + (tp_sec - tp1)*(v2-v1)/(tp2 - tp1);
		#print ("{0},{1} {2},{3} : {4} = {5}".format(tp1, v1, tp2, v2, tp_sec, val)) 
	neu_val.append(val)

# set new climate data
ccd.tp = neu_tp
ccd.values = neu_val

# write ccd file
ccd.write("", output)

