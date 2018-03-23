#include "util/common.hpp"
#include "asset.hpp"
#include <stdarg.h>


namespace rack {


static FILE *logFile = stderr;
static auto startTime = std::chrono::high_resolution_clock::now();


void loggerInit() {
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

static void printTimestamp() {
}

static void printLog(const char *type, const char *format, va_list args) {
	auto nowTime = std::chrono::high_resolution_clock::now();
	int duration = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - startTime).count();
	printTimestamp();
	fprintf(logFile, "[%.03f %s] ", duration / 1000.0, type);
	vfprintf(logFile, format, args);
	fprintf(logFile, "\n");
	fflush(logFile);
}

void debug(const char *format, ...) {
	va_list args;
	va_start(args, format);
	printLog("debug", format, args);
	va_end(args);
}

void info(const char *format, ...) {
	va_list args;
	va_start(args, format);
	printLog("info", format, args);
	va_end(args);
}

void warn(const char *format, ...) {
	va_list args;
	va_start(args, format);
	printLog("warn", format, args);
	va_end(args);
}

void fatal(const char *format, ...) {
	va_list args;
	va_start(args, format);
	printLog("fatal", format, args);
	va_end(args);
}


} // namespace rack
