#include "util/common.hpp"
#include <stdarg.h>


namespace rack {


FILE *gLogFile = stderr;

void debug(const char *format, ...) {
	va_list args;
	va_start(args, format);
	fprintf(gLogFile, "[debug] ");
	vfprintf(gLogFile, format, args);
	fprintf(gLogFile, "\n");
	fflush(gLogFile);
	va_end(args);
}

void info(const char *format, ...) {
	va_list args;
	va_start(args, format);
	fprintf(gLogFile, "[info] ");
	vfprintf(gLogFile, format, args);
	fprintf(gLogFile, "\n");
	fflush(gLogFile);
	va_end(args);
}

void warn(const char *format, ...) {
	va_list args;
	va_start(args, format);
	fprintf(gLogFile, "[warning] ");
	vfprintf(gLogFile, format, args);
	fprintf(gLogFile, "\n");
	fflush(gLogFile);
	va_end(args);
}

void fatal(const char *format, ...) {
	va_list args;
	va_start(args, format);
	fprintf(gLogFile, "[fatal] ");
	vfprintf(gLogFile, format, args);
	fprintf(gLogFile, "\n");
	fflush(gLogFile);
	va_end(args);
}


} // namespace rack
