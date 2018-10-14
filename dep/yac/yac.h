/// ----
/// ---- file   : yac.h
/// ----
/// ---- author : (c) 2003 - 2018 by bsp
/// ----
/// ---- date   : 21-Nov-2003 /  8-Jun-2003 /  1-Aug-2003 / 24-Dec-2003 / 15-Jan-2004 /  9-Feb-2004 /
/// ----          17-Feb-2004 / 19-Feb-2004 / 28-Feb-2004 / 26-Mar-2004 / 27-Mar-2004 / 19-Sep-2004 /
/// ----          05-Nov-2004 / 06-Nov-2004 / 26-Dec-2004 /  9-Jan-2005 / 16-Jan-2005 / 05-Feb-2005 /
/// ----          14-Feb-2005 / 24-Feb-2005 / 27-Feb-2005 / 12-Mar-2005 / 13-Jun-2005 / 16-Jun-2005
/// ----          27-Jun-2005 / 22-Aug-2005 / 27-Aug-2005 / 05-Sep-2005 / 11-Sep-2005 / 28-Nov-2005
/// ----          13-Jan-2006 / 14-Jan-2006 / 04-Feb-2006 / 07-Feb-2006 / 25-Apr-2006 / 13-Mai-2006
/// ----          19-Jan-2008 / 31-Jan-2008 / 01-Feb-2008 / 04-Feb-2008 / 11-Feb-2008 / 18-Feb-2008
/// ----          24-Feb-2008 / 25-Feb-2008 / 28-Feb-2008 / 05-Mar-2008 / 22-Mar-2008 / 14-Sep-2008
/// ----          08-Jan-2009 / 10-Jan-2009 / 31-Jan-2009 / 05-Mar-2009 / 01-Apr-2009 / 03-Apr-2009
/// ----          17-Apr-2009 / 28-Apr-2009 / 04-May-2009 / 01-Jun-2009 / 22-Apr-2010 / 11-Jul-2010
/// ----          25-Sep-2010 / 01-Oct-2010 / 21-Apr-2011 / 21-Dec-2012 / 04-Jun-2013 / 13-Aug-2013
/// ----          24-Aug-2013 / 05-Feb-2014 / 11-Feb-2014 / 03-Mar-2014 / 07-Apr-2018 / 26-Jun-2018
/// ----
/// ---- info   : YAC - Yet Another Component object model.  YAC is a self contained, binary level
/// ----          C++ component/reflectance model and plugin SDK.
/// ----          YAC is accompanied by YInG- the YAC interface generator.
/// ----          YInG and YAC can be used to e.g. extend the TKScript language by new ADTs. 
/// ----          
/// ----          The YAC_Object and YAC_Host classes basically define a virtual table
/// ----          setup that has to be used on both host and plugin sides.
/// ----          
/// ----          By deriving an extension class from the YAC_Object class, the extension
/// ----          class inherits the YAC_Object virtual table and can now overwrite 
/// ----          vtable entries (e.g. by declaring a yacMethodGetAdr() method) so
/// ----          when the plugin host calls this vtable entry the yacMethodGetAdr() method
/// ----          of your extension class is actually called (instead of the 'stub' included
/// ----          in this header file)
/// ----
/// ----          Vice versa, the YAC_Host interface can be used to implement a new application
/// ----          host that handles the registration/instanciation of new YAC_Object classes.
/// ----          Usually, the TKScript engine takes care of this but you are free to implement
/// ----          your own plugin hosts (e.g. for testing/debugging new libraries).
/// ----
/// ---- ack    : thanks to Chaos^fr for his s* datatypes, limits, 
/// ----          sABS- macros and vector/matrix classes (included with the tkoldmath plugin).
/// ----
/// ---- license: MIT. See LICENSE.txt.
/// ---- 
/// ----

#ifndef __YAC_H__
#define __YAC_H__

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/// Define in the project/makefile to disable the use of thread local storage
/* #define YAC_FORCE_NO_TLS defined */
#define YAC_FORCE_NO_PRINTF_TLS defined  // avoid huge 128k TLS printf buffer

// ----
// ---- Version number
// ----
#define YAC_VERSION_MAJOR 916
#define YAC_VERSION_MINOR 2

// ----
// ---- Configuration
// ----
#ifndef YAC_PRINTF
/// ---- Define if you want a YAC_Host::printf() tool function
#define YAC_PRINTF defined
#endif

/// ---- Define if you want printf(), append(),... methods in the YAC_String class
#ifndef YAC_BIGSTRING
#define YAC_BIGSTRING defined
#endif // YAC_BIGSTRING

/// ---- Adds validation tag to YAC_Object class (yac_host must support this, too!!). 
/// ---- This makes it possible to detect double-free'd objects, not only zero-pointers.
#define YAC_OBJECT_TAGS defined 

/// ---- Adds static object_counter field to YAC_Object class
#define YAC_OBJECT_COUNTER defined

/// ---- Adds YAC interface to the YAC_Object class
#define YAC_OBJECT_YAC defined 

/// ---- Define to enable object pooling
#define YAC_OBJECT_POOL defined

/// ---- Define to use placement new to initialize pooled objects. If not defined, copy vtable* and call constructors. 
/// ---- Note: GCC does not allow C::C() so keep this defined if you use GCC!
#define YAC_PLACEMENT_NEW defined

/// ---- Define to use a custom global new/delete replacement
/// ---- This is mainly useful for debugging memory allocations.
/// ---- The global var "yac_global_newdelete_counter" tracks the total number of bytes currently allocated via
/// ---- the global new/delete operators. This also includes the total size of all ObjectPools, regardless of how many
/// ---- pool elements are actually in use. The counter also includes all memory allocated/freed in a plugin via new/delete.
/// ---- Please make sure that your plugin does not call new/delete before YAC_Init() has been called and the
/// ---- yac_host variable has been set.
/// ---- Please re-compile all plugins plus the YAC_Host if you change this setting.
/// ---- There might be issues with new/delete being called before yac_host is set so be careful.
//#define YAC_GLOBAL_NEWDELETE defined

/// ---- Define to define Dflt*_abs and Ddbl*_abs macros, helpful to epsilon compare float/double values
/// ---- Please define this in your plugin sources if you need it
//#define YAC_EPSILONCOMPARE_ABS defined

/// ---- Define to use absolute epsilon float comparisons by default (also requires YAC_EPSILONCOMPARE_ABS)
/// ---- Note: Only one of YAC_EPSILONCOMPARE_ABS_DEFAULT and YAC_EPSILONCOMPARE_REL_DEFAULT may be enabled
//#define YAC_EPSILONCOMPARE_ABS_DEFAULT defined

/// ---- Define to define Dflt*_rel and Ddbl*_rel macros, helpful to epsilon compare float/double values
//#define YAC_EPSILONCOMPARE_REL defined

/// ---- Define to use relative epsilon float comparisons by default (also requires YAC_EPSILONCOMPARE_REL)
//#define YAC_EPSILONCOMPARE_REL_DEFAULT defined


/// ---- Define to give YAC_TypeS, YAC_ValueS a vtable
/// ---- This is used to satisfy the Intel C++ compiler, i.e. to get no warnings 
/// ---- when classes with virtual methods are derived from this class/YAC_Value)
/// ---- If you change this define, make sure to recompile the host and all plugins
/// ---- since the member offsets will shift.
//#define YAC_VIRTUAL_VALUE defined 

//
//#define YAC_NO_EXPORTS defined 
//
/// ---- Define if you want to re-implement the YAC_Object YAC interface (custom YAC_Hosts)
//#define YAC_CUST_OBJECT defined 
//#define YAC_CUST_STRING defined        // define to skip declaration of the YAC_String and YAC_StringArray class.
//#define YAC_CUST_EVENT defined         // define to skip declaration of the YAC_Event class.
//#define YAC_CUST_VALUE defined         // define to skip declaration of the YAC_Value, 
                                         // YAC_ValueObject and YAC_ValueArray classes.
//#define YAC_CUST_NUMBEROBJECTS defined // define to skip the declaration of the YAC_Integer, YAC_Float, YAC_Byte... classes
//#define YAC_CUST_LISTNODE defined      // define to skip declaration of the YAC_ListNode class.
//#define YAC_CUST_TREENODE defined      // define to skip declaration of the YAC_TreeNode class.
//#define YAC_CUST_STREAMBASE defined    // define to skip declaration of the YAC_StreamBase class.
//#define YAC_CUST_BUFFER defined        // define to skip declaration of the YAC_Buffer class.
//#define YAC_CUST_INTARRAY defined      // define to skip declaration of the YAC_IntArray class.
//#define YAC_CUST_FLOATARRAY defined    // define to skip declaration of the YAC_FloatArray class.

//#define YAC_TRACK_CHARALLOC defined    // define to keep track of total amount of allocated string chars

// ----
// ----
// ---- capability flags for an object class ----
// ----
// ----
typedef enum __yac_interfaces {
   YAC_INTERFACE_REFLECTION    = (1<<0),
   YAC_INTERFACE_OPERATOR      = (1<<1),
   YAC_INTERFACE_STREAM        = (1<<2),
   YAC_INTERFACE_SERIALIZATION = (1<<3),
   YAC_INTERFACE_ITERATOR      = (1<<4),
   YAC_INTERFACE_ARRAY         = (1<<5),
   YAC_INTERFACE_METACLASS     = (1<<6),
   YAC_INTERFACE_SIGNALS       = (1<<7),
   YAC_INTERFACE_ALL           =  0xFF

} e_yac_interfaces;
// ----
// ----
// ---- capability flags for the application host ----
// ----
// ----
typedef enum __yac_host_interfaces {
   YAC_HOST_INTERFACE_REFLECTION   = (1<<0),
   YAC_HOST_INTERFACE_DEBUG        = (1<<1),
   YAC_HOST_INTERFACE_STRING       = (1<<2),
   YAC_HOST_INTERFACE_SCRIPTING    = (1<<3),
   YAC_HOST_INTERFACE_EVENT        = (1<<4),
   YAC_HOST_INTERFACE_EXCEPTION    = (1<<5),
   YAC_HOST_INTERFACE_CALLBACK     = (1<<6),
   YAC_HOST_INTERFACE_MUTEX        = (1<<7),
   YAC_HOST_INTERFACE_ALL          =  0xFF
} e_yac_host_interfaces;
// ----
// ----
// ---- OS detection 
// ----
// ----

// ---- Linux ?
#ifdef __linux__
#ifndef YAC_LINUX
  #define YAC_LINUX defined
#endif // YAC_LINUX
  #define YAC_POSIX defined
  #define YAC_OSNAME "linux"
#endif
// ---- Amiga OS ?
#ifdef AMIGA
#ifndef YAC_AMIGA
  #define YAC_AMIGA defined
#endif // YAC_AMIGA
  #define YAC_OSNAME "c= amiga"
#endif
// ---- QNX ?
#if defined(__QNXNTO__)
#ifndef YAC_QNX
  #define YAC_QNX defined
#endif // YAC_QNX
  #define YAC_OSNAME "qnx"
  #define YAC_POSIX defined
#endif
// ---- OS/2 ?
#ifdef OS2
#ifndef YAC_OS2
  #define YAC_OS2 defined
#endif // YAC_OS2
  #define YAC_OSNAME "os/2"
#endif
// ---- Win32 ?
#ifdef _WIN32
#ifndef WIN32
#define WIN32
#endif
#endif
#ifdef WIN32
#ifndef YAC_WIN32
  #define YAC_WIN32 defined
#endif // YAC_WIN32
  #define YAC_OSNAME "win32"
#ifdef _MSC_VER
  #define YAC_VC defined
#endif
#endif
// ---- MS/DOS ?
#ifdef MSDOS
#ifndef YAC_MSDOS
  #define YAC_MSDOS defined
#endif // YAC_MSDOS
  #define YAC_OSNAME "msdos (compatible)"
#endif
// ---- BSD ? (untested)
#ifdef BSD
#ifndef YAC_BSD
  #define YAC_BSD defined
#endif // YAC_BSD
  #define YAC_POSIX defined
  #define YAC_OSNAME "bsd"
#endif
// ---- MacOsX ? (untested)
#ifdef __APPLE__
#ifndef YAC_MACOSX
  #define YAC_MACOSX defined
#endif // YAC_MACOSX
  #define YAC_POSIX defined
  #define YAC_BIG_ENDIAN defined
  #define YAC_OSNAME "macosx"
#endif
// ---- NeXT ? (untested)
#ifdef NeXT
#ifndef YAC_NEXT
  #define YAC_NEXT defined
#endif // YAC_NEXT
  #define YAC_OSNAME "NeXT"
#endif
// ---- Unknown OS.. please report..
#if !defined(YAC_OSNAME)
  #ifdef __unix__
  #define YAC_UNIX defined
  #define YAC_POSIX defined
  #define YAC_OSNAME "unix"
  #else
    #ifdef SYSV
    #define YAC_UNIX defined
    #define YAC_POSIX defined
    #define YAC_OSNAME "sysv"
    #else
      #define YAC_OSNAME "unknown"
    #endif
  #endif
#endif

#ifdef __CYGWIN32__
#define YAC_CYGWIN defined
#undef YAC_FORCE_NO_TLS
#define YAC_FORCE_NO_TLS defined
#endif // __CYGWIN32__


// ---- C99 support
#ifdef __STDC_VERSION__ 
#if (__STDC_VERSION__ >= 199901L) 
#define YAC_C99 defined
#endif 
#endif 
// ---- GCC detection ----
#ifdef __GNUC__
#define YAC_GCC defined
#endif
// ---- endianess ----
#if !defined(YAC_BIG_ENDIAN) && !defined(YAC_LITTLE_ENDIAN)
#ifdef YAC_WIN32
#define YAC_LITTLE_ENDIAN defined
#else
#ifdef YAC_POSIX
  #include <endian.h>
  #if (__BYTE_ORDER == __BIG_ENDIAN)
    #define YAC_BIG_ENDIAN defined
  #else
    #define YAC_LITTLE_ENDIAN defined
  #endif
#else
// !!! "please define YAC_BIG_ENDIAN or YAC_LITTLE_ENDIAN! (falling back to YAC_LITTLE_ENDIAN)" !!!
#define YAC_LITTLE_ENDIAN defined
#endif
#endif
#endif
// ---- check whether we are running on (x86) 64bit ----
#ifdef __x86_64__
#define YAC_64
#else
#ifdef __AMD64__
#define YAC_64
#else
#ifdef __amd64__
#define YAC_64
#else
#ifdef __LP64__
#define YAC_64
#else
#ifdef _M_IA64
#define YAC_64
#else
#ifdef _M_X64
#define YAC_64
#endif
#endif
#endif
#endif
#endif
#endif
// ----
// ---- Calling convention for yac* methods and global functions
// ---- X86/64 uses a custom calling convention; (most?) c++ compilers
// ---- cannot handle __cdecl 64bit methods/functions.
// ----
#ifndef YAC_CALL
#ifdef YAC_64 
#define YAC_CALL
#else
#ifdef __GNUC__
#define YAC_CALL
#else
#define YAC_CALL __cdecl
#endif
#endif
#endif
/* // ---- YAC virtual method decorator */
#ifdef YAC_WIN32
//#define YAC_VCALL __cdecl
#define YAC_VCALL
#else
#define YAC_VCALL
#endif
// ----
// ----
// ---- determine way to export symbols
// ----
// ----
#ifndef YAC_NO_EXPORTS
#ifdef YAC_VC
 #define YAC_APIC extern "C" __declspec(dllexport)
 #define YAC_API __declspec(dllexport)
#else
 #define YAC_APIC extern "C"
 #define YAC_API
#endif
#else
#define YAC_APIC extern "C"
#define YAC_API
#endif
// ----
// ---- determine thread local storage attribute syntax
// ----
#ifdef YAC_VC
#define YAC_TLS __declspec(thread)
#endif
#ifdef YAC_GCC
#define YAC_TLS __thread
#endif

// Note: using TLS in .dlls is only allowed since Windows Vista. 
//       However, I noticed that static TLS arrays increase the .dll file size 
//       by the size of the array (!). e.g.: a 128kb buffer will increase the filesize
//       by 128kb. not even UPX crunching can solve that issue (but why?)
//       so, to keep the file size small and the .dll compatible with Windows XP and prior versions,
//       you should define "YAC_FORCE_NO_TLS" in plugin_msvc.mk
//       also see <http://msdn.microsoft.com/en-us/library/2s9wt68x.aspx>
//       
#ifdef YAC_FORCE_NO_TLS
#undef YAC_TLS
#define YAC_TLS
#endif // YAC_FORCE_NO_TLS


// ----
// ---- determine how to "restrict" pointer aliasing
// ----
#ifdef YAC_VC
#define YAC_RESTRICT __restrict
#else
#ifdef YAC_GCC
#define YAC_RESTRICT __restrict__
#else
#define YAC_RESTRICT 
#endif // YAC_GCC
#endif // YAC_VC


// ----
// ---- determine how to "inline" functions
// ----
#ifdef YAC_VC
#define YAC_INLINE __inline
#else
// GCC
#define YAC_INLINE static inline
#endif


// ----
// ---- Version string
// ----
#define YAC__X(a) # a
#define YAC__Y(a) YAC__X(a)
#define YAC__Z(a,b) a ## . ## b
#define YAC_VERSION_STRING YAC__Y(YAC__Z(YAC_VERSION_MAJOR, YAC_VERSION_MINOR)) 
#undef YAC__X
#undef YAC__Y
#undef YAC__Z

// ----
// ----
// ---- interface tags for YInG 
// ----
// ----
#define YF YAC_APIC
#define YC
#define YCS
#define YCR
#define YCI
#define YCF
#define YG(a)
#define YM
#define YP

// ----
// ---- The only reason to use a macro for "const" is that it 
// ---- eases C++ type/name parsing 
// ----  (Note: actual parsing would be required to distinguish e.g. "YAC_String *_constName" and "char *const _name",
// ----         hence this workaround)
// ---- 
#define YAC_CONST const

