#! /bin/awk -f



/\[.*node.*\]/ 	\
	{ 
		if (DeviceId != "g")  
		{ 
		
		  # DeviceId(hex), DNA(hex), NetId, Level, PwrNo, Status
		  printf( "%s, %s, %d, %s, %s, %s\n", DeviceId, DNA, NodeId, Level, PowerNo, Status); 	
		  
		  DeviceId="g";   
		  DNA=""
		  NodeId="0"
		  
		  PowerNo="0"
		  Level="0xff"
		  Status="0"		  
	    } 	
	} 
		
		
/DeviceId *=/ { DeviceId=$3; }
/NodeId *=/ { NodeId=$3}

/DNA *=/ { DNA=$3}

/PowerNo *=/ { PowerNo=("0x" $3) }
/Level *=/ { Level=("0x" $3); }
/Status *=/ { Status=("0x" $3); }

BEGIN \
	  { 
		print "# DeviceId(hex), DNA(hex), NetId, Level, PwrNo, Status" ; 
		DeviceId="g"; 

		PowerNo="0"
	    Level="0xff"
		Status="0"		  

		
	  }
END { printf( "%s, %s, %d, %s, %s, %s\n#end_file", DeviceId, DNA, NodeId, Level, PowerNo, Status);  }

 

