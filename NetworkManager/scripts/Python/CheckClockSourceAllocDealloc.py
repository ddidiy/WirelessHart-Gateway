#!/usr/bin/python
#
# Used to detect errors in clock source allocation and deallocation
# complaints: radu marginean
#

import re
import os
import string

#
# process nmNetworkTopology.log file: extract a list of active devices 
# 
def _ProcessNetworkTopologyLogFile (logFile_, activeNodes_):

	log = open(logFile_, "r")

	for line in log:

		nodeAddress = ''

		if re.search("WHART NETWORK MANAGER VERSION", line) is not None:
			
			activeNodes_.clear()
			continue

		# filter out lines not sarting with: nodeAddress=
		if re.search("nodeAddress=", line) is None:
			continue
	
		# extract node addrees
		indx = line.find('nodeAddress=')
		if indx > -1:
			line = line[indx + 12:]
			items = string.split(line, ',')
			nodeAddress = re.sub(r'\s', '', items[0])
			activeNodes_.add(nodeAddress)
		else:
			print "Unexpected log line format: \n", line
			exit (1)

	log.close()
		
#
# process Operations log file, line by line:
# 
def _ProcessOperationLogFile (logFile_, allocatedNodeSet_, allNodesSet_, printAlreadAlloc_, printAlreadyDealloc_):

	log = open(logFile_, "r")

	for line in log:

		myline = line

		if re.search('Set Clock Source', line) is None:
			continue

		indx = line.find('own=')
 		if indx == -1:
			continue

		line = line[indx + 4:]

		indx = line.find(',')
 		if indx == -1:
			continue

		owner = re.sub(r'\s', '', line[:indx])
		if owner == '1':
			continue

		indx = line.find('flags:')
 		if indx == -1:
			continue

		line = line[indx + 6:]

		indx = line.find(',')
 		if indx == -1:
			continue

		flag = re.sub(r'\s', '', line[:indx])

		indx = line.find('rc:')
 		if indx == -1:
			continue

		line = line[indx + 3:]

		indx = line.find('}')
 		if indx == -1:
			continue

		rc = re.sub(r'\s', '', line[:indx])

		if (rc != '0'):
			print "Clock source allocation error found in:", logFile_
			print myline

		# allocate clock source
		if flag == '1':
			
			if owner in allocatedNodeSet_:
				if printAlreadAlloc_ == True:
					print "Node already allocated in", logFile_ + ":"  
					print myline
			else:
				allocatedNodeSet_.add(owner)
				allNodesSet_.add(owner)

		# deallocate clock source
		elif flag == '0': 
			
			if owner not in allocatedNodeSet_:
				if printAlreadyDealloc_ == True:
					print "Node is not allocated", logFile_ + ":"
					print myline
			else:
				allocatedNodeSet_.remove(owner)
				allNodesSet_.add(owner)
		
		# allocation error		
		else:
			print "Print unexpected allocation flag in:", logFile_
			print myline						

	log.close()
	
#
# returns list of nmOperations.log files in a directory
#
def _GetLogsFromFolder(dir_):
	
	matchingFiles = []
	if not os.path.exists(dir_):
		return matchingFiles

	#logs file may be ordered in two ways
	# 1. in the reversed order of creation if log file names look like nm.log, nm.log.1, nm.log.2 etc	
	# 2. in the order of creation if log file names look like: nm.log.1_2010_11_27_09_04_52
	logsOrderIsReversed = True

	files = os.listdir(dir_)
	for file in files:
		if file == '.' or file == '..': continue
		if file[:16] != 'nmOperations.log': continue
		if logsOrderIsReversed and re.search( r'\d\d\d\d_\d\d_\d\d_\d\d_\d\d_\d\d', file) is not None: 
			logsOrderIsReversed = False
		matchingFiles.append(dir_ + '/' + file)

	matchingFiles.sort()
	if logsOrderIsReversed:

		tmp = {}

		for file in matchingFiles:
			indx = file.find('.log')
 			if indx == -1:
				print "Unexpected log file name format. Exit."
				exit(1)
			if indx + 4 == len(file):
				tmp[0] = file
			else:
				tmp[int(file[indx+5:])] = file

		tmp.keys().sort()
		matchingFiles = [tmp[key] for key in tmp.keys()]
		matchingFiles.reverse()


	return matchingFiles

#########################	
# Main point of entry
#########################

def ProcessLogs(logsFolder_, printAlreadAlloc_, printAlreadyDealloc_):

	# nodes with clock source allocated 
	allocatedNodeSet = set ()
	# nodes with clock source deallocated
	allNodesSet = set ()
	# active nodes as readen out fron nmNetworkTopology log
	activeNodes = set ()

	logs = _GetLogsFromFolder(logsFolder_)

	for log in logs:
		_ProcessOperationLogFile(log, allocatedNodeSet, allNodesSet, printAlreadAlloc_, printAlreadyDealloc_)

	_ProcessNetworkTopologyLogFile (logsFolder_ + '/nmNetworkTopology.log', activeNodes)

	print "\n Nodes allocated at the end:"
	print allocatedNodeSet

	print "\n Nodes not allocated at the end:"
	print allNodesSet - allocatedNodeSet

	print "\n Nodes active (found in nmNetworkTopology.log at the end) allocated:"
	print activeNodes & allocatedNodeSet

	print "\n Nodes active (found nmNetworkTopology.log at the end) not allocated:"
	print activeNodes & (allNodesSet - allocatedNodeSet)

	print "\n Nodes allocated not active:"
	print allocatedNodeSet & (allNodesSet - activeNodes)


