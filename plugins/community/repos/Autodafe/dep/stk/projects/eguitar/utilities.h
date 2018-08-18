// Miscellaneous parsing and error functions for use with STK projects.
//
// Gary P. Scavone, 1999.

#include "FileWvOut.h"
#include "Messager.h"

void usage(char *function);

int checkArgs(int numArgs, char *args[]);

bool parseArgs(int numArgs, char *args[], stk::WvOut **output, stk::Messager& messager);
