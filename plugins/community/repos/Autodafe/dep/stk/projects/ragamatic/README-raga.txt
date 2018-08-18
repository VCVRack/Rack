This is RagaMatic (tm) by Perry Cook.
It was written for Ken Steiglitz's birthday in 1999.

Sitar and Drones are physical models.
Vocalize drums and Tabla drums are samples.

In the RagaMatic directory, type:

> make

to compile and then

> Raga.bat

to have fun and achieve inner peace.

If you ask me, I think this band needs a flute player too.  If you
like, team up and see if you can add the flute model to the project.
This requires adding a few files to the Makefile, a few lines to the
ragamat.cpp file (including how the flute player should play, etc.),
and another slider to the TCL script to control the flute's
contributions.  This might only run on the fastest machines once
you've added the flute.

Since latency isn't much of an issue in raga-land, you might bump up
the RT_BUFFER_SIZE in Stk.h to something around 1024, depending on the
speed of your machine.

All is Bliss...
All is Bliss...
