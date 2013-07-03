# Microsoft Developer Studio Project File - Name="HawkVoiceDI_DLL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=HawkVoiceDI_DLL - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "HawkVoiceDI_DLL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "HawkVoiceDI_DLL.mak" CFG="HawkVoiceDI_DLL - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "HawkVoiceDI_DLL - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "HawkVoiceDI_DLL - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "HawkVoiceDI_DLL - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDLL"
# PROP Intermediate_Dir "ReleaseDLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Za /W4 /GX /O2 /I "../" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /version:0.50 /subsystem:windows /dll /machine:I386 /out:"bin/hvdi.dll"
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=copy     ReleaseDLL\*.lib     bin\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "HawkVoiceDI_DLL - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugDLL"
# PROP Intermediate_Dir "DebugDLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Za /W4 /Gm /GX /Zi /Od /I "../" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /version:0.50 /subsystem:windows /dll /profile /debug /machine:I386 /out:"bin/hvdi.dll"
# Begin Special Build Tool
SOURCE=$(InputPath)
PostBuild_Cmds=copy     DebugDLL\*.lib     bin\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "HawkVoiceDI_DLL - Win32 Release"
# Name "HawkVoiceDI_DLL - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=.\src\decpacket.c
# End Source File
# Begin Source File

SOURCE=.\src\encpacket.c
# End Source File
# Begin Source File

SOURCE=.\src\hcrypt.c
# ADD CPP /Ze
# End Source File
# Begin Source File

SOURCE=.\src\hvdi.c
# End Source File
# Begin Source File

SOURCE=.\src\rate.c
# End Source File
# End Group
# Begin Group "Include Files"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\src\hcrypt.h
# End Source File
# Begin Source File

SOURCE=.\src\hvdi.h
# End Source File
# Begin Source File

SOURCE=.\src\hvdint.h
# End Source File
# End Group
# Begin Group "Doc Files"

# PROP Default_Filter ".txt"
# Begin Source File

SOURCE=.\src\api.txt
# End Source File
# Begin Source File

SOURCE=.\src\cryptapi.txt
# End Source File
# Begin Source File

SOURCE=.\src\HVDIchanges.txt
# End Source File
# Begin Source File

SOURCE=.\src\readme.txt
# End Source File
# End Group
# End Target
# End Project
