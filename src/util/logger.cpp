#include "util/common.hpp"
#include "asset.hpp"
#include <stdarg.h>


namespace rack {


static FILE *logFile = stderr;
static std::chrono::high_resolution_clock::time_point startTime;


void loggerInit() {
	startTime = std::chrono::high_resolution_clock::now();
#ifdef RELEASE
	std::string logFilename = assetLocal("log.txt");
	logFile = fopen(logFilename.c_str(), "w");
#endif
}

void loggerDestroy() {
#ifdef RELEASE
	fclose(logFile);
#endif
}

static void loggerLogVa(const char *level, const char *file, int line, const char *format, va_list args) {

	auto nowTime = std::chrono::high_resolution_clock::now();
	int duration = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - startTime).count();
	fprintf(logFile, "[%.03f %s %s:%d] ", duration / 1000.0, level, file, line);
	vfprintf(logFile, format, args);
	fprintf(logFile, "\n");
	fflush(logFile);
}

void loggerLog(const char *level, const char *file, int line, const char *format, ...) {
	va_list args;
	va_start(args, format);
	loggerLogVa(level, file, line, format, args);
	va_end(args);
}

/** Deprecated. Included for ABI compatibility */

#undef debug
#undef info
#undef warn
#undef fatal

void debug(const char *format, ...) {
	va_list args;
	va_start(args, format);
	loggerLogVa("debug", "", 0, format, args);
	va_end(args);
}

void info(const char *format, ...) {
	va_list args;
	va_start(args, format);
	loggerLogVa("info", "", 0, format, args);
	va_end(args);
}

void warn(const char *format, ...) {
	va_list args;
	va_start(args, format);
	loggerLogVa("warn", "", 0, format, args);
	va_end(args);
}

void fatal(const char *format, ...) {
	va_list args;
	va_start(args, format);
	loggerLogVa("fatal", "", 0, format, args);
	va_end(args);
}


} // namespace rack