YG("core");

// ---- s[USF] types originally from Dierk Ohlerich <chaos@vcc.de> <chaos@ohlerich.org> (thanks) ----
// ----
// ----
// ---- type conventions / abbreviations for common C/C++ datatypes
// ----
// ----
typedef char                    sChar;
typedef unsigned char           sU8;
typedef signed char             sS8;
typedef unsigned short          sU16;
typedef signed short            sS16;
typedef unsigned int            sU32;
typedef signed int              sS32;
#ifdef YAC_VC
 typedef unsigned __int64        sU64;
 typedef signed __int64          sS64;
#else
 typedef unsigned long long      sU64;
 typedef signed long long        sS64;
#endif
typedef float                   sF32;
typedef double                  sF64; 
typedef unsigned int            sBool;
// ---- datatype limits ----
#define sU8_MIN    0x00
#define sU8_MAX    0xff
#define sS8_MIN   -0x80
#define sS8_MAX    0x7f
#define sU16_MIN   0x0000
#define sU16_MAX   0xffff
#define sS16_MIN  -0x8000
#define sS16_MAX   0x7fff
#define sU32_MIN   0x00000000
#define sU32_MAX   0xffffffff
#define sS32_MIN  -0x80000000
#define sS32_MAX   0x7fffffff
#define sU64_MIN   0x0000000000000000
#define sU64_MAX   0xffffffffffffffff
#define sS64_MIN  -0x8000000000000000
#define sS64_MAX   0x7fffffffffffffff
#define sINT_MIN  -0x80000000
#define sINT_MAX   0x7fffffff
#define sF32_MIN   1.2E-38f
#define sF32_MAX   3.4e+38f
#define SIZEOF_CHAR   sizeof(sChar)
#define SIZEOF_BYTE   1
#define SIZEOF_INT    4
#define SIZEOF_FLOAT  4
#define SIZEOF_DOUBLE 8
//#define SIZEOF_RINT   sizeof(void*)  // # of bytes / CPU register; sizeof(void*)
#define SIZEOF_RINT   4
// ---- void* size, # of bytes per CPU register ----
#if (SIZEOF_RINT) == 4
  typedef sU32 sUI;
  typedef sS32 sSI;
  #define RINT_SIZE_SHIFT 2
  #define sUI_MIN sU32_MIN
  #define sUI_MAX sU32_MAX
  #define sSI_MIN sS32_MIN
  #define sSI_MAX sS32_MAX
#else
  #if (SIZEOF_RINT) == 8
    typedef sU64 sUI;
    typedef sS64 sSI;
    #define RINT_SIZE_SHIFT 3
    #define sUI_MIN sU64_MIN
    #define sUI_MAX sU64_MAX
    #define sSI_MIN sS64_MIN
    #define sSI_MAX sS64_MAX
  #else
    #error "error: trying to compile with 16-bit ints (128bit?)."
  #endif
#endif

// ---- 'sBool' true/false constants ----
#define YAC_TRUE   (1)
#define YAC_FALSE  (0)

// ---- maximum iterator size, see yacIteratorInit()
#define YAC_MAX_ITERATOR_SIZE (sizeof(void*)*16)

// ---- float constants ----
#define YAC_FLOAT_DEVIATION  0.000001f
#define YAC_DOUBLE_DEVIATION 0.0000000001f

// ---- math constants ----
#define sM_E        2.7182818284590452353602874713526625
#define sM_LOG2E    1.4426950408889634073599246810018922
#define sM_LOG10E   0.4342944819032518276511289189166051
#define sM_LN2      0.6931471805599453094172321214581766
#define sM_LN10     2.3025850929940456840179914546843642
#define sM_PI       3.1415926535897932384626433832795029
#define sM_PI_2     1.5707963267948966192313216916397514
#define sM_PI_4     0.7853981633974483096156608458198757
#define sM_1_PI     0.3183098861837906715377675267450287
#define sM_2_PI     0.6366197723675813430755350534900574
#define sM_2_SQRTP  1.1283791670955125738961589031215452
#define sM_SQRT1_2  0.7071067811865475244008443621048490
#define sM_SQRT2    1.4142135623730950488016887242096981

// ---- math constants for use with 80bit doubles (taken from GNU math.h).
#define sM_El           2.7182818284590452353602874713526625L  /* e */
#define sM_LOG2El       1.4426950408889634073599246810018922L  /* log_2 e */
#define sM_LOG10El      0.4342944819032518276511289189166051L  /* log_10 e */
#define sM_LN2l         0.6931471805599453094172321214581766L  /* log_e 2 */
#define sM_LN10l        2.3025850929940456840179914546843642L  /* log_e 10 */
#define sM_PIl          3.1415926535897932384626433832795029L  /* pi */
#define sM_PI_2l        1.5707963267948966192313216916397514L  /* pi/2 */
#define sM_PI_4l        0.7853981633974483096156608458198757L  /* pi/4 */
#define sM_1_PIl        0.3183098861837906715377675267450287L  /* 1/pi */
#define sM_2_PIl        0.6366197723675813430755350534900574L  /* 2/pi */
#define sM_2_SQRTPIl    1.1283791670955125738961589031215452L  /* 2/sqrt(pi) */
#define sM_SQRT1_2l     0.7071067811865475244008443621048490L  /* 1/sqrt(2) */
#define sM_SQRT2l       1.4142135623730950488016887242096981L  /* sqrt(2) */

// ---- floating point endianess helpers ----
typedef union yac_ieee_float_u {

   sF32 f32;

#ifdef YAC_LITTLE_ENDIAN
   struct {
      sUI mantissa : 23;
      sUI exponent : 8;
      sUI sign     : 1;
   } le;

   struct {
      sUI sign     : 1;
      sUI exponent : 8;
      sUI mantissa : 23;
   } be;
#else
   struct {
      sUI sign     : 1;
      sUI exponent : 8;
      sUI mantissa : 23;
   } le;

   struct {
      sUI mantissa : 23;
      sUI exponent : 8;
      sUI sign     : 1;
   } be;
#endif // YAC_LITTLE_ENDIAN

   sU8 u8[4];
   sU64 u32;

} yac_ieee_float_t;

typedef union yac_ieee_double_u {

   sF64 f64;

#ifdef YAC_LITTLE_ENDIAN
   // (note) C only permits integer types in bitfields => split the mantissa
   struct {
      sUI mantissa1 : 26;
      sUI mantissa2 : 26;
      sUI exponent  : 11;
      sUI sign      : 1;
   } le;

   struct {
      sUI sign      : 1;
      sUI exponent  : 11;
      sUI mantissa1 : 26;
      sUI mantissa2 : 26;
   } be;
#else
   struct {
      sUI sign      : 1;
      sUI exponent  : 11;
      sUI mantissa1 : 26;
      sUI mantissa2 : 26;
   } le;

   struct {
      sUI mantissa1 : 26;
      sUI mantissa2 : 26;
      sUI exponent  : 11;
      sUI sign      : 1;
   } be;
#endif // YAC_LITTLE_ENDIAN

   sU8 u8[8];
   sU32 u64;

} yac_ieee_double_t;

// ---- useful macros ----
#define sMIN(a,b) (((a)>(b))?(b):(a))
#define sMAX(a,b) (((a)>(b))?(a):(b))
#define sSIGN(x) (((x)==0)?0:(((x)>0)?1:-1))
#define sABS(x) (((x)>0)?(x):-(x))
#define sRANGE(x,max,min) (((x)<(min))?(min):(((x)>(max))?(max):(x)))
#define sALIGN(x,a) (((x)+(a)-1)&~((a)-1))
#define sMAKE2(b,a)      (((a)<<16)+(b))                      // special macros to pack multiple numbers in one integer.
#define sMAKE4(d,c,b,a)  (((a)<<24)+((b)<<16)+((c)<<8)+(d))   // usefull for if() and switch() statements
#define sMAKEID(d,c,b,a) (((a)<<24)+((b)<<16)+((c)<<8)+(d))

// ---- prefix for printf %p
#ifdef YAC_WIN32
#define YAC_PRINTF_PTRPREFIX "0x"
#else
#define YAC_PRINTF_PTRPREFIX ""
#endif

// ---- handle types ----
typedef void * YAC_ModuleHandle;
typedef void * YAC_FunctionHandle;
typedef void * YAC_VariableHandle;
typedef void * YAC_ContextHandle;
typedef void * YAC_MutexHandle;

// ---- nameid types ----
typedef sUI    YAC_ExceptionId;
typedef sSI    YAC_CallbackId;

// ---- other anonymous pointer types ----
typedef void * YAC_CFunctionPtr;

typedef struct __YAC_PoolHandle {
   sUI pool_id;
   sUI object_id;
} YAC_PoolHandle;

// ---- object validation tags ----
#define YAC_VALID_TAG   0x900DF00D
#define YAC_INVALID_TAG 0xD34DBEEF

// ---- base classes and structures ----
class YAC_API YAC_Buffer;
class YAC_API YAC_Event;
class YAC_API YAC_Iterator;
class YAC_API YAC_Object;
class YAC_API YAC_String;
class YAC_API YAC_Value;

// ---- (untyped) memory  ----
typedef union _yacmem {
   sUI                ui;
   sSI                si;
   sF32               f4;
   YAC_Object        *o ;
   YAC_Object       **io;
   YAC_String        *s;
   void              *any;
} yacmem; //this has size 4byte on 32bit systems and 8byte on 64bit systems

typedef union _yacmem64 {
   sUI                ui4;
   sSI                si4;
   sU64               ui8;
   sS64               si8;
   sF32                f4;
   sF64                f8;
   YAC_Object         *o;
   YAC_Object        **io;
   YAC_String         *s;
   void               *any;
} yacmem64; //this is always 8byte (except on bigger than 64bit systems with larger ptr)

// ---- pointer to (untyped) memory ----
typedef union _yacmemptr {
   yacmem            *mem;
   void              *any;
   sChar             *c;
   sS8               *s1;
   sU8               *u1;
   sS16              *s2;
   sU16              *u2;
   sS32              *s4;
   sU32              *u4;
   sU64              *u8;
   sF32              *f4;
   sF64              *f8;
   sSI               *si;
   sUI               *ui;
   YAC_Object        *o;
   // ---- pointer to array of pointers ----
   void             **iany;
   sChar            **ic;
   sS8              **is1;
   sU8              **iu1;
   sS16             **is2;
   sU16             **iu2;
   sS32             **is4;
   sU32             **iu4;
   sF32             **if4;
   sSI              **isi;
   sUI              **iui;
   YAC_Object       **io;
   YAC_Object      ***iio;
   // ---- union with register int (32, 64bit) ----
   sUI                _ui;
   sSI                _si;
} yacmemptr;

// ---- 
// ---- YAC uses a custom RTTI. 
// ---- 
// ---- The following (absolute) class IDs are mapped to a set of core classes.
// ---- (The YAC_Host application host assigns dynamic class IDs to dynamically loaded classes)
// ---- 
// ---- 
enum __yac_class_IDs {
   YAC_CLID_OBJECT=0,        // 0: base class for all API classes; cannot be instanciated

   // ---- v a l u e   o  b j e c t s ----
   YAC_CLID_BOOLEAN,         // 1: object representation of a boolean (1bit)
   YAC_CLID_BYTE,            // 2: object representation of a signed byte (8bit)
   YAC_CLID_SHORT,           // 3: object representation of a signed short (16bit)
   YAC_CLID_INTEGER,         // 4: object representation of a 32bit integer 
   YAC_CLID_LONG,            // 5: object representation of a signed long long (64bit)
   YAC_CLID_UNSIGNEDBYTE,    // 6: object representation of an unsigned byte (8bit)
   YAC_CLID_UNSIGNEDSHORT,   // 7: object representation of an unsigned short (16bit)
   YAC_CLID_UNSIGNEDINTEGER, // 8: object representation of an unsigned int (32bit)
   YAC_CLID_UNSIGNEDLONG,    // 9: object representation of an unsigned long (64bit)
   YAC_CLID_FLOAT,           // 10: object representation of a 32bit float
   YAC_CLID_DOUBLE,          // 11: object representation of a 64bit double
   YAC_CLID_STRING,          // 12: a (buffered) char sequence (has special fastpaths)
   YAC_CLID_EVENT,	          // 13: a time-stamped String
   YAC_CLID_VALUE,           // 14: an int/float/String or Object value that is wrapped in an object
   YAC_CLID_LISTNODE,        // 15: a single node of a list (or a list header). derived from Value.
   YAC_CLID_TREENODE,        // 16: a single node of a tree (or a tree header). derived from Value.

   // ---- c o n t a i n e r    o b j e c t s ----
   YAC_CLID_CLASS,           // 17: base class for meta class objects (e.g. script class objects)
   YAC_CLID_LIST,            // 18: object container for a double linked list, see YAC_ListNode.
   YAC_CLID_INTARRAY,        // 19: an Array of ints, see yacArray*
   YAC_CLID_FLOATARRAY,      // 20: an Array of floats, see yacArray*
   YAC_CLID_STRINGARRAY,     // 21: an Array of YAC_Strings (all read-write)
   YAC_CLID_OBJECTARRAY,     // 22: an Array of (uniform) YAC_Objects (all read-write)
   YAC_CLID_CLASSARRAY,      // 23: an Array of meta class objects (all read-write)
   YAC_CLID_VALUEARRAY,      // 24: an Array of YAC_Values
   YAC_CLID_POINTERARRAY,    // 25: an Array of (mixed) YAC_Objects wrapped in YAC_Values
   YAC_CLID_HASHTABLE,       // 26: an associative Array, see yacHash*

   // ---- s t r e a m  s ----
   YAC_CLID_STREAM,          // 27: base class for Files und Buffers, see yacStream*, YAC_StreamBase
   YAC_CLID_STDINSTREAM,     // 28: standard input filestream, derived from YAC_StreamBase
   YAC_CLID_STDOUTSTREAM,    // 29: standard output filestream, derived from YAC_StreamBase
   YAC_CLID_STDERRSTREAM,    // 30: standard error filestream, derived from YAC_StreamBase
   YAC_CLID_BUFFER,          // 31: an Array of bytes, derived from YAC_StreamBase
   YAC_CLID_FILE,            // 32: used to access local filesystems, derived from YAC_StreamBase
   YAC_CLID_PAKFILE,         // 33: used to access virtual file systems, derived from YAC_StreamBase
   YAC_CLID_PIPE,

   // Note: Be careful with the preallocated String/array variants. 
   //       A buffer reallocation will waste the initial buffer memory (not leaked but it cannot be used for anything
   //       else as long as the object is alive!). This means: Choose the variant wisely!
   //       The good thing is that in conjunction with object pooling, it is possible to write code that
   //       works with *zero* malloc() calls. Without these variants, a malloc() would be necessary to allocate
   //       the actual buffer data even if the object is pooled.
   //       Finally, keep in mind that if you use static variables, the Object is not re-initialized the next
   //       time a function is called, i.e. the buffer will still be there and no malloc() will be necessary
   //       even without these variants.
   YAC_CLID_STRING8,         // 34: variation of String that comes with preallocated char buffer of 8 chars
   YAC_CLID_STRING16,        // 35: variation of String that comes with preallocated char buffer of 16 chars
   YAC_CLID_STRING32,        // 36: variation of String that comes with preallocated char buffer of 32 chars
   YAC_CLID_STRING64,        // 37: variation of String that comes with preallocated char buffer of 64 chars
   YAC_CLID_STRING128,       // 38: variation of String that comes with preallocated char buffer of 128 chars
   YAC_CLID_INTARRAY8,       // 39: variation of IntArray that comes with a preallocated buffer of 8 ints
   YAC_CLID_INTARRAY16,      // 40: variation of IntArray that comes with a preallocated buffer of 16 ints
   YAC_CLID_INTARRAY32,      // 41: variation of IntArray that comes with a preallocated buffer of 32 ints
   YAC_CLID_INTARRAY64,      // 42: variation of IntArray that comes with a preallocated buffer of 64 ints
   YAC_CLID_INTARRAY128,     // 43: variation of IntArray that comes with a preallocated buffer of 128 ints
   YAC_CLID_FLOATARRAY8,     // 44: variation of FloatArray that comes with a preallocated buffer of 8 floats
   YAC_CLID_FLOATARRAY16,    // 45: variation of FloatArray that comes with a preallocated buffer of 16 floats
   YAC_CLID_FLOATARRAY32,    // 46: variation of FloatArray that comes with a preallocated buffer of 32 floats
   YAC_CLID_FLOATARRAY64,    // 47: variation of FloatArray that comes with a preallocated buffer of 64 floats
   YAC_CLID_FLOATARRAY128,   // 48: variation of FloatArray that comes with a preallocated buffer of 128 floats
   
   YAC_NUM_CORE_CLASSES = 128
};

// ---- hard coded limits/"magic" constants ----
#define YAC_MAX_CLASSES  256         // maximum number of C++ classes
#define YAC_MAX_COMMANDS 64          // maximum number of C++ methods per class
#define YAC_LOSTKEY      ((sU32)-1)  // indicates that YAC_String hashkey needs to be recalc'd

// ---- YAC class types
#define YAC_CLASSTYPE_STATIC 0   // C++ class cannot be instantiated in scripts or native code
#define YAC_CLASSTYPE_NORMAL 1   // regular C++ class
#define YAC_CLASSTYPE_NOINST 2   // C++ class objects can only be instantiated by native code

// ---- basic datatype enumeration
#define YAC_TYPE_VOID     0  // not used
#define YAC_TYPE_VARIANT  0  // not defined (yet)
#define YAC_TYPE_INT      1  // currently 32bit integer (may become 64bit signed long long on 64bit architectures)
#define YAC_TYPE_FLOAT    2  // currently 32bit float (may become 64bit double on 64bit architectures)
#define YAC_TYPE_OBJECT   3  // pointer to Object (YAC_Object *)
#define YAC_TYPE_STRING   4  // only used in YAC_Value::type field, script engine hint

