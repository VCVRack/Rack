#include "logger.hpp"
#include "asset.hpp"
#include <chrono>


namespace rack {
namespace logger {


static FILE *outputFile = NULL;
static std::chrono::high_resolution_clock::time_point startTime;


void init(bool devMode) {
	startTime = std::chrono::high_resolution_clock::now();
	if (devMode) {
		outputFile = stderr;
	}
	else {
		std::string logFilename = asset::local("log.txt");
		outputFile = fopen(logFilename.c_str(), "w");
	}
}

void destroy() {
	if (outputFile != stderr) {
		fclose(outputFile);
	}
}

static const char* const levelLabels[] = {
	"debug",
	"info",
	"warn",
	"fatal"
};

static const int levelColors[] = {
	35,
	34,
	33,
	31
};

static void logVa(Level level, const char *filename, int line, const char *format, va_list args) {
	auto nowTime = std::chrono::high_resolution_clock::now();
	int duration = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - startTime).count();
	if (outputFile == stderr)
		fprintf(outputFile, "\x1B[%dm", levelColors[level]);
	fprintf(outputFile, "[%.03f %s %s:%d] ", duration / 1000.0, levelLabels[level], filename, line);
	if (outputFile == stderr)
		fprintf(outputFile, "\x1B[0m");
	vfprintf(outputFile, format, args);
	fprintf(outputFile, "\n");
	fflush(outputFile);
}

void log(Level level, const char *filename, int line, const char *format, ...) {
	va_list args;
	va_start(args, format);
	logVa(level, filename, line, format, args);
	va_end(args);
}


} // namespace logger
} // namespace rack
