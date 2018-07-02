#include "global_pre.hpp"
#include "util/common.hpp"
#include "asset.hpp"
#include <stdarg.h>
#include "global.hpp"


namespace rack {


void loggerInit(bool devMode) {
	global->logger.startTime = std::chrono::high_resolution_clock::now();
	if (devMode) {
		global->logger.logFile = stderr;
	}
	else {
		std::string logFilename = assetLocal("log.txt");
		global->logger.logFile = fopen(logFilename.c_str(), "w");
	}
}

void loggerDestroy() {
	if (global->logger.logFile != stderr) {
		fclose(global->logger.logFile);
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
	int duration = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - global->logger.startTime).count();
	if (global->logger.logFile == stderr)
		fprintf(global->logger.logFile, "\x1B[%dm", loggerColor[level]);
	fprintf(global->logger.logFile, "[%.03f %s %s:%d] ", duration / 1000.0, loggerText[level], file, line);
	if (global->logger.logFile == stderr)
		fprintf(global->logger.logFile, "\x1B[0m");
	vfprintf(global->logger.logFile, format, args);
   vprintf(format, args); // xxx
   printf("\n"); // xxx
	fprintf(global->logger.logFile, "\n");
	fflush(global->logger.logFile);
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
