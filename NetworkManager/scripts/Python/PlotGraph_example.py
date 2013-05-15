#!/usr/bin/python
#
# example script for plotting graphs from nmNetworkTopology log files
#

import sys
from PlotGraph import *

# input can be a log file name or a directory containing logs
# .. using a directory as input will be only slightly slower
logsFolder = '/data/Logs/778'
timeStamp = '11-17 15:54:02,972'
graphId = '19C'
# application to view plots: eye of gnome
imageViewerApp = 'eog' 

# if using command line arguments
if len(sys.argv) == 3:
	timeStamp = sys.argv[1] 
	graphId = sys.argv[2]

# if graphId is omitted all graphs with required timestamp will be displayed
PlotGraph(imageViewerApp, logsFolder, timeStamp, graphId) 



