#include "util/common.hpp"
#include "asset.hpp"
#include <stdarg.h>


namespace rack {


static FILE *logFile = NULL;
static std::chrono::high_resolution_clock::time_point startTime;


void loggerInit(bool devMode) {
	startTime = std::chrono::high_resolution_clock::now();
	if (devMode) {
		logFile = stderr;
	}
	else {
		std::string logFilename = assetLocal("log.txt");
		logFile = fopen(logFilename.c_str(), "w");
	}
}

void loggerDestroy() {
	if (logFile != stderr) {
		fclose(logFile);
	}
}

static const char* const loggerText[] = {
	"debug",
	"info",
	"warn",
	"fatal"
};

static const int loggerColor[] = {
	35,
	34,
	33,
	31
};

static void loggerLogVa(LoggerLevel level, const char *file, int line, const char *format, va_list args) {
	auto nowTime = std::chrono::high_resolution_clock::now();
	int duration = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - startTime).count();
	if (logFile == stderr)
		fprintf(logFile, "\x1B[%dm", loggerColor[level]);
	fprintf(logFile, "[%.03f %s %s:%d] ", duration / 1000.0, loggerText[level], file, line);
	if (logFile == stderr)
		fprintf(logFile, "\x1B[0m");
	vfprintf(logFile, format, args);
	fprintf(logFile, "\n");
	fflush(logFile);
}

void loggerLog(LoggerLevel level, const char *file, int line, const char *format, ...) {
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
	loggerLogVa(DEBUG_LEVEL, "", 0, format, args);
	va_end(args);
}

void info(const char *format, ...) {
	va_list args;
	va_start(args, format);
	loggerLogVa(INFO_LEVEL, "", 0, format, args);
	va_end(args);
}

void warn(const char *format, ...) {
	va_list args;
	va_start(args, format);
	loggerLogVa(WARN_LEVEL, "", 0, format, args);
	va_end(args);
}

void fatal(const char *format, ...) {
	va_list args;
	va_start(args, format);
	loggerLogVa(FATAL_LEVEL, "", 0, format, args);
	va_end(args);
}


} // namespace rack
