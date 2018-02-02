#include <stdarg.h>
#include <string.h>
#include <random>
#include <algorithm>
#include <libgen.h> // for dirname and basename
#include <sys/time.h>

#if ARCH_WIN
#include <windows.h>
#include <shellapi.h>
#endif
