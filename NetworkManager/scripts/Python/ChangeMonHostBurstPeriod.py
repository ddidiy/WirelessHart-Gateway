#!/usr/bin/python
#
# edit Monitor_Host_Publishers conf file and changes burst update period
# complaints: radu marginean
#

import os
import re
import string

def ChangeBurstPeriod (confFileName_, newBurstPeriod_):

	# used to store conf file content
	lines = []

	# read conf file
	confFile = open(confFileName_, "r")

	for line in confFile:
		lines.append(line)

	confFile.close()

	# move old conf file
	os.system('cp ' + confFileName_ + ' ' + confFileName_ + '.old')

	# write new conf file
	confFile = open(confFileName_, "w")

	for line in lines:
	
		if re.search("BURST_SET_STAT_0", line) is not None:

			indx = line.find('=')
			if indx == -1:
				print "Unexpected file format. Exit."
				exit(1)

			line = line[:indx] + '= NOT_SET\n'

		elif re.search("BURST =", line) is not None:
		
			items = string.split(line, ',')
			items[2] = str(newBurstPeriod_)
			line = items[0]
			for i in range(1, len(items)):
				line += ', ' + re.sub(r'\s', '', items[i])
			if line [len(line) -1] != '\n':
				line += '\n'

		confFile.write(line)

	confFile.close()
	return None


