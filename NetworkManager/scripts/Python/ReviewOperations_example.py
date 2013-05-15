#!/usr/bin/python
#
# example script detecting unconfirmed operations from  
# nmOperations.log, unconfirmed packets and events in nm.log file
#

from ReviewOperations import *

#edit:
logsFolder = '/data/Logs/836'
nodeNickname = '625'
verbose = False

# list unconfirmed packets
#ListUnconfirmedPacktes(logsFolder, verbose)

# list unconfirmed events
#ListUnconfirmedEvents(logsFolder, verbose)

# list unconfirmed operations for the node with this nickname
#ListUnconfirmedOperations(logsFolder, verbose, nodeNickname)

# list unconfirmed operations for all nodes, ordered by owner nickname THAN timestamp
ListUnconfirmedOperations(logsFolder, verbose)


