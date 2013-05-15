#!/usr/bin/python
#
# checks link allocation by collecting information from nm.log, nmOperations.log and nmLinkEngine.log
# complaints: radu marginean
#
# (1) the script starts searching the nm.log files in the order of their creation
# (2) for each reported difference between channelTimeslotAllocations and links, extract the respective operation id and timestamp
# (3) using the timestamp and op id it searches the operation entry on the nmOperations logs and extracts:
# 	(owner, peer, handler, sfID, pP, sIx, tIx, ch, tx, rx, sh)
# (4) using the above info, searches nmLinkEngine.log files in the order of their creation, finds when the link and the reciprocal link was created;  
#   If the link is later removed it checks if the corresponding entry is competely removed from the link table entries  before the end of logs as 
#   we expect it should be. THE PROBLEM this script is trying to detect is cases when a link once removed may have the corresponding table entries 
#   lingering until te program terminates.


import os
import re
import string
import sys

# last opened file
currentOperationsLogFile = ''

def _getTimeDifference(ts2_, ts1_):

	"returns time difference ts2 - ts1 in seconds"

	ts2 = ts2_
	ts1 = ts1_
		
	# remove leading whitespaces if any 
	ts1_ = ts1_.lstrip()
	ts2_ = ts2_.lstrip()

	# get rid of miliseconds
	if len(ts1_) > 14: ts1_ = ts1_[:14]
	if len(ts2_) > 14: ts2_ = ts2_[:14]

	if ts2_ == ts1_: return 0

	indx2 = ts2_.find('-')
 	if indx2 == -1: 
		print sys._getframe().f_code.co_name, "():", ts2, ' ', ts1
		print "Unexpected format. Exit (1)."
		exit(1)

	month2 = int(ts2_[:indx2])
	ts2_ = ts2_[indx2 + 1:]

	indx1 = ts1_.find('-')
 	if indx1 == -1: 
		print sys._getframe().f_code.co_name, "():", ts2, ' ', ts1
		print "Unexpected format. Exit (2)."
		exit(1)

	month1 = int(ts1_[:indx1])
	ts1_ = ts1_[indx1 + 1:]

	indx2 = ts2_.find(' ')
 	if indx2 == -1: 
		print sys._getframe().f_code.co_name, "():", ts2, ' ', ts1
		print "Unexpected format. Exit (3)."
		exit(1)

	day2 = int(ts2_[:indx2])
	ts2_ = ts2_[indx2 + 1:]

	indx1 = ts1_.find(' ')
 	if indx1 == -1: 
		print sys._getframe().f_code.co_name, "():", ts2, ' ', ts1
		print "Unexpected format. Exit (4)."
		exit(1)

	day1 = int(ts1_[:indx1])
	ts1_ = ts1_[indx1 + 1:]

	indx2 = ts2_.find(':')
 	if indx2 == -1: 
		print sys._getframe().f_code.co_name, "():", ts2, ' ', ts1
		print "Unexpected format. Exit (5)."
		exit(1)

	hour2 = int(ts2_[:indx2])
	ts2_ = ts2_[indx2 + 1:]

	indx1 = ts1_.find(':')
 	if indx1 == -1: 
		print sys._getframe().f_code.co_name, "():", ts2, ' ', ts1
		print "Unexpected format. Exit (6)."
		exit(1)

	hour1 = int(ts1_[:indx1])
	ts1_ = ts1_[indx1 + 1:]

	indx2 = ts2_.find(':')
 	if indx2 == -1: 
		print sys._getframe().f_code.co_name, "():", ts2, ' ', ts1
		print "Unexpected format. Exit (7)."
		exit(1)

	min2 = int(ts2_[:indx2])
	ts2_ = ts2_[indx2 + 1:]

	indx1 = ts1_.find(':')
 	if indx1 == -1: 
		print sys._getframe().f_code.co_name, "():", ts2, ' ', ts1
		print "Unexpected format. Exit (8)."
		exit(1)

	min1 = int(ts1_[:indx1])
	ts1_ = ts1_[indx1 + 1:]

	sec1 = int(ts1_)
	sec2 = int(ts2_)

	time1 = 0
	time2 = 0

	if month2 == 1:
		time2 += 0
	elif month2 == 2:
		time2 += 31 * 24 * 3600
	elif month2 == 3:
		time2 += (31 + 28)  * 24 * 3600
	elif month2 == 4:
		time2 += (31 + 28 + 31)  * 24 * 3600
	elif month2 == 5:
		time2 += (31 + 28 + 31 + 30)  * 24 * 3600
	elif month2 == 6:
		time2 += (31 + 28 + 31 + 30 + 31)  * 24 * 3600
	elif month2 == 7:
		time2 += (31 + 28 + 31 + 30 + 31 + 30)  * 24 * 3600
	elif month2 == 8:
		time2 += (31 + 28 + 31 + 30 + 31 + 30 + 31)  * 24 * 3600
	elif month2 == 9:
		time2 += (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31)  * 24 * 3600
	elif month2 == 10:
		time2 += (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30)  * 24 * 3600
	elif month2 == 11:
		time2 += (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31)  * 24 * 3600
	else:
		time2 += (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30)  * 24 * 3600

	if month1 == 1:
		time1 += 0
	elif month1 == 2:
		time1 += 31 * 24 * 3600
	elif month1 == 3:
		time1 += (31 + 28)  * 24 * 3600
	elif month1 == 4:
		time1 += (31 + 28 + 31)  * 24 * 3600
	elif month1 == 5:
		time1 += (31 + 28 + 31 + 30)  * 24 * 3600
	elif month1 == 6:
		time1 += (31 + 28 + 31 + 30 + 31)  * 24 * 3600
	elif month1 == 7:
		time1 += (31 + 28 + 31 + 30 + 31 + 30)  * 24 * 3600
	elif month1 == 8:
		time1 += (31 + 28 + 31 + 30 + 31 + 30 + 31)  * 24 * 3600
	elif month1 == 9:
		time1 += (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31)  * 24 * 3600
	elif month1 == 10:
		time1 += (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30)  * 24 * 3600
	elif month1 == 11:
		time1 += (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31)  * 24 * 3600
	else:
		time1 += (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30)  * 24 * 3600	


	time2 += day2 * 24 * 3600
	time1 += day1 * 24 * 3600
	time2 += hour2 * 3600
	time1 += hour1 * 3600
	time2 += min2 * 60
	time1 += min1 * 60
	time2 += sec2
	time1 += sec1
 
	return time2 - time1
	

