# Microsoft Developer Studio Project File - Name="bsapersist" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=bsapersist - Win32 DebugAnsi
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bsapersist.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bsapersist.mak" CFG="bsapersist - Win32 DebugAnsi"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bsapersist - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "bsapersist - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "bsapersist - Win32 DebugAnsi" (based on "Win32 (x86) Static Library")
!MESSAGE "bsapersist - Win32 ReleaseAnsi" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "bsapersist"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bsapersist - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "bsapersist___Win32_Release"
# PROP BASE Intermediate_Dir "bsapersist___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Release\I386"
# PROP Intermediate_Dir "Release\I386"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../include" /D "NDEBUG" /D "WIN32" /D "BS_PERST_XML" /D "_UNICODE" /D "_LIB" /D "BSAUTIL_LIB" /D "BSAPERSIST_LIB" /D "BSAXML_LIB" /D "BSABS_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bsapersist - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "bsapersist___Win32_Debug"
# PROP BASE Intermediate_Dir "bsapersist___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug\I386"
# PROP Intermediate_Dir "Debug\I386"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include" /I "../incl" /D "_DEBUG" /D "BS_PERST_XML" /D "WIN32" /D "_UNICODE" /D "_LIB" /D "BSAUTIL_LIB" /D "BSAPERSIST_LIB" /D "BSAXML_LIB" /D "BSABS_LIB" /D "Windows" /D "UNICODE" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bsapersist - Win32 DebugAnsi"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "bsapersist___Win32_DebugAnsi"
# PROP BASE Intermediate_Dir "bsapersist___Win32_DebugAnsi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug\I386A"
# PROP Intermediate_Dir "Debug\I386A"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include" /D "_DEBUG" /D "BS_PERST_XML" /D "WIN32" /D "_UNICODE" /D "_LIB" /D "BSAUTIL_LIB" /D "BSAPERSIST_LIB" /D "BSAXML_LIB" /D "BSABS_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include" /D "_DEBUG" /D "BS_PERST_XML" /D "WIN32" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "bsapersist - Win32 ReleaseAnsi"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "bsapersist___Win32_ReleaseAnsi"
# PROP BASE Intermediate_Dir "bsapersist___Win32_ReleaseAnsi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Release\I386A"
# PROP Intermediate_Dir "Release\I386A"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "../include" /D "NDEBUG" /D "WIN32" /D "BS_PERST_XML" /D "_UNICODE" /D "_LIB" /D "BSAUTIL_LIB" /D "BSAPERSIST_LIB" /D "BSAXML_LIB" /D "BSABS_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "BS_PERST_XML" /YX /FD /c
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

# Name "bsapersist - Win32 Release"
# Name "bsapersist - Win32 Debug"
# Name "bsapersist - Win32 DebugAnsi"
# Name "bsapersist - Win32 ReleaseAnsi"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\persist.cpp
# ADD CPP /Yu"persistx.h"
# End Source File
# Begin Source File

SOURCE=.\persistx.cpp
# ADD CPP /Yc"persistx.h"
# End Source File
# Begin Source File

SOURCE=.\perstini.cpp
# ADD CPP /Yu"persistx.h"
# End Source File
# Begin Source File

SOURCE=.\perstrc.cpp
# ADD CPP /Yu"persistx.h"
# End Source File
# Begin Source File

SOURCE=.\perstreg.cpp
# ADD CPP /Yu"persistx.h"
# End Source File
# Begin Source File

SOURCE=.\perstxml.cpp
# ADD CPP /Yu"persistx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\bsapersist.h
# End Source File
# Begin Source File

SOURCE=.\persistx.h
# End Source File
# End Group
# End Target
# End Project
