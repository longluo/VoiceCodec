# Microsoft Developer Studio Project File - Name="HawkVoiceDI" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=HawkVoiceDI - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "HawkVoiceDI.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "HawkVoiceDI.mak" CFG="HawkVoiceDI - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "HawkVoiceDI - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "HawkVoiceDI - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "HawkVoiceDI - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Za /W4 /GX /O2 /I "../" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN_STATIC_LIB" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release\HawkVoiceDIstatic.lib"

!ELSEIF  "$(CFG)" == "HawkVoiceDI - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Za /W4 /GX /Z7 /Od /I "../" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WIN_STATIC_LIB" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\HawkVoiceDIstatic.lib"

!ENDIF 

# Begin Target

# Name "HawkVoiceDI - Win32 Release"
# Name "HawkVoiceDI - Win32 Debug"
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
