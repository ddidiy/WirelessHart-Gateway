#!/usr/bin/python
#
# python module for detecting unconfirmed operations from  
# nmOperations.log, packets and events in nm.log file
# complaints: radu marginean
#

import re
import os

_operations = {}
_packets = {}
_events = {}

def _PrintUnconfirmedOperations(nodeNickname_):

	global _operations

	tmpDict = {}

	for (id, own) in _operations.keys():
		(reason, line) = _operations[(id, own)]
		timestamp = reason[:18]
		tmpDict[(timestamp, own, id)] = (reason, line)

	keys = tmpDict.keys ()
	keys.sort()

	print "\nList of unconfirmed operations: \n"

	for (own, timestamp, id) in keys:
		(reason, line) = tmpDict[(own, timestamp, id)]
		if nodeNickname_ == '0' or nodeNickname_ == own:
			print "Unconfirmed operation:"
			print reason, line
			print ''

def _PrintUnconfirmedPackets():
	
	global _packets
	tmpDict = {}

	for handle in _packets.keys():
		line = _packets[handle]
		timestamp = line[:18]
		if timestamp in tmpDict.keys():
			tmpDict[timestamp] += line;
		else:
			tmpDict[timestamp] = line;

	timestamps = tmpDict.keys();
	timestamps.sort();

	if len(timestamps) > 0:
		print "Unconfirmed packets:"
		for timestamp in timestamps:
			print tmpDict[timestamp]
	else:
		print "No unconfirmed packets."
	
def _PrintUnconfirmedEvents():
	
	global _events
	tmpDict = {}

	for evtId in _events.keys():
		line = _events[evtId]
		timestamp = line[:18]
		if timestamp in tmpDict.keys():
			tmpDict[timestamp] += line;
		else:
			tmpDict[timestamp] = line;

	timestamps = tmpDict.keys();
	timestamps.sort();

	if len(timestamps) > 0:
		print "Unconfirmed events:"
		for timestamp in timestamps:
			print tmpDict[timestamp]
	else:
		print "No unconfirmed events."

def _CheckOperations(logFile_, verbose_, nodeNickname_):

	global _operations
	log = open(logFile_, "r")

	reason = ''
	lineString = ''

	for line in log:

		if re.search("Reason: Write session F981, parent=F980", line) is not None:
			
			print "Network manager was restarted at:", line[:19]
			_PrintUnconfirmedOperations(nodeNickname_)
			_operations.clear()
			continue
		
		if re.search("Reason:", line) is not None:
			reason = line;
			continue
		
		lineString = line;

		# extract id, owner and state
		indx = line.find('id=')
		if indx == -1:
			continue

		line = line[indx + 3:]

		indx = line.find(',')
		if indx == -1:
			continue

		id = re.sub(r"\s", "", line[:indx])
		line = line[indx + 1:]

		indx = line.find('own=')
		if indx == -1:
			continue
		
		line = line[indx + 4:]

		indx = line.find(',')
		if indx == -1:
			continue

		own = re.sub(r"\s", "", line[:indx])
		line = line[indx + 1:]

		indx = line.find('state=')
		if indx == -1:
			continue

		line = line[indx + 6:]
	
		indx = line.find(',')
		if indx == -1:
			continue
		
		state = re.sub(r"\s", "", line[:indx])
		
		#confirmed
		if state == 'C':
			if (id, own) in _operations.keys():
				del _operations[(id, own)]
			else:
				if verbose_:
					print "Operation confirmed without being listed as generated:"
					print reason, lineString
		elif state == 'G':
			_operations[(id, own)] = (reason, lineString)
		
	log.close()			

def _CheckPackets(logFile_, verbose_):

	global _packets
	log = open(logFile_, "r")

	for line in log:
		
		lineString = line

		if re.search("Start NetworkManager", line) is not None:
			
			print "\nNetwork Manager restart detected. Unconfirmed packets so far:"
			_PrintUnconfirmedPackets()
			_packets.clear()

			print "\nNew Network Manager session starts here:"
			print line
				
			continue

		if re.search("logSendPacket()", line) is not None:

			indx = line.find('handle=')
			if indx == -1:
				continue

			line = line[indx + 7:]

			indx = line.find(',')
			if indx == -1:
				continue

			handle = re.sub(r"\s", "", line[:indx])

			_packets[handle] = 	lineString
			continue

		if re.search("logConfPacket()", line) is not None:

			indx = line.find('handle=')
			if indx == -1:
				continue

			line = line[indx + 7:]

			indx = line.find(',')
			if indx == -1:
				continue

			handle = re.sub(r"\s", "", line[:indx])

			if handle in _packets.keys():
				del _packets[handle]
			else:
				if verbose_:
					print "Packet was confirmed without being listed as sent:"
					print lineString
				pass				

	log.close()			

def _CheckEvents(logFile_, verbose_):

	global _events
	log = open(logFile_, "r")
	
	for line in log:
		
		lineString = line

		if re.search("Start NetworkManager", line) is not None:

			print "\nNetwork Manager restart detected. Unconfirmed events so far:"
			_PrintUnconfirmedEvents()
			_events.clear()
			
			print "\nNew Network Manager session starts here:"
			print line
				
			continue
				
		if re.search("logNewEvent()", line) is not None:

			indx = line.find('evId=')
			if indx == -1:
				continue

			line = line[indx + 5:]

			indx = line.find(';')
			if indx == -1:
				continue

			evId = re.sub(r"\s", "", line[:indx])
			
			_events[evId] = lineString
			continue

		if re.search("logConfEvent()", line) is not None:

			indx = line.find('evId=')
			if indx == -1:
				continue

			line = line[indx + 5:]

			indx = line.find(';')
			if indx == -1:
				continue

			evId = re.sub(r"\s", "", line[:indx])
			
			if evId in _events.keys():
				del _events[evId]
			else:
				if verbose_:
					print "Event was confirmed without being listed as new:"
					print lineString
				
	log.close()

def ListUnconfirmedOperations(logsFolder_, verbose_, nodeNickname_ = '0'):
	
	if not os.path.exists(logsFolder_):
		print "Input folder does not exist. Exit."
		exit(1)

	if os.path.isdir(logsFolder_) == False:
		print "Script takes as input a folder name."
		exit(1)	
	
	files = []

	#logs file may be ordered in two ways
	# 1. in the reversed order of creation if log file names look like nm.log, nm.log.1, nm.log.2 etc	
	# 2. in the order of creation if log file names look like: nm.log.1_2010_11_27_09_04_52
	logsOrderIsReversed = True
	
	for file in os.listdir(logsFolder_):
		if file == '.' or file == '..': continue
		if file[:16] != 'nmOperations.log': continue
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
		_CheckOperations(logsFolder_ + '/' + file, verbose_, nodeNickname_)

	_PrintUnconfirmedOperations(nodeNickname_)

def ListUnconfirmedPacktes(logsFolder_, verbose_):

	if not os.path.exists(logsFolder_):
		print "Input folder does not exist. Exit."
		exit(1)

	if os.path.isdir(logsFolder_) == False:
		print "Script takes as input a folder name."
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
		_CheckPackets(logsFolder_ + '/' + file, verbose_)

	_PrintUnconfirmedPackets();


def ListUnconfirmedEvents(logsFolder_, verbose_):

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
		_CheckEvents(logsFolder_ + '/' + file, verbose_)

	_PrintUnconfirmedEvents()

