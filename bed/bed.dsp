# Microsoft Developer Studio Project File - Name="bed" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=bed - Win32 DebugAnsi
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bed.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bed.mak" CFG="bed - Win32 DebugAnsi"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bed - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "bed - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "bed - Win32 DebugAnsi" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "bed"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bed - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Release\I386"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "BSAUTIL_LIB" /D "BSAPERSIST_LIB" /D "BSABS_LIB" /D "BSAWIZ_LIB" /D "BSASHELL_LIB" /D "BSAXML_LIB" /D "BSAFTP_LIB" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 bsawiz.lib bsaframe.lib bsapersist.lib bsautils.lib bsaxml.lib bsabs.lib bsashell.lib bsaftp.lib bcalclib.lib bsaopenfile.lib comctl32.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /libpath:"..\Release\I386"

!ELSEIF  "$(CFG)" == "bed - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug\I386"
# PROP Intermediate_Dir "Debug\I386"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include" /I "..\include" /I "../incl" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "Windows" /D "UNICODE" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 bsasercom.lib rpcrt4.lib bsawiz.lib bsaframe.lib bsapersist.lib bsautils.lib bsaxml.lib bsabs.lib bsashell.lib bsaftp.lib bcalclib.lib bsaopenfile.lib comctl32.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\Debug\I386"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "bed - Win32 DebugAnsi"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "bed___Win32_DebugAnsi"
# PROP BASE Intermediate_Dir "bed___Win32_DebugAnsi"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\Debug\I386A"
# PROP Intermediate_Dir "Debug\I386A"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
LIB32=link.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include" /I "..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "BS_PERST_XML" /D "BSAUTIL_LIB" /D "BSAPERSIST_LIB" /D "BSABS_LIB" /D "BSAWIZ_LIB" /D "BSASHELL_LIB" /D "BSAXML_LIB" /D "BSAFTP_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include" /I "..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "BS_PERST_XML" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 bsawiz.lib wsock32.lib bsapersist.lib bsautils.lib bsaxml.lib bsabs.lib bsashell.lib bsaftp.lib bsasercom.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\Debug\I386"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 bsasercom.lib bsawiz.lib bsaframe.lib bsapersist.lib bsautils.lib bsaxml.lib bsabs.lib bsashell.lib bsaftp.lib bcalclib.lib bsaopenfile.lib comctl32.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\Debug\I386A"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "bed - Win32 Release"
# Name "bed - Win32 Debug"
# Name "bed - Win32 DebugAnsi"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bdbg.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\bed.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\bedx.cpp
# ADD CPP /Yc"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\bsccs.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\buffer.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\buffile.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\bufftp.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\bufraw.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\colordial.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\dismember.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\dispatch.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\edit.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\fontdial.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\ftpauth.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\help.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\infobufs.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\infobuild.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\infodbg.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\infofile.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\infofunc.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\infogrep.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\infopane.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\inforesults.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\infosccs.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\infoshell.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\keybind.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\line.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\macro.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\openfile.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\parm.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\parsec.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\pdb.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\print.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\project.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\protwind.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\sandr.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\tclscript.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\token.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\view.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\viewasm.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\viewc.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\viewhex.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\viewhtml.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\viewjava.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\viewrs232.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\viewshell.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\viewstream.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\viewtcl.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\viewtelnet.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# Begin Source File

SOURCE=.\viewterm.cpp
# ADD CPP /Yu"bedx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\bdbg.h
# End Source File
# Begin Source File

SOURCE=.\bed.h
# End Source File
# Begin Source File

SOURCE=.\bedx.h
# End Source File
# Begin Source File

SOURCE=.\bsccs.h
# End Source File
# Begin Source File

SOURCE=.\buffer.h
# End Source File
# Begin Source File

SOURCE=.\infodbg.h
# End Source File
# Begin Source File

SOURCE=.\infopane.h
# End Source File
# Begin Source File

SOURCE=.\keybind.h
# End Source File
# Begin Source File

SOURCE=.\line.h
# End Source File
# Begin Source File

SOURCE=.\parm.h
# End Source File
# Begin Source File

SOURCE=.\pdb.h
# End Source File
# Begin Source File

SOURCE=.\print.h
# End Source File
# Begin Source File

SOURCE=.\project.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\tclscript.h
# End Source File
# Begin Source File

SOURCE=.\view.h
# End Source File
# Begin Source File

SOURCE=.\viewshell.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\bed128.bmp
# End Source File
# Begin Source File

SOURCE=.\bed32.ico
# End Source File
# Begin Source File

SOURCE=.\bedres.rc
# End Source File
# Begin Source File

SOURCE=.\icons\buf16.bmp
# End Source File
# Begin Source File

SOURCE=.\icons\buf16mod.bmp
# End Source File
# Begin Source File

SOURCE=.\icons\class.ico
# End Source File
# Begin Source File

SOURCE=.\icons\data.ico
# End Source File
# Begin Source File

SOURCE=.\icons\dir16.bmp
# End Source File
# Begin Source File

SOURCE=.\icons\dir16op.bmp
# End Source File
# Begin Source File

SOURCE=.\icons\file16.bmp
# End Source File
# Begin Source File

SOURCE=.\icons\file16ro.bmp
# End Source File
# Begin Source File

SOURCE=.\icons\func.ico
# End Source File
# Begin Source File

SOURCE=.\hsize.cur
# End Source File
# Begin Source File

SOURCE=.\vsize.cur
# End Source File
# Begin Source File

SOURCE=.\wizimg.bmp
# End Source File
# End Group
# End Target
# End Project
