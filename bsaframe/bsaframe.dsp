# Microsoft Developer Studio Project File - Name="bsaframe" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=bsaframe - Win32 DebugAnsi
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bsaframe.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bsaframe.mak" CFG="bsaframe - Win32 DebugAnsi"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bsaframe - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "bsaframe - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "bsaframe - Win32 DebugAnsi" (based on "Win32 (x86) Static Library")
!MESSAGE "bsaframe - Win32 ReleaseAnsi" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "bsaframe"
# PROP Scc_LocalPath "..\bsaframe"
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bsaframe - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../include" /D "NDEBUG" /D "WIN32" /D "_UNICODE" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bsaframe - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include" /I "../incl" /D "_DEBUG" /D "WIN32" /D "_UNICODE" /D "_LIB" /D "Windows" /D "UNICODE" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bsaframe - Win32 DebugAnsi"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugAnsi"
# PROP BASE Intermediate_Dir "DebugAnsi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug\I386A"
# PROP Intermediate_Dir "Debug\I386A"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include" /D "_DEBUG" /D "WIN32" /D "_UNICODE" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bsaframe - Win32 ReleaseAnsi"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "bsaframe___Win32_ReleaseAnsi"
# PROP BASE Intermediate_Dir "bsaframe___Win32_ReleaseAnsi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Release\I386A"
# PROP Intermediate_Dir "Release\I386A"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "../include" /D "NDEBUG" /D "WIN32" /D "_UNICODE" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /YX /FD /c
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

# Name "bsaframe - Win32 Release"
# Name "bsaframe - Win32 Debug"
# Name "bsaframe - Win32 DebugAnsi"
# Name "bsaframe - Win32 ReleaseAnsi"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bsacomp.cpp
# ADD CPP /Yu"framex.h"
# End Source File
# Begin Source File

SOURCE=.\bsaframe.cpp
# ADD CPP /Yu"framex.h"
# End Source File
# Begin Source File

SOURCE=..\bsaframe\bsapane.cpp
# ADD CPP /Yu"framex.h"
# End Source File
# Begin Source File

SOURCE=..\bsaframe\bsapanel.cpp
# ADD CPP /Yu"framex.h"
# End Source File
# Begin Source File

SOURCE=.\bsizer.cpp
# ADD CPP /Yu"framex.h"
# End Source File
# Begin Source File

SOURCE=.\framex.cpp
# ADD CPP /Yc"framex.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\bsaframe.h
# End Source File
# Begin Source File

SOURCE=..\bsaframe\framex.h
# End Source File
# End Group
# End Target
# Begin Group "Header Files No. 1"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Project
