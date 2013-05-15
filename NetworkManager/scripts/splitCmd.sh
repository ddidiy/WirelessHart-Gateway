#/bin/bash

# Reads from the standard input lines representing cmd responses and tries to parse them.
# Ex : echo  0322080002020001F98001090002F981010900 | splitCmds.sh
# @author Radu Pop

if [ $# -eq 0 ] 
then 
	echo
fi

function f782 {
	string=$1;
	_sed='s/'
	_sed=$_sed'\([0-9A-Z]\{4\}\)'           # \1 CMD ID
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \2 ERR CODE
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \3 session index
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \4 no entries read
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \5 no active sessions
	_sed=$_sed'\([0-9A-Z]*\)'               # \6 sessions
	_sed=$_sed'/CMDID:\1\terror:\2\tIDX:\3\tEntriesRead:\4\tNoActiveSessions:\5\n\6/g'
	#echo $_sed
	
	_sed2='s/'
	_sed2=$_sed2'\([0-9A-Za-z]\{32\}\)/\1\n/g'
	
	_sed3='s/'
	_sed3=$_sed3'^\(.\{2\}\)\(.\{4\}\)\(.\{10\}\)\(.\{8\}\)\(.\{8\}\)'
	_sed3=$_sed3'/sessionType:\1\tNick:\2\tUniqueID:\3\tPeerDevNonceCounterVal:\4\tDevNonceCounterVal:\5/g'
	#echo $_sed3
	
	eval 'echo $string | sed $_sed | grep CMD '
	eval 'echo $string | sed $_sed | grep -v CMD | sed $_sed2  | sed $_sed3'
}

function f783 {
	string=$1;
	_sed='s/'
	_sed=$_sed'\([0-9A-Z]\{4\}\)'           # \1 CMD ID
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \2 ERR CODE
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \3 superframe index
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \4 no entries read
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \5 no active superframes
	_sed=$_sed'\([0-9A-Z]*\)'               # \6 superframes
	_sed=$_sed'/CMDID:\1\terror:\2\tSfrIdx:\3\tEntriesRead:\4\tNoActiveSuperframes:\5\n\6/g'
	#echo $_sed
	
	_sed2='s/'
	_sed2=$_sed2'\([0-9A-Za-z]\{8\}\)/\1\n/g'   # 4 bytes
	
	_sed3='s/'
	_sed3=$_sed3'^\(.\{2\}\)\(.\{2\}\)\(.\{4\}\)'
	_sed3=$_sed3'/sfrID:\1\tFlags:\2\tNoSlots:\3/g'
	#echo $_sed3
	
	eval 'echo $string | sed $_sed | grep CMD '
	eval 'echo $string | sed $_sed | grep -v CMD | sed $_sed2  | sed $_sed3'
}
 
function f784 {
	string=$1;
	_sed='s/'
	_sed=$_sed'\([0-9A-Z]\{4\}\)'           # \1 CMD ID
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \2 ERR CODE

	_sed=$_sed'\([0-9A-Z]\{4\}\)'           # \3 link index
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \4 no entries read
	_sed=$_sed'\([0-9A-Z]\{4\}\)'           # \5 no active links
	_sed=$_sed'\([0-9A-Z]*\)'               # \6 links
	_sed=$_sed'/CMDID:\1\terror:\2\tLinkIdx:\3\tEntriesRead:\4\tNoActiveLinks:\5\n\6/g'
	#echo $_sed
	
	_sed2='s/'
	_sed2=$_sed2'\([0-9A-Za-z]\{16\}\)/\1\n/g'   # 8 bytes
	
	_sed3='s/'
	_sed3=$_sed3'^\(.\{2\}\)\(.\{4\}\)\(.\{2\}\)\(.\{4\}\)\(.\{2\}\)\(.\{2\}\)'
	_sed3=$_sed3'/sfrID:\1\tslotNoForThisLink:\2\tchannelOffsetForThisLink:\3\tnicknameOfNeighborForThisLink:\4\tlinkOptions:\5\tlinkType:\6/g'
	#echo $_sed3
	
	eval 'echo $string | sed $_sed | grep CMD '
	eval 'echo $string | sed $_sed | grep -v CMD | sed $_sed2  | sed $_sed3'
}

function f785 {
	string=$1;
	_sed='s/'
	_sed=$_sed'\([0-9A-Z]\{4\}\)'           # \1 CMD ID
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \2 ERR CODE

	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \3 graph list index
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \4 total no of graphs
	_sed=$_sed'\([0-9A-Z]\{4\}\)'           # \5 graph id
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \6 no of neighbors
	_sed=$_sed'\([0-9A-Z]*\)'               # \7 neighbors
	_sed=$_sed'/CMDID:\1\terror:\2\tGraphIdx:\3\tTotalNoGraphs:\4\tGraphId:\5\tNoNeighbors:\6\n\7/g'
	#echo $_sed
	
	_sed2='s/'
	_sed2=$_sed2'\([0-9A-Z]\{4\}\)/\1\n/g'   # 2 bytes
	#echo $_sed2

	_sed3='s/'
	_sed3=$_sed3'\(.\{4\}\)'
	_sed3=$_sed3'/neigh:\1\t/g'
	#echo $_sed3
	
	eval 'echo $string | sed $_sed | grep CMD ' 
	eval 'echo $string | sed $_sed | grep -v CMD | sed $_sed2  | sed $_sed3'
} 

function f800 {
	string=$1;
	_sed='s/'
	_sed=$_sed'\([0-9A-Z]\{4\}\)'           # \1 CMD ID
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \2 ERR CODE

	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \3 service index
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \4 no of entries read
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \5 no of active services
	_sed=$_sed'\([0-9A-Z]*\)'               # \6 services
	_sed=$_sed'/CMDID:\1\terror:\2\tServiceIdx:\3\tNoEntriesRead:\4\tNoActiveEntries:\5\n\6/g'
	#echo $_sed
	
	_sed2='s/'
	_sed2=$_sed2'\([0-9A-Z]\{20\}\)/\1\n/g'   # 10 bytes
	#echo $_sed2

	_sed3='s/'
	_sed3=$_sed3'^\(.\{2\}\)\(.\{2\}\)\(.\{2\}\)\(.\{4\}\)\(.\{8\}\)\(.\{2\}\)'
	_sed3=$_sed3'/serviceID:\1\tsrvReqFlags:\2\tappDomain:\3\tnicknameOfPeer:\4\tperiod:\5\trouteId:\6/g'
	#echo $_sed3
	#echo $_sed3
	
	eval 'echo $string | sed $_sed | grep CMD ' 
	eval 'echo $string | sed $_sed | grep -v CMD | sed $_sed2  | sed $_sed3'
}


function f802 {
	string=$1;
	_sed='s/'
	_sed=$_sed'\([0-9A-Z]\{4\}\)'           # \1 CMD ID
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \2 ERR CODE

	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \3 route index
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \4 no of entries read
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \5 no of active routes
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \6 no of routes remaining
	_sed=$_sed'\([0-9A-Z]*\)'               # \7 services
	_sed=$_sed'/CMDID:\1\terror:\2\tRouteIdx:\3\tNoEntriesRead:\4\tNoActiveEntries:\5\tNoRoutesRemaining:\6\n\7/g'
	#echo $_sed
	
	_sed2='s/'
	_sed2=$_sed2'\([0-9A-Z]\{12\}\)/\1\n/g'   # 6 bytes
	#echo $_sed2

	_sed3='s/'
	_sed3=$_sed3'^\(.\{2\}\)\(.\{4\}\)\(.\{4\}\)\(.\{2\}\)'
	_sed3=$_sed3'/routeID:\1\tdestNickname:\2\tgraphId:\3\tsourceRouteAttached:\4\t/g'
	#echo $_sed3
	#echo $_sed3
	
	eval 'echo $string | sed $_sed | grep CMD ' 
	eval 'echo $string | sed $_sed | grep -v CMD | sed $_sed2  | sed $_sed3'
}



function f803 {
	string=$1;
	_sed='s/'
	_sed=$_sed'\([0-9A-Z]\{4\}\)'           # \1 CMD ID
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \2 ERR CODE

	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \3 route id
	_sed=$_sed'\([0-9A-Z]\{2\}\)'           # \4 no of hops
	_sed=$_sed'\([0-9A-Z]*\)'               # \5 hops
	_sed=$_sed'/CMDID:\1\terror:\2\tRouteId:\3\tNoHops:\4\n\5/g'
	#echo $_sed
	
	_sed2='s/'
	_sed2=$_sed2'\([0-9A-Z]\{4\}\)/\1\n/g'   # 2 bytes
	#echo $_sed2

	_sed3='s/'
	_sed3=$_sed3'^\(.\{4\}\)'
	_sed3=$_sed3'/HopNickname:\1\t/g'
	#echo $_sed3
	#echo $_sed3
	
	eval 'echo $string | sed $_sed | grep CMD ' 
	eval 'echo $string | sed $_sed | grep -v CMD | sed $_sed2  | sed $_sed3'
}


# 782
string=030E08000420020000F8700000020000000000000000020000F8700000030000000000000000020000F8700000040000000000000000020000F8700000050000000000000000 
# 783
string=030F000004090100190102003201030064010400C801
# 784
string=031008000008000E06000003FFFF050306000003FFFF0203060000020014020005000002001601000600000100010200060000020014020006000001000102000500000100010100
# 785
string=0311000207010701F981
# 800
string=032008000303800502F9800007D00001810700F9810007D00002010100F9810003E80002
# 802
string=0322080002020001F98001090002F981010900
# 803
string=0323000100

string=$1

 
while echo "..."; read string; do
	echo
	if [ `echo $string | grep -c "^030E"` -gt 0 ] 
	then 
		echo calls 782 \(030E\) parser for $string
		f782 $string;
	elif [ `echo $string | grep -c "^030F"` -gt 0 ] 
	then 
		echo calls 783 \(030F\) parser for $string
		f783 $string;
	elif [ `echo $string | grep -c "^0310"` -gt 0 ] 
	then 
		echo calls 784 \(0310\) parser for $string
		f784 $string;
	elif [ `echo $string | grep -c "^0311"` -gt 0 ] 
	then 
		echo calls 785 \(0311\) parser for $string
		f785 $string;
	elif [ `echo $string | grep -c "^0320"` -gt 0 ] 
	then 
		echo calls 800 \(0320\) parser for $string
		f800 $string;
	elif [ `echo $string | grep -c "^0322"` -gt 0 ] 
	then 
		echo calls 802 \(0322\) parser for $string
		f802 $string;
	elif [ `echo $string | grep -c "^0323"` -gt 0 ] 
	then 
		echo calls 803 \(0323\) parser for $string
		f803 $string;
	else 
		echo There is no parser\(yet\) for command "`echo $string | sed 's/\([0-9a-zA-Z]\{4\}\).*/\1/g'`"
	fi 

done;