#
# searches nmLinkEngine.log for the required link and reports: time status new,
# time status active, time status deleted, time gone for the direct and reciprocal link
#
def _Get_LinkReport_From_LinkEngineLog (logsFolder_, opTimeStamp_, owner_, peer_, handler_, sfID_, pP_, sIx_, tIx_, ch_, tx_, rx_, sh_):
	
	if not os.path.exists(logsFolder_):
		print "Input folder does not exist. Exit."
		exit(1)

	if os.path.isdir(logsFolder_) == False:
		print "Script takes as input a folder name. Exit."
		exit(1)	
	
	fileList = []

	#logs file may be ordered in two ways
	# 1. in the reversed order of creation if log file names look like nm.log, nm.log.1, nm.log.2 etc	
	# 2. in the order of creation if log file names look like: nm.log.1_2010_11_27_09_04_52
	logsOrderIsReversed = True
	
	for file in os.listdir(logsFolder_):
		if file == '.' or file == '..': continue
		if file[:16] != 'nmLinkEngine.log': continue
		if logsOrderIsReversed and re.search( r'\d\d\d\d_\d\d_\d\d_\d\d_\d\d_\d\d', file) is not None: 
			logsOrderIsReversed = False
		fileList.append(file)

	fileList.sort()
	if logsOrderIsReversed:

		tmp = {}

		for file in fileList:
			indx = file.find('.log')
 			if indx == -1:
				print "Unexpected log file name format. Exit."
				exit(1)
			if indx + 4 == len(file):
				tmp[0] = file
			else:
				tmp[int(file[indx+5:])] = file

		tmp.keys().sort()
		fileList = [tmp[key] for key in tmp.keys()]
		fileList.reverse()

	
	# set various variables
	linkReport = ''
	reciprocalLinkReport = ''
	lastStatus = 0
	lastReciprocalStatus = 0
	lastTimeStamp = ''
	myDirectCounter = 0
	myReciprocalCounter = 0
	directLinkComplete = False
	reciprocalLinkComplete = False

	for file in fileList:
		
		logFile = open(logsFolder_ + '/' + file, "r")
		timeStamp = ''
				
		for line in logFile:
			
			if re.search('WHART NETWORK MANAGER VERSION:', line) is not None:
				timestamp = line[:18]
				myDirectCounter += 1
				myReciprocalCounter += 1
				continue

			# one entry has passed without finding the link and the reciprocal link
			if directLinkComplete == False and myDirectCounter == 2 and len(linkReport) > 0:
				linkReport += lastTimeStamp + ' - NO LONGER PRESENT!' + '\n'
				directLinkComplete = True

			if reciprocalLinkComplete == False and myReciprocalCounter == 2 and len(reciprocalLinkReport) > 0:
				reciprocalLinkReport += lastTimeStamp + ' - NO LONGER PRESENT!' + '\n'
				reciprocalLinkComplete = True
				
			if directLinkComplete and (reciprocalLinkComplete or len(reciprocalLinkReport) == 0):
				logFile.close ()
				return (linkReport, reciprocalLinkReport)

			# continue with processing
			lastTimeStamp = timestamp				

			if line[0] != ' ': continue

			if re.search('Address', line) is not None: continue

			items = []
			line = re.sub('\n','', line)

			for item in string.split(line, ' '):
				if len(item) > 0: 
					items.append(item)

			if len(items) < 13: continue

			(owner, peer, handler, sfID, pP, sIx, tIx, ch, lt, tx, rx, sh, status) = \
			(items[0], items[1], items[2], items[3], items[4], items[5], items[6], items[7], items[8], items[9], items[10], items[11], items[12])

			if (owner_, peer_, handler_, sfID_, pP_, sIx_, tIx_, ch_, tx_, rx_, sh_) == (owner, peer, handler, sfID, pP, sIx, tIx, ch, tx, rx, sh):

				myDirectCounter = 0
				if len(linkReport) == 0:

					linkReport = 'Link entries from ' + file + ':\n'
					linkReport += '        		   Address     Peer   Handler  SId   Pp  Six  Tix   Ch   Lt   Tx   Rx   Sh    Status\n'
					linkReport += timestamp + line
					if int(status) == 3:
						linkReport += ' - NEW!'
					elif int(status) == 4:
						linkReport += ' - ACTIVE!'
					elif int(status) == 9:
						linkReport += ' - DELETED!'
					else:
						linkReport += ' - UNKNOWN STATUS: ' + status
					linkReport += '\n'
					lastStatus = int(status)
					continue

				elif int(status) != lastStatus:

					linkReport += timestamp + line
					if int(status) == 3:
						linkReport += ' - NEW!'
					elif int(status) == 4:
						linkReport += ' - ACTIVE!'
					elif int(status) == 9:
						linkReport += ' - DELETED!'
					else:
						linkReport += ' - UNKNOWN STATUS: ' + status
					linkReport += '\n'
					lastStatus = int(status)

			if (owner_, peer_, handler_, sfID_, pP_, sIx_, tIx_, ch_, tx_, rx_, sh_) == (peer, owner, handler, sfID, pP, sIx, tIx, ch, rx, tx, sh):

				myReciprocalCounter = 0
				if len(reciprocalLinkReport) == 0:

					reciprocalLinkReport = 'Reciprocal link entries from ' + file + ':\n'
					reciprocalLinkReport += '        		   Address     Peer   Handler  SId   Pp  Six  Tix   Ch   Lt   Tx   Rx   Sh    Status\n'
					reciprocalLinkReport += timestamp + line
					if int(status) == 3:
						reciprocalLinkReport += ' - NEW!'
					elif int(status) == 4:
						reciprocalLinkReport += ' - ACTIVE!'
					elif int(status) == 9:
						linkReport += ' - DELETED!'
					else:
						linkReport += ' - UNKNOWN STATUS: ' + status
					reciprocalLinkReport += '\n'
					lastReciprocalStatus = int(status)
					continue

				elif int(status) != lastReciprocalStatus:

					reciprocalLinkReport += timestamp + line
					if int(status) == 3:
						reciprocalLinkReport += ' - NEW!'
					elif int(status) == 4:
						reciprocalLinkReport += ' - ACTIVE!'
					elif int(status) == 9:
						reciprocalLinkReport += ' - DELETED!'
					elif int(status) == 9:
						linkReport += ' - DELETED!'
					else:
						linkReport += ' - UNKNOWN STATUS: ' + status
					reciprocalLinkReport += '\n'
					lastReciprocalStatus = int(status)
					
		logFile.close ()

	if len(linkReport) > 0 and directLinkComplete == False:
		linkReport += 'ENTRY STILL PRESENT AT THE END OF LOGS'
		if (lastStatus == 9): linkReport += "!!!\n"
		else: linkReport += "\n"

	if len(reciprocalLinkReport) > 0 and reciprocalLinkComplete == False:
		reciprocalLinkReport += 'ENTRY STILL PRESENT AT THE END OF LOGS'
		if (lastReciprocalStatus == 9): reciprocalLinkReport += "!!!\n"
		else: reciprocalLinkReport += "\n"

	return (linkReport, reciprocalLinkReport)

