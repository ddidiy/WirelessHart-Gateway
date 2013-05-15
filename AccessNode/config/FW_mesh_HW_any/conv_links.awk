#! /bin/awk -f



/\[.*link.*\]/ 	\
	{ 
		if (NodeId > 0)  
		{ 
		
		  # NodeId, PrevNodeId, Status, Cost
		  printf( "%d, %d, %s, %d\n", NodeId, PrevNodeId, Status, LinkCost); 	
		  
		  NodeId=0
		  PrevNodeId=0		  
		  Status="0"
		  LinkCost=0		  
	    } 	
	} 
		
		
/PrevNodeId *=/ { PrevNodeId=$3; }
/[^v]NodeId *=/ { NodeId=$3}

/Status *=/ { Status=("0x" $3); }
/LinkCost *=/ { LinkCost=$3; }


BEGIN \
	  {   print "# NodeId, PrevNodeId, Status, Cost" ; 

		  NodeId=0
		  PrevNodeId=0		  
		  Status="0"
		  LinkCost=0		  		
	  }
	  
	  
END { printf( "%d, %d, %s, %d\n#end_file", NodeId, PrevNodeId, Status, LinkCost ); }

 

