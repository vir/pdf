# Microsoft Developer Studio Project File - Name="wxtab" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=wxtab - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wxtab.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wxtab.mak" CFG="wxtab - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wxtab - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "wxtab - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wxtab - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I ".." /I "../win32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "wxUSE_UNICODE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 user32.lib gdi32.lib shell32.lib Comdlg32.lib comctl32.lib rpcrt4.lib winmm.lib advapi32.lib wsock32.lib ole32.lib oleaut32.lib uuid.lib zdll.lib libpdf.lib /nologo /subsystem:windows /machine:I386 /libpath:"..\libpdf\Release" /libpath:"..\win32" /libpath:"tabulator\Release"

!ELSEIF  "$(CFG)" == "wxtab - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "wxtab___Win32_Debug"
# PROP BASE Intermediate_Dir "wxtab___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I ".." /I "../win32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /D "wxUSE_UNICODE" /D "wxDEBUG" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 user32.lib gdi32.lib shell32.lib Comdlg32.lib comctl32.lib rpcrt4.lib winmm.lib advapi32.lib wsock32.lib ole32.lib oleaut32.lib uuid.lib zdll.lib libpdf.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\libpdf\Debug" /libpath:"..\win32" /libpath:"tabulator\Debug"

!ENDIF 

# Begin Target

# Name "wxtab - Win32 Release"
# Name "wxtab - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\wxtab.cxx
# End Source File
# Begin Source File

SOURCE=.\WxTabCanvas.cxx
# End Source File
# Begin Source File

SOURCE=.\WxTabDocument.cxx
# End Source File
# Begin Source File

SOURCE=.\WxTabFrame.cxx
# End Source File
# Begin Source File

SOURCE=.\WxTabTabulator.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\WxTabCanvas.hpp
# End Source File
# Begin Source File

SOURCE=.\WxTabDocument.hpp
# End Source File
# Begin Source File

SOURCE=.\WxTabFrame.hpp
# End Source File
# Begin Source File

SOURCE=.\WxTabTabulator.hpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