#
# find a requested operation on the nmOperations logs
# extract operation entry, verify if op was confirmed
#
def _Find_OpId_In_OperationsLog(opId_, opTimeStamp_, logsFolder_):
	
	global currentOperationsLogFile

	if not os.path.exists(logsFolder_):
		print "Input folder does not exist. Exit."
		exit(1)

	if os.path.isdir(logsFolder_) == False:
		print "Script takes as input a folder name. Exit."
		exit(1)	

	fileList = []

	#logs file may be ordered in two ways
	# 1. in the reversed order of creation if log file names look like nm.log, nm.log.1, nm.log.2 etc	
	# 2. in the order of creation if log file names look like: nm.log.1_2010_11_27_09_04_52
	logsOrderIsReversed = True
	
	for file in os.listdir(logsFolder_):
		if file == '.' or file == '..': continue
		if file[:16] != 'nmOperations.log': continue
		if logsOrderIsReversed and re.search( r'\d\d\d\d_\d\d_\d\d_\d\d_\d\d_\d\d', file) is not None: 
			logsOrderIsReversed = False
		fileList.append(file)


	fileList.sort()
	if logsOrderIsReversed:

		tmp = {}

		for file in fileList:
			indx = file.find('.log')
 			if indx == -1:
				print "Unexpected log file name format. Exit."
				exit(1)
			if indx + 4 == len(file):
				tmp[0] = file
			else:
				tmp[int(file[indx+5:])] = file

		tmp.keys().sort()
		fileList = [tmp[key] for key in tmp.keys()]
		fileList.reverse()

	# attempt to start reading with the latest opened file
	shortFileList = []
	for file in fileList:
		if len(shortFileList) == 0 and len(currentOperationsLogFile) != 0 and file != currentOperationsLogFile:
			continue
		shortFileList.append(file)
 	
	match = ''
	if len(str(opId_)) == 1:
		match = "{id=       " + str(opId_)
	elif len(str(opId_)) == 2:
		match = "{id=      " + str(opId_)
	elif len(str(opId_)) == 3:
		match = "{id=     " + str(opId_)
	elif len(str(opId_)) == 4:
		match = "{id=    " + str(opId_)
	elif len(str(opId_)) == 5:
		match = "{id=   " + str(opId_)
	elif len(str(opId_)) == 6:
		match = "{id=  " + str(opId_)

	# this tuple will be returned
	(reason, (operationGenerated, genTime), (operationConfirmed, confTime), (owner, peer, handler, sfID, pP, sIx, tIx, ch, tx, rx, sh)) = \
	(None, (None, None), (None, None), (None, None, None, None, None, None, None, None, None, None, None))

	# used to find out if we sweeped the time interest region 
	timeRangeSweeped = 0		

	for file in shortFileList:
		
		try:
			logFile = open(logsFolder_ + '/' + file, "r")
			lastReason = ''

			for line in logFile:

				timeStamp = line[:14]
			
				if re.search("Reason:", line) is not None and re.search( r'\s\d\d[:]\d\d[:]\d\d[,]', line) is not None:
					
					currentTimeStamp = line[:14]

					if _getTimeDifference(opTimeStamp_, currentTimeStamp) > 3600:
						continue
					else: 
						timeRangeSweeped = 1

					if _getTimeDifference(currentTimeStamp, opTimeStamp_) > 3600:
						
						if timeRangeSweeped == 1: 
							timeRangeSweeped = 2 # the time interest region was sweeped
						raise

					lastReason = re.sub('\n', '', line)
								
				elif len(lastReason) > 0 and re.search(match, line) is not None:
					
					if operationGenerated is None and (re.search('state= G', line) or re.search('state=G', line)):

						currentOperationsLogFile = file

						reason = lastReason[20:]
						operationGenerated = re.sub('\n','', line)
						genTime = lastReason[:19]
			
						indx = line.find('own=')
 						if indx == -1: continue
						line = line[indx + 5:]
						indx = line.find(',')
						owner = re.sub('\s','', line[:indx])
						
						indx = line.find('handler=')
 						if indx == -1: continue
						line = line[indx + 8:]
						indx = line.find(',')
						handler = re.sub('\s','', line[:indx])

						indx = line.find('sfID:')
 						if indx == -1: continue
						line = line[indx + 5:]
						indx = line.find(',')
						sfID = re.sub('\s','', line[:indx])

						indx = line.find('pP:')
 						if indx == -1: continue
						line = line[indx + 3:]
						indx = line.find(',')
						pP = re.sub('\s','', line[:indx])

						indx = line.find('sIx:')
 						if indx == -1: continue
						line = line[indx + 4:]
						indx = line.find(',')
						sIx = re.sub('\s','', line[:indx])

						indx = line.find('tIx:')
 						if indx == -1: continue
						line = line[indx + 4:]
						indx = line.find(',')
						tIx = re.sub('\s','', line[:indx])

						indx = line.find('ch:')
 						if indx == -1: continue
						line = line[indx + 3:]
						indx = line.find(',')
						ch = re.sub('\s','', line[:indx])

						indx = line.find('peer:')
 						if indx == -1: continue
						line = line[indx + 5:]
						indx = line.find(',')
						peer = re.sub('\s','', line[:indx])

						indx = line.find('Tx:')
 						if indx == -1: continue
						line = line[indx + 3:]
						indx = line.find(',')
						tx = re.sub('\s','', line[:indx])
					
						indx = line.find('Rx:')
 						if indx == -1: continue
						line = line[indx + 3:]
						indx = line.find(',')
						rx = re.sub('\s','', line[:indx])

						indx = line.find('Sh:')
 						if indx == -1: continue
						line = line[indx + 3:]
						indx = line.find(',')
						sh = re.sub('\s','', line[:indx])
											
					elif re.search('state= C', line) or re.search('state=C', line):

						operationConfirmed = re.sub('\n','', line)
						confTime = lastReason[:19]
						raise

				continue							
		except:
			pass
		finally:
			logFile.close ()
			pass

	
	if operationGenerated is not None or timeRangeSweeped == 2:
		return (reason, (operationGenerated, genTime), (operationConfirmed, confTime), (owner, peer, handler, sfID, pP, sIx, tIx, ch, tx, rx, sh))

	# Something went wrong. Could not find requested entry.
	# Loop again this time over all files in the logs folder
	#print "Searching for operation id. Starting from scratch .."

	# reset tuple to be returned
	(reason, (operationGenerated, genTime), (operationConfirmed, confTime), (owner, peer, handler, sfID, pP, sIx, tIx, ch, tx, rx, sh)) = \
	(None, (None, None), (None, None), (None, None, None, None, None, None, None, None, None, None, None))

	for file in fileList:

		try:
			logFile = open(logsFolder_ + '/' + file, "r")
			
			lastReason = ''
			for line in logFile:
			
				if re.search("Reason:", line) is not None  and re.search( r'\s\d\d[:]\d\d[:]\d\d[,]', line) is not None:

					currentTimeStamp = line[:14]

					if _getTimeDifference(opTimeStamp_, currentTimeStamp) > 3600:
						continue

					if _getTimeDifference(currentTimeStamp, opTimeStamp_) > 3600:
						raise

					lastReason = re.sub('\n', '', line)
													
				elif len(lastReason) > 0 and re.search(match, line) is not None:
				
					if operationGenerated is None and (re.search('state= G', line) or re.search('state=G', line)):

						currentOperationsLogFile = file

						reason = lastReason[20:]
						operationGenerated = re.sub('\n','', line)
						genTime = lastReason[:19]

						indx = line.find('own=')
 						if indx == -1: continue
						line = line[indx + 5:]
						indx = line.find(',')
						owner = re.sub('\s','', line[:indx])
						
						indx = line.find('handler=')
 						if indx == -1: continue
						line = line[indx + 8:]
						indx = line.find(',')
						handler = re.sub('\s','', line[:indx])

						indx = line.find('sfID:')
 						if indx == -1: continue
						line = line[indx + 5:]
						indx = line.find(',')
						sfID = re.sub('\s','', line[:indx])

						indx = line.find('pP:')
 						if indx == -1: continue
						line = line[indx + 3:]
						indx = line.find(',')
						pP = re.sub('\s','', line[:indx])

						indx = line.find('sIx:')
 						if indx == -1: continue
						line = line[indx + 4:]
						indx = line.find(',')
						sIx = re.sub('\s','', line[:indx])

						indx = line.find('tIx:')
 						if indx == -1: continue
						line = line[indx + 4:]
						indx = line.find(',')
						tIx = re.sub('\s','', line[:indx])

						indx = line.find('ch:')
 						if indx == -1: continue
						line = line[indx + 3:]
						indx = line.find(',')
						ch = re.sub('\s','', line[:indx])

						indx = line.find('peer:')
 						if indx == -1: continue
						line = line[indx + 5:]
						indx = line.find(',')
						peer = re.sub('\s','', line[:indx])

						indx = line.find('Tx:')
 						if indx == -1: continue
						line = line[indx + 3:]
						indx = line.find(',')
						tx = re.sub('\s','', line[:indx])
					
						indx = line.find('Rx:')
 						if indx == -1: continue
						line = line[indx + 3:]
						indx = line.find(',')
						rx = re.sub('\s','', line[:indx])

						indx = line.find('Sh:')
 						if indx == -1: continue
						line = line[indx + 3:]
						indx = line.find(',')
						sh = re.sub('\s','', line[:indx])

					elif re.search('state= C', line) or re.search('state=C', line):

						operationConfirmed = re.sub('\n','', line)
						confTime = lastReason[:19]
						raise

				continue						
		except:
			pass
		finally:
			logFile.close ()

	return (reason, (operationGenerated, genTime), (operationConfirmed, confTime), (owner, peer, handler, sfID, pP, sIx, tIx, ch, tx, rx, sh))


