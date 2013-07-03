# Microsoft Developer Studio Project File - Name="celp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=celp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "celp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "celp.mak" CFG="celp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "celp - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "celp - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "celp - Win32 Release"

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
# ADD CPP /nologo /W4 /GX /O2 /Ob1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "celp - Win32 Debug"

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
# ADD CPP /nologo /W4 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "celp - Win32 Release"
# Name "celp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=.\autohf.c

!IF  "$(CFG)" == "celp - Win32 Release"

# ADD CPP /Ze

!ELSEIF  "$(CFG)" == "celp - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\celp_decode.c
# End Source File
# Begin Source File

SOURCE=.\celp_encode.c
# End Source File
# Begin Source File

SOURCE=.\csub.c

!IF  "$(CFG)" == "celp - Win32 Release"

# ADD CPP /Ze

!ELSEIF  "$(CFG)" == "celp - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gaincode.c

!IF  "$(CFG)" == "celp - Win32 Release"

# ADD CPP /Ze

!ELSEIF  "$(CFG)" == "celp - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ham.c

!IF  "$(CFG)" == "celp - Win32 Release"

# ADD CPP /Ze

!ELSEIF  "$(CFG)" == "celp - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\lsp34.c

!IF  "$(CFG)" == "celp - Win32 Release"

# ADD CPP /Ze

!ELSEIF  "$(CFG)" == "celp - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\lsptopc.c

!IF  "$(CFG)" == "celp - Win32 Release"

# ADD CPP /Ze

!ELSEIF  "$(CFG)" == "celp - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pctolsp2.c

!IF  "$(CFG)" == "celp - Win32 Release"

# ADD CPP /Ze

!ELSEIF  "$(CFG)" == "celp - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pctolsp3.c
# End Source File
# Begin Source File

SOURCE=.\pgain.c

!IF  "$(CFG)" == "celp - Win32 Release"

# ADD CPP /Ze

!ELSEIF  "$(CFG)" == "celp - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pitchvq.c

!IF  "$(CFG)" == "celp - Win32 Release"

# ADD CPP /Ze

!ELSEIF  "$(CFG)" == "celp - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\polefilt.c

!IF  "$(CFG)" == "celp - Win32 Release"

# ADD CPP /Ze

!ELSEIF  "$(CFG)" == "celp - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\psearch.c

!IF  "$(CFG)" == "celp - Win32 Release"

# ADD CPP /Ze

!ELSEIF  "$(CFG)" == "celp - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ptcode.c

!IF  "$(CFG)" == "celp - Win32 Release"

# ADD CPP /Ze

!ELSEIF  "$(CFG)" == "celp - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\zerofilt.c

!IF  "$(CFG)" == "celp - Win32 Release"

# ADD CPP /Ze

!ELSEIF  "$(CFG)" == "celp - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "Include Files"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\celp.h
# End Source File
# Begin Source File

SOURCE=.\celpint.h
# End Source File
# Begin Source File

SOURCE=.\codebook.h
# End Source File
# Begin Source File

SOURCE=.\pdelay.h
# End Source File
# Begin Source File

SOURCE=.\submult.h
# End Source File
# End Group
# End Target
# End Project