// ---- Hint flags for the YAC_Host to use different pools for different types of object allocations (e.g. short-lived vs. long-term)
// ---- YAC_NEW() should be used for seldomly deleted objects (i.e. to prevent pooling)
#define YAC_POOL_HINT_DEFAULT 0
#define YAC_POOL_HINT_TEMPORARY 1

// ---- Pool priorities
#define YAC_POOL_PRIORITY_LOW    0
#define YAC_POOL_PRIORITY_MEDIUM 1
#define YAC_POOL_PRIORITY_HIGH   2
#define YAC_NUM_POOL_PRIORITIES  3

#ifdef YAC_PLACEMENT_NEW
#include <stdlib.h>
#include <new>
#endif // YAC_PLACEMENT_NEW

#if !defined(RACK_PLUGIN) && !defined(RACK_HOST)

// ---- basic type structure (no con-/destructors) ----
class YAC_API YAC_TypeS { 
  public:
#ifdef YAC_VIRTUAL_VALUE
   inline virtual ~YAC_TypeS() { } // To satisfy the Intel C++ compiler :/ (i.e. to get no warnings when virtual classes are derived from this class/YAC_Value)
#endif // YAC_VIRTUAL_VALUE
 public:
  sUI type;       // YAC_TYPE_xxx 
  sSI class_type; // if type==YAC_TYPE_OBJECT then this field stores the internal object class type 
};

// ---- basic script value structure (no con-/destructors) ----
class YAC_API YAC_ValueS : public YAC_TypeS {
public:
	union __my_value {	
		sF32        float_val; 
		sSI         int_val; 
		YAC_Object *object_val; 
		YAC_String *string_val;
      void       *any;
	} value; 
	sUI deleteme; // ---- this flag is used to decide whether it is safe to delete the object
};

//
// How to allocate string characters
//
#ifdef YAC_TRACK_CHARALLOC
extern sSI yac_string_total_char_size;
extern sU8 *yac_string_alloc_chars(size_t _n);
extern void yac_string_free_chars(sU8 *_chars);
#define Dyacallocchars(n) yac_string_alloc_chars(n)
#define Dyacfreechars(n) yac_string_free_chars(n)
#else
#define Dyacallocchars(n) ((sU8*)::malloc(n))
#define Dyacfreechars(n) ::free(n)
#endif // YAC_TRACK_CHARALLOC

#ifndef YAC_CUST_VALUE
// ---- 
// ---- 
// ----  YAC_Value:
// ---- 
// ----       - stores an int/float or YAC_Object* 
// ----       - dynamic type
// ----       - track object pointer ownership
// ----       - used to return values from methods/functions 
// ---- 
// ---- 
// ---- 
class YAC_API YAC_Value : public YAC_ValueS {
public:
    YAC_Value            (void                      );
    ~YAC_Value           (                          );
    void initVoid        (void                      );
    void initInt         (sSI _i                    );
    void initFloat       (sF32 _f                   );
    void initString      (YAC_String *_s, sBool _del);
    void initNewString   (YAC_String *s             );
    void initObject      (YAC_Object *_o, sBool _del);
    void initNull        (void                      );
    void typecast        (sUI _type                 );
    void unsetFast       (void                      );  // delete object if it is volatile (deleteme flag)
    void unset           (void                      );  // delete object if it is volatile (deleteme flag), reset members to 0
    void operator =      (YAC_Value*_v              );
    void safeInitInt     (sSI _i                    );
    void safeInitFloat   (sF32 _f                   );
    void safeInitString  (YAC_String *_s, sBool _del);
    void safeInitObject  (YAC_Object *_o, sBool _new);
    void initEmptyString (void                      );
    void copyRef         (const YAC_Value *_o       );
};
#endif // YAC_CUST_VALUE

// ---- macros for the c++ interface ----
#define YAC_RETI(a)   _r->initInt((sSI)(a))
                             // the return value of an API function (to the hostengine) is passed in a YAC_Value *_r argument, e.g. getValue(YAC_Value *_r) { _r->initInt(42); }. lists and arrays can be returned using apprioriate objects (e.g. PointerArray)
#define YAC_RETF(a)   _r->initFloat((sF32)(a))
                             // return a float value, also see YAC_RETI
#define YAC_RETO(a,b) _r->initObject((YAC_Object*)(a),b)
                             // return an object. new objects have to be allocated with YAC_Host::yacNew(), also see YAC_RETI
#define YAC_RETS(a,b) _r->initString((YAC_String*)(a), b)                                                                                                 // return a string. new strings have to be allocated with YAC_Host::yacNew(), also see YAC_RETI
#define YAC_RETSC(a)  _r->initNewString((YAC_String*)(a))
#define YAC_H(a)      YAC_Object*YAC_VCALL yacNewObject(void);const sChar*YAC_VCALL yacClassName(void)
                             // ---- start an interface definition, e.g. YAC_H(MyClass);
#define YAC(a)    YAC_Object * YAC_VCALL yacNewObject         (void);\
   const sChar*  YAC_VCALL yacClassName                     (void);\
   sUI           YAC_VCALL yacMemberGetNum                  (void);\
   const char ** YAC_VCALL yacMemberGetNames                (void);\
	const sUI   * YAC_VCALL yacMemberGetTypes                (void);\
	const char ** YAC_VCALL yacMemberGetObjectTypes          (void);\
	const sU8  ** YAC_VCALL yacMemberGetOffsets              (void);\
	sUI           YAC_VCALL yacMethodGetNum                  (void);\
	const char ** YAC_VCALL yacMethodGetNames                (void);\
	const sUI   * YAC_VCALL yacMethodGetNumParameters        (void);\
	const sUI  ** YAC_VCALL yacMethodGetParameterTypes       (void);\
	const char*** YAC_VCALL yacMethodGetParameterObjectTypes (void);\
	const sUI   * YAC_VCALL yacMethodGetReturnTypes          (void);\
	const char ** YAC_VCALL yacMethodGetReturnObjectTypes    (void);\
	const void ** YAC_VCALL yacMethodGetAdr                  (void);\
	sUI           YAC_VCALL yacConstantGetNum                (void);\
	const char ** YAC_VCALL yacConstantGetNames              (void);\
	const sUI   * YAC_VCALL yacConstantGetTypes              (void);\
	yacmemptr     YAC_VCALL yacConstantGetValues             (void)

#define YAC_POOLED_H(a, p) \
   void         YAC_VCALL yacPoolInit        (YAC_Object *_this); \
   sUI          YAC_VCALL yacPoolGetSize     (void); \
   sUI          YAC_VCALL yacPoolGetPriority (void) { return p; } \
   void         YAC_VCALL yacFinalizeObject  (YAC_ContextHandle)

#define YAC_POOLED(a, p) YAC(a); YAC_POOLED_H(a, p)

#define YAC_C(a, b) YAC_Object *YAC_VCALL a::yacNewObject(void){YAC_Object*r=new a();r->class_ID=class_ID;return r;}const sChar*a::yacClassName(void){return b;}

#define YAC_C_CORE(a, b, c) YAC_Object *YAC_VCALL a::yacNewObject(void){YAC_Object*r=new a();r->class_ID=c;return r;}const sChar*a::yacClassName(void){return b;}


#ifdef YAC_PLACEMENT_NEW
#define YAC_C_POOLED__NEWOBJECT(a) \
   void YAC_VCALL a::yacPoolInit(YAC_Object *_this) { new(_this)a(); } 
#else
#define YAC_C_POOLED__NEWOBJECT(a) \
   void YAC_VCALL a::yacPoolInit(YAC_Object *_this) { *(void**)_this = *(void**)this; ((a*)_this)->a::a(); } 
#endif // YAC_PLACEMENT_NEW

#define YAC_C_POOLED(a, b) YAC_C(a, b) YAC_C_POOLED__NEWOBJECT(a) \
   void       YAC_VCALL a::yacFinalizeObject (YAC_ContextHandle) { this->a::~a(); } \
   sUI        YAC_VCALL a::yacPoolGetSize    (void)              { return sizeof(a); }

#define YAC_C_CORE_POOLED(a, b, c) YAC_C_CORE(a, b, c) YAC_C_POOLED__NEWOBJECT(a) \
   void       YAC_VCALL a::yacFinalizeObject (YAC_ContextHandle) { this->a::~a(); } \
   sUI        YAC_VCALL a::yacPoolGetSize    (void)              { return sizeof(a); }

#define YAC_CHK(a,b)  ((a)&&((a)->class_ID==b))
                       // ---- custom RTTI, used to verify Object arguments passed from scripts
#define YAC_BCHK(a,b) ((a)&&(yac_host->cpp_typecast_map[(a)->class_ID][b]))
                       // ---- custom RTTI, used to verify if Object is baseclass of another Object, also to validate script arguments

// For regular class names (e.g. MyClass)
#define YAC_NEW(a) (a*)yac_host->yacNewByID(clid_##a)

// For mangled class name (e.g. _MyClass)
#define YAC_NEW_(a) (_##a*)yac_host->yacNewByID(clid_##a)

// Note: do not pass stack objects to other yac* methods (object may have different vtable!)
#define YAC_NEW_STACK(t, v) _##t v; v.class_ID = clid_##t

#define YAC_NEW_CORE(c) yac_host->yacNewByID(c)

#define YAC_NEW_POOLED(a) (_##a*)yac_host->yacNewPooledByID(clid_##a, YAC_POOL_HINT_DEFAULT)

#define YAC_NEW_TEMP(a) (_##a*)yac_host->yacNewPooledByID(clid_##a, YAC_POOL_HINT_TEMPORARY)

#define YAC_NEW_CORE_POOLED(c) yac_host->yacNewPooledByID(c, YAC_POOL_HINT_DEFAULT)

#define YAC_NEW_CORE_TEMP(c) yac_host->yacNewPooledByID(c, YAC_POOL_HINT_TEMPORARY)

// ---- Cloning creates a new object and calls yacOperatorInit. 
#define YAC_CLONE_POOLED(x, a) (a)->yacNewPooled((YAC_ContextHandle)x, YAC_POOL_HINT_DEFAULT)

#define YAC_CLONE_TEMP(x, a) (a)->yacNewPooled((YAC_ContextHandle)x, YAC_POOL_HINT_TEMPORARY)

#define YAC_DELETE(a) yac_host->yacDelete(a)

#define YAC_DELETE_SAFE(a) if(NULL != a) { YAC_DELETE(a); a = NULL; } else (void)0

// ---- operation codes for YAC_Object::yacOperator() ----
enum __yacoperators {
    YAC_OP_ASSIGN=0, // ---- also see YAC_Object::yacOperator()
    YAC_OP_ADD,
    YAC_OP_SUB,
    YAC_OP_MUL,
    YAC_OP_DIV,
    YAC_OP_MOD,
    YAC_OP_SHL,
    YAC_OP_SHR,
    YAC_OP_CEQ,
    YAC_OP_CNE,
    YAC_OP_CLE,
    YAC_OP_CLT,
    YAC_OP_CGE,
    YAC_OP_CGT,
    YAC_OP_AND,
    YAC_OP_OR,
    YAC_OP_EOR,
    YAC_OP_NOT,
    YAC_OP_BITNOT, // iinv
    YAC_OP_LAND,
    YAC_OP_LOR,
    YAC_OP_LEOR,
    YAC_OP_NEG,
    YAC_OP_INIT, // init class, call constructors (or put on cs list)
    YAC_NUMOPERATORS
};

// ---- double arg operator priorities (sUI YAC_Object::yacOperatorPriority(void))
#define YAC_OP_PRIO_BOOLEAN  0x00001000
#define YAC_OP_PRIO_BYTE     0x00002000
#define YAC_OP_PRIO_SHORT    0x00003000
#define YAC_OP_PRIO_INTEGER  0x00004000
#define YAC_OP_PRIO_LONG     0x00005000
#define YAC_OP_PRIO_FLOAT    0x00006000
#define YAC_OP_PRIO_DOUBLE   0x00007000
#define YAC_OP_PRIO_STRING   0x00010000

// ---- Return values for yacTensorRank()
#define YAC_TENSOR_RANK_NONE  -2 // Definitely not a math object, e.g. Time
#define YAC_TENSOR_RANK_VAR   -1 // Some object's tensor rank can be variant resp. interpreted differently, e.g. String, ListNode, ValueObject
#define YAC_TENSOR_RANK_SCALAR 0 // Integer, Float, Double, ..
#define YAC_TENSOR_RANK_VECTOR 1 // IntArray, FloatArray, ObjectArray, ..
#define YAC_TENSOR_RANK_MATRIX 2 // e.g. tkmath::Matrix3f

