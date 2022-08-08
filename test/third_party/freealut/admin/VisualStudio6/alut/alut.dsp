# Microsoft Developer Studio Project File - Name="alut" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=alut - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "alut.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "alut.mak" CFG="alut - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "alut - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "alut - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "alut - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ALUT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\..\include" /I "C:\Program Files\OpenAL 1.1 with EFX SDK\include" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "ALUT_EXPORTS" /D "WIN32" /D "_MBCS" /D "ALUT_BUILD_LIBRARY" /D "HAVE__STAT" /D "HAVE_BASETSD_H" /D "HAVE_SLEEP" /D "HAVE_WINDOWS_H" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib openal32.lib /nologo /dll /machine:I386 /libpath:"C:\Program Files\OpenAL 1.1 with EFX SDK\libs\Win32"

!ELSEIF  "$(CFG)" == "alut - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ALUT_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\include" /I "C:\Program Files\OpenAL 1.1 with EFX SDK\include" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "ALUT_EXPORTS" /D "WIN32" /D "_MBCS" /D "ALUT_BUILD_LIBRARY" /D "HAVE__STAT" /D "HAVE_BASETSD_H" /D "HAVE_SLEEP" /D "HAVE_WINDOWS_H" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib openal32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"C:\Program Files\OpenAL 1.1 with EFX SDK\libs\Win32"

!ENDIF 

# Begin Target

# Name "alut - Win32 Release"
# Name "alut - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\alutBufferData.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\alutCodec.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\alutError.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\alutInit.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\alutInputStream.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\alutLoader.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\alutOutputStream.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\alutUtil.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\alutVersion.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\alutWaveform.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\src\alutInternal.h
# End Source File
# End Group
# Begin Group "ALUT Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\include\AL\alut.h
# End Source File
# End Group
# End Target
# End Project
