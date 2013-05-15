#!/usr/bin/python
#
# module for plotting graphs from nmNetworkTopology log files; so far only works on linux
# complaints: radu marginean
#

import re
import os
import string
import sys
import tempfile
import time

try:
    import yapgvb

except ImportError:

	if os.name == 'posix':
		print "You must first install graphviz and python binding for graphviz with:"
		print "sudo apt-get install graphviz"
		print "sudo apt-get install python-yapgvb"
	else:
		print os.name
		exit(1)
		print "You must first install graphviz and yapgvb python binding for graphviz from:" 
		print "http://www.graphviz.org/"
		print "http://code.google.com/p/yapgvb"
	
	exit(1)

_firstPlot = True

#
# plots graph
#
def _PlotAndDisplay(imageViewerApp_, graph_, timeStamp_):

	global _firstPlot

	if _which(imageViewerApp_) == None:
		print 'Image rendering application:', '"' + imageViewerApp_ + '"', 'does not exist on your machine.' 
		print 'Change application name from your master script.'
		exit(1)

	(graphId, (source, destination), (primaryClockSources, secondaryClockSources), path) = graph_

	if len(path) == 0:
		return None
	
	if source == 'F980' or source == 'F981':
		source = '1'

	if destination == 'F980' or destination == 'F981':
		destination = '1'

	# node that the clockSource will be provided for
	clockNode = source
	if source == '1':
		clockNode = destination

	pClckSource = ''
	sClckSource = ''
	if clockNode in  primaryClockSources.keys(): 
 		pClckSource = primaryClockSources[clockNode]
	if clockNode in secondaryClockSources.keys():
		sClckSource = secondaryClockSources[clockNode]	

	# ploting starts here
	graphviz_graph = yapgvb.Digraph()

	colors = "#000000"
	colord = "#000000"
	for (s, d) in path:

		if s == source:
			colors = yapgvb.colors.blue
		else:
			colors = "#000000"

		if d == destination:
			colord = yapgvb.colors.blue
		else:
			colord = "#000000"

		source_label = s
		if s == pClckSource:
			source_label = s + '\nPCS'
		elif s == sClckSource:
			source_label = s + '\nSCS'

		dest_label = d
		if d == pClckSource:
			dest_label = d + '\nPCS'
		elif d == sClckSource:
			dest_label = d + '\nSCS'

		source_node = graphviz_graph.add_node(s, label = source_label, shape = yapgvb.shapes.circle, color = colors, fontsize = 10)
		dest_node = graphviz_graph.add_node(d, label = dest_label, shape = yapgvb.shapes.circle, color = colord, fontsize = 10)
		source_node >> dest_node


	graphviz_graph.add_node("GraphId", label = 'Graph ID: ' + graphId + ', time stamp:\n' + timeStamp_, 
		shape = yapgvb.shapes.rectangle, color = yapgvb.colors.violet, fontsize = 10)

	
	#graphviz_graph.label = 'Graph ID: ' + graphId + ', time stamp:\n' + timeStamp_
	graphviz_graph.layout(yapgvb.engines.circo)
	
	# get a temp file
	tmpFile = tempfile.NamedTemporaryFile(prefix='graphviz', suffix='.png', delete='False')
	tmpFile.close()  

	# render plot
	graphviz_graph.render(tmpFile.name)

	# avoid crashes when rendering multiple plots
	if _firstPlot == True:
		_firstPlot = False
	else:
		time.sleep(2)

	# show image
	os.system(imageViewerApp_ + ' ' + tmpFile.name + ' &')

	return None	

#
# which unix command implemented in python
#	
def _which(program_):
    
    def is_exe(fpath):
        return os.path.exists(fpath) and os.access(fpath, os.X_OK)

    fpath, fname = os.path.split(program_)
    if fpath:
        if is_exe(program_):
            return program_
    else:
        for path in os.environ["PATH"].split(os.pathsep):
            exe_file = os.path.join(path, program_)
            if is_exe(exe_file):
                return exe_file

    return None