#
# Process one nm.log file, search for each detected difference between channelTimeslotAllocations and links
#
def _ProcessNMLogFile(logsFolder_, file_):

	logFile = open(logsFolder_ + '/' + file_, "r")

	lines = ''
	keepAdding = False
	channel = 1
	defects = 0
	noLines = 0
	opId = 0
	summary = ''
	newDiff = ''
	oldDiff = ''
	opTimeStamp = ''

	for line in logFile:

		# start logging out lines: match Attempting to confirm opeation id=
		if re.search("Attempting to confirm op", line) is not None:

			opTimeStamp = line[:14]

			# printed summary starts with this line
			summary = '\n\n' + file_ + ' fragment:'
			summary += '\n' + re.sub('\n','', line)
			keepAdding = True

			# extract operation id
			indx = line.find('opeation id=')

			if indx == -1: 
				print "Unexepected log format:"
				print sys._getframe().f_code.co_name, "():"
				exit(1)

			opId =  re.sub('\s','', line[indx + 12:])
			continue

		# add line to printout
		if keepAdding:
			summary += '\n' + re.sub('\n','', line)
		
		# stop adding nm.log lines to printout	
		if re.search(": Channel 0", line) is not None:

			keepAdding = False
			continue

		# if line indicates new channel
		if re.search("Channel ", line):

			if re.search("Channel 0", line):
				channel = 1
			elif re.search("Channel 1\n", line):
				channel = 2
			elif re.search("Channel 2", line):
				channel = 3
			elif re.search("Channel 3", line):
				channel = 4
			elif re.search("Channel 4", line):
				channel = 5
			elif re.search("Channel 5", line):
				channel = 6
			elif re.search("Channel 6", line):
				channel = 7
			elif re.search("Channel 7", line):
				channel = 8
			elif re.search("Channel 8", line):
				channel = 9
			elif re.search("Channel 9", line):
				channel = 10
			elif re.search("Channel 10", line):
				channel = 11
			elif re.search("Channel 11", line):
				channel = 12
			elif re.search("Channel 12", line):
				channel = 13
			elif re.search("Channel 13", line):
				channel = 14
			elif re.search("Channel 14", line):
				channel = 15
			elif re.search("Channel 15", line):
				channel = 16

			continue

		# if comparison line
		if re.search("    -    ", line):
	
			newDiff += line 
			if re.search('[\s][*]', line): defects += 1
			noLines += 1

			if noLines == 25:

				if defects > 0:
					summary += '\n' + str(defects) + " differences found in channel: " +  str(channel)

				defects = 0
				noLines = 0

				if channel == 16:

					#reset channel number
					channel = 0

					if (newDiff != oldDiff):

						(reason, (operationGenerated, genTime), (operationConfirmed, confTime), (owner, peer, handler, sfID, pP, sIx, tIx, ch, tx, rx, sh)) = _Find_OpId_In_OperationsLog(opId, opTimeStamp, logsFolder_)

						if reason is not None:
							(linkReport, reciprocalLinkReport) = _Get_LinkReport_From_LinkEngineLog(logsFolder_, opTimeStamp, owner, peer, handler, sfID, pP, sIx, tIx, ch, tx, rx, sh)

						print summary

						if reason is not None:
							print "\nOperation entry from nmOperations.log:"
							print reason
							print ' generated:', genTime
							print ' ', operationGenerated
							if confTime is not None:
								print ' confirmed:', confTime
								print ' ', operationConfirmed
							else:
								print "Operation is not confirmed !!"
							print '\n',linkReport
							print reciprocalLinkReport
						else:
							print "Operation entry not found in nmOperations.log files."

						oldDiff = newDiff
						
					newDiff = ''

								
	# close log	file
	logFile.close ()
	return None

#
# Main point of entry
#
def ReviewLinkAllocation(logsFolder_):

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
		if file == '.' or file == '..': continue
		if file[:6] != 'nm.log': continue
		if logsOrderIsReversed and re.search( r'\d\d\d\d_\d\d_\d\d_\d\d_\d\d_\d\d', file) is not None: 
			logsOrderIsReversed = False
		files.append(file)

	files.sort()
	if logsOrderIsReversed:

		tmp = {}

		for file in files:
			indx = file.find('.log')
 			if indx == -1:
				print "Unexpected log file name format. Exit."
				exit(1)
			if indx + 4 == len(file):
				tmp[0] = file
			else:
				tmp[int(file[indx+5:])] = file

		tmp.keys().sort()
		files = [tmp[key] for key in tmp.keys()]
		files.reverse()

	for file in files:
		_ProcessNMLogFile(logsFolder_, file)



