# Microsoft Developer Studio Project File - Name="H264AVCDecoderLibTestStatic" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=H264AVCDecoderLibTestStatic - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "H264AVCDecoderLibTestStatic.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "H264AVCDecoderLibTestStatic.mak" CFG="H264AVCDecoderLibTestStatic - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "H264AVCDecoderLibTestStatic - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "H264AVCDecoderLibTestStatic - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "H264AVCDecoderLibTestStatic - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\..\..\build\windows\test\H264AVCDecoderLibTest\ReleaseStatic"
# PROP BASE Intermediate_Dir "..\..\..\build\windows\test\H264AVCDecoderLibTest\ReleaseStatic"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\build\windows\test\H264AVCDecoderLibTest\ReleaseStatic"
# PROP Intermediate_Dir "..\..\..\build\windows\test\H264AVCDecoderLibTest\ReleaseStatic"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "..\..\..\include" /I "." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "H264AVCVIDEOIOLIB_LIB" /D "H264AVCCOMMONLIB_LIB" /D "H264AVCDECODERLIB_LIB" /YX"H264AVCDecoderLibTest.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ws2_32.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /profile /debug /machine:I386 /out:"..\..\..\..\..\bin\H264AVCDecoderLibTestStatic.exe" /libpath:"..\..\..\..\..\lib"

!ELSEIF  "$(CFG)" == "H264AVCDecoderLibTestStatic - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\..\..\build\windows\test\H264AVCDecoderLibTest\DebugStatic"
# PROP BASE Intermediate_Dir "..\..\..\build\windows\test\H264AVCDecoderLibTest\DebugStatic"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\build\windows\test\H264AVCDecoderLibTest\DebugStatic"
# PROP Intermediate_Dir "..\..\..\build\windows\test\H264AVCDecoderLibTest\DebugStatic"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\..\include" /I "." /D "_CONSOLE" /D "H264AVCVIDEOIOLIB_LIB" /D "H264AVCCOMMONLIB_LIB" /D "H264AVCDECODERLIB_LIB" /D "_DEBUG" /D "WIN32" /D "_MBCS" /YX"H264AVCDecoderLibTest.h" /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:"..\..\..\..\..\bin\H264AVCDecoderLibTestStaticd.pdb" /debug /machine:I386 /out:"..\..\..\..\..\bin\H264AVCDecoderLibTestStaticd.exe" /pdbtype:sept /libpath:"..\..\..\..\..\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "H264AVCDecoderLibTestStatic - Win32 Release"
# Name "H264AVCDecoderLibTestStatic - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\DecoderParameter.cpp
# End Source File
# Begin Source File

SOURCE=.\H264AVCDecoderLibTest.cpp
# ADD CPP /Yc"H264AVCDecoderLibTest.h"
# End Source File
# Begin Source File

SOURCE=.\H264AVCDecoderTest.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\DecoderParameter.h
# End Source File
# Begin Source File

SOURCE=.\H264AVCDecoderLibTest.h
# End Source File
# Begin Source File

SOURCE=.\H264AVCDecoderTest.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
