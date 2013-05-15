#!/usr/bin/python
#
# Used to detect Clok Source Cycles in nmNetworkTopology.log files
# complaints: radu marginean
#

import re
import os
import string

#
# retrieves previous node from the clock source path
#
def HopBack(source_, path_, clockPath_, time_, file_, primaryOnly_):
		
	for (node, sources) in clockPath_.iteritems():
		
		if node == source_:

			for source in sources:

				if source in path_:
					if source != '0':
						path_.append(source)
						print "Clock cycle detected: ", file_
						print "Timestamp: ", time_
						print "Path: ", path_
						print "\n"
				else:
					if source != '1':
						path_.append(source)
						HopBack(source, path_[:], clockPath_, time_, file_, primaryOnly_)
						path_.pop()

				if primaryOnly_ == True:
					break;
				 

#
# check clock paths for cycles
#
def EvaluatePath(clockPath_, time_, file_, primaryOnly_):
	
	for (node, sources) in clockPath_.iteritems():
		
		if node == '1' or node == '0':
			continue 

		for source in sources:
			path = [node, source]
			HopBack(source, path[:], clockPath_, time_, file_, primaryOnly_)
			if primaryOnly_ == True:
				break;
	
#
# process single log file, line by line:
# 
def ProcessLogFile (logFile_, primaryOnly_):

	log = open(logFile_, "r")

	time = ''
	clockPath = {}

	for line in log:

		primaryClockSource = ''
		secondaryClockSource = ''
		nodeAddress = ''

		if re.search("WHART NETWORK MANAGER VERSION", line) is not None:
			
			# check for clock source cycles
			if len(clockPath.keys ()) > 0:
				EvaluatePath(clockPath, time, logFile_, primaryOnly_)
				# reset clock source paths
				clockPath.clear ()

			indx = line.find(',')
 			if indx > -1:
				time = line[:indx + 4]
			else:
				print "Unexpected log line format: \n", line
				exit (1)
			continue

		# filter out lines not sarting with: nodeAddress=
		if re.search("nodeAddress=", line) is None:
			continue
	
		# extract node addrees
		indx = line.find('nodeAddress=')
		if indx > -1:
			line = line[indx + 12:]
			items = string.split(line, ',')
			nodeAddress = items[0].lstrip().rstrip()
		else:
			print "Unexpected log line format: \n", line
			exit (1)

		# extract primary clock source address
		indx = line.find('inP=')
		if indx > -1:
			line = line[indx + 4:]
			items = string.split(line, ',')
			primaryClockSource = items[0].lstrip().rstrip()
		else:
			print "Unexpected log line format: \n", line
			exit (1)

		# extract secondary clock source address
		indx = line.find('secondaryClkSrc=')
		if indx > -1:
			line = line[indx + 16:]
			items = string.split(line, ',')
			secondaryClockSource = items[0].lstrip().rstrip()
		else:
			print "Unexpected log line format: \n", line
			exit (1)
		
		# add clock to paths
		clockPath[nodeAddress] = (primaryClockSource, secondaryClockSource)
				

	# check for clock source cycles
	if len(clockPath.keys ()) > 0:
		EvaluatePath(clockPath, time, logFile_, primaryOnly_)

	log.close()
	
#
# returns list of nmNetworkTopology.log files in a directory
#
def GetLogsFromFolder(dir_):
	
	matchingFiles = []
	if not os.path.exists(dir_):
		return matchingFiles

	files = os.listdir(dir_)
	for file in files:
		if file == '.' or file == '..': continue
		if file[:21] != 'nmNetworkTopology.log': continue
		matchingFiles.append(dir_ + '/' + file)

	return matchingFiles

#########################	
# Main point of entry
#########################

def ProcessLogs(logsFolder_, primaryOnly_):

	logs = GetLogsFromFolder(logsFolder_)

	for log in logs:
		ProcessLogFile(log, primaryOnly_)


