#!/usr/bin/python
#
# example script to detect errors in clock source allocation and deallocation
#

from CheckClockSourceAllocDealloc import ProcessLogs

# edit this:
logsFolder = "/data/Logs/tuesday"
printMsgAlreadAlloc = True
printMsgAlreadyDealloc = True

ProcessLogs(logsFolder, printMsgAlreadAlloc, printMsgAlreadyDealloc)



