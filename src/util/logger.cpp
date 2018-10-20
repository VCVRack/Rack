#include "global_pre.hpp"
#include "util/common.hpp"
#include "asset.hpp"
#include <stdarg.h>
#include "global.hpp"


namespace rack {

// #define LOG_STDFILE stderr
#define LOG_STDFILE stdout


void loggerInit(bool devMode) {
#ifdef RACK_HOST
	global->logger.startTime = std::chrono::high_resolution_clock::now();
	if (devMode) {
		global->logger.logFile = LOG_STDFILE;
	}
	else {
		std::string logFilename = assetLocal("log.txt");
		global->logger.logFile = fopen(logFilename.c_str(), "w");
	}
#endif // RACK_HOST
}

void loggerDestroy() {
#ifdef RACK_HOST
	if (global->logger.logFile != LOG_STDFILE) {
		fclose(global->logger.logFile);
	}
#endif // RACK_HOST
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
#ifdef RACK_HOST
	auto nowTime = std::chrono::high_resolution_clock::now();
	int duration = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - global->logger.startTime).count();
	if (global->logger.logFile == LOG_STDFILE)
		fprintf(global->logger.logFile, "\x1B[%dm", loggerColor[level]);
	fprintf(global->logger.logFile, "[%.03f %s %s:%d] ", duration / 1000.0, loggerText[level], file, line);
	if (global->logger.logFile == LOG_STDFILE)
		fprintf(global->logger.logFile, "\x1B[0m");
	vfprintf(global->logger.logFile, format, args);

	// TODO
	// At least for me, this will cause a complete crash from __strlen_avx512
   // vprintf(format, args); // xxx

   printf("\n"); // xxx
	fprintf(global->logger.logFile, "\n");
	fflush(global->logger.logFile);
#endif // RACK_HOST
}

void loggerLog(LoggerLevel level, const char *file, int line, const char *format, ...) {
#ifdef RACK_HOST
	va_list args;
	va_start(args, format);
	loggerLogVa(level, file, line, format, args);
	va_end(args);
#endif // RACK_HOST
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
