# Process this file with autoconf to produce a configure script.
AC_INIT(STK, 4.6.0, gary@music.mcgill.ca, stk)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(src/Stk.cpp)
AC_CONFIG_FILES(Makefile src/Makefile projects/demo/Makefile projects/effects/Makefile projects/ragamatic/Makefile projects/examples/Makefile projects/examples/libMakefile projects/eguitar/Makefile)

# Fill GXX with something before test.
AC_SUBST( GXX, ["no"] )

# Checks for programs.
AC_PROG_CXX(g++ CC c++ cxx)
AC_PROG_RANLIB
AC_PATH_PROG(AR, ar, no)
if [[ $AR = "no" ]] ; then
    AC_MSG_ERROR("Could not find ar - needed to create a library");
fi

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h sys/ioctl.h sys/time.h termio.h unistd.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_C_BIGENDIAN
AC_EGREP_CPP(yes,
[#ifndef WORDS_BIGENDIAN
  yes
#endif
], [AC_SUBST( byte_order, [-D__LITTLE_ENDIAN__] )] )

AC_MSG_CHECKING(for RAWWAVE_PATH argument)
AC_SUBST( rawwaves, $RAWWAVE_PATH )
AC_MSG_RESULT($RAWWAVE_PATH)

AC_MSG_CHECKING(for INCLUDE_PATH argument)
AC_SUBST( include, $INCLUDE_PATH )
AC_MSG_RESULT($INCLUDE_PATH)

# Check for realtime support disable
AC_MSG_CHECKING(whether to compile realtime support)
AC_ARG_ENABLE(realtime,
         [  --disable-realtime = only compile generic non-realtime classes],
         realtime=$enableval)
if test "$realtime" = "no"; then
   AC_SUBST( sound_api, [] )
else
   AC_SUBST( realtime, [yes] )
fi
AC_MSG_RESULT($realtime)

AC_MSG_CHECKING(whether to build the static library)
AC_ARG_ENABLE(static,
         [  --disable-static = do not compile static library ],
	 build_static=$enableval,
	 build_static=yes)
AC_SUBST(build_static)
AC_MSG_RESULT($build_static)

AC_MSG_CHECKING(whether to build the shared library)
AC_ARG_ENABLE(shared,
         [  --enable-shared = compile the shared library ],
	 build_shared=$enableval,
	 build_shared=no)
AC_SUBST(build_shared)
AC_MSG_RESULT($build_shared)

if test x$build_static = xno -a x$build_shared = xno ; then
        AC_MSG_ERROR([ both static and shared libraries are disabled], 1)
fi

# Check for math library
AC_CHECK_LIB(m, cos, , AC_MSG_ERROR(math library is needed!))

# Check for debug
AC_MSG_CHECKING(whether to compile debug version)
AC_ARG_ENABLE(debug,
        [  --enable-debug = enable various debug output],
        debug=$enableval)
if test "$debug" = "yes"; then
   AC_SUBST( cppflag, ["-D_STK_DEBUG_ -D__RTAUDIO_DEBUG__ -D__RTMIDI_DEBUG__"] )
   AC_SUBST( cxxflag, ["-g"] )
   AC_SUBST( object_path, [Debug] )
else
  AC_SUBST( debug, [no] )
  AC_SUBST( cppflag, [] )
  AC_SUBST( cxxflag, [-O3] )
  AC_SUBST( object_path, [Release] )
fi
AC_MSG_RESULT($debug)

# Checks for functions
if test $realtime = yes; then
  AC_CHECK_FUNCS(select socket)
  AC_CHECK_FUNC(gettimeofday, [cppflag="$cppflag -DHAVE_GETTIMEOFDAY"], )
fi

# For -I and -D flags
CPPFLAGS="$CPPFLAGS $cppflag"

# For debugging and optimization ... overwrite default because it has both -g and -O2
CXXFLAGS="$cxxflag"

# Check compiler and use -Wall if gnu.
if [test $GXX = "yes" ;] then
  AC_SUBST( cxxflag, [-Wall] )
fi

CXXFLAGS="$CXXFLAGS $cxxflag"

AC_CANONICAL_HOST

basesharedname="libstk-\$(RELEASE)"

AC_SUBST( sharedlib, ["libstk.so"] )
AC_SUBST( sharedname, [${basesharedname}.so] )
AC_SUBST( libflags, ["-shared -Wl,-soname,${basesharedname}.so -o $sharedname"] )
case $host in
  *-apple*)
  AC_SUBST( sharedlib, ["libstk.dylib"] )
  AC_SUBST( sharedname, ["${basesharedname}.dylib"] )
  AC_SUBST( libflags, ["-dynamiclib -o ${basesharedname}.dylib"] )
esac

