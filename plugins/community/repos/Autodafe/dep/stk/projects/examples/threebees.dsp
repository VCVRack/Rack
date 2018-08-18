# Microsoft Developer Studio Project File - Name="threebees" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=threebees - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "threebees.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "threebees.mak" CFG="threebees - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "threebees - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "threebees - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "threebees - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "threebees___Win32_Release"
# PROP BASE Intermediate_Dir "threebees___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../../include" /I "../../src/include" /D "NDEBUG" /D "__WINDOWS_DS__" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__LITTLE_ENDIAN__" /D "__WINDOWS_MM__" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Wsock32.lib winmm.lib dsound.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "threebees - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "threebees___Win32_Debug"
# PROP BASE Intermediate_Dir "threebees___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../src/include" /D "_DEBUG" /D "__WINDOWS_DS__" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__LITTLE_ENDIAN__" /D "__WINDOWS_MM__" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Wsock32.lib winmm.lib dsound.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "threebees - Win32 Release"
# Name "threebees - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\ADSR.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\BeeThree.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Envelope.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\FileRead.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\FileWvIn.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\FM.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Messager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Mutex.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\RtAudio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\RtMidi.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\RtWvOut.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\SineWave.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\SKINI.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Socket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Stk.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\TcpServer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Thread.cpp
# End Source File
# Begin Source File

SOURCE=.\threebees.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\TwoZero.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Voicer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\FileLoop.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\ADSR.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BeeThree.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Envelope.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Filter.h
# End Source File
# Begin Source File

SOURCE=..\..\include\FM.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Instrmnt.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Messager.h
# End Source File
# Begin Source File

SOURCE=..\..\include\RtAudio.h
# End Source File
# Begin Source File

SOURCE=..\..\include\RtMidi.h
# End Source File
# Begin Source File

SOURCE=..\..\include\RtWvOut.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SKINI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Socket.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Stk.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Thread.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TwoZero.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Voicer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\FileLoop.h
# End Source File
# Begin Source File

SOURCE=..\..\include\WvIn.h
# End Source File
# Begin Source File

SOURCE=..\..\include\WvOut.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
