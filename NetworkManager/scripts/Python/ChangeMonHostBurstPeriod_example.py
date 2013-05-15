#!/usr/bin/python
#
# example script to changes burst update period in Monitor_Host_Publishers conf files
#

from ChangeMonHostBurstPeriod import ChangeBurstPeriod

# edit these:
confFileName = "./Monitor_Host_Publishers.conf"
newBurstPeriod = 4

ChangeBurstPeriod (confFileName, newBurstPeriod)
