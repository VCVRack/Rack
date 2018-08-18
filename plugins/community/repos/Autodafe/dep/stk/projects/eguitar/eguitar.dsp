# Microsoft Developer Studio Project File - Name="eguitar" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=eguitar - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ragamat.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ragamat.mak" CFG="eguitar - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "eguitar - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "eguitar - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "eguitar - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir "release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__WINDOWS_DS__" /D "__LITTLE_ENDIAN__" /D "__WINDOWS_MM__" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Wsock32.lib dsound.lib winmm.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "eguitar - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__WINDOWS_DS__" /D "__LITTLE_ENDIAN__" /D "__WINDOWS_MM__" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Wsock32.lib dsound.lib winmm.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "eguitar - Win32 Release"
# Name "eguitar - Win32 Debug"
# Begin Source File

SOURCE=..\..\src\Fir.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Fir.h
# End Source File
# Begin Source File

SOURCE=..\..\src\Delay.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Delay.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DelayA.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\DelayA.h
# End Source File
# Begin Source File

SOURCE=..\..\src\DelayL.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\DelayL.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Effect.h
# End Source File
# Begin Source File

SOURCE=..\..\src\Twang.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Twang.h
# End Source File
# Begin Source File

SOURCE=..\..\src\Guitar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Guitar.h
# End Source File
# Begin Source File

SOURCE=..\..\src\FileRead.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\FileRead.h
# End Source File
# Begin Source File

SOURCE=..\..\src\FileWrite.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\FileWrite.h
# End Source File
# Begin Source File

SOURCE=..\..\src\FileWvOut.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\FileWvOut.h
# End Source File
# Begin Source File

SOURCE=..\..\src\FileWvIn.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\FileWvIn.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Filter.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Generator.h
# End Source File
# Begin Source File

SOURCE=..\..\src\JCRev.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\JCRev.h
# End Source File
# Begin Source File

SOURCE=..\..\src\Messager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Messager.h
# End Source File
# Begin Source File

SOURCE=..\..\src\Mutex.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Mutex.h
# End Source File
# Begin Source File

SOURCE=..\..\src\Noise.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Noise.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Cubic.h
# End Source File
# Begin Source File

SOURCE=..\..\src\OnePole.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\OnePole.h
# End Source File
# Begin Source File

SOURCE=.\eguitar.cpp
# End Source File
# Begin Source File

SOURCE=.\utilities.cpp
# End Source File
# Begin Source File

SOURCE=.\utilities.h
# End Source File
# Begin Source File

SOURCE=..\..\src\RtAudio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\RtAudio.h
# End Source File
# Begin Source File

SOURCE=..\..\src\RtMidi.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\RtMidi.h
# End Source File
# Begin Source File

SOURCE=..\..\src\SKINI.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\SKINI.h
# End Source File
# Begin Source File

SOURCE=..\..\src\Socket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Socket.h
# End Source File
# Begin Source File

SOURCE=..\..\src\Stk.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Stk.h
# End Source File
# Begin Source File

SOURCE=..\..\src\TcpServer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\TcpServer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\Thread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\include\Thread.h
# End Source File
# Begin Source File

SOURCE=..\..\include\WvIn.h
# End Source File
# Begin Source File

SOURCE=..\..\include\WvOut.h
# End Source File
# End Target
# End Project
