# Microsoft Developer Studio Project File - Name="wxview" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=wxview - Win32 Ansi Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wxview.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wxview.mak" CFG="wxview - Win32 Ansi Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wxview - Win32 Unicode Debug" (based on "Win32 (x86) Application")
!MESSAGE "wxview - Win32 Unicode Release" (based on "Win32 (x86) Application")
!MESSAGE "wxview - Win32 Ansi Debug" (based on "Win32 (x86) Application")
!MESSAGE "wxview - Win32 Ansi Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wxview - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "wxview___Win32_Unicode_Debug"
# PROP BASE Intermediate_Dir "wxview___Win32_Unicode_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "wxview___Win32_Unicode_Debug"
# PROP Intermediate_Dir "wxview___Win32_Unicode_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I ".." /I "../win32" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "wxUSE_UNICODE" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I ".." /I "../win32" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "wxUSE_UNICODE" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib rpcrt4.lib winmm.lib advapi32.lib wsock32.lib ole32.lib oleaut32.lib uuid.lib zdll.lib libpdf.lib getopt.lib wxmsw28ud_core.lib wxbase28ud.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\win32" /libpath:"..\libpdf\Debug"
# ADD LINK32 wxmsw28ud_core.lib wxbase28ud.lib user32.lib gdi32.lib shell32.lib Comdlg32.lib comctl32.lib rpcrt4.lib winmm.lib advapi32.lib wsock32.lib ole32.lib oleaut32.lib uuid.lib zdll.lib libpdf.lib getopt.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\win32" /libpath:"..\libpdf\Debug"

!ELSEIF  "$(CFG)" == "wxview - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "wxview___Win32_Unicode_Release"
# PROP BASE Intermediate_Dir "wxview___Win32_Unicode_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "wxview___Win32_Unicode_Release"
# PROP Intermediate_Dir "wxview___Win32_Unicode_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I ".." /I "../win32" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "wxUSE_UNICODE" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I ".." /I "../win32" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "wxUSE_UNICODE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib rpcrt4.lib winmm.lib advapi32.lib wsock32.lib ole32.lib oleaut32.lib uuid.lib zdll.lib libpdf.lib getopt.lib /nologo /subsystem:windows /machine:I386 /libpath:"..\win32" /libpath:"..\libpdf\Release"
# ADD LINK32 wxmsw28u_core.lib wxbase28u.lib user32.lib gdi32.lib shell32.lib Comdlg32.lib comctl32.lib rpcrt4.lib winmm.lib advapi32.lib wsock32.lib ole32.lib oleaut32.lib uuid.lib zdll.lib libpdf.lib getopt.lib /nologo /subsystem:windows /machine:I386 /libpath:"..\win32" /libpath:"..\libpdf\Release"

!ELSEIF  "$(CFG)" == "wxview - Win32 Ansi Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "wxview___Win32_Ansi_Debug"
# PROP BASE Intermediate_Dir "wxview___Win32_Ansi_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "wxview___Win32_Ansi_Debug"
# PROP Intermediate_Dir "wxview___Win32_Ansi_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I ".." /I "../win32" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "wxUSE_UNICODE" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I ".." /I "../win32" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "wxUSE_WCHAR_T" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib rpcrt4.lib winmm.lib advapi32.lib wsock32.lib ole32.lib oleaut32.lib uuid.lib zdll.lib libpdf.lib getopt.lib wxmsw28ud_core.lib wxbase28ud.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\win32" /libpath:"..\libpdf\Debug"
# ADD LINK32 wxmsw28d_core.lib wxbase28d.lib user32.lib gdi32.lib shell32.lib Comdlg32.lib comctl32.lib rpcrt4.lib winmm.lib advapi32.lib wsock32.lib ole32.lib oleaut32.lib uuid.lib zdll.lib libpdf.lib getopt.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\win32" /libpath:"..\libpdf\Debug"

!ELSEIF  "$(CFG)" == "wxview - Win32 Ansi Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "wxview___Win32_Ansi_Release"
# PROP BASE Intermediate_Dir "wxview___Win32_Ansi_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "wxview___Win32_Ansi_Release"
# PROP Intermediate_Dir "wxview___Win32_Ansi_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I ".." /I "../win32" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "wxUSE_UNICODE" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I ".." /I "../win32" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "wxUSE_WCHAR_T" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib rpcrt4.lib winmm.lib advapi32.lib wsock32.lib ole32.lib oleaut32.lib uuid.lib zdll.lib libpdf.lib getopt.lib /nologo /subsystem:windows /machine:I386 /libpath:"..\win32" /libpath:"..\libpdf\Release"
# ADD LINK32 wxmsw28_core.lib wxbase28.lib user32.lib gdi32.lib shell32.lib Comdlg32.lib comctl32.lib rpcrt4.lib winmm.lib advapi32.lib wsock32.lib ole32.lib oleaut32.lib uuid.lib zdll.lib libpdf.lib getopt.lib /nologo /subsystem:windows /machine:I386 /libpath:"..\win32" /libpath:"..\libpdf\Release"

!ENDIF 

# Begin Target

# Name "wxview - Win32 Unicode Debug"
# Name "wxview - Win32 Unicode Release"
# Name "wxview - Win32 Ansi Debug"
# Name "wxview - Win32 Ansi Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\MyCanvas.cxx
# End Source File
# Begin Source File

SOURCE=.\MyDocument.cxx
# End Source File
# Begin Source File

SOURCE=.\MyFrame.cxx
# End Source File
# Begin Source File

SOURCE=.\wxview.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\MyCanvas.hpp
# End Source File
# Begin Source File

SOURCE=.\MyDocument.hpp
# End Source File
# Begin Source File

SOURCE=.\MyFrame.hpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
