# Microsoft Developer Studio Project File - Name="bsaopenfile" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=bsaopenfile - Win32 DebugAnsi
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bsaopenfile.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bsaopenfile.mak" CFG="bsaopenfile - Win32 DebugAnsi"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bsaopenfile - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "bsaopenfile - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "bsaopenfile - Win32 DebugAnsi" (based on "Win32 (x86) Static Library")
!MESSAGE "bsaopenfile - Win32 ReleaseAnsi" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "bsaopenfile"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bsaopenfile - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../include" /D "NDEBUG" /D "WIN32" /D "BSAOPENFILE_LIB" /D "_LIB" /D "_UNICODE" /D "UNICODE" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bsaopenfile - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include" /I "../incl" /D "Windows" /D "UNICODE" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bsaopenfile - Win32 DebugAnsi"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "bsaopenfile___Win32_DebugAnsi"
# PROP BASE Intermediate_Dir "bsaopenfile___Win32_DebugAnsi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug\I386A"
# PROP Intermediate_Dir "Debug\I386A"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include" /D "_DEBUG" /D "WIN32" /D "BSAOPENFILE_LIB" /D "_LIB" /D "_UNICODE" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include" /D "_DEBUG" /D "WIN32" /D "_LIB" /D "_MBCS" /Yu"openfilex.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bsaopenfile - Win32 ReleaseAnsi"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "bsaopenfile___Win32_ReleaseAnsi"
# PROP BASE Intermediate_Dir "bsaopenfile___Win32_ReleaseAnsi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Release\I386A"
# PROP Intermediate_Dir "Release\I386A"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "../include" /D "NDEBUG" /D "WIN32" /D "BSAOPENFILE_LIB" /D "_LIB" /D "_UNICODE" /D "UNICODE" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../include" /D "NDEBUG" /D "WIN32" /D "BSAOPENFILE_LIB" /D "_LIB" /D "_MBCS" /YX /FD /c
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

# Name "bsaopenfile - Win32 Release"
# Name "bsaopenfile - Win32 Debug"
# Name "bsaopenfile - Win32 DebugAnsi"
# Name "bsaopenfile - Win32 ReleaseAnsi"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\offile.cpp
# ADD CPP /Yu"openfilex.h"
# End Source File
# Begin Source File

SOURCE=.\ofres.cpp
# ADD CPP /Yu"openfilex.h"
# End Source File
# Begin Source File

SOURCE=.\ofsym.cpp
# ADD CPP /Yu"openfilex.h"
# End Source File
# Begin Source File

SOURCE=.\openfile.cpp
# ADD CPP /Yu"openfilex.h"
# End Source File
# Begin Source File

SOURCE=.\openfilex.cpp
# ADD CPP /Yc"openfilex.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ofpriv.h
# End Source File
# Begin Source File

SOURCE=.\openfilex.h
# End Source File
# End Group
# End Target
# End Project