#
# identify graph using timestamp and graph Id
#
def _RetrieveGraphsFromFile (logFile_, timeStamp_, graphId_):

	log = open(logFile_, "r")
	
	graphs = []
	foundTimeStamp = False
	primaryClockSources = {}
	secondaryClockSources = {}
	
	for line in log:

		if re.search("WHART NETWORK MANAGER VERSION", line) is not None:
			
			if re.search(timeStamp_, line) is not None:
				foundTimeStamp = True
				primaryClockSources.clear()
				secondaryClockSources.clear()
				continue
			
			if foundTimeStamp == True:
				break

		if foundTimeStamp == False:
			continue

		if re.search(timeStamp_, line) is not None:
			continue
		
		if re.search("nodeAddress=", line) is not None:

			indx = line.find('nodeAddress=')
			line = line[indx + 12:]

			indx = line.find(',')
			if indx == -1:
				print sys._getframe().f_code.co_name, "():" 
				print "Unexpected log line format: \n", line
				exit (1)

			nodeAddr = re.sub(r"\s", "", line[:indx])
			line = line[indx+1:]
			
			indx = line.find('inP=')
			if indx == -1:
				print sys._getframe().f_code.co_name, "():" 
				print "Unexpected log line format: \n", line
				exit (1)

			line = line[indx + 4:]

			indx = line.find(',')
			if indx == -1:
				print sys._getframe().f_code.co_name, "():" 
				print "Unexpected log line format: \n", line
				exit (1)

			primaryClk = re.sub(r"\s", "", line[:indx])
			line = line[indx+1:]
	
			indx = line.find('secondaryClkSrc=')
			if indx == -1:
				print sys._getframe().f_code.co_name, "():" 
				print "Unexpected log line format: \n", line
				exit (1)

			line = line[indx + 16:]

			indx = line.find(',')
			if indx == -1:
				print sys._getframe().f_code.co_name, "():" 
				print "Unexpected log line format: \n", line
				exit (1)

			secondaryClk = re.sub(r"\s", "", line[:indx])

			primaryClockSources[nodeAddr] = primaryClk
			secondaryClockSources[nodeAddr] = secondaryClk
			continue
		
		if re.search("Path {graphId=", line) is None:
			continue
		
		indx1 = line.find('graphId=')
		if indx1 == -1:
			print sys._getframe().f_code.co_name, "():" 
			print "Unexpected log line format: \n", line
			exit (1)

		indx2 = line.find(',')
 		if indx2 == -1:
			print sys._getframe().f_code.co_name, "():" 
			print "Unexpected log line format: \n", line
			exit (1)

		graphId = line[indx1 + 8:indx2].lstrip().rstrip()

		if graphId_ != 0 and graphId_ != graphId:
			continue

		line = line[indx2 + 1:]

		indx = line.find(',')
		if indx == -1:
			print sys._getframe().f_code.co_name, "():" 
			print "Unexpected log line format: \n", line
			exit (1)

		tmp = line[:indx]
		indx = tmp.find('=>')

		if indx == -1:
			print sys._getframe().f_code.co_name, "():" 
			print "Unexpected log line format: \n", line
			exit (1)

		source = tmp[:indx].lstrip().rstrip()
		destination = tmp[indx + 2:].lstrip().rstrip()

		indx = line.find('graph=')

		if indx == -1:
			print sys._getframe().f_code.co_name, "():" 
			print "Unexpected log line format: \n", line
			exit (1)

		line = line[indx + 6:]
		line = line.replace(") (", ",")
		line = line.replace(")", "")
		line = line.replace("(", "")
		line = line.replace("}", "")
		line = line.replace("\n", "")
		line = re.sub(r"[:]\d+", "", line)
		line = re.sub(r"\s", "", line)
		nodes = string.split(line, ',')

		graph = []
		for i in range(0, len(nodes) - 1, 2):
			graph.append((nodes[i], nodes[i + 1]))

		graphs.append((graphId, (source, destination), (primaryClockSources, secondaryClockSources), graph[:]))
	
	log.close()
	return (graphs, foundTimeStamp)
	

#
# get the first timestamp from log file
#
def _GetFirstTimeStamp(logFile_):
	
	if re.search("nmNetworkTopology.log", logFile_) is None:
		print sys._getframe().f_code.co_name, "():" 
		print "Unexpected log file name. Exit."
		exit(1)

	log = open(logFile_, "r", 1)
	
	for line in log:

		if re.search("WHART NETWORK MANAGER VERSION", line) is not None:
			indx = line.find('WHART NETWORK MANAGER VERSION')
			timeStamp = line[:indx].lstrip().rstrip()
			log.close ()
			return timeStamp

	print sys._getframe().f_code.co_name, "():"
	print "Could not find required timestamp in log file: ", logFile_, " Exit."  
	log.close()
	exit(1)

#
# GetFileName
#
def GetFileNameContainingTimeStamp(path_, timeStamp_):

	files = []

	#logs file may be ordered in two ways
	# 1. in the reversed order of creation if log file names look like nm.log, nm.log.1, nm.log.2 etc	
	# 2. in the order of creation if log file names look like: nm.log.1_2010_11_27_09_04_52
	logsOrderIsReversed = True
	
	for file in os.listdir(path_):
		if file == '.' or file == '..': continue
		if file[:21] != 'nmNetworkTopology.log': continue
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

	if len(files) == 0:
		print "NetworkTopology plots missing from: ", path_
		exit(1)
	elif len(files) == 1:
		return files[0]

	while len(files) > 1:
		tmp = files.pop()
		initialTimeStamp = _GetFirstTimeStamp(path_ + '/' + tmp)
		if (timeStamp_ >= initialTimeStamp):
			return tmp
	
	return files[0]
 
#########################	
# Main point of entry
#########################

def PlotGraph(imageViewerApp_, path_, timeStamp_, graphId_ = 0):

	if not os.path.exists(path_):
		print "Input file or folder does not exist. Exit."
		exit(1)

	# delete old plots
	if os.name == 'posix':
		os.system('rm -f /tmp/graphviz*.png')
	else:
		if os.path.exists('C:/temp/'):
			os.system('del C:/temp/graphviz*.png')
		elif os.path.exists('C:/tmp/'):
			os.system('del C:/tmp/graphviz*.png')

	graphs = ()
	foundTimeStamp = False;

	if os.path.isdir(path_) == True:
		
		file = GetFileNameContainingTimeStamp(path_, timeStamp_)
		if file is not None:
			(graphs, foundTimeStamp) = _RetrieveGraphsFromFile(path_ + '/' + file, timeStamp_, graphId_)
		
	else:
		(graphs, foundTimeStamp) = _RetrieveGraphsFromFile(path_, timeStamp_, graphId_)

	for graph in graphs:
		
		if len(graph[3]) > 0:
			_PlotAndDisplay(imageViewerApp_, graph, timeStamp_)
		else:
			print "Graph", graph[0], "is empty."

	if len(graphs) == 0:
		if foundTimeStamp == False:
			print "Timestamp", timeStamp_, "not found."
		else:
			print "Graph Id",  graphId_, "not found."

	return None
