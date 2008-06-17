# Microsoft Developer Studio Project File - Name="libpdf" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libpdf - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libpdf.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libpdf.mak" CFG="libpdf - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libpdf - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libpdf - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libpdf - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "C:\DEVEL\ZLib\include" /I "../win32" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libpdf - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libpdf___Win32_Debug"
# PROP BASE Intermediate_Dir "libpdf___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "C:\DEVEL\ZLib\include" /I "../win32" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libpdf - Win32 Release"
# Name "libpdf - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Document.cxx
# End Source File
# Begin Source File

SOURCE=.\File.cxx
# End Source File
# Begin Source File

SOURCE=.\Filter.cxx
# End Source File
# Begin Source File

SOURCE=.\Filter_Base85.cxx
# End Source File
# Begin Source File

SOURCE=.\Filter_Flate.cxx
# End Source File
# Begin Source File

SOURCE=.\Font.cxx
# End Source File
# Begin Source File

SOURCE=.\Font_CMap.cxx
# End Source File
# Begin Source File

SOURCE=.\Font_Encoding.cxx
# End Source File
# Begin Source File

SOURCE=.\Object.cxx
# End Source File
# Begin Source File

SOURCE=.\ObjectsCache.cxx
# End Source File
# Begin Source File

SOURCE=.\Page.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Ctm.hpp
# End Source File
# Begin Source File

SOURCE=.\Document.hpp
# End Source File
# Begin Source File

SOURCE=.\Exceptions.hpp
# End Source File
# Begin Source File

SOURCE=.\File.hpp
# End Source File
# Begin Source File

SOURCE=.\Filter.hpp
# End Source File
# Begin Source File

SOURCE=.\Font.hpp
# End Source File
# Begin Source File

SOURCE=.\Font_CMap.hpp
# End Source File
# Begin Source File

SOURCE=.\Font_Encoding.hpp
# End Source File
# Begin Source File

SOURCE=.\Media.hpp
# End Source File
# Begin Source File

SOURCE=.\Object.hpp
# End Source File
# Begin Source File

SOURCE=.\ObjectsCache.hpp
# End Source File
# Begin Source File

SOURCE=.\OH.hpp
# End Source File
# Begin Source File

SOURCE=.\Page.hpp
# End Source File
# Begin Source File

SOURCE=.\Point.hpp
# End Source File
# Begin Source File

SOURCE=.\Rect.hpp
# End Source File
# End Group
# End Target
# End Project
