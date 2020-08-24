# Microsoft Developer Studio Project File - Name="bsabs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=bsabs - Win32 DebugAnsi
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bsabs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bsabs.mak" CFG="bsabs - Win32 DebugAnsi"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bsabs - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "bsabs - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "bsabs - Win32 DebugAnsi" (based on "Win32 (x86) Static Library")
!MESSAGE "bsabs - Win32 ReleaseAnsi" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "bsabs"
# PROP Scc_LocalPath "..\bsabs"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bsabs - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Release\I386"
# PROP Intermediate_Dir "Release\I386"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /D "NDEBUG" /D "_UNICODE" /D "WIN32" /D "_LIB" /D "BSABS_LIB" /D "BSAUTIL_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bsabs - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug\I386"
# PROP Intermediate_Dir "Debug\I386"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "../incl" /D "_DEBUG" /D "_UNICODE" /D "WIN32" /D "_LIB" /D "BSABS_LIB" /D "BSAUTIL_LIB" /D "Windows" /D "UNICODE" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bsabs - Win32 DebugAnsi"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "bsabs___Win32_DebugAnsi"
# PROP BASE Intermediate_Dir "bsabs___Win32_DebugAnsi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug\I386A"
# PROP Intermediate_Dir "Debug\I386A"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /D "_DEBUG" /D "_UNICODE" /D "WIN32" /D "_LIB" /D "BSABS_LIB" /D "BSAUTIL_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /D "_DEBUG" /D "_MBCS" /D "WIN32" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bsabs - Win32 ReleaseAnsi"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "bsabs___Win32_ReleaseAnsi"
# PROP BASE Intermediate_Dir "bsabs___Win32_ReleaseAnsi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Release\I386A"
# PROP Intermediate_Dir "Release\I386A"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\include" /D "NDEBUG" /D "_UNICODE" /D "WIN32" /D "_LIB" /D "BSABS_LIB" /D "BSAUTIL_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "bsabs - Win32 Release"
# Name "bsabs - Win32 Debug"
# Name "bsabs - Win32 DebugAnsi"
# Name "bsabs - Win32 ReleaseAnsi"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\bsabs\bs.cpp

!IF  "$(CFG)" == "bsabs - Win32 Release"

!ELSEIF  "$(CFG)" == "bsabs - Win32 Debug"

# ADD CPP /Yu"bsx.h"

!ELSEIF  "$(CFG)" == "bsabs - Win32 DebugAnsi"

# ADD BASE CPP /Yu"bsx.h"
# ADD CPP /Yu"bsx.h"

!ELSEIF  "$(CFG)" == "bsabs - Win32 ReleaseAnsi"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\bsabs\bsinit.cpp

!IF  "$(CFG)" == "bsabs - Win32 Release"

!ELSEIF  "$(CFG)" == "bsabs - Win32 Debug"

# ADD CPP /Yu"bsx.h"

!ELSEIF  "$(CFG)" == "bsabs - Win32 DebugAnsi"

# ADD BASE CPP /Yu"bsx.h"
# ADD CPP /Yu"bsx.h"

!ELSEIF  "$(CFG)" == "bsabs - Win32 ReleaseAnsi"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\bsabs\bsx.cpp

!IF  "$(CFG)" == "bsabs - Win32 Release"

!ELSEIF  "$(CFG)" == "bsabs - Win32 Debug"

# ADD CPP /Yc"bsx.h"

!ELSEIF  "$(CFG)" == "bsabs - Win32 DebugAnsi"

# ADD BASE CPP /Yc"bsx.h"
# ADD CPP /Yc"bsx.h"

!ELSEIF  "$(CFG)" == "bsabs - Win32 ReleaseAnsi"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\bsabs.h
# End Source File
# Begin Source File

SOURCE=..\bsabs\bsx.h
# End Source File
# End Group
# End Target
# End Project
