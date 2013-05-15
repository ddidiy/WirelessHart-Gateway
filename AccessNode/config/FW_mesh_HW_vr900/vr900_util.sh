#! /bin/sh


vr900_tr_init()
{
	#Added for testing RESET ISSUE
	# Configure processor I/O pins as nPCI_BG type for PCI_BG
	devmem 0xF0000A48 16 0x030F
	
	# Set (R1.RESET=0) ; (R2.RESET=0) ; 
	devmem 0xF0000A09 8 0x00
	
	# Set (R1.RESET=Output) ; (R2.RESET=Output) ; keep new signal (PW.OK=Input) 
	devmem 0xF0000A19 8 0x0C

        # Power on the acquisition board radios
	devmem 0xF0000810 32 0x00000000
	sleep 1
	devmem 0xF0000810 32 0x00000024

	# Dan Cornescu Recomendations (what do they do?)
	#devmem 0xF0000A48 16 0x030F
	# Keep (R1.RESET=Output) ; (R2.RESET=Output) ; (PW.OK=Input) 
	devmem 0xF0000A19 8 0x0C
	
	# Release RESET (R1.RESET=1) ; (R2.RESET=1) 
	devmem 0xF0000A09 8 0x0C
}
