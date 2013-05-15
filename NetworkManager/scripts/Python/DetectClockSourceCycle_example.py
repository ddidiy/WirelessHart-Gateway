#!/usr/bin/python
#
# example script used to detect clock source cycles in topology logs
#

from DetectClockSourceCycle import ProcessLogs

# edit these:
logsFolder = "/data/Logs/05-08-11-2010/"
primaryOnly = False

ProcessLogs(logsFolder, primaryOnly)



