#!/usr/bin/python
#
# example script for searching for entries in nmLinkEngine.log files 
# for entries with duplicate Six+Tix; Requesting channel match is optional.
# also list entries with tx or rx >= 2
#

from ListIrregularTxRxEntries import SearchLinkEngineLogs

#
# parameters to edit:
#

# set this True in order to skip discovery links
skipAdvertiseLinks = True

# if set True, print out entries if Six Tix AND Channel are identical
channelNoMustAlsoMatch = True

# log file location
logsFolderName = "/tmp"



# run the script
SearchLinkEngineLogs(logsFolderName, skipAdvertiseLinks, channelNoMustAlsoMatch)