// ---- all plugin class(es) have to be derived from this virtual interface class ----
YCR class YAC_API YAC_Object {
public:
   sUI class_ID;                 /// ---- set by YAC_Host::yacRegisterClass()

#ifdef YAC_OBJECT_TAGS
   sUI validation_tag;           /// ---- YAC_VALID_TAG ord YAC_INVALID_TAG
#endif

#ifdef YAC_OBJECT_POOL
   YAC_PoolHandle pool_handle;   /// ---- != 0 if the object was allocated from a pool
#endif 

#ifdef YAC_OBJECT_COUNTER
   static sUI object_counter;    /// ---- tracks the total number of objects
#endif

#ifdef YAC_PLACEMENT_NEW
public:
   void *operator new    (size_t _size)        { return ::malloc(_size); }
   void  operator delete (void *_ptr)          { 
      if(((YAC_Object*)_ptr)->pool_handle.pool_id)
      {
         ::printf("[---] delete: object is pooled (handle=%08x:%08x)!!\n", 
                  ((YAC_Object*)_ptr)->pool_handle.pool_id,
                  ((YAC_Object*)_ptr)->pool_handle.object_id
                  ); 
      }
      else 
      {
         ::free(_ptr); 
      }
   }

   void *operator new    (size_t, void *_this) { return _this; }
#endif // YAC_PLACEMENT_NEW
public:

   // ---- 
   // ---- 
   // ---- LEVEL (1<<0) interface 
   // ----                    ( C++ reflectance support )
   // ---- 
   // ---- 
                           YAC_Object                      (void);
  virtual                  ~YAC_Object                     ();
  virtual sUI              YAC_VCALL yacQueryInterfaces              (void);
  virtual const sChar     *YAC_VCALL yacClassName                    (void);                                  // get human readable class name
  virtual YAC_Object      *YAC_VCALL yacNewObject                    (void);                                  // create new instance of object class, also see non-virtual yacNew(). autogenerated by YAC_H(), YAC_C() macros
  virtual sUI              YAC_VCALL yacMemberGetNum                 (void);
  virtual const char     **YAC_VCALL yacMemberGetNames               (void);
  virtual const sUI       *YAC_VCALL yacMemberGetTypes               (void);
  virtual const char     **YAC_VCALL yacMemberGetObjectTypes         (void);
  virtual const sU8      **YAC_VCALL yacMemberGetOffsets             (void);
  virtual sUI              YAC_VCALL yacMethodGetNum                 (void);
  virtual const char     **YAC_VCALL yacMethodGetNames               (void);
  virtual const sUI       *YAC_VCALL yacMethodGetNumParameters       (void);
  virtual const sUI      **YAC_VCALL yacMethodGetParameterTypes      (void);
  virtual const char    ***YAC_VCALL yacMethodGetParameterObjectTypes(void);
  virtual const sUI       *YAC_VCALL yacMethodGetReturnTypes         (void);
  virtual const char     **YAC_VCALL yacMethodGetReturnObjectTypes   (void);
  virtual const void     **YAC_VCALL yacMethodGetAdr                 (void); 
  virtual sUI              YAC_VCALL yacConstantGetNum               (void);
  virtual const char     **YAC_VCALL yacConstantGetNames             (void);
  virtual const sUI       *YAC_VCALL yacConstantGetTypes             (void);
  virtual yacmemptr        YAC_VCALL yacConstantGetValues            (void);
  virtual void             YAC_VCALL yacFinalizeObject               (YAC_ContextHandle _context);
  virtual void             YAC_VCALL yacGetConstantStringList        (YAC_String *);      // !!WILL BE REMOVED SOON!! write constant list to _retstring. example: "CONST_A:42 CONST_B:77 CONST_C:$7f"
  virtual sBool            YAC_VCALL yacIsComposite                  (void); // return true if object stores references to other objects, used for serialization
  virtual sUI              YAC_VCALL yacPoolGetSize                  (void); // return 0 to prevent pooling for this class type (default)
  virtual void             YAC_VCALL yacPoolInit                     (YAC_Object *_this); // initialize new object (install vtable, call constructors)
  virtual sUI              YAC_VCALL yacPoolGetPriority              (void); // return pool priority for this class type. see YAC_POOL_PRIORITY_xxx
  virtual void             YAC_VCALL vtable_entry_0_25_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_0_26_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_0_27_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_0_28_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_0_29_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_0_30_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_0_31_reserved      (void);

   // ---- 
   // ---- 
   // ---- LEVEL (1<<1) interface 
   // ----                    ( operator support )
   // ----                        - also see YAC_OP_xxx constants
   // ---- 
   // ---- 
  virtual sBool            YAC_VCALL yacCopy                         (YAC_Object *_o);                                // copy from other object
  virtual sBool            YAC_VCALL yacEquals                       (YAC_Object *_o);                                // compare objects
  virtual void             YAC_VCALL yacOperator                     (sSI _cmd, YAC_Object *_robj, YAC_Value *_r);    // call operator (see YAC_OP_)
  virtual void             YAC_VCALL yacOperatorInit                 (void *_context, YAC_Object *_robj);             // used to initialize a class-like YAC_Object by using a template
  virtual void             YAC_VCALL yacOperatorAssign               (YAC_Object *_robj);                             // assign YAC_Object _robj (i.e. copy the members)
  virtual void             YAC_VCALL yacOperatorAdd                  (YAC_Object *_robj);                             // add YAC_Object _robj
  virtual void             YAC_VCALL yacOperatorSub                  (YAC_Object *_robj);                             // subtract YAC_Object _robj
  virtual void             YAC_VCALL yacOperatorMul                  (YAC_Object *_robj);                             // multiply with YAC_Object _robj
  virtual void             YAC_VCALL yacOperatorDiv                  (YAC_Object *_robj);                             // divide by YAC_Object _robj
  virtual void             YAC_VCALL yacOperatorClamp                (YAC_Object *_min, YAC_Object *_max);            // clamp object to boundaries [_min, _max] (e.g. Vector)
  virtual void             YAC_VCALL yacOperatorWrap                 (YAC_Object *_min, YAC_Object *_max);            // wrap object around boundaries [_min, _max] (e.g. Vector)
  virtual sBool            YAC_VCALL yacScanI                        (sSI *_ip);                                      // convert object to integer
  virtual sBool            YAC_VCALL yacScanF32                      (sF32 *_fp);                                     // convert object to 32bit IEEE float
  virtual sBool            YAC_VCALL yacScanF64                      (sF64 *_dp);                                     // convert object to 64bit IEEE double
  virtual sBool            YAC_VCALL yacToString                     (YAC_String *s) const;                           // convert object to String object
  virtual sBool            YAC_VCALL yacScanI64                      (sS64 *_lip);                                    // convert object to long or long long integer (platform dependent)
  virtual void             YAC_VCALL yacOperatorI                    (sSI _cmd, sSI _val, YAC_Value *_r);             // operate on object with integer argument
  virtual void             YAC_VCALL yacOperatorF32                  (sSI _cmd, sF32 _val, YAC_Value *_r);            // operate on object with 32bit IEEE float argument
  virtual void             YAC_VCALL yacOperatorF64                  (sSI _cmd, sF64 _val, YAC_Value *_r);            // operato on object with 64bit IEEE double argument
  virtual void             YAC_VCALL yacValueOfI                     (sSI);                                           // set object value by integer argument
  virtual void             YAC_VCALL yacValueOfF32                   (sF32);                                          // set object value by 32bit IEEE float argument
  virtual void             YAC_VCALL yacValueOfF64                   (sF64);                                          // set object value by 64bit IEEE double argument
  virtual sUI              YAC_VCALL yacOperatorPriority             (void);                                          // query priority of object in double-arg expressions (for type conversions)
  virtual void             YAC_VCALL yacValueOfI64                   (sS64);                                          // set object value by 64bit long long integer
  virtual sSI              YAC_VCALL yacTensorRank                   (void);                                          // query tensor rank of math object, -1=not a math object, 0=scalar, 1=vector, 2=matrix, ..
  virtual sBool            YAC_VCALL yacToParsableString             (YAC_String *s) const;                           // convert object to script-parsable String object
  virtual void             YAC_VCALL yacOperatorI64                  (sSI _cmd, sS64 _val, YAC_Value *_r);            // operate on object with 64bit integer argument
  virtual void             YAC_VCALL vtable_entry_1_28_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_1_29_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_1_30_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_1_31_reserved      (void);

   // ---- 
   // ---- 
   // ---- LEVEL (1<<2) interface 
   // ----                    ( stream support )
   // ---- 
   // ---- 
   // ---- 
  virtual sBool            YAC_VCALL yacIsStream                     (void);                                                 // test if the object supports the stream interface
  virtual void             YAC_VCALL yacStreamClose                  (void);                                                 // close previously opened stream
  virtual sBool            YAC_VCALL yacStreamOpenLocal              (sChar *_local_pathname, sSI _access);                  // open local filestream; _access must be YAC_IOS_IN, YAC_IOS_IN or YAC_IOS_IN. return true (=ok) or false (=error)
  virtual sBool            YAC_VCALL yacStreamOpenLogic              (sChar *_name_in_pakfile);                              // open filestream stored in a "pak" file. return true (=ok) or false (=error)
  virtual sUI              YAC_VCALL yacStreamGetByteOrder           (void);                                                 // get byteorder of stream (YAC_LITTLEENDIAN, YAC_BIGENDIAN)
  virtual void             YAC_VCALL yacStreamSetByteOrder           (sUI);                                                  // set byteorder of stream (YAC_LITTLEENDIAN, YAC_BIGENDIAN)
  virtual sBool            YAC_VCALL yacStreamEOF                    (void);                                                 // return 1 (true) if the stream end has been reached
  virtual void             YAC_VCALL yacStreamSeek                   (sSI _off, sUI _mode);                                  // seek to a position in the stream. _mode must be one of YAC_BEG (absolute seek), YAC_CUR (add offset to current position), YAC_END (seek relative to stream end, if available)
  virtual sUI              YAC_VCALL yacStreamGetOffset              (void);                                                 // get current stream position (byte offset)
  virtual void             YAC_VCALL yacStreamSetOffset              (sUI);                                                  // set current stream position (byte offset)
  virtual sUI              YAC_VCALL yacStreamGetSize                (void);                                                 // get size of stream (if known)
  virtual sSI              YAC_VCALL yacStreamRead                   (sU8 *, sUI _num);                                      // read _num bytes to buffer, return number of bytes actually read
  virtual sU8              YAC_VCALL yacStreamReadI8                 (void);                                                 // read a single byte
  virtual sU16             YAC_VCALL yacStreamReadI16                (void);                                                 // read a single word, convert endianess from stream to host endianess
  virtual sU32             YAC_VCALL yacStreamReadI32                (void);                                                 // read double word, convert endianess from stream to host endianess
  virtual sF32             YAC_VCALL yacStreamReadF32                (void);                                                 // read standard IEEE 32bit float
  virtual void             YAC_VCALL yacStreamReadObject             (YAC_Object *_dest);                                    // deserialize object from stream
  virtual sSI              YAC_VCALL yacStreamReadString             (YAC_String *_dest, sUI _maxlen);                       // read up to _maxlen bytes to _dest (until ASCIIZ is found)
  virtual sSI              YAC_VCALL yacStreamReadBuffer             (YAC_Buffer *_dest, sUI _off, sUI _num, sBool _resize); // read _num bytes into buffer starting at buffer offset _off. resize buffer if necessary and _resize==true. return number of bytes actually read
  virtual sSI              YAC_VCALL yacStreamReadLine               (YAC_String *_s, sUI _maxlen);
  virtual sSI              YAC_VCALL yacStreamWrite                  (sU8 *, sUI _num);                                      // write _num bytes from _buf into stream. return number of bytes actually written.
  virtual void             YAC_VCALL yacStreamWriteI8                (sU8);                                                  // write a single byte
  virtual void             YAC_VCALL yacStreamWriteI16               (sU16);                                                 // write a single word
  virtual void             YAC_VCALL yacStreamWriteI32               (sS32);                                                 // write a single double word
  virtual void             YAC_VCALL yacStreamWriteF32               (sF32);                                                 // write a standard IEEE 32bit float
  virtual void             YAC_VCALL yacStreamWriteObject            (YAC_Object*);                                          // serialize object into stream
  virtual sSI              YAC_VCALL yacStreamWriteString            (YAC_String *, sUI _off, sUI _num);                     // write _num chars from string starting at string offset _off into stream. return number of chars (bytes) actually written.
  virtual sSI              YAC_VCALL yacStreamWriteBuffer            (YAC_Buffer *_buf, sUI _off, sUI _num);                 // write _num bytes from _buf starting at buffer offset _off into stream. return number of bytes actually written.
  virtual sSI              YAC_VCALL yacStreamGetErrorCode           (void);                                                 // return last error code (or 0==no error)
  virtual void             YAC_VCALL yacStreamGetErrorStringByCode   (sSI _code, YAC_Value *_r);                             // convert last error code to human readable string
  virtual sF64             YAC_VCALL yacStreamReadF64                (void);                                                 // read standard IEEE 64bit double
  virtual void             YAC_VCALL yacStreamWriteF64               (sF64);                                                 // write a standard IEEE 64bit double
  virtual sU64             YAC_VCALL yacStreamReadI64                (void);                                                 // read 64bit signed long long
  virtual void             YAC_VCALL yacStreamWriteI64               (sS64);                                                 // write 64bit signed long long
  virtual void             YAC_VCALL vtable_entry_2_35_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_2_36_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_2_37_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_2_38_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_2_39_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_2_40_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_2_41_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_2_42_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_2_43_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_2_44_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_2_45_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_2_46_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_2_47_reserved      (void);

   // ---- 
   // ---- 
   // ---- LEVEL (1<<3) interface 
   // ----                    ( serialization support )
   // ---- 
   // ---- 
   // ---- 
  virtual void             YAC_VCALL yacSerializeClassName           (YAC_Object *_ofs);                      // write pascal style string (sU32 len + string chars) to stream _ofs. may differ from yacClassName()
  virtual void             YAC_VCALL yacSerialize                    (YAC_Object *_ofs, sUI _usetypeinfo);    // serialize object into _ofs stream
  virtual sUI              YAC_VCALL yacDeserialize                  (YAC_Object *_ifs, sUI _usetypeinfo);    // deserialize object from _ifs stream
  virtual void             YAC_VCALL vtable_entry_3_3_reserved       (void);
  virtual void             YAC_VCALL vtable_entry_3_4_reserved       (void);
  virtual void             YAC_VCALL vtable_entry_3_5_reserved       (void);
  virtual void             YAC_VCALL vtable_entry_3_6_reserved       (void);
  virtual void             YAC_VCALL vtable_entry_3_7_reserved       (void);
  virtual void             YAC_VCALL vtable_entry_3_8_reserved       (void);
  virtual void             YAC_VCALL vtable_entry_3_9_reserved       (void);
  virtual void             YAC_VCALL vtable_entry_3_10_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_3_11_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_3_12_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_3_13_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_3_14_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_3_15_reserved      (void);

   // ---- 
   // ---- 
   // ---- LEVEL (1<<4) interface 
   // ----                    ( iterator support )
   // ---- 
   // ---- 
   // ---- 
  virtual sBool            YAC_VCALL yacIteratorInit                 (YAC_Iterator *) const;                  // initialize iterator for a container-like object. The maximum size of an iterator is YAC_MAX_ITERATOR_SIZE bytes.
  virtual void             YAC_VCALL vtable_entry_4_1_reserved       (void);
  virtual void             YAC_VCALL vtable_entry_4_2_reserved       (void);
  virtual void             YAC_VCALL vtable_entry_4_3_reserved       (void);
  virtual void             YAC_VCALL vtable_entry_4_4_reserved       (void);
  virtual void             YAC_VCALL vtable_entry_4_5_reserved       (void);
  virtual void             YAC_VCALL vtable_entry_4_6_reserved       (void);
  virtual void             YAC_VCALL vtable_entry_4_7_reserved       (void);
  virtual void             YAC_VCALL vtable_entry_4_8_reserved       (void);
  virtual void             YAC_VCALL vtable_entry_4_9_reserved       (void);
  virtual void             YAC_VCALL vtable_entry_4_10_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_4_11_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_4_12_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_4_13_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_4_14_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_4_15_reserved      (void);

   // ---- 
   // ---- 
   // ---- LEVEL (1<<5) interface 
   // ----                    ( array and hashtable support )
   // ---- 
   // ---- 
   // ---- 
  virtual YAC_Object      *YAC_VCALL yacArrayNew                     (void);                                              // return array class instance for "scalar" type. e.g. return yac_host->yacNew("MyClassArray");
  virtual sUI              YAC_VCALL yacArrayAlloc                   (sUI _sx, sUI _sy=0, sUI _type=0, sUI _elementbytesize=0); // allocate new array elements
  virtual sUI              YAC_VCALL yacArrayRealloc                 (sUI _sx, sUI _sy=0, sUI _type=0, sUI _elementbytesize=0); // re-allocate new array elements
  virtual sUI              YAC_VCALL yacArrayGetNumElements          (void);                                              // return number of used elements in array
  virtual sUI              YAC_VCALL yacArrayGetMaxElements          (void);                                              // return maximum array size (buffer size)
  virtual void             YAC_VCALL yacArrayCopySize                (YAC_Object *_arrayobject);                          // copy size of other array object (used to construct arrays from default objects, e.g. a class template)
  virtual void             YAC_VCALL yacArraySet                     (void *_context, sUI _index, YAC_Value *_value);     // set an array value
  virtual void             YAC_VCALL yacArrayGet                     (void *_context, sUI _index, YAC_Value *_r);         // get an array value (references only)
  virtual sUI              YAC_VCALL yacArrayGetWidth                (void);                                              // get width of array object (x maxElements)
  virtual sUI              YAC_VCALL yacArrayGetHeight               (void);                                              // get height of array object (y maxElements)
  virtual sUI              YAC_VCALL yacArrayGetElementType          (void);                                              // 0=no array,1=int,2=float,3=object,4=string
  virtual sUI              YAC_VCALL yacArrayGetElementByteSize      (void);                                              // # of bytes per element
  virtual sUI              YAC_VCALL yacArrayGetStride               (void);                                              // # of bytes to next row
  virtual void            *YAC_VCALL yacArrayGetPointer              (void);                                              // get pointer to first element of array object
  virtual void             YAC_VCALL yacArraySetWidth                (sUI);                                               // set # of used elements
  virtual void             YAC_VCALL yacArraySetTemplate             (YAC_Object *);                                      // set template for yacArrayAlloc(). Used for object and class arrays.
  virtual void             YAC_VCALL yacArrayGetDeref                (void *_context, sUI _index, YAC_Value *_r);         // get/unlink value from array
  virtual void             YAC_VCALL vtable_entry_5_17_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_18_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_19_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_20_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_21_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_22_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_23_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_24_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_25_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_26_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_27_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_28_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_29_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_30_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_31_reserved      (void);
  virtual void             YAC_VCALL yacHashSet                      (void *_context, YAC_String*_key, YAC_Value *_value);                // set value 
  virtual void             YAC_VCALL yacHashGet                      (void *_context, YAC_String*_key, YAC_Value *_r);                    // get value (reference) 
  virtual void             YAC_VCALL yacHashGetDeref                 (void *_context, YAC_String*_key, YAC_Value *_r);                    // get value 
  virtual void             YAC_VCALL vtable_entry_5_35_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_36_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_37_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_38_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_5_39_reserved      (void);

   // ---- 
   // ---- 
   // ---- LEVEL (1<<6) interface 
   // ----                    ( metaclass support )
   // ---- 
   // ---- 
   // ---- 
  virtual sChar           *YAC_VCALL yacMetaClassName                (void);                                // get meta class class name of object (e.g. user defined class name)
  virtual sUI              YAC_VCALL yacMetaClassMemberGetNum        (void);                                // get number of members
  virtual sUI              YAC_VCALL yacMetaClassMemberGetAccessKeyByIndex  (sUI _idx);                     // get access key to member nr. _idx
  virtual sUI              YAC_VCALL yacMetaClassMemberGetAccessKeyByName   (const sChar *_name);           // get access key to member called _name
  virtual sUI              YAC_VCALL yacMetaClassMemberGetType       (sUI _accesskey);                      // get member type (0=void,1=int,2=float,3=object)
  virtual sChar           *YAC_VCALL yacMetaClassMemberGetName       (sUI _accesskey);                      // get member name by access key
  virtual void             YAC_VCALL yacMetaClassMemberSet           (sUI _accesskey, YAC_Value *_value);   // set member value
  virtual void             YAC_VCALL yacMetaClassMemberGet           (sUI _accesskey, YAC_Value *_r);       // get member value
  virtual sSI              YAC_VCALL yacMetaClassInstanceOf          (YAC_Object *_object);                 // check if this is an instance of _object meta class type
  virtual void             YAC_VCALL vtable_entry_6_10_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_6_11_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_6_12_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_6_13_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_6_14_reserved      (void);
  virtual void             YAC_VCALL vtable_entry_6_15_reserved      (void);

   // ---- 
   // ---- 
   // ---- LEVEL (1<<7) interface 
   // ----                    ( signal/callback support )
   // ---- 
   // ---- 
   // ---- 
  virtual void             YAC_VCALL yacRegisterSignal               (sUI _id, YAC_FunctionHandle _f);        // register script function callback for signal _id (sequential index of signal name in signal list, see below)
  virtual void             YAC_VCALL yacGetSignalStringList          (YAC_String *_retstring);                // write signal name+rtti list to _retstring. example: "onSignal:1 onKeyboard:3 onMouse:85 ". the signals are enumerated (starting with 0) , see yacRegisterSignal()

   // ---- 
   // ---- 
   // ---- non-virtual helper methods
   // ---- 
   // ---- 
   // ---- 
  YAC_Object      * yacNew                   (YAC_ContextHandle _context); // call yacNewObject() and yacOperatorInit()
  YAC_Object      * yacNewPooled             (YAC_ContextHandle _context, sUI _poolHint); // call yacNewObject() and yacOperatorInit()
  sBool             yacCanDeserializeClass   (YAC_Object *_stream);   // read up to 64 chars from stream and compare with class name. implemented below.
  sBool             yacInstanceOf            (YAC_Object *_object);   // check if this is an instance of _object class type


#ifdef YAC_OBJECT_YAC
   // ---- YAC interface for YAC_Object itself 
   YM void _yacClassName                     (YAC_Value *_r);
   YM void _yacNewObject                     (YAC_Value *_r);
   // ---- m e m b e r s
   YM sSI  _yacMemberGetNum                  (void);
   YM void _yacMemberGetNames                (YAC_Value *_r);
   YM void _yacMemberGetTypes                (YAC_Value *_r);
   YM void _yacMemberGetObjectTypes          (YAC_Value *_r);
   YM void _yacMemberGetOffsets              (YAC_Value *_r);
   // ---- m e t h o d s
   YM sSI  _yacMethodGetNum                  (void);
   YM void _yacMethodGetNames                (YAC_Value *_r);
   YM void _yacMethodGetNumParameters        (YAC_Value *_r);
   YM void _yacMethodGetParameterTypes       (YAC_Value *_r);
   YM void _yacMethodGetParameterObjectTypes (YAC_Value *_r);
   YM void _yacMethodGetReturnTypes          (YAC_Value *_r);
   YM void _yacMethodGetReturnObjectTypes    (YAC_Value *_r);
   YM void _yacMethodGetAdr                  (YAC_Value *_r);
   // ---- c o n s t a n t s
   YM sSI  _yacConstantGetNum                (void);
   YM void _yacConstantGetNames              (YAC_Value *_r);
   YM void _yacConstantGetTypes              (YAC_Value *_r);
   YM void _yacConstantGetValues             (YAC_Value *_r);
   // ---- o p e r a t o r s
   YM sSI  _yacCopy                          (YAC_Object *_os);
   YM sSI  _yacEquals                        (YAC_Object *_ro);
   YM void _yacOperator                      (sSI _cmd, YAC_Object *_ro, YAC_Value *_r);
   YM void _yacOperatorInit                  (YAC_Object *_ro);
   YM void _yacOperatorAssign                (YAC_Object *_ro);
   YM void _yacOperatorAdd                   (YAC_Object *_ro);
   YM void _yacOperatorSub                   (YAC_Object *_ro);
   YM void _yacOperatorMul                   (YAC_Object *_ro);
   YM void _yacOperatorDiv                   (YAC_Object *_ro);
   YM void _yacOperatorClamp                 (YAC_Object *_min, YAC_Object *_max);
   YM void _yacOperatorWrap                  (YAC_Object *_min, YAC_Object *_max);
   YM sSI  _yacScanI32                       (YAC_Object *_vo);
   YM sSI  _yacScanI64                       (YAC_Object *_vo);
   YM sSI  _yacScanF32                       (YAC_Object *_vo);
   YM sSI  _yacScanF64                       (YAC_Object *_vo);
   YM sSI  _yacToString                      (YAC_Object *_s) const;
   YM void _yacOperatorI                     (sSI _cmd, sSI _i, YAC_Value *_r);
   YM void _yacOperatorI64                   (sSI _cmd, YAC_Object *_no, YAC_Value *_r);
   YM void _yacOperatorF32                   (sSI _cmd, sF32 _f32, YAC_Value *_r);
   YM void _yacOperatorF64                   (sSI _cmd, YAC_Object *_no, YAC_Value *_r);
   YM void _yacValueOfI                      (sSI _i);
   YM void _yacValueOfF32                    (sF32 _f32);
   YM void _yacValueOfF64                    (YAC_Object *_no);
   YM void _yacValueOfI64                    (YAC_Object *_no);
   YM sSI  _yacToParsableString              (YAC_Object *_s) const;
   // ---- s t r e a m s
   YM sSI  _yacIsStream                      (void); 
   YM void _yacStreamClose                   (void);
   YM sSI  _yacStreamOpenLocal               (YAC_Object *_name, sSI _access);
   YM sSI  _yacStreamOpenLogic               (YAC_Object *_name);
   YM sSI  _yacStreamGetByteOrder            (void);
   YM void _yacStreamSetByteOrder            (sSI _order);
   YM sSI  _yacStreamEOF                     (void);
   YM void _yacStreamSeek                    (sSI _off, sSI _mode);
   YM sSI  _yacStreamGetOffset               (void);
   YM void _yacStreamSetOffset               (sSI _off);
   YM sSI  _yacStreamGetSize                 (void);
   YM sSI  _yacStreamRead                    (YAC_Object *_ret, sSI _num);
   YM sSI  _yacStreamReadI8                  (void);
   YM sSI  _yacStreamReadI16                 (void);
   YM sSI  _yacStreamReadI32                 (void);
   YM void _yacStreamReadI64                 (YAC_Value *_r);
   YM sF32 _yacStreamReadF32                 (void);
   YM void _yacStreamReadF64                 (YAC_Value *_r);
   YM void _yacStreamReadObject              (YAC_Object *_p);
   YM sSI  _yacStreamReadString              (YAC_Object *_s, sSI _maxlen);
   YM sSI  _yacStreamReadBuffer              (YAC_Object *_buffer, sSI _off, sSI _num, sSI _resize);
   YM sSI  _yacStreamReadLine                (YAC_Object *_s, sSI _maxlen);
   YM sSI  _yacStreamWrite                   (YAC_Object *_in, sSI _num);
   YM void _yacStreamWriteI8                 (sSI _i);
   YM void _yacStreamWriteI16                (sSI _i);
   YM void _yacStreamWriteI32                (sSI _i);
   YM void _yacStreamWriteI64                (YAC_Object *_no);
   YM void _yacStreamWriteF32                (sF32 _f);
   YM void _yacStreamWriteF64                (YAC_Object *_no);
   YM void _yacStreamWriteObject             (YAC_Object *_p);
   YM sSI  _yacStreamWriteString             (YAC_Object *_s, sSI _off, sSI _num);
   YM sSI  _yacStreamWriteBuffer             (YAC_Object *_b, sSI _off, sSI _num);
   YM sSI  _yacStreamGetErrorCode            (void);
   YM void _yacStreamGetErrorStringByCode    (sSI _code, YAC_Value *_r);
   YM void _yacSerializeClassName            (YAC_Object *_ofs);
   YM void _yacSerialize                     (YAC_Object *_ofs, sSI _usetypeinfo);
   YM sSI  _yacDeserialize                   (YAC_Object *_ifs, sSI _usetypeinfo);
   // ---- i t e r a t o r s
   // ...
   // ---- a r r a y s   /   h a s h t a b l e s
   YM void _yacArrayNew                      (YAC_Value *_r);
   YM sSI  _yacArrayAlloc                    (sSI _sx, sSI _sy, sSI _type, sSI _ebytesize);
   YM sSI  _yacArrayRealloc                  (sSI _sx, sSI _sy, sSI _type, sSI _ebytesize);
   YM sSI  _yacArrayGetNumElements           (void);
   YM sSI  _yacArrayGetMaxElements           (void);
   YM void _yacArrayCopySize                 (YAC_Object *_p);
   YM void _yacArraySet                      (sSI _index, YAC_Object *_value);
   YM void _yacArrayGet                      (sSI _index, YAC_Value *_r);
   YM sSI  _yacArrayGetWidth                 (void);
   YM sSI  _yacArrayGetHeight                (void);
   YM sSI  _yacArrayGetElementType           (void);
   YM sSI  _yacArrayGetElementByteSize       (void);
   YM sSI  _yacArrayGetStride                (void);
   YM sSI  _yacArrayGetPointer               (void);
   YM void _yacArraySetWidth                 (sSI _width);
   YM void _yacArraySetTemplate              (YAC_Object *_template);
   YM void _yacArrayGetDeref                 (sSI _index, YAC_Value *_r);
   YM void _yacHashSet                       (YAC_Object *_key, YAC_Object *_value);
   YM void _yacHashGet                       (YAC_Object *_key, YAC_Value *_r);
   YM void _yacHashGetDeref                  (YAC_Object *_key, YAC_Value *_r);
   // ---- s i g n a l s
   YM void _yacGetSignalStringList           (YAC_Object *_s);
   // ---- m e t a c l a s s e s
   YM void _yacMetaClassName                      (YAC_Value *_r);
   YM sSI  _yacMetaClassMemberGetNum              (void);
   YM sSI  _yacMetaClassMemberGetAccessKeyByIndex (sSI _index);
   YM sSI  _yacMetaClassMemberGetAccessKeyByName  (YAC_Object *_s);
   YM sSI  _yacMetaClassMemberGetType             (sSI _ak);
   YM void _yacMetaClassMemberGetName             (sSI _ak, YAC_Value *_r);
   YM void _yacMetaClassMemberSet                 (sSI _ak, YAC_Object *_value);
   YM void _yacMetaClassMemberGet                 (sSI _ak, YAC_Value *_r);
   YM sSI  _yacMetaClassInstanceOf                (YAC_Object *_o);
   // ---- n o n - v i r t u a l
   YM void _yacNew                                (YAC_Value *_r);
   YM sSI  _yacCanDeserializeClass                (YAC_Object *_ifs);
   YM sSI  _yacInstanceOf                         (YAC_Object *_o);
#endif

   
};

