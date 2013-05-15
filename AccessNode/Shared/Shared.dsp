# Microsoft Developer Studio Project File - Name="Shared" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Shared - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Shared.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Shared.mak" CFG="Shared - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Shared - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Shared - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "AccessNode/Shared"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Shared - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "Shared - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Shared - Win32 Release"
# Name "Shared - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AlarmEx.cpp
# End Source File
# Begin Source File

SOURCE=.\ANEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\app.cpp
# End Source File
# Begin Source File

SOURCE=.\ArmADC.cpp
# End Source File
# Begin Source File

SOURCE=.\ArmDPort.cpp
# End Source File
# Begin Source File

SOURCE=.\ArmGPIO.cpp
# End Source File
# Begin Source File

SOURCE=.\BuffLink.cpp
# End Source File
# Begin Source File

SOURCE=.\BusyWatcher.cpp
# End Source File
# Begin Source File

SOURCE=.\CfgBuffVar.cpp
# End Source File
# Begin Source File

SOURCE=.\CLookupTable.cpp
# End Source File
# Begin Source File

SOURCE=.\CmdLine.cpp
# End Source File
# Begin Source File

SOURCE=.\CmdLineArgs.cpp
# End Source File
# Begin Source File

SOURCE=.\Common.cpp
# End Source File
# Begin Source File

SOURCE=.\Config.cpp
# End Source File
# Begin Source File

SOURCE=.\croutetodnc.cpp
# End Source File
# Begin Source File

SOURCE=.\CString.cpp
# End Source File
# Begin Source File

SOURCE=.\des.cpp
# End Source File
# Begin Source File

SOURCE=.\DevMem.cpp
# End Source File
# Begin Source File

SOURCE=.\DSTHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\DynCommMed.cpp
# End Source File
# Begin Source File

SOURCE=.\ErActivity.cpp
# End Source File
# Begin Source File

SOURCE=.\Events.cpp
# End Source File
# Begin Source File

SOURCE=.\EventsMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\EventsStruct.cpp
# End Source File
# Begin Source File

SOURCE=.\ExtendingBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\FileLock.cpp
# End Source File
# Begin Source File

SOURCE=.\h.cpp
# End Source File
# Begin Source File

SOURCE=.\hash_test.cpp
# End Source File
# Begin Source File

SOURCE=.\HashTable.cpp
# End Source File
# Begin Source File

SOURCE=.\IdsBooking.cpp
# End Source File
# Begin Source File

SOURCE=.\IniFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IniParser.cpp
# End Source File
# Begin Source File

SOURCE=.\link.cpp
# End Source File
# Begin Source File

SOURCE=.\linklog.cpp
# End Source File
# Begin Source File

SOURCE=.\log.cpp
# End Source File
# Begin Source File

SOURCE=.\log2flash.cpp
# End Source File
# Begin Source File

SOURCE=.\ModulesActivity.cpp
# End Source File
# Begin Source File

SOURCE=.\MsgParameters.cpp
# End Source File
# Begin Source File

SOURCE=.\NcLink.cpp
# End Source File
# Begin Source File

SOURCE=.\NcLink.h
# End Source File
# Begin Source File

SOURCE=.\PacketStream.cpp
# End Source File
# Begin Source File

SOURCE=.\PersistentInteger.cpp
# End Source File
# Begin Source File

SOURCE=.\PersistentMsgId.cpp
# End Source File
# Begin Source File

SOURCE=.\pipe.cpp
# End Source File
# Begin Source File

SOURCE=.\PipeSim.cpp
# End Source File
# Begin Source File

SOURCE=.\ProtocolPacket.cpp
# End Source File
# Begin Source File

SOURCE=.\PThreadWrapper.cpp
# End Source File
# Begin Source File

SOURCE=".\RSA-core.cpp"
# End Source File
# Begin Source File

SOURCE=.\RuleDefFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\SharedVariables.cpp
# End Source File
# Begin Source File

SOURCE=.\SignalsMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SimpleStream.cpp
# End Source File
# Begin Source File

SOURCE=.\SimpleTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\Socket.cpp
# End Source File
# Begin Source File

SOURCE=.\TcpSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\UdpSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils.cpp
# End Source File
# Begin Source File

SOURCE=.\VarParam.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AlarmEx.h
# End Source File
# Begin Source File

SOURCE=.\ANEvent.h
# End Source File
# Begin Source File

SOURCE=.\AnPaths.h
# End Source File
# Begin Source File

SOURCE=.\app.h
# End Source File
# Begin Source File

SOURCE=.\ArmADC.h
# End Source File
# Begin Source File

