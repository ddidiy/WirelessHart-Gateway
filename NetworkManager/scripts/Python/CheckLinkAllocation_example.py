#!/usr/bin/python
#
# Example script for checking link allocation by comparing information using nm.log, nmOperations.log and nmLinkEngine.log:
#
# (1) the script starts searching the nm.log files in the order of their creation
# (2) for each reported difference between channelTimeslotAllocations and links, extract the respective operation id and timestamp
# (3) using the timestamp and op id it searches the operation entry on the nmOperations logs and extracts:
# 	(owner, peer, handler, sfID, pP, sIx, tIx, ch, tx, rx, sh)
# (4) using the above info, searches nmLinkEngine.log files in the order of their creation, finds when the link and the reciprocal link was created;  
#   If the link is later removed it checks if the corresponding entry is competely removed from the link table entries  before the end of logs as 
#   we expect it should be. THE PROBLEM this script is trying to detect is cases when a link once removed may have the corresponding table entries 
#   lingering until te program terminates.

from CheckLinkAllocation import ReviewLinkAllocation

# to edit
logsFolder = "/data/Logs/tnight/"

# run the script
ReviewLinkAllocation(logsFolder)
