==============================================================================

        FFTReal
        Version 2.11

        Fourier transformation (FFT, IFFT) library specialised for real data
        Portable ISO C++

        Copyright (c) 1999-2010 Laurent de Soras
        Object Pascal port (c) Frederic Vanmol

==============================================================================



Contents:

1. Legal
2. Content
3. Using FFTReal
3.1 FFTReal - Length fixed at run-time
3.2 FFTRealFixLen - Length fixed at compile-time
3.3 Data organisation
4. Compilation and testing
5. History
6. Contact



1. Legal
--------

FFTReal is distributed under the terms of the Do What The Fuck You Want To
Public License. 

Check the file license.txt to get full information about the license.



2. Content
----------

FFTReal is a library to compute Discrete Fourier Transforms (DFT) with the
FFT algorithm (Fast Fourier Transform) on arrays of real numbers. It can
also compute the inverse transform.

You should find in this package a lot of files ; some of them are of
particular interest:
- readme.txt          : you are reading it
- ffft/FFTReal.h      : FFT, length fixed at run-time
- ffft/FFTRealFixLen.h: FFT, length fixed at compile-time
- delphi/FFTReal.pas  : Pascal implementation (working but not up-to-date)



3. Using FFTReal
----------------

Important - if you were using older versions of FFTReal (up to 1.03), some
things have changed. FFTReal is now a template. Therefore use FFTReal<float>
or FFTReal<double> in your code depending on the application datatype. The
flt_t typedef has been removed. And if you were previously using FFTReal 2.0,
note that all the classes have moved to the ffft namespace.

You have two ways to use FFTReal. In the first way, the FFT has its length
fixed at run-time, when the object is instanciated. It means that you have
not to know the length when you write the code. This is the usual way of
proceeding.


3.1 FFTReal - Length fixed at run-time
--------------------------------------

Just instanciate one time a FFTReal object. Specify the data type you want
as template parameter (only floating point: float, double, long double or
custom type). The constructor precompute a lot of things, so it may be a bit
long. The parameter is the number of points used for the next FFTs. It must
be a power of 2:

   #include "ffft/FFTReal.h"
   ...
   long len = 1024;
   ...
   // 1024-point FFT object constructed.
   ffft::FFTReal <float> fft_object (len);

Then you can use this object to compute as many FFTs and IFFTs as you want.
They will be computed very quickly because a lot of work has been done in the
object construction.

   float x [1024];
   float f [1024];

   ...
   fft_object.do_fft (f, x);     // x (real) --FFT---> f (complex)
   ...
   fft_object.do_ifft (f, x);    // f (complex) --IFFT--> x (real)
   fft_object.rescale (x);       // Post-scaling should be done after FFT+IFFT
   ...

x [] and f [] are floating point number arrays. x [] is the real number
sequence which we want to compute the FFT. f [] is the result, in the
"frequency" domain. f has the same number of elements as x [], but f []
elements are complex numbers. The routine uses some FFT properties to
optimize memory and to reduce calculations: the transformaton of a real
number sequence is a conjugate complex number sequence: F [k] = F [-k]*.


3.2 FFTRealFixLen - Length fixed at compile-time
------------------------------------------------

This class is significantly faster than the previous one, giving a speed
gain between 50 and 100 %. The template parameter is the base-2 logarithm of
the FFT length. The datatype is float; it can be changed by modifying the
DataType typedef in FFTRealFixLenParam.h. As FFTReal class, it supports
only floating-point types or equivalent.

Use is similar as the one of FFTReal. To instanciate the object, just proceed
as indicated below:

   #include "ffft/FFTRealFixLen.h"
   ...
   // 1024-point (2^10) FFT object constructed.
   ffft::FFTRealFixLen <10> fft_object;

Warning: long FFT objects may take a very long time to compile, depending on
the compiler and its optimisation options. If compilation time is too high,
encapsulate the FFT object in a seprate class whose header doesn't need
to include FFTRealFixLen.h, so you just have to compile the wrapper once
and only link it the other times. For example (quick, dirty and incomplete):

ffft/FFTWrapper.h:            | ffft/FFTWrapper.cpp:
                              |
class FFTWrapper              | #include "ffft/FFTRealFixLen.h"
{                             | #include "ffft/FFTWrapper.h"
public:                       |
   FFTWrapper ();             | FFTWrapper::FFTWrapper ()
   ~FFTWrapper ();            | :  _impl_ptr ((void*) new FTRealFixLen <10>)
   void  do_fft (...);        | {
   void  do_ifft (...);       | }
private:                      |
   void *_impl_ptr;           | ...
}                             |


3.3 Data organisation
---------------------

Mathematically speaking, DFT formulas below show what does FFTReal:

do_fft() : f(k) = sum (p = 0, N-1, x(p) * exp (+j*2*pi*k*p/N))
do_ifft(): x(k) = sum (p = 0, N-1, f(p) * exp (-j*2*pi*k*p/N))

Where j is the square root of -1. The formulas differ only by the sign of
the exponential. When the sign is positive, the transform is called positive.
Common formulas for Fourier transform are negative for the direct tranform and
positive for the inverse one.