SOURCE=.\ArmDPort.h
# End Source File
# Begin Source File

SOURCE=.\ArmGPIO.h
# End Source File
# Begin Source File

SOURCE=.\BuffLink.h
# End Source File
# Begin Source File

SOURCE=.\BusyWatcher.h
# End Source File
# Begin Source File

SOURCE=.\CfgBuffVar.h
# End Source File
# Begin Source File

SOURCE=.\CLookupTable.h
# End Source File
# Begin Source File

SOURCE=.\CmdLine.h
# End Source File
# Begin Source File

SOURCE=.\CmdLineArgs.h
# End Source File
# Begin Source File

SOURCE=.\Common.h
# End Source File
# Begin Source File

SOURCE=.\Config.h
# End Source File
# Begin Source File

SOURCE=.\croutetodnc.h
# End Source File
# Begin Source File

SOURCE=.\cseriallink.h
# End Source File
# Begin Source File

SOURCE=.\CString.h
# End Source File
# Begin Source File

SOURCE=.\des.h
# End Source File
# Begin Source File

SOURCE=.\DevMem.h
# End Source File
# Begin Source File

SOURCE=.\DSTHandler.h
# End Source File
# Begin Source File

SOURCE=.\DynCommMed.h
# End Source File
# Begin Source File

SOURCE=.\EasyBuffer.h
# End Source File
# Begin Source File

SOURCE=.\ErActivity.h
# End Source File
# Begin Source File

SOURCE=.\Events.h
# End Source File
# Begin Source File

SOURCE=.\EventsMgr.h
# End Source File
# Begin Source File

SOURCE=.\EventsStruct.h
# End Source File
# Begin Source File

SOURCE=.\ExtendingBuffer.h
# End Source File
# Begin Source File

SOURCE=.\FileLock.h
# End Source File
# Begin Source File

SOURCE=.\gmp.h
# End Source File
# Begin Source File

SOURCE=.\h.h
# End Source File
# Begin Source File

SOURCE=.\HashTable.h
# End Source File
# Begin Source File

SOURCE=.\IdsBooking.h
# End Source File
# Begin Source File

SOURCE=.\IniFile.h
# End Source File
# Begin Source File

SOURCE=.\IniParser.h
# End Source File
# Begin Source File

SOURCE=.\link.h
# End Source File
# Begin Source File

SOURCE=.\linklog.h
# End Source File
# Begin Source File

SOURCE=.\Locks.h
# End Source File
# Begin Source File

SOURCE=.\log.h
# End Source File
# Begin Source File

SOURCE=.\log2flash.h
# End Source File
# Begin Source File

SOURCE=.\ModulesActivity.h
# End Source File
# Begin Source File

SOURCE=.\MsgParameters.h
# End Source File
# Begin Source File

SOURCE=.\ObjList.h
# End Source File
# Begin Source File

SOURCE=.\PacketStream.h
# End Source File
# Begin Source File

SOURCE=.\PersistentInteger.h
# End Source File
# Begin Source File

SOURCE=.\PersistentMsgId.h
# End Source File
# Begin Source File

SOURCE=.\pipe.h
# End Source File
# Begin Source File

SOURCE=.\ProtocolLayer.h
# End Source File
# Begin Source File

SOURCE=.\ProtocolPacket.h
# End Source File
# Begin Source File

SOURCE=.\PThreadWrapper.h
# End Source File
# Begin Source File

SOURCE=.\PtrList.h
# End Source File
# Begin Source File

SOURCE=".\RSA-core.h"
# End Source File
# Begin Source File

SOURCE=.\RuleDefFile.h
# End Source File
# Begin Source File

SOURCE=.\ServerSocket.h
# End Source File
# Begin Source File

SOURCE=.\SharedVariables.h
# End Source File
# Begin Source File

SOURCE=.\SignalsMgr.h
# End Source File
# Begin Source File

SOURCE=.\SimpleStream.h
# End Source File
# Begin Source File

SOURCE=.\SimpleTimer.h
# End Source File
# Begin Source File

SOURCE=.\Socket.h
# End Source File
# Begin Source File

SOURCE=.\StreamLink.h
# End Source File
# Begin Source File

SOURCE=.\structures.h
# End Source File
# Begin Source File

SOURCE=.\TcpSocket.h
# End Source File
# Begin Source File

SOURCE=.\UdpSocket.h
# End Source File
# Begin Source File

SOURCE=.\Utils.h
# End Source File
# Begin Source File

SOURCE=.\VarParam.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\Makefile
# End Source File
# End Target
# End Project
