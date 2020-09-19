#include <mutex>

#include <common.hpp>
#include <asset.hpp>
#include <settings.hpp>
#include <system.hpp>
#include <unistd.h> // for dup2


namespace rack {
namespace logger {


static FILE* outputFile = NULL;
static int64_t startTime = 0;
static std::mutex logMutex;


void init() {
	startTime = system::getNanoseconds();
	// Don't open a file in development mode.
	if (settings::devMode) {
		outputFile = stderr;
		return;
	}

	assert(!outputFile);
	outputFile = std::fopen(asset::logPath.c_str(), "w");
	if (!outputFile) {
		std::fprintf(stderr, "Could not open log at %s\n", asset::logPath.c_str());
	}

	// Redirect stdout and stderr to the file
	// Actually, disable this because we don't want to steal stdout/stderr from the DAW in Rack for DAWs.
	// dup2(fileno(outputFile), fileno(stdout));
	// dup2(fileno(outputFile), fileno(stderr));
}

void destroy() {
	if (outputFile && outputFile != stderr) {
		// Print end token so we know if the logger exited cleanly.
		std::fprintf(outputFile, "END");
		std::fclose(outputFile);
	}
	outputFile = NULL;
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

static void logVa(Level level, const char* filename, int line, const char* func, const char* format, va_list args) {
	std::lock_guard<std::mutex> lock(logMutex);
	if (!outputFile)
		return;

	int64_t nowTime = system::getNanoseconds();
	double duration = (nowTime - startTime) / 1e9;
	if (outputFile == stderr)
		std::fprintf(outputFile, "\x1B[%dm", levelColors[level]);
	std::fprintf(outputFile, "[%.03f %s %s:%d %s] ", duration, levelLabels[level], filename, line, func);
	if (outputFile == stderr)
		std::fprintf(outputFile, "\x1B[0m");
	std::vfprintf(outputFile, format, args);
	std::fprintf(outputFile, "\n");
	std::fflush(outputFile);
}

void log(Level level, const char* filename, int line, const char* func, const char* format, ...) {
	va_list args;
	va_start(args, format);
	logVa(level, filename, line, func, format, args);
	va_end(args);
}

static bool fileEndsWith(FILE* file, std::string str) {
	// Seek to last `len` characters
	size_t len = str.size();
	std::fseek(file, -long(len), SEEK_END);
	char actual[len];
	if (std::fread(actual, 1, len, file) != len)
		return false;
	return std::string(actual, len) == str;
}

bool isTruncated() {
	if (settings::devMode)
		return false;

	// Open existing log file
	FILE* file = std::fopen(asset::logPath.c_str(), "r");
	if (!file)
		return false;
	DEFER({std::fclose(file);});

	if (fileEndsWith(file, "END"))
		return false;
	// legacy <=v1
	if (fileEndsWith(file, "Destroying logger\n"))
		return false;
	return true;
}


} // namespace logger
} // namespace rack
