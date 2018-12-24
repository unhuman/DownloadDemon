# Microsoft Developer Studio Project File - Name="DownloadDemon" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=DownloadDemon - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DownloadDemon.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DownloadDemon.mak" CFG="DownloadDemon - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DownloadDemon - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "DownloadDemon - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DownloadDemon - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "VC_EXTRALEAN"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 wininet.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "DownloadDemon - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL" /d "VC_EXTRALEAN"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wininet.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "DownloadDemon - Win32 Release"
# Name "DownloadDemon - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ButtonPush.cpp
# End Source File
# Begin Source File

SOURCE=.\Dialup.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgRegister.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgSettingsModem.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgStoppingNotice.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgStripExtensions.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgWaiver.cpp
# End Source File
# Begin Source File

SOURCE=.\DownloadDemon.cpp
# End Source File
# Begin Source File

SOURCE=.\DownloadDemon.rc
# End Source File
# Begin Source File

SOURCE=.\DownloadDemonDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DownloadDemonDropTarget.cpp
# End Source File
# Begin Source File

SOURCE=.\DownloadListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\Label.cpp
# End Source File
# Begin Source File

SOURCE=.\ModemConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\Registry.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ButtonPush.h
# End Source File
# Begin Source File

SOURCE=.\Dialup.h
# End Source File
# Begin Source File

SOURCE=.\DlgProperties.h
# End Source File
# Begin Source File

SOURCE=.\DlgRegister.h
# End Source File
# Begin Source File

SOURCE=.\DlgSettings.h
# End Source File
# Begin Source File

SOURCE=.\DlgSettingsModem.h
# End Source File
# Begin Source File

SOURCE=.\DlgStoppingNotice.h
# End Source File
# Begin Source File

SOURCE=.\DlgStripExtensions.h
# End Source File
# Begin Source File

SOURCE=.\DlgWaiver.h
# End Source File
# Begin Source File

SOURCE=.\DownloadDemon.h
# End Source File
# Begin Source File

SOURCE=.\DownloadDemonDlg.h
# End Source File
# Begin Source File

SOURCE=.\DownloadDemonDropTarget.h
# End Source File
# Begin Source File

SOURCE=.\DownloadListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\InetAuxFunctions.h
# End Source File
# Begin Source File

SOURCE=.\Label.h
# End Source File
# Begin Source File

SOURCE=.\ModemConnection.h
# End Source File
# Begin Source File

SOURCE=.\REGISTRY.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\RetrieveItem.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\RES\connect.bmp
# End Source File
# Begin Source File

SOURCE=.\res\DownloadDemon.ico
# End Source File
# Begin Source File

SOURCE=.\res\DownloadDemon.rc2
# End Source File
# Begin Source File

SOURCE=.\RES\failed.bmp
# End Source File
# Begin Source File

SOURCE=.\res\FileOpenErr.bmp
# End Source File
# Begin Source File

SOURCE=.\res\FileWriteErr.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\finished.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\looking.bmp
# End Source File
# Begin Source File

SOURCE=.\res\NotFound.bmp
# End Source File
# Begin Source File

SOURCE=.\res\skipped.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\stopped.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\timedout.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\transfer.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\wait.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
