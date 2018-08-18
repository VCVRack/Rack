# Microsoft Developer Studio Project File - Name="demo" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=demo - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "demo.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "demo.mak" CFG="demo - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "demo - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "demo - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "demo - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../../include" /D "NDEBUG" /D "__LITTLE_ENDIAN__" /D "__WINDOWS_MM__" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__WINDOWS_DS__" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dsound.lib winmm.lib Wsock32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "demo - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "__LITTLE_ENDIAN__" /D "__WINDOWS_MM__" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__WINDOWS_DS__" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dsound.lib winmm.lib Wsock32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "demo - Win32 Release"
# Name "demo - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\ADSR.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Asymp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\BandedWG.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\BeeThree.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\BiQuad.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\BlowBotl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\BlowHole.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Bowed.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Brass.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Clarinet.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Fir.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Delay.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DelayA.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\DelayL.cpp
# End Source File
# Begin Source File

SOURCE=.\demo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Drummer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Envelope.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\FileRead.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\FileWrite.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\FileWvIn.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\FileWvOut.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Flute.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\FM.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\FMVoices.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\FormSwep.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\HevyMetl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\JCRev.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Mandolin.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Mesh2D.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Messager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Modal.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\ModalBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Modulate.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Moog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Mutex.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Noise.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\NRev.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\OnePole.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\OneZero.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\PercFlut.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Phonemes.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Plucked.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Twang.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\PoleZero.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\PRCRev.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Resonate.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Rhodey.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\RtAudio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\RtMidi.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\RtWvIn.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\RtWvOut.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Sampler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Saxofony.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Shakers.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Simple.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\SineWave.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\SingWave.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Sitar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\SKINI.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Socket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Sphere.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\StifKarp.cpp
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

SOURCE=..\..\src\TubeBell.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\TwoPole.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\TwoZero.cpp
# End Source File
# Begin Source File

SOURCE=.\utilities.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Voicer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\VoicForm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\FileLoop.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Whistle.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Wurley.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\ADSR.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Asymp.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BandedWG.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BeeThree.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BiQuad.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BlowBotl.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BlowHole.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Bowed.h
# End Source File
# Begin Source File

SOURCE=..\..\include\BowTable.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Brass.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Clarinet.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Fir.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Delay.h
# End Source File
# Begin Source File

SOURCE=..\..\include\DelayA.h
# End Source File
# Begin Source File

SOURCE=..\..\include\DelayL.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Drummer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Effect.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Envelope.h
# End Source File
# Begin Source File

SOURCE=..\..\include\FileRead.h
# End Source File
# Begin Source File

SOURCE=..\..\include\FileWrite.h
# End Source File
# Begin Source File

SOURCE=..\..\include\FileWvIn.h
# End Source File
# Begin Source File

SOURCE=..\..\include\FileWvOut.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Filter.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Flute.h
# End Source File
# Begin Source File

SOURCE=..\..\include\FM.h
# End Source File
# Begin Source File

SOURCE=..\..\include\FMVoices.h
# End Source File
# Begin Source File

SOURCE=..\..\include\FormSwep.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Function.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Generator.h
# End Source File
# Begin Source File

SOURCE=..\..\include\HevyMetl.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Instrmnt.h
# End Source File
# Begin Source File

SOURCE=..\..\include\JCRev.h
# End Source File
# Begin Source File

SOURCE=..\..\include\JetTable.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Mandolin.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Mesh2D.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Messager.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Modal.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ModalBar.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Modulate.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Moog.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Mutex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Noise.h
# End Source File
# Begin Source File

SOURCE=..\..\include\NRev.h
# End Source File
# Begin Source File

SOURCE=..\..\include\OnePole.h
# End Source File
# Begin Source File

SOURCE=..\..\include\OneZero.h
# End Source File
# Begin Source File

SOURCE=..\..\include\PercFlut.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Phonemes.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Plucked.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Twang.h
# End Source File
# Begin Source File

SOURCE=..\..\include\PoleZero.h
# End Source File
# Begin Source File

SOURCE=..\..\include\PRCRev.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ReedTable.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Resonate.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Rhodey.h
# End Source File
# Begin Source File

SOURCE=..\..\include\RtAudio.h
# End Source File
# Begin Source File

SOURCE=..\..\include\RtMidi.h
# End Source File
# Begin Source File

SOURCE=..\..\include\RtWvIn.h
# End Source File
# Begin Source File

SOURCE=..\..\include\RtWvOut.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Sampler.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Saxofony.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Shakers.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Simple.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SineWave.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SingWave.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Sitar.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SKINI.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Socket.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Sphere.h
# End Source File
# Begin Source File

SOURCE=..\..\include\StifKarp.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Stk.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TcpServer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Thread.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TubeBell.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TwoPole.h
# End Source File
# Begin Source File

SOURCE=..\..\include\TwoZero.h
# End Source File
# Begin Source File

SOURCE=.\utilities.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Vector3D.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Voicer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\FileLoop.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Whistle.h
# End Source File
# Begin Source File

SOURCE=..\..\include\Wurley.h
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