However in these formulas, f is an array of complex numbers and doesn't
correspound exactly to the f[] array taken as function parameter. The
following table shows how the f[] sequence is mapped onto the usable FFT
coefficients (called bins):

   FFTReal output | Positive FFT equiv.   | Negative FFT equiv.
   ---------------+-----------------------+-----------------------
   f [0]          | Real (bin 0)          | Real (bin 0)
   f [...]        | Real (bin ...)        | Real (bin ...)
   f [length/2]   | Real (bin length/2)   | Real (bin length/2)
   f [length/2+1] | Imag (bin 1)          | -Imag (bin 1)
   f [...]        | Imag (bin ...)        | -Imag (bin ...)
   f [length-1]   | Imag (bin length/2-1) | -Imag (bin length/2-1)

And FFT bins are distributed in f [] as above:

               |                | Positive FFT    | Negative FFT
   Bin         | Real part      | imaginary part  | imaginary part
   ------------+----------------+-----------------+---------------
   0           | f [0]          | 0               | 0
   1           | f [1]          | f [length/2+1]  | -f [length/2+1]
   ...         | f [...],       | f [...]         | -f [...]
   length/2-1  | f [length/2-1] | f [length-1]    | -f [length-1]
   length/2    | f [length/2]   | 0               | 0
   length/2+1  | f [length/2-1] | -f [length-1]   | f [length-1]
   ...         | f [...]        | -f [...]        | f [...]
   length-1    | f [1]          | -f [length/2+1] | f [length/2+1]

f [] coefficients have the same layout for FFT and IFFT functions. You may
notice that scaling must be done if you want to retrieve x after FFT and IFFT.
Actually, IFFT (FFT (x)) = x * length(x). This is a not a problem because
most of the applications don't care about absolute values. Thus, the operation
requires less calculation. If you want to use the FFT and IFFT to transform a
signal, you have to apply post- (or pre-) processing yourself. Multiplying
or dividing floating point numbers by a power of 2 doesn't generate extra
computation noise.



4. Compilation and testing
--------------------------

Drop the following files into your project or makefile:

ffft/Array.*
ffft/def.h
ffft/DynArray.*
ffft/FFTReal*.h*
ffft/OscSinCos.*

Other files are for testing purpose only, do not include them if you just need
to use the library; they are not needed to use FFTReal in your own programs.

FFTReal may be compiled in two versions: release and debug. Debug version
has checks that could slow down the code. Define NDEBUG to set the Release
mode. For example, the command line to compile the test bench on GCC would
look like:

Debug mode:
g++ -Wall -I. -o ./fftreal_debug.exe ffft/test/*.cpp ffft/test/stopwatch/*.cpp

Release mode:
g++ -Wall -I. -o ./fftreal_release.exe -DNDEBUG -O3 ffft/test/*.cpp ffft/test/stopwatch/*.cpp

It may be tricky to compile the test bench because the speed tests use the
stopwatch sub-library, which is not that cross-platform. If you encounter
any problem that you cannot easily fix while compiling it, edit the file
ffft/test/conf.h and un-define the speed test macro. Remove the stopwatch
directory from your source file list, too.

If it's not done by default, you should activate the exception handling
of your compiler to get the class memory-leak-safe. Thus, when a memory
allocation fails (in the constructor), an exception is thrown and the entire
object is safely destructed. It reduces the permanent error checking overhead
in the client code. Also, the test bench requires Run-Time Type Information
(RTTI) to be enabled in order to display the names of the tested classes -
sometimes mangled, depending on the compiler.

Please note: the test bench may take an insane time to compile, especially in
Release mode, because a lot of recursive templates are instanciated.



5. History
----------

v2.11 (2010.09.12)
- The LGPL was not well suited to 100% template code, therefore I changed
the license again. Everything is released under the WTFPL.
- Removed warnings in the testcode on MSVC++ 8.0
- Fixed the multiple definition linking error with template specialisations
on GCC 4.

v2.10 (2008.05.28)
- Classes are now in the ffft namespace
- Changed directory structure
- Fixed compilation information in the documentation

v2.00 (2005.10.18)
- Turned FFTReal class into template (data type as parameter)
- Added FFTRealFixLen
- Trigonometric tables are size-limited in order to preserve cache memory;
over a given size, sin/cos functions are computed on the fly.
- Better test bench for accuracy and speed
- Changed license to LGPL

v1.03 (2001.06.15)
- Thanks to Frederic Vanmol for the Pascal port (works with Delphi).
- Documentation improvement

v1.02 (2001.03.25)
- sqrt() is now precomputed when the object FFTReal is constructed, resulting
in speed impovement for small size FFT.

v1.01 (2000)
- Small modifications, I don't remember what.

v1.00 (1999.08.14)
- First version released



6. Contact
----------

Please address any comment, bug report or flame to:

Laurent de Soras
laurent.de.soras@free.fr
http://ldesoras.free.fr

For the Pascal port:
Frederic Vanmol
frederic@fruityloops.com