YAC_API void YAC_CALL Object__operator(void*,yacmemptr,YAC_Value*);

#ifdef YAC_OBJECT_YAC
// ---- forward declarations for external YAC_Object YAC interface implementation ----
YAC_APIC void YAC_CALL yac_object_yacClassName                    (YAC_Object *_this, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacNewObject                    (YAC_Object *_this, YAC_Value *_r);

// ---- m e m b e r s
YAC_APIC sSI  YAC_CALL yac_object_yacMemberGetNum                 (YAC_Object *_this);
YAC_APIC void YAC_CALL yac_object_yacMemberGetNames               (YAC_Object *_this, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacMemberGetTypes               (YAC_Object *_this, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacMemberGetObjectTypes         (YAC_Object *_this, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacMemberGetOffsets             (YAC_Object *_this, YAC_Value *_r);

// ---- m e t h o d s
YAC_APIC sSI  YAC_CALL yac_object_yacMethodGetNum                 (YAC_Object *_this);
YAC_APIC void YAC_CALL yac_object_yacMethodGetNames               (YAC_Object *_this, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacMethodGetNumParameters       (YAC_Object *_this, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacMethodGetParameterTypes      (YAC_Object *_this, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacMethodGetParameterObjectTypes(YAC_Object *_this, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacMethodGetReturnTypes         (YAC_Object *_this, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacMethodGetReturnObjectTypes   (YAC_Object *_this, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacMethodGetAdr                 (YAC_Object *_this, YAC_Value *_r);

// ---- c o n s t a n t s
YAC_APIC sSI  YAC_CALL yac_object_yacConstantGetNum               (YAC_Object *_this);
YAC_APIC void YAC_CALL yac_object_yacConstantGetNames             (YAC_Object *_this, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacConstantGetTypes             (YAC_Object *_this, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacConstantGetValues            (YAC_Object *_this, YAC_Value *_r);

// ---- o p e r a t o r s
YAC_APIC sSI  YAC_CALL yac_object_yacCopy                         (YAC_Object *_this, YAC_Object *_os);
YAC_APIC sSI  YAC_CALL yac_object_yacEquals                       (YAC_Object *_this, YAC_Object *_ro);
YAC_APIC void YAC_CALL yac_object_yacOperator                     (YAC_Object *_this, sSI _cmd, YAC_Object *_ro, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacOperatorInit                 (YAC_Object *_this, YAC_Object *_ro);
YAC_APIC void YAC_CALL yac_object_yacOperatorAssign               (YAC_Object *_this, YAC_Object *_ro);
YAC_APIC void YAC_CALL yac_object_yacOperatorAdd                  (YAC_Object *_this, YAC_Object *_ro);
YAC_APIC void YAC_CALL yac_object_yacOperatorSub                  (YAC_Object *_this, YAC_Object *_ro);
YAC_APIC void YAC_CALL yac_object_yacOperatorMul                  (YAC_Object *_this, YAC_Object *_ro);
YAC_APIC void YAC_CALL yac_object_yacOperatorDiv                  (YAC_Object *_this, YAC_Object *_ro);
YAC_APIC void YAC_CALL yac_object_yacOperatorClamp                (YAC_Object *_this, YAC_Object *_min, YAC_Object *_max);
YAC_APIC void YAC_CALL yac_object_yacOperatorWrap                 (YAC_Object *_this, YAC_Object *_min, YAC_Object *_max);
YAC_APIC sSI  YAC_CALL yac_object_yacScanI32                      (YAC_Object *_this, YAC_Object *_vo);
YAC_APIC sSI  YAC_CALL yac_object_yacScanI64                      (YAC_Object *_this, YAC_Object *_vo);
YAC_APIC sSI  YAC_CALL yac_object_yacScanF32                      (YAC_Object *_this, YAC_Object *_vo);
YAC_APIC sSI  YAC_CALL yac_object_yacScanF64                      (YAC_Object *_this, YAC_Object *_vo);
YAC_APIC sSI  YAC_CALL yac_object_yacToString                     (const YAC_Object *_this, YAC_Object *_s);
YAC_APIC void YAC_CALL yac_object_yacOperatorI                    (YAC_Object *_this, sSI _cmd, sSI _i, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacOperatorI64                  (YAC_Object *_this, sSI _cmd, YAC_Object *_no, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacOperatorF32                  (YAC_Object *_this, sSI _cmd, sF32 _f32, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacOperatorF64                  (YAC_Object *_this, sSI _cmd, YAC_Object *_no, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacValueOfI                     (YAC_Object *_this, sSI _i);
YAC_APIC void YAC_CALL yac_object_yacValueOfI64                   (YAC_Object *_this, YAC_Object *_no);
YAC_APIC void YAC_CALL yac_object_yacValueOfF32                   (YAC_Object *_this, sF32 _f32);
YAC_APIC void YAC_CALL yac_object_yacValueOfF64                   (YAC_Object *_this, YAC_Object *_no);
YAC_APIC sSI  YAC_CALL yac_object_yacToParsableString             (const YAC_Object *_this, YAC_Object *_s);

// ---- s t r e a m s
YAC_APIC sSI  YAC_CALL yac_object_yacIsStream                     (YAC_Object *_this);
YAC_APIC void YAC_CALL yac_object_yacStreamClose                  (YAC_Object *_this);
YAC_APIC sSI  YAC_CALL yac_object_yacStreamOpenLocal              (YAC_Object *_this, YAC_Object *_name, sSI _access);
YAC_APIC sSI  YAC_CALL yac_object_yacStreamOpenLogic              (YAC_Object *_this, YAC_Object *_name);
YAC_APIC sSI  YAC_CALL yac_object_yacStreamGetByteOrder           (YAC_Object *_this);
YAC_APIC void YAC_CALL yac_object_yacStreamSetByteOrder           (YAC_Object *_this, sSI _order);
YAC_APIC sSI  YAC_CALL yac_object_yacStreamEOF                    (YAC_Object *_this);
YAC_APIC void YAC_CALL yac_object_yacStreamSeek                   (YAC_Object *_this, sSI _off, sSI _mode);
YAC_APIC sSI  YAC_CALL yac_object_yacStreamGetOffset              (YAC_Object *_this);
YAC_APIC void YAC_CALL yac_object_yacStreamSetOffset              (YAC_Object *_this, sSI _off);
YAC_APIC sSI  YAC_CALL yac_object_yacStreamGetSize                (YAC_Object *_this);
YAC_APIC sSI  YAC_CALL yac_object_yacStreamRead                   (YAC_Object *_this, YAC_Object *_ret, sSI _num);
YAC_APIC sSI  YAC_CALL yac_object_yacStreamReadI8                 (YAC_Object *_this);
YAC_APIC sSI  YAC_CALL yac_object_yacStreamReadI16                (YAC_Object *_this);
YAC_APIC sSI  YAC_CALL yac_object_yacStreamReadI32                (YAC_Object *_this);
YAC_APIC void YAC_CALL yac_object_yacStreamReadI64                (YAC_Object *_this, YAC_Value *_r);
YAC_APIC sF32 YAC_CALL yac_object_yacStreamReadF32                (YAC_Object *_this);
YAC_APIC void YAC_CALL yac_object_yacStreamReadF64                (YAC_Object *_this, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacStreamReadObject             (YAC_Object *_this, YAC_Object *_p);
YAC_APIC sSI  YAC_CALL yac_object_yacStreamReadString             (YAC_Object *_this, YAC_Object *_s, sSI _maxlen);
YAC_APIC sSI  YAC_CALL yac_object_yacStreamReadBuffer             (YAC_Object *_this, YAC_Object *_buf, sSI _off, sSI _n, sSI _resize);
YAC_APIC sSI  YAC_CALL yac_object_yacStreamReadLine               (YAC_Object *_this, YAC_Object *_s, sSI _maxlen);
YAC_APIC sSI  YAC_CALL yac_object_yacStreamWrite                  (YAC_Object *_this, YAC_Object *_in, sSI _num);
YAC_APIC void YAC_CALL yac_object_yacStreamWriteI8                (YAC_Object *_this, sSI _i);
YAC_APIC void YAC_CALL yac_object_yacStreamWriteI16               (YAC_Object *_this, sSI _i);
YAC_APIC void YAC_CALL yac_object_yacStreamWriteI32               (YAC_Object *_this, sSI _i);
YAC_APIC void YAC_CALL yac_object_yacStreamWriteI64               (YAC_Object *_this, YAC_Object *_no);
YAC_APIC void YAC_CALL yac_object_yacStreamWriteF32               (YAC_Object *_this, sF32 _f);
YAC_APIC void YAC_CALL yac_object_yacStreamWriteF64               (YAC_Object *_this, YAC_Object *_no);
YAC_APIC void YAC_CALL yac_object_yacStreamWriteObject            (YAC_Object *_this, YAC_Object *_p);
YAC_APIC sSI  YAC_CALL yac_object_yacStreamWriteString            (YAC_Object *_this, YAC_Object *_s, sSI _off, sSI _num);
YAC_APIC sSI  YAC_CALL yac_object_yacStreamWriteBuffer            (YAC_Object *_this, YAC_Object *_b, sSI _off, sSI _num);
YAC_APIC sSI  YAC_CALL yac_object_yacStreamGetErrorCode           (YAC_Object *_this);
YAC_APIC void YAC_CALL yac_object_yacStreamGetErrorStringByCode   (YAC_Object *_this, sSI _code, YAC_Value *_r);

// ---- s e r i a l i z a t i o n
YAC_APIC void YAC_CALL yac_object_yacSerializeClassName           (YAC_Object *_this, YAC_Object *_ofs);
YAC_APIC void YAC_CALL yac_object_yacSerialize                    (YAC_Object *_this, YAC_Object *_ofs, sSI _usetypeinfo);
YAC_APIC sSI  YAC_CALL yac_object_yacDeserialize                  (YAC_Object *_this, YAC_Object *_ifs, sSI _usetypeinfo);

// ---- i t e r a t o r s
// ---- a r r a y s   /   h a s h t a b l e s
YAC_APIC void YAC_CALL yac_object_yacArrayNew                     (YAC_Object *_this, YAC_Value *_r);
YAC_APIC sSI  YAC_CALL yac_object_yacArrayAlloc                   (YAC_Object *_this, sSI _sx, sSI _sy, sSI _type, sSI _ebytesize);
YAC_APIC sSI  YAC_CALL yac_object_yacArrayRealloc                 (YAC_Object *_this, sSI _sx, sSI _sy, sSI _type, sSI _ebytesize);
YAC_APIC sSI  YAC_CALL yac_object_yacArrayGetNumElements          (YAC_Object *_this);
YAC_APIC sSI  YAC_CALL yac_object_yacArrayGetMaxElements          (YAC_Object *_this);
YAC_APIC void YAC_CALL yac_object_yacArrayCopySize                (YAC_Object *_this, YAC_Object *_p);
YAC_APIC void YAC_CALL yac_object_yacArraySet                     (YAC_Object *_this, sSI _index, YAC_Object *_value);
YAC_APIC void YAC_CALL yac_object_yacArrayGet                     (YAC_Object *_this, sSI _index, YAC_Value *_r);
YAC_APIC sSI  YAC_CALL yac_object_yacArrayGetWidth                (YAC_Object *_this);
YAC_APIC sSI  YAC_CALL yac_object_yacArrayGetHeight               (YAC_Object *_this);
YAC_APIC sSI  YAC_CALL yac_object_yacArrayGetElementType          (YAC_Object *_this);
YAC_APIC sSI  YAC_CALL yac_object_yacArrayGetElementByteSize      (YAC_Object *_this);
YAC_APIC sSI  YAC_CALL yac_object_yacArrayGetStride               (YAC_Object *_this);
YAC_APIC sSI  YAC_CALL yac_object_yacArrayGetPointer              (YAC_Object *_this);
YAC_APIC void YAC_CALL yac_object_yacArraySetWidth                (YAC_Object *_this, sSI _width);
YAC_APIC void YAC_CALL yac_object_yacArraySetTemplate             (YAC_Object *_this, YAC_Object *_template);
YAC_APIC void YAC_CALL yac_object_yacArrayGetDeref                (YAC_Object *_this, sSI _index, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacHashSet                      (YAC_Object *_this, YAC_Object *_key, YAC_Object *_value);
YAC_APIC void YAC_CALL yac_object_yacHashGet                      (YAC_Object *_this, YAC_Object *_key, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacHashGetDeref                 (YAC_Object *_this, YAC_Object *_key, YAC_Value *_r);

// ---- s i g n a l s
YAC_APIC void YAC_CALL yac_object_yacGetSignalStringList          (YAC_Object *_this, YAC_Object *_s);

// ---- m e t a c l a s s e s
YAC_APIC void YAC_CALL yac_object_yacMetaClassName                      (YAC_Object *_this, YAC_Value *_r);
YAC_APIC sSI  YAC_CALL yac_object_yacMetaClassMemberGetNum              (YAC_Object *_this);
YAC_APIC sSI  YAC_CALL yac_object_yacMetaClassMemberGetAccessKeyByIndex (YAC_Object *_this, sSI _index);
YAC_APIC sSI  YAC_CALL yac_object_yacMetaClassMemberGetAccessKeyByName  (YAC_Object *_this, YAC_Object *_s);
YAC_APIC sSI  YAC_CALL yac_object_yacMetaClassMemberGetType             (YAC_Object *_this, sSI _ak);
YAC_APIC void YAC_CALL yac_object_yacMetaClassMemberGetName             (YAC_Object *_this, sSI _ak, YAC_Value *_r);
YAC_APIC void YAC_CALL yac_object_yacMetaClassMemberSet                 (YAC_Object *_this, sSI _ak, YAC_Object *_value);
YAC_APIC void YAC_CALL yac_object_yacMetaClassMemberGet                 (YAC_Object *_this, sSI _ak, YAC_Value *_r);
YAC_APIC sSI  YAC_CALL yac_object_yacMetaClassInstanceOf                (YAC_Object *_this, YAC_Object *_o);

// ---- n o n - v i r t u a l
YAC_APIC void YAC_CALL yac_object_yacNew                          (YAC_Object *_this, YAC_Value *_r);
YAC_APIC sSI  YAC_CALL yac_object_yacCanDeserializeClass          (YAC_Object *_this, YAC_Object *_ifs);
YAC_APIC sSI  YAC_CALL yac_object_yacInstanceOf                   (YAC_Object *_this, YAC_Object *_o);
#endif // YAC_OBJECT_YAC

#ifndef YAC_CUST_VALUE
// ---- YAC_Object representation of a YAC_Value ----
class YAC_API YAC_ValueObject : public YAC_Object, public YAC_Value { 
};

// ---- an array of YAC_Values ----
class YAC_ValueArray : public YAC_Object {
public:
    sUI          max_elements;
    sUI          num_elements;
    YAC_Value   *elements; // arbitrary int/float/Object values

    YAC_ValueArray(void) {
       max_elements = 0;
       num_elements = 0;
       elements = NULL;
    }
};

// ---- an array of YAC_Objects wrapped in YAC_Values ----
class YAC_PointerArray :  public YAC_Object {
public:
    sUI          max_elements;
    sUI          num_elements;
    YAC_Value   *elements; // only Object values allowed

    YAC_PointerArray(void) {
       max_elements = 0;
       num_elements = 0;
       elements = NULL;
    }

    sBool realloc       (sUI _maxElements);
    sBool add           (YAC_Object *_o, sBool _bDelete);
    void  removeIndex   (sUI _idx);
    sSI   indexOfPointer(YAC_Object *_o, sUI _off);
    
};
#endif // YAC_CUST_VALUE

#ifndef YAC_CUST_EVENT
// ---- event class; a time-stamped String ----
class YAC_API YAC_Event : public YAC_ValueObject {
public:
	sSI id;
   sSI sub_id;
public:
	YAC_Event (void);
	~YAC_Event();
};
#endif // YAC_CUST_EVENT


// ---- seek modes for streams, see yacStreamSeek() ----
enum __yac_stream_seekmodes {
   YAC_BEG          =0,
   YAC_CUR          =1,
   YAC_END          =2
};

// ---- byte order for streams, see yacStreamGetByteOrder(), yacStreamSetByteOrder()
enum __yac_stream_byteorder {
   YAC_LITTLEENDIAN =0,
   YAC_BIGENDIAN    =1
};

// ---- open modes for yacStreamOpenLocal()
enum __yac_stream_openmodes {
   YAC_IOS_IN            =0,
   YAC_IOS_OUTIN         =1,
   YAC_IOS_INOUT         =2
};

// ---- file stream types
enum __yac_stream_openmodesx {
   YAC_LOGIC             =0,
   YAC_LOCAL             =1
};

// ---- error codes for yacStream*() interface, see yacStreamGetErrorCode()
enum __streamerrorcodes {
    YAC_NOERROR       =0,
    YAC_ERRINVALIDSEEK=1,
    YAC_ERRIO,
    YAC_ERRCREATEFILE,
    YAC_ERROPENFILE,
    YAC_ERRPAD
};

// ---- also see Object::serialize(), Object::deserialize() ----
#define YAC_IS_STREAM(a)       ((a)&&((a)->yacIsStream()))                                                                         // test if class supports the stream interface
#define YAC_BEG_DESERIALIZE()  if((_rtti)&&!yacCanDeserializeClass((_ifs)))return 0                                                // start object deserialization, read string from stream and compare with class name
#define YAC_BEG_SERIALIZE()    if(_rtti){YAC_String s;char *t=(char*)yacMetaClassName();if(!t)t=(char*)yacClassName();s.visit(t);_ofs->yacStreamWriteString(&s, 0, s.length);} // start object serialization, write (meta) class name string into stream
#define YAC_SERIALIZE_I8(a)    _ofs->yacStreamWriteI8(a)      // write a byte (8bit)
#define YAC_SERIALIZE_I16(a)   _ofs->yacStreamWriteI16(a)     // write a 16bit word (with transparent byteorder conversion)
#define YAC_SERIALIZE_I32(a)   _ofs->yacStreamWriteI32(a)     // write a 32bit double word (with transparent byteorder conversion)
#define YAC_SERIALIZE_I64(a)   _ofs->yacStreamWriteI64(a)     // write a 64bit quad word (with transparent byteorder conversion)
#define YAC_SERIALIZE_F32(a)   _ofs->yacStreamWriteF32(a)     // write a 32bit IEEE float
#define YAC_SERIALIZE_F64(a)   _ofs->yacStreamWriteF64(a)     // write a 64bit IEEE double
#define YAC_SERIALIZE_OBJ(a)   _ofs->yacStreamWriteObject(a)  // write an object
#define YAC_DESERIALIZE_I8()   _ifs->yacStreamReadI8()        // read a byte (8bit)
#define YAC_DESERIALIZE_I16()  _ifs->yacStreamReadI16()       // read a 16bit word (with transparent byteorder conversion)
#define YAC_DESERIALIZE_I32()  _ifs->yacStreamReadI32()       // read a 32bit double word (with transparent byteorder conversion)
#define YAC_DESERIALIZE_I64()  _ifs->yacStreamReadI64()       // read a 64bit quad word (with transparent byteorder conversion)
#define YAC_DESERIALIZE_F32()  _ifs->yacStreamReadF32()       // read a 32bit IEEE float
#define YAC_DESERIALIZE_F64()  _ifs->yacStreamReadF64()       // read a 64bit IEEE double
#define YAC_DESERIALIZE_OBJ(a) _ifs->yacStreamReadObject(a)   // read an object

// ---- base stream class with no methods ----
#ifndef YAC_CUST_STREAMBASE
class YAC_API YAC_StreamBase : public YAC_Object {
public:
	sUI byteOrder;
};
#ifndef YAC_CUST_BUFFER
// ---- a binary buffer that supports the yacStream interface ----
class YAC_Buffer : public YAC_StreamBase {
public:
	sUI   size;
	sUI   io_offset;
	sU8 * buffer;
   sBool deleteme;
public:
          YAC_Buffer                 (void);                          //
          ~YAC_Buffer                ();                              //

    void  YAC_VCALL yacArraySet                (void *_context, sUI _index, YAC_Value *_value)  override; // set a single value
    void  YAC_VCALL yacArrayGet                (void *_context, sUI _index, YAC_Value *_r)  override;     // read a single value
    sUI   YAC_VCALL yacArrayGetWidth           (void) override;                 // get number of elements/lines
    sUI   YAC_VCALL yacArrayGetHeight          (void) override;                 // get number of lines
    sUI   YAC_VCALL yacArrayGetElementType     (void) override;                 // get type of element (1=int, 2=float)
    sUI   YAC_VCALL yacArrayGetElementByteSize (void) override;                 // get bytes / element (1,2,4,...)
    sUI   YAC_VCALL yacArrayGetStride          (void) override;                 // return byte offset to next line
    void *YAC_VCALL yacArrayGetPointer         (void) override;                 // return pointer to first element in first line
};
#endif
#endif // YAC_CUST_STREAMBASE
// ---- see YAC_Object::yacGetIterator() ----
class YAC_API YAC_Iterator {
public:
	sUI current_index;
public:
                       YAC_Iterator(void); //
    virtual            ~YAC_Iterator();    //

    virtual void YAC_VCALL getNext(YAC_Value *); // get next value
    virtual void YAC_VCALL begin(void);          // start iteration
    virtual void YAC_VCALL end(void);            // finish iteration
};
// ---- plugin host interface -----
class YAC_API YAC_Host { 
public:
    sU8 cpp_typecast_map[YAC_MAX_CLASSES][YAC_MAX_CLASSES]; // used to test whether class b is a base class of class a, i.e. cpp_typecast_map[a_class_id][b_class_id]==1
public:
                         YAC_Host                   (void);
    virtual              ~YAC_Host                  ();
	// ----
	// ----
	// ----
	// ---- LEVEL (1<<0) interface:
	// ----                       ( C/C++ reflection support )
	// ----
	// ----
    virtual sUI          YAC_VCALL yacQueryInterfaces         (void) = 0;
    virtual sUI          YAC_VCALL yacRegisterClass           (YAC_Object *_template, sUI _may_instanciate) = 0; // returns class_ID assigned by YAC_Host
    virtual YAC_Object  *YAC_VCALL yacNew                     (const char *_namespaceName, const char *_classname) = 0; // allocate unknown API object (e.g. Texture or a plugin class)
    virtual YAC_Object  *YAC_VCALL yacNewByID                 (sUI _class_ID) = 0;
    virtual void         YAC_VCALL yacDelete                  (YAC_Object *_apiobject) = 0; // delete previously allocated API object
    virtual sUI          YAC_VCALL yacGetClassIDByName        (sChar *_name) = 0;
    virtual sUI          YAC_VCALL yacRegisterFunction        (void *_adr, const char *_name, sUI _returntype, const char *_return_otype, sUI _numargs, const sUI *_argtypes, const char **_argtypenames, sUI _callstyle) = 0;
    virtual sSI          YAC_VCALL yacEvalMethodByName        (YAC_Object *_apiobject, const char *_name, YAC_Value *_args, sUI _numargs, YAC_Value *_r) = 0;  // lookup method by name, typecast arguments and evaluate. return true if everything worked OK, false if method _name was not found.
    virtual YAC_Object * YAC_VCALL yacGetClassTemplateByID    (sUI _class_ID) = 0;
    virtual YAC_Object * YAC_VCALL yacNewPooledByID           (sUI _class_ID, sUI _poolHint) = 0;
    virtual void         YAC_VCALL yacNewDeleteModifyCounter  (sSI _deltaByteSize) = 0;
    virtual sSI          YAC_VCALL yacNewDeleteGetCounter     (void) = 0;
    virtual void         YAC_VCALL vtable_entry_0_12_reserved (void) = 0;
    virtual void         YAC_VCALL vtable_entry_0_13_reserved (void) = 0;
    virtual void         YAC_VCALL vtable_entry_0_14_reserved (void) = 0;
    virtual void         YAC_VCALL vtable_entry_0_15_reserved (void) = 0;
	// ----
	// ----
	// ----
	// ---- LEVEL (1<<1) interface:
	// ----                       ( debug support )
	// ----
	// ----
    virtual sUI          YAC_VCALL yacGetDebugLevel           (void) = 0;
    virtual void         YAC_VCALL yacPrint                   (const sChar *_s) = 0; // print to default debug console (stdout, stderr, or textfile)
    virtual void         YAC_VCALL vtable_entry_1_2_reserved  (void) = 0;
    virtual void         YAC_VCALL vtable_entry_1_3_reserved  (void) = 0;
    virtual void         YAC_VCALL vtable_entry_1_4_reserved  (void) = 0;
    virtual void         YAC_VCALL vtable_entry_1_5_reserved  (void) = 0;
    virtual void         YAC_VCALL vtable_entry_1_6_reserved  (void) = 0;
    virtual void         YAC_VCALL vtable_entry_1_7_reserved  (void) = 0;
	// ----
	// ----
	// ----
	// ---- LEVEL (1<<2) interface:
	// ----                       ( string support )
	// ----
	// ----
    virtual sSI          YAC_VCALL yacStringReplace           (YAC_String *_d, YAC_String *_a, YAC_String *_b) = 0;
    virtual sSI          YAC_VCALL yacStringScan              (YAC_String *,sSI*) = 0;
    virtual sSI          YAC_VCALL yacStringScan              (YAC_String *,sF32*) = 0;
    virtual sUI          YAC_VCALL yacScanFlags               (YAC_String*) = 0;
    virtual void         YAC_VCALL vtable_entry_2_4_reserved  (void) = 0;
    virtual void         YAC_VCALL vtable_entry_2_5_reserved  (void) = 0;
    virtual void         YAC_VCALL vtable_entry_2_6_reserved  (void) = 0;
    virtual void         YAC_VCALL vtable_entry_2_7_reserved  (void) = 0;
    virtual void         YAC_VCALL vtable_entry_2_8_reserved  (void) = 0;
    virtual void         YAC_VCALL vtable_entry_2_9_reserved  (void) = 0;
    virtual void         YAC_VCALL vtable_entry_2_10_reserved (void) = 0;
    virtual void         YAC_VCALL vtable_entry_2_11_reserved (void) = 0;
    virtual void         YAC_VCALL vtable_entry_2_12_reserved (void) = 0;
    virtual void         YAC_VCALL vtable_entry_2_13_reserved (void) = 0;
    virtual void         YAC_VCALL vtable_entry_2_14_reserved (void) = 0;
    virtual void         YAC_VCALL vtable_entry_2_15_reserved (void) = 0;
	// ----
	// ----
	// ----
	// ---- LEVEL (1<<3) interface:
	// ----                       ( scripting support )
	// ----
	// ----
    virtual sUI                YAC_VCALL yacRunning                 (void) = 0;
    virtual YAC_FunctionHandle YAC_VCALL yacFindFunction            (sChar *_name) = 0;
    virtual sUI                YAC_VCALL yacEvalFunction            (YAC_ContextHandle _context, YAC_FunctionHandle _script_function, sUI _numargs, YAC_Value *_args) = 0; // evaluate script function, e.g. used for tks signal callbacks; see YAC_Object::yacRegisterSignal(),yacGetSignalStringList(). return true or false depending on whether the function call succeeded.
    virtual YAC_ModuleHandle   YAC_VCALL yacCompileModule           (sChar *_source) = 0;
    virtual void               YAC_VCALL yacDeleteModule            (YAC_ModuleHandle _mod) = 0;
    virtual YAC_FunctionHandle YAC_VCALL yacFindFunctionInModule    (YAC_ModuleHandle _mod, sChar *_name) = 0;
    virtual YAC_VariableHandle YAC_VCALL yacFindVariableInModule    (YAC_ModuleHandle _mod, sChar *_name) = 0;
    virtual YAC_FunctionHandle YAC_VCALL yacFindVariableInFunction  (YAC_FunctionHandle _fn, sChar *_name) = 0;
    virtual void               YAC_VCALL yacSetVariable             (YAC_VariableHandle _var, YAC_Value *_v) = 0;
    virtual void               YAC_VCALL yacGetVariable             (YAC_VariableHandle _var, YAC_Value *_r) = 0;
    virtual sUI                YAC_VCALL yacEvalFunctionReturn      (YAC_ContextHandle _context, YAC_FunctionHandle _script_function, sUI _numargs, YAC_Value *_args, YAC_Value *_r) = 0; // evaluate script function, e.g. used for tks signal callbacks; see YAC_Object::yacRegisterSignal(),yacGetSignalStringList(). return true or false depending on whether the function call succeeded.
    virtual YAC_ContextHandle  YAC_VCALL yacContextCreate           (void) = 0; // Create script execution context
    virtual void               YAC_VCALL yacContextDestroy          (YAC_ContextHandle _context) = 0; // Destroy script execution context
    virtual YAC_ContextHandle  YAC_VCALL yacContextGetDefault       (void) = 0;
    virtual void               YAC_VCALL yacContextSetDefault       (YAC_ContextHandle _context) = 0; // Set default script context (for use in threads created by plugins / not by a Thread object)
    virtual void               YAC_VCALL vtable_entry_3_15_reserved (void) = 0;
    virtual void               YAC_VCALL vtable_entry_3_16_reserved (void) = 0;
    virtual void               YAC_VCALL vtable_entry_3_17_reserved (void) = 0;
    virtual void               YAC_VCALL vtable_entry_3_18_reserved (void) = 0;
    virtual void               YAC_VCALL vtable_entry_3_19_reserved (void) = 0;
    virtual void               YAC_VCALL vtable_entry_3_20_reserved (void) = 0;
    virtual void               YAC_VCALL vtable_entry_3_21_reserved (void) = 0;
    virtual void               YAC_VCALL vtable_entry_3_22_reserved (void) = 0;
    virtual void               YAC_VCALL vtable_entry_3_23_reserved (void) = 0;
    virtual void               YAC_VCALL vtable_entry_3_24_reserved (void) = 0;
    virtual void               YAC_VCALL vtable_entry_3_25_reserved (void) = 0;
    virtual void               YAC_VCALL vtable_entry_3_26_reserved (void) = 0;
    virtual void               YAC_VCALL vtable_entry_3_27_reserved (void) = 0;
    virtual void               YAC_VCALL vtable_entry_3_28_reserved (void) = 0;
    virtual void               YAC_VCALL vtable_entry_3_29_reserved (void) = 0;
    virtual void               YAC_VCALL vtable_entry_3_30_reserved (void) = 0;
    virtual void               YAC_VCALL vtable_entry_3_31_reserved (void) = 0;
	// ----
	// ----
	// ----
	// ---- LEVEL (1<<4) interface:
	// ----                       ( event support )
	// ----
	// ----
    virtual void         YAC_VCALL yacSendUserEvent           (YAC_Object *_event_or_string) = 0; // send user event to running application
    virtual sUI          YAC_VCALL yacMilliSeconds            (void) = 0; // Query milliseconds since startup
    virtual void         YAC_VCALL vtable_entry_4_2_reserved  (void) = 0;
    virtual void         YAC_VCALL vtable_entry_4_3_reserved  (void) = 0;
    virtual void         YAC_VCALL vtable_entry_4_4_reserved  (void) = 0;
    virtual void         YAC_VCALL vtable_entry_4_5_reserved  (void) = 0;
    virtual void         YAC_VCALL vtable_entry_4_6_reserved  (void) = 0;
    virtual void         YAC_VCALL vtable_entry_4_7_reserved  (void) = 0;
    
    // ---- 
    // ---- 
    // ---- LEVEL (1<<5) interface 
    // ----                    ( exception support )
    // ---- 
    // ---- 
    // ---- 
    virtual YAC_ExceptionId  YAC_VCALL yacExceptionRegister       (const char *_name, sUI _baseException) = 0; // register new exception type. Return exception id or 0==registration failed
    virtual YAC_ExceptionId  YAC_VCALL yacExceptionGetIdByName    (const char *_name) = 0; // Look up ID for exception called _name
    virtual void             YAC_VCALL yacExceptionRaise          (YAC_ContextHandle _context, sUI _id, const char *_message=NULL, const char *_file=NULL, sSI _line=0) = 0; // raise new Exception with type _id.
    virtual void             YAC_VCALL vtable_entry_5_3_reserved  (void) = 0;
    virtual void             YAC_VCALL vtable_entry_5_4_reserved  (void) = 0;
    virtual void             YAC_VCALL vtable_entry_5_5_reserved  (void) = 0;
    virtual void             YAC_VCALL vtable_entry_5_6_reserved  (void) = 0;
    virtual void             YAC_VCALL vtable_entry_5_7_reserved  (void) = 0;
    virtual void             YAC_VCALL vtable_entry_5_8_reserved  (void) = 0;
    virtual void             YAC_VCALL vtable_entry_5_9_reserved  (void) = 0;

    // ---- 
    // ---- 
    // ---- LEVEL (1<<6) interface 
    // ----                    ( callback support )
    // ---- 
    // ---- 
    // ---- 
    virtual YAC_CallbackId   YAC_VCALL yacCallbackCreate          (const char *_name) = 0; // create named callback slot and returns callback id, -1=unable to create
    virtual YAC_CallbackId   YAC_VCALL yacCallbackGetIdByName     (const char *_name) = 0; // map callback name to slot id, -1=not found
    virtual sBool            YAC_VCALL yacCallbackSetFunById      (YAC_CallbackId _id, YAC_CFunctionPtr _fun) = 0; // register callback function for given slot, _fun=NULL => unregister
    virtual sSI              YAC_VCALL yacCallbackSetFunByName    (const char *_name, YAC_CFunctionPtr _fun) = 0; // register callback function for given slot, _fun=NULL => unregister
                                                                                   // this implicitely creates a callback slot in order to make the plugin
                                                                                   // loading order irrelevant
    virtual YAC_CFunctionPtr YAC_VCALL yacCallbackGetFunById      (YAC_CallbackId _callbackId) = 0;   // query current C function (cdecl) binding for the given callback slot
    virtual void             YAC_VCALL vtable_entry_6_6_reserved  (void) = 0;
    virtual void             YAC_VCALL vtable_entry_6_7_reserved  (void) = 0;
    virtual void             YAC_VCALL vtable_entry_6_8_reserved  (void) = 0;
    virtual void             YAC_VCALL vtable_entry_6_9_reserved  (void) = 0;

    // ---- 
    // ---- 
    // ---- LEVEL (1<<7) interface 
    // ----                    ( mutual exclusive semaphore support )
    // ---- 
    // ---- 
    // ---- 
    virtual YAC_MutexHandle  YAC_VCALL yacMutexCreate             (void) = 0; // Create new mutex, NULL=create failed
    virtual void             YAC_VCALL yacMutexDestroy            (YAC_MutexHandle _mutexHandle) = 0; // Destroy mutex that was created with yacMutexCreate()
    virtual YAC_MutexHandle  YAC_VCALL yacMutexFindByName         (const char *_name) = 0; // Find named sync point 
    virtual void             YAC_VCALL yacMutexLock               (YAC_MutexHandle _mutexHandle) = 0; // Lock mutex, beware of dead locks!
    virtual void             YAC_VCALL yacMutexUnlock             (YAC_MutexHandle _mutexHandle) = 0; // Unlock mutex
    virtual void             YAC_VCALL vtable_entry_7_5_reserved  (void) = 0;
    virtual void             YAC_VCALL vtable_entry_7_6_reserved  (void) = 0;
    virtual void             YAC_VCALL vtable_entry_7_7_reserved  (void) = 0;
    virtual void             YAC_VCALL vtable_entry_7_8_reserved  (void) = 0;
    virtual void             YAC_VCALL vtable_entry_7_9_reserved  (void) = 0;
    


#ifdef YAC_PRINTF
	void printf(const char *_fmt, ...);
#endif
};
#ifndef YAC_CUST_STRING
// ---- simple String class 
// ---- Import notes: * Never delete[]chars in Strings obtained from YAC_New_String()!
// ----               * Never pass Strings NOT allocated with YAC_New_String() to the YAC_Host !
class YAC_API YAC_String : public YAC_Object {
public:
	enum __stringflags {
      QUOT       = (1<<24),
      UTAG1      = (1<<25),
      DEL        = (1<<31)
   };

 public:
  sUI    buflen; // ---- total buffer size
  sUI    bflags; // ---- stringflags (e.g. deleteme flag)
  sUI    length; // ---- number of used chars in buffer
  sUI    key;    // ---- hashkey that is used for fast object comparison
  sU8   *chars;  // ---- pointer to first char in buffer
  void  *clones; // ---- internal StaticList::Node*, see tks-list.h if you need this member (used for split() string lists)
 public:
        YAC_String     (void);
        ~YAC_String    ();

  void  free           (void                             );  // Note: Never delete Strings obtained from the YAC_Host!
  void  visit          (const sChar *_cstring            );  // set read-only reference to (const) char array
  sBool compare        (const sChar *e                   );  // compare with "C" string (slow)
  sSI   lastIndexOf    (sChar _c, sUI _start=0           );
  sSI   indexOf        (sChar _c, sUI _start=0           );
#ifdef YAC_BIGSTRING // ---- use with care; don't mix YAC_Host/plugin *chars
  sUI   sum            (void                             );  // 
  void  genKey         (void                             );  //
  sUI   getKey         (void                             );  // 
  sBool compare        (YAC_String *s                    );  // compare with YAC_String object (fast)
  void  fixLength      (void                             );  // counter number of chars (including ASCIIZ)
  sBool alloc          (sU32 len                         );
  sBool copy           (const sChar *e                   );  // copy c-string, 
  sBool copy           (YAC_String *_s                   );  // copy YAC_String object
  sBool realloc        (sU32 len                         );  // resize string buffer
  sBool createEmpty    (void                             );   
  sBool append         (YAC_String *                     );  // append YAC_String
  sBool append         (const char *                     );  // append "C" string
  sBool substring      (YAC_String *s, sUI start, sUI len);  // extract substring
  sBool empty          (void                             );
  void  printf         (const char *_fmt, ...            );
#endif // YAC_BIGSTRING
};

#ifdef YAC_BIGSTRING
sUI YAC_strlen(const char *_s);
#endif

// ---- an array of YAC_Strings ----
class YAC_StringArray : public YAC_Object { 
public:
  sUI         max_elements;
  sUI         num_elements;
  YAC_String *elements;

  YAC_StringArray(void) {
     max_elements = 0;
     num_elements = 0;
     elements = NULL;
  }
};
#endif // YAC_CUST_STRING

#ifndef YAC_CUST_FLOATARRAY
// ---- 
class YAC_FloatArray : public YAC_Object {
public:
	sBool own_data;
    sUI   max_elements;
    sUI   num_elements;
    sF32 *elements;

    YAC_FloatArray(void) {
       own_data = 0;
       max_elements = 0u;
       num_elements = 0u;
       elements = NULL;
    }
};
#endif // YAC_CUST_FLOATARRAY

#ifndef YAC_CUST_INTARRAY
// ----
class YAC_IntArray : public YAC_Object { 
public:
	sSI   own_data;
	sUI   max_elements;
	sUI   num_elements;
	sSI  *elements;

   YAC_IntArray(void) {
      own_data = 0;
      max_elements = 0u;
      num_elements = 0u;
      elements = NULL;
   }
};
#endif // YAC_CUST_INTARRAY

#ifndef YAC_CUST_LISTNODE
// ---- a list of value objects ----
class YAC_API YAC_ListNode : public YAC_ValueObject {
public:
	YAC_ListNode *next;
	YAC_ListNode *prev;

	YAC_ListNode(void);
	~YAC_ListNode();
};

// ---- object wrapper for a YAC_ListNode list ----
class YAC_API YAC_List : public YAC_Object { 
  YAC_ListNode *head; 
  YAC_ListNode *tail; 

  YAC_List(void) {
     head = NULL;
     tail = NULL;
  }
};
#endif // YAC_CUST_LISTNODE

#ifndef YAC_CUST_TREENODE
// ---- a tree of values ----
class YAC_API YAC_TreeNode : public YAC_ValueObject {
public:
	YAC_TreeNode      *left;
	YAC_TreeNode      *right;
	YAC_TreeNode      *parent;
	YAC_String         name;
	YAC_String         id;

	YAC_TreeNode(void);
	~YAC_TreeNode();
};
#endif // YAC_CUST_LISTNODE

#ifndef YAC_CUST_NUMBEROBJECTS
// ---- Object shells/wrappers for 8-64 bit integer resp. floating point values
// ---- YAC_Number is part of the YAC_Object interface thus there is no "Number" class 
class YAC_API YAC_UnsignedByte    : public YAC_Object { public: sU8  value; };
class YAC_API YAC_Byte            : public YAC_Object { public: sS8  value; };
class YAC_API YAC_Boolean         : public YAC_Object { public: sBool value;};
class YAC_API YAC_UnsignedShort   : public YAC_Object { public: sU16 value; };
class YAC_API YAC_Short           : public YAC_Object { public: sS16 value; };
class YAC_API YAC_UnsignedInteger : public YAC_Object { public: sU32 value; };
class YAC_API YAC_Integer         : public YAC_Object { public: sS32 value; };
class YAC_API YAC_Long            : public YAC_Object { public: sS64 value; };
class YAC_API YAC_Float           : public YAC_Object { public: sF32 value; };
class YAC_API YAC_Double          : public YAC_Object { public: sF64 value; };
#endif // YAC_CUST_NUMBEROBJECTS


// ---- magic loader symbols ----
YAC_APIC void YAC_Init   (YAC_Host *); // ---- called when a plugin is (un-)loaded
YAC_APIC void YAC_Exit   (YAC_Host *); // ---- your plugin needs to define at least these
YAC_APIC sUI  YAC_Version(void      ); // ---- query plugin version information. 0xaabbccdd, e.g. 0x00050203.

// ---- helper macros to instanciate new YAC_Objects  ----
#define YAC_New_Boolean()         (YAC_Boolean*)         yac_host->yacNewByID (YAC_CLID_BOOLEAN        )
#define YAC_New_Byte()            (YAC_Byte*)            yac_host->yacNewByID (YAC_CLID_BYTE           )
#define YAC_New_Short()           (YAC_Short*)           yac_host->yacNewByID (YAC_CLID_SHORT          )
#define YAC_New_Integer()         (YAC_Integer*)         yac_host->yacNewByID (YAC_CLID_INTEGER        )
#define YAC_New_Long()            (YAC_Long*)            yac_host->yacNewByID (YAC_CLID_LONG           )
#define YAC_New_UnsignedByte()    (YAC_UnsignedByte*)    yac_host->yacNewByID (YAC_CLID_UNSIGNEDBYTE   )
#define YAC_New_UnsignedShort()   (YAC_UnsignedShort*)   yac_host->yacNewByID (YAC_CLID_UNSIGNEDSHORT  )
#define YAC_New_UnsignedInteger() (YAC_UnsignedInteger*) yac_host->yacNewByID (YAC_CLID_UNSIGNEDINTEGER)
#define YAC_New_Float()           (YAC_Float*)           yac_host->yacNewByID (YAC_CLID_FLOAT          )
#define YAC_New_Double()          (YAC_Double*)          yac_host->yacNewByID (YAC_CLID_DOUBLE         )
#define YAC_New_String()          (YAC_String*)          yac_host->yacNewByID (YAC_CLID_STRING         )
#define YAC_New_Event()           (YAC_Event*)           yac_host->yacNewByID (YAC_CLID_EVENT          )
#define YAC_New_Value()           (YAC_ValueObject*)     yac_host->yacNewByID (YAC_CLID_VALUE          ) 
#define YAC_New_ListNode()        (YAC_ListNode*)        yac_host->yacNewByID (YAC_CLID_LISTNODE       )
#define YAC_New_TreeNode()        (YAC_TreeNode*)        yac_host->yacNewByID (YAC_CLID_TREENODE       )
#define YAC_New_IntArray()        (YAC_IntArray*)        yac_host->yacNewByID (YAC_CLID_INTARRAY       )
#define YAC_New_FloatArray()      (YAC_FloatArray*)      yac_host->yacNewByID (YAC_CLID_FLOATARRAY     )
#define YAC_New_StringArray()     (YAC_StringArray*)     yac_host->yacNewByID (YAC_CLID_STRINGARRAY    )
#define YAC_New_ObjectArray( )    (YAC_ObjectArray*)     yac_host->yacNewByID (YAC_CLID_OBJECTARRAY    )
#define YAC_New_ClassArray()      (YAC_ClassArray*)      yac_host->yacNewByID (YAC_CLID_CLASSARRAY     )
#define YAC_New_ValueArray()      (YAC_ValueArray*)      yac_host->yacNewByID (YAC_CLID_VALUEARRAY     )
#define YAC_New_PointerArray()                           yac_host->yacNewByID (YAC_CLID_POINTERARRAY   )
#define YAC_New_HashTable()                              yac_host->yacNewByID (YAC_CLID_HASHTABLE      )
#define YAC_New_Buffer()          (YAC_Buffer*)          yac_host->yacNewByID (YAC_CLID_BUFFER         )
#define YAC_New_File()                                   yac_host->yacNewByID (YAC_CLID_FILE           )
#define YAC_New_PakFile()                                yac_host->yacNewByID (YAC_CLID_PAKFILE        )
#define YAC_New_Pipe()                                   yac_host->yacNewByID (YAC_CLID_PIPE           )

// ---- helper macros to check whether a YAC_Object is safe to cast to the respective type
#define YAC_Is_String(a)          YAC_BCHK(a, YAC_CLID_STRING)
#define YAC_Is_Buffer(a)          YAC_BCHK(a, YAC_CLID_BUFFER)
#define YAC_Is_IntArray(a)        YAC_BCHK(a, YAC_CLID_INTARRAY)
#define YAC_Is_FloatArray(a)      YAC_BCHK(a, YAC_CLID_FLOATARRAY)
#define YAC_Is_StringArray(a)     YAC_BCHK(a, YAC_CLID_STRINGARRAY)
#define YAC_Is_HashTable(a)       YAC_BCHK(a, YAC_CLID_HASHTABLE)
#define YAC_Is_Value(a)           YAC_BCHK(a, YAC_CLID_VALUE)
#define YAC_Is_ListNode(a)        YAC_BCHK(a, YAC_CLID_LISTNODE)
#define YAC_Is_TreeNode(a)        YAC_BCHK(a, YAC_CLID_TREENODE)
#define YAC_Is_PointerArray(a)    YAC_BCHK(a, YAC_CLID_POINTERARRAY)
#define YAC_Is_File(a)            YAC_BCHK(a, YAC_CLID_FILE)
#define YAC_Is_Boolean(a)         YAC_BCHK(a, YAC_CLID_BOOLEAN)
#define YAC_Is_UnsignedByte(a )   YAC_BCHK(a, YAC_CLID_UNSIGNEDBYTE)
#define YAC_Is_UnsignedShort(a)   YAC_BCHK(a, YAC_CLID_UNSIGNEDSHORT)
#define YAC_Is_UnsignedInteger(a) YAC_BCHK(a, YAC_CLID_UNSIGNEDINTEGER)
#define YAC_Is_Byte(a)            YAC_BCHK(a, YAC_CLID_BYTE)
#define YAC_Is_Short(a)           YAC_BCHK(a, YAC_CLID_SHORT)
#define YAC_Is_Integer(a)         YAC_BCHK(a, YAC_CLID_INTEGER)
#define YAC_Is_Long(a)            YAC_BCHK(a, YAC_CLID_LONG)
#define YAC_Is_Float(a)           YAC_BCHK(a, YAC_CLID_FLOAT)
#define YAC_Is_Double(a)          YAC_BCHK(a, YAC_CLID_DOUBLE)

// ---- used for e.g. scriptclasses (YAC_Object type=Class, metaclasstype=MyClass ..) ----
#define YAC_IS_METACLASS(a) ((a)->yacMetaClassName()!=0)

// ---- object class template macros ----
// -------- regular template ----
/* template <class T> class YAC_Template  {public:T *ctemplate; public: YAC_Template  (YAC_Host *_host) { ctemplate=new T(); _host->yacRegisterClass(ctemplate, YAC_CLASSTYPE_NORMAL); } ~YAC_Template() { delete ctemplate; } };  */
/* // -------- singleton type template, may not be instanciated ---- */
/* template <class T> class YAC_STemplate {public:T *ctemplate; public: YAC_STemplate (YAC_Host *_host) { ctemplate=new T(); _host->yacRegisterClass(ctemplate, YAC_CLASSTYPE_STATIC); } ~YAC_STemplate() { delete ctemplate; } };  */
/* // -------- interface template, objects are only created by plugins, may not be instanciated but pointer variables may be declared ---- */
/* template <class T> class YAC_RTemplate {public:T *ctemplate; public: YAC_RTemplate (YAC_Host *_host) { ctemplate=new T(); _host->yacRegisterClass(ctemplate, YAC_CLASSTYPE_NOINST); } ~YAC_RTemplate() { delete ctemplate; } };  */

// Note: the template objects are declared static so that operator new is never called.
template <class T> class YAC_Template  {public: T ctemplate; YAC_Template(YAC_Host *_host) { _host->yacRegisterClass(&ctemplate, YAC_CLASSTYPE_NORMAL); } ~YAC_Template() { } }; 
// -------- singleton type template, may not be instanciated ----
template <class T> class YAC_STemplate {public: T ctemplate; YAC_STemplate(YAC_Host *_host) { _host->yacRegisterClass(&ctemplate, YAC_CLASSTYPE_STATIC); } ~YAC_STemplate() { } }; 
// -------- interface template, objects are only created by plugins, may not be instanciated but pointer variables may be declared ----
template <class T> class YAC_RTemplate {public: T ctemplate; YAC_RTemplate(YAC_Host *_host) { _host->yacRegisterClass(&ctemplate, YAC_CLASSTYPE_NOINST); } ~YAC_RTemplate() { } }; 


#ifndef YAC_NO_EXPORTS
extern YAC_Host *yac_host;
#endif

#ifdef YAC_OBJECT_TAGS
#define YAC_VALID(a) ((a)&&((a)->validation_tag==YAC_VALID_TAG))
//#define YAC_ILL(a) ((a)?((a)->validation_tag!=YAC_VALID_TAG):0)
#else
#define YAC_VALID(a) a
//#define YAC_ILL(a) 0
#endif // YAC_OBJECT_TAGS

// ---- "C" function handling (0..8 args, YAC_Value *_r return) ----
// YAC_APIC const sChar*YAC_GetFunctionStringList(void);   // called to get list of "c" functions


// ----
// ----
// ---- Epsilon float comparisons
// ----
// ----
#define YAC_FLT_EPSILON 0.000001f
#define YAC_DBL_EPSILON 0.000000000001

#ifdef YAC_EPSILONCOMPARE_ABS
// 
// absolute epsilon floating point value comparisons (taken from tks-source/tks.h)
// 
#define Dfltnonzero_abs(a) ( ((a)>YAC_FLT_EPSILON) || ((a)<-YAC_FLT_EPSILON) )
#define Dfltequal_abs(a,b) ( (((a)-YAC_FLT_EPSILON) <= (b)) && (((a)+YAC_FLT_EPSILON) >= (b)) )
#define Dfltnotequal_abs(a,b) ( (((a)-YAC_FLT_EPSILON) > (b)) || (((a)+YAC_FLT_EPSILON) < (b)) )
#define Dfltzero_abs(a) Dfltequal(a, 0.0f)

#define Ddblnonzero_abs(a) ( ((a)>YAC_DBL_EPSILON) || ((a)<-YAC_DBL_EPSILON) )
#define Ddblequal_abs(a,b) ( (((a)-YAC_DBL_EPSILON) <= (b)) && (((a)+YAC_DBL_EPSILON) >= (b)) )
#define Ddblnotequal_abs(a,b) ( (((a)-YAC_DBL_EPSILON) > (b)) || (((a)+YAC_DBL_EPSILON) < (b)) )
#define Ddblzero_abs(a) Ddblequal(a, 0.0)

#endif // YAC_EPISLONCOMPARE_ABS



#ifdef YAC_EPSILONCOMPARE_REL
//
// alternative floating point value comparisons that shift epsilon 
// with the exponent. 
// test against zero are special cases
//
// contributed by Carsten Busse <carsten.busse@gmail.com>
// 
extern sSI yac_epsilon_flt_units;
extern sS64 yac_epsilon_dbl_units;
extern sF32 yac_epsilon_flt;
extern sF64 yac_epsilon_dbl;
//fast version
YAC_APIC sSI YAC_CALL yac_fltcmp_rel_fast(sF32, sF32);
YAC_APIC sSI YAC_CALL yac_dblcmp_rel_fast(sF64, sF64);
//"real" tolerance version
YAC_APIC sSI YAC_CALL yac_fltcmp_rel(sF32, sF32, sF32);
YAC_APIC sSI YAC_CALL yac_dblcmp_rel(sF64, sF64, sF64);

#define Dfltnonzero_rel(a) ( ((a)>YAC_FLT_EPSILON) || ((a)<-YAC_FLT_EPSILON) )
#define Dfltzero_rel(a) ( ((a)<=YAC_FLT_EPSILON) && ((a)>=-YAC_FLT_EPSILON) )
#define Dfltequal_rel(a,b) ( yac_fltcmp_rel_fast((sF32)a,(sF32)(b)) == 0 )
#define Dfltnotequal_rel(a,b) ( yac_fltcmp_rel_fast((sF32)(a), (sF32)(b)) != 0 )

#define Ddblnonzero_rel(a) ( ((a)>YAC_DBL_EPSILON) || ((a)<-YAC_DBL_EPSILON) )
#define Ddblzero_rel(a) ( ((a)<=YAC_DBL_EPSILON) && ((a)>=-YAC_DBL_EPSILON) )
#define Ddblequal_rel(a,b) ( yac_dblcmp_rel_fast((sF64)(a), (sF64)(b) ) == 0 )
#define Ddblnotequal_rel(a,b) ( yac_dblcmp_rel_fast((sF64)(a), (sF64)(b) ) != 0 )

#endif // YAC_EPSILONCOMPARE_REL



// ----
// ---- Default float comparison, either abs or rel
// ----

#ifdef YAC_EPSILONCOMPARE_ABS_DEFAULT
// ---- Use absolute epsilon comparison macros by default
#define Dfltnonzero(a)     Dfltnonzero_abs  (a)
#define Dfltequal(a, b)    Dfltequal_abs    (a, b)
#define Dfltnotequal(a, b) Dfltnotequal_abs (a, b)
#define Dfltzero(a)        Dfltzero_abs     (a)
#define Ddblnonzero(a)     Ddblnonzero_abs  (a)
#define Ddblequal(a,b)     Ddblequal_abs    (a, b)
#define Ddblnotequal(a, b) Ddblnotequal_abs (a, b)
#define Ddblzero(a)        Ddblzero_abs     (a)
#elif defined(YAC_EPSILONCOMPARE_REL_DEFAULT)
// ---- Use relative epsilon comparison macros by default
#define Dfltnonzero(a)     Dfltnonzero_rel  (a)
#define Dfltequal(a, b)    Dfltequal_rel    (a, b)
#define Dfltnotequal(a, b) Dfltnotequal_rel (a, b)
#define Dfltzero(a)        Dfltzero_rel     (a)
#define Ddblnonzero(a)     Ddblnonzero_rel  (a)
#define Ddblequal(a,b)     Ddblequal_rel    (a, b)
#define Ddblnotequal(a, b) Ddblnotequal_rel (a, b)
#define Ddblzero(a)        Ddblzero_rel     (a)
#endif // YAC_EPSILONCOMPARE_REL_DEFAULT


// ----                             ----
// ----      exception helpers      ----
// ----                             ----

// Raise runtime exception "a" with message text "b" in context _ctx
#define Dyac_throw(a, b)  yac_host->yacExceptionRaise(_ctx, exid_##a, b, __FILE__, __LINE__)

// Raise runtime exception "a" with message text "b" in default context (**only use if context is not available!**)
#define Dyac_throw_def(a, b)  yac_host->yacExceptionRaise(yac_host->yacContextGetDefault(), exid_##a, b, __FILE__, __LINE__)

// Forward declarate an exception id
#define Dyac_exid_decl(a) extern YAC_ExceptionId exid_##a

// Implement exception id variable
#define Dyac_exid_impl(a) YAC_ExceptionId exid_##a

// Resolve exception id
#define Dyac_exid_resolve(a) exid_##a = yac_host->yacExceptionGetIdByName(#a)

// Register custom exception id (e=exception name, b=base class exception name)
#define Dyac_exid_register(e, b) exid_##e = yac_host->yacExceptionRegister(#e, exid_##b)

// Forward declarate all "standard" YAC/TkScript exceptions
#define Dyac_std_exid_decl \
Dyac_exid_decl(CriticalError);\
Dyac_exid_decl(UncriticalError);\
Dyac_exid_decl(InvalidPointer);\
Dyac_exid_decl(Death);\
Dyac_exid_decl(TypeMismatch);\
Dyac_exid_decl(ClassTypeMismatch);\
Dyac_exid_decl(NativeClassTypeMismatch);\
Dyac_exid_decl(ScriptClassTypeMismatch);\
Dyac_exid_decl(ClassMemberNotFound);\
Dyac_exid_decl(NativeClassMemberNotFound);\
Dyac_exid_decl(ScriptClassMemberNotFound);\
Dyac_exid_decl(ModuleNotFound);\
Dyac_exid_decl(ModuleMemberNotFound);\
Dyac_exid_decl(ArrayOutOfBounds);\
Dyac_exid_decl(ReadArrayOutOfBounds);\
Dyac_exid_decl(WriteArrayOutOfBounds);\
Dyac_exid_decl(ConstraintViolation);\
Dyac_exid_decl(NotNullConstraintViolation)

// Implement all "standard" YAC/TkScript exception id variables
#define Dyac_std_exid_impl \
Dyac_exid_impl(CriticalError);\
Dyac_exid_impl(UncriticalError);\
Dyac_exid_impl(InvalidPointer);\
Dyac_exid_impl(Death);\
Dyac_exid_impl(TypeMismatch);\
Dyac_exid_impl(ClassTypeMismatch);\
Dyac_exid_impl(NativeClassTypeMismatch);\
Dyac_exid_impl(ScriptClassTypeMismatch);\
Dyac_exid_impl(ClassMemberNotFound);\
Dyac_exid_impl(NativeClassMemberNotFound);\
Dyac_exid_impl(ScriptClassMemberNotFound);\
Dyac_exid_impl(ModuleNotFound);\
Dyac_exid_impl(ModuleMemberNotFound);\
Dyac_exid_impl(ArrayOutOfBounds);\
Dyac_exid_impl(ReadArrayOutOfBounds);\
Dyac_exid_impl(WriteArrayOutOfBounds);\
Dyac_exid_impl(ConstraintViolation);\
Dyac_exid_impl(NotNullConstraintViolation)

// Resolve all "standard" YAC/TkScript exception ids
#define Dyac_std_exid_resolve \
Dyac_exid_resolve(CriticalError);\
Dyac_exid_resolve(UncriticalError);\
Dyac_exid_resolve(InvalidPointer);\
Dyac_exid_resolve(Death);\
Dyac_exid_resolve(TypeMismatch);\
Dyac_exid_resolve(ClassTypeMismatch);\
Dyac_exid_resolve(NativeClassTypeMismatch);\
Dyac_exid_resolve(ScriptClassTypeMismatch);\
Dyac_exid_resolve(ClassMemberNotFound);\
Dyac_exid_resolve(NativeClassMemberNotFound);\
Dyac_exid_resolve(ScriptClassMemberNotFound);\
Dyac_exid_resolve(ModuleNotFound);\
Dyac_exid_resolve(ModuleMemberNotFound);\
Dyac_exid_resolve(ArrayOutOfBounds);\
Dyac_exid_resolve(ReadArrayOutOfBounds);\
Dyac_exid_resolve(WriteArrayOutOfBounds);\
Dyac_exid_resolve(ConstraintViolation);\
Dyac_exid_resolve(NotNullConstraintViolation)


#ifdef YAC_GLOBAL_NEWDELETE
extern sSI yac_global_newdelete_counter;    // Tracks currently allocated # of bytes
extern sSI yac_global_newdelete_numallocs;  // Tracks total number of calls to "new"
extern sSI yac_global_newdelete_numfrees;   // Tracks total number of calls to "delete"
#endif // YAC_GLOBAL_NEWDELETE


#endif // RACK_PLUGIN


// Local var/typecast from arg helper
#define YAC_CAST_ARG(t,l,a) t *l = (t *) a

// Local var/typecast/unlink from ValueObject arg helper
#define YAC_DEREF_ARG(t,l,a) t*l=NULL; sBool l##_deleteme=0; if(YAC_Is_Value(a)){ YAC_ValueObject*l##vo=(YAC_ValueObject*)a; if(l##vo->type >= YAC_TYPE_OBJECT) { l = (t*) l##vo->value.object_val; l##_deleteme = l##vo->deleteme; l##vo->deleteme = 0; } } else { l = (t*) a; }

// Check if 32bit float is denormalized (can cause severe slowdowns on Intel processors (faktor 5-8)
//  a float is denormalized if the exponent part is 0 and the fractional part is not
#define Dyac_chkdenorm_32(a) ((0==((*(sUI*)&(a)) & 0x7f800000u)) && (0 != ((*(sUI*)&(a)) & 0x007FFFFFu)))

// Correct denormalized 32bit float
//#define Dyac_denorm_32(a) (Dyac_chkdenorm_32(a) ? 0.0f : (a))
#define Dyac_denorm_32(a) ( ((a)+10.0f) - 10.0f ) // kb's denormalize trick!

// Check if 32bit float is (plus or minus) infinity
#define Dyac_chkinf_32(a) (0x7f800000u == ((*(sUI*)&(a)) & 0x7f800000u))

// Check if 32bit float is NaN
#define Dyac_chknan_32(a) ( ((*(sUI*)&(a)) == 0x7F820000u) || ((*(sUI*)&(a)) == 0xFF9112AAu) )

// Check if 32bit float is -0
#define Dyac_chkm0_32(a) ( ((*(sUI*)&(a)) == 0x80000000u) )

// Debug: print warning if float is denormalized, infinity, or NaN
#define Dyac_dbgflt_32(a) if(1) { \
      if(Dyac_chkdenorm_32(a)) printf(#a " denorm\n"); \
      if(Dyac_chkinf_32(a)) printf(#a " inf\n"); \
      if(Dyac_chknan_32(a)) printf(#a " NaN\n"); \
      if(Dyac_chkm0_32(a)) printf(#a " m0\n"); \
   } else (void)0

#endif // ifndef __YAC_H__
