#!/usr/bin/python
#
# used to search nmLinkEngine.log files for entries with duplicate Six+Tix;
# Requesting channel match is optional. Also list entries with tx or rx >= 2
# 

import string
import re
import os

def __ProcessEntries(lines_, skipAdvertiseLinks_, channelNoMustAlsoMatch_):

	result = []
	valuesAndLineTupleArray = []	

	for line in lines_:

		items = string.split(line, ' ')

		entries = []
		for item in items:
			item = item.lstrip().rstrip()
			if len(item): 
				entries.append(item)

		valuesAndLineTupleArray.append((entries, line))

	for i in range(len(valuesAndLineTupleArray)):

		(Foo, Bar) = valuesAndLineTupleArray[i]

		if skipAdvertiseLinks_ and Foo[1] == 'FFFF' and Foo[2] == 'FFFFFFFF':
			continue

		if (int(Foo[9]) >= 2 or int(Foo[10]) >= 2):
			result.append("!!! " + Bar[3:])	

		if (i + 1 >= len(valuesAndLineTupleArray)):
			continue

		for j in range(i + 1, len(valuesAndLineTupleArray)):
				
			(foo, bar) =  valuesAndLineTupleArray[j]

			if (Foo[5] == foo[5]\
			and Foo[6] == foo[6]\
			and (Foo[9] == foo[9] or Foo[10] == foo[10])\
			and (not channelNoMustAlsoMatch_ or Foo[7] == foo[7])):

				result.append(Bar)
				result.append(bar)
				result.append('\n')
				
	return result
#
# processes on log file
#
def __ProcessLogFile(logsFolder_, fileName_,  skipAdvertiseLinks_, channelNoMustAlsoMatch_):

	filePath = logsFolder_ +  '/' + fileName_

	startProcessing = False
	linesBuffer = []

	with open(filePath, "r") as log:

		for line in log:
			
			if re.search( r'Address     Peer   Handler  SId   Pp  Six  Tix   Ch   Lt   Tx   Rx   Sh    Status', line):
				startProcessing = True
				continue

			if not startProcessing:
				continue

			if re.search ("WHART NETWORK MANAGER VERSION:", line):

				startProcessing = False
				dubiousEntries = __ProcessEntries(linesBuffer, skipAdvertiseLinks_, channelNoMustAlsoMatch_)

				if dubiousEntries:
					print "Before ", line[:18], " time stamp, the following entries were found in " + fileName_
					print " Address     Peer   Handler  SId   Pp  Six  Tix   Ch   Lt   Tx   Rx   Sh    Status"
					for entry in dubiousEntries:
						print entry
					print ""

				# reset entries
				linesBuffer = []
				continue

			if not re.search ("\d+\s+\d+\s+\d+\s+\d+\s+\d+\s+\d+\s+", line):
				continue

			linesBuffer.append(line)

		if len(linesBuffer) > 0:

			dubiousEntries = __ProcessEntries(linesBuffer, skipAdvertiseLinks_, channelNoMustAlsoMatch_)

			if dubiousEntries:
				print "Before the end of file, the following entries were found in " + fileName_
				print " Address     Peer   Handler  SId   Pp  Six  Tix   Ch   Lt   Tx   Rx   Sh    Status"
				for entry in dubiousEntries:
					print entry
				print ""


######################
# main point of entry
######################

def SearchLinkEngineLogs(logsFolder_, skipAdvertiseLinks_, channelNoMustAlsoMatch_):

	if not os.path.exists(logsFolder_):
		print "Input folder does not exist. Exit."
		exit(1)

	if os.path.isdir(logsFolder_) == False:
		print "Script takes as input a folder name. Exit."
		exit(1)	

	files = []

	#logs file may be ordered in two ways
	# 1. in the reversed order of creation if log file names look like nm.log, nm.log.1, nm.log.2 etc	
	# 2. in the order of creation if log file names look like: nm.log.1_2010_11_27_09_04_52
	logsOrderIsReversed = True
	
	for file in os.listdir(logsFolder_):
		if file[:16] != 'nmLinkEngine.log': continue
		if logsOrderIsReversed and re.search( r'\d\d\d\d_\d\d_\d\d_\d\d_\d\d_\d\d', file) is not None: 
			logsOrderIsReversed = False
		files.append(file)

	files.sort()
	if logsOrderIsReversed:
		files.reverse()

	for fileName in files:
		__ProcessLogFile(logsFolder_, fileName,  skipAdvertiseLinks_, channelNoMustAlsoMatch_)