if test $realtime = yes; then
  # Checks for package options and external software
  AC_MSG_CHECKING(for audio API)

  AC_ARG_WITH(jack, [  --with-jack = choose JACK server support (mac and linux only)])
  AS_IF([test "x$with_jack" == "xyes"], [
api="$api -D__UNIX_JACK__"
  AC_MSG_RESULT(using JACK)
  AC_CHECK_LIB(jack, jack_client_open, , AC_MSG_ERROR(JACK support requires the jack library!))])

  case $host in
    *-*-linux*)
    # Look for ALSA flag
    AC_ARG_WITH(alsa, [  --with-alsa = choose native ALSA API support (linux only)])
    AS_IF([test "x$with_alsa" == "xyes"], [
    api="$api -D__LINUX_ALSA__"
    AC_MSG_RESULT(using ALSA)
    AC_CHECK_LIB(asound, snd_pcm_open, , AC_MSG_ERROR(ALSA support requires the asound library!))])

    # Look for OSS flag
    AC_ARG_WITH(oss, [  --with-oss = choose OSS API support (unixes only)])
    AS_IF([test "x$with_oss" == "xyes"], [
    api="$api -D__LINUX_OSS__ -D__LINUX_ALSA__"
    AC_MSG_RESULT(using OSS)
    AC_CHECK_LIB(asound, snd_pcm_open, , AC_MSG_ERROR(OSS support requires ALSA for RtMidi!))])

    # If no audio api flags specified, use ALSA
    AS_IF([test "$api" == ""], [
      AC_MSG_RESULT(using ALSA)
      AC_SUBST( api, [-D__LINUX_ALSA__] )
      AC_CHECK_LIB(asound, snd_pcm_open, , AC_MSG_ERROR(ALSA support requires the asound library!))
    ])

    AC_CHECK_LIB(pthread, pthread_create, , AC_MSG_ERROR(RtAudio requires the pthread library!))
    ;;

    *-apple*)
    # Look for Core flag
    AC_ARG_WITH(core, [  --with-core = choose CoreAudio API support (mac only)])
    AS_IF([test "x$with_core" == "xyes"], [
    api="$api -D__MACOSX_CORE__"
    AC_MSG_RESULT(using CoreAudio)
    AC_CHECK_HEADER(CoreAudio/CoreAudio.h, [], [AC_MSG_ERROR(CoreAudio header files not found!)] )
    LIBS="$LIBS -framework CoreAudio -framework CoreFoundation -framework CoreMIDI" ])

    # If no audio api flags specified, use CoreAudio
    AS_IF([test "$api" == ""], [
      AC_SUBST( api, [-D__MACOSX_CORE__] )
      AC_MSG_RESULT(using CoreAudio)
      AC_CHECK_HEADER(CoreAudio/CoreAudio.h,
      [],
      [AC_MSG_ERROR(CoreAudio header files not found!)] )
      AC_SUBST( LIBS, ["-framework CoreAudio -framework CoreFoundation -framework CoreMIDI"] )
    ])

    AC_CHECK_LIB(pthread, pthread_create, , AC_MSG_ERROR(RtAudio requires the pthread library!))
    ;;

    *-mingw32*)
    AC_ARG_WITH(asio, [  --with-asio = choose ASIO API support (windoze only)])
    AS_IF([test "x$with_asio" == "xyes"], [
    api="$api -D__WINDOWS_ASIO__"
    AC_MSG_RESULT(using ASIO)
    AC_SUBST( objects, ["asio.o asiodrivers.o asiolist.o iasiothiscallresolver.o"] ) ])

    # Look for DirectSound flag
    AC_ARG_WITH(ds, [  --with-ds = choose DirectSound API support (windoze only)])
    AS_IF([test "x$with_ds" == "xyes"], [
    api="$api -D__WINDOWS_DS__"
    AC_MSG_RESULT(using DirectSound)
    LIBS="-ldsound $LIBS" ])

    # Look for WASAPI flag
    AC_ARG_WITH(wasapi, [  --with-wasapi = choose Windows Audio Session API support (windoze only)])
    AS_IF([test "x$with_wasapi" == "xyes"], [
    api="$api -D__WINDOWS_WASAPI__"
    AC_MSG_RESULT(using WASAPI)
    LIBS="-luuid -lksuser $LIBS" ])

    # If no audio api flags specified, use DirectSound
    AS_IF([test "$api" == ""], [
      AC_SUBST( api, [-D__WINDOWS_DS__] )
      AC_MSG_RESULT(using DirectSound)
      LIBS="-ldsound -lwinmm $LIBS"
    ])

    api="$api -D__WINDOWS_MM__"
    LIBS="-lole32 -lwinmm -lwsock32 $LIBS"
    ;;

    *)
    # Default case for unknown realtime systems.
    AC_MSG_ERROR(Unknown system type for realtime support ... try --disable-realtime argument!)
    ;;
  esac

CPPFLAGS="$CPPFLAGS $api"
fi

AC_OUTPUT
