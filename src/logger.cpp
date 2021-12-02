#include <mutex>

#include <common.hpp>
#include <asset.hpp>
#include <system.hpp>
#include <settings.hpp>
// #include <unistd.h> // for dup2


namespace rack {
namespace logger {


std::string logPath;
static FILE* outputFile = NULL;
static std::mutex mutex;
static bool truncated = false;


static bool fileEndsWith(FILE* file, std::string str) {
	// Seek to last `len` characters
	size_t len = str.size();
	std::fseek(file, -long(len), SEEK_END);
	char actual[len];
	if (std::fread(actual, 1, len, file) != len)
		return false;
	return std::string(actual, len) == str;
}

static bool isTruncated() {
	if (logPath.empty())
		return false;

	// Open existing log file
	FILE* file = std::fopen(logPath.c_str(), "r");
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


void init() {
	assert(!outputFile);
	std::lock_guard<std::mutex> lock(mutex);
	truncated = false;

	// Don't open a file in development mode.
	if (logPath.empty()) {
		outputFile = stderr;
	}
	else {
		truncated = isTruncated();

		outputFile = std::fopen(logPath.c_str(), "w");
		if (!outputFile) {
			std::fprintf(stderr, "Could not open log at %s\n", logPath.c_str());
		}
	}

	// Redirect stdout and stderr to the file
	// Actually, disable this because we don't want to steal stdout/stderr from the DAW in Rack for DAWs.
	// dup2(fileno(outputFile), fileno(stdout));
	// dup2(fileno(outputFile), fileno(stderr));
}

void destroy() {
	std::lock_guard<std::mutex> lock(mutex);
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
	"fatal",
};

static const int levelColors[] = {
	35,
	34,
	33,
	31,
};

static void logVa(Level level, const char* filename, int line, const char* func, const char* format, va_list args) {
	if (!outputFile)
		return;
	double nowTime = system::getTime();
	std::lock_guard<std::mutex> lock(mutex);

	if (outputFile == stderr)
		std::fprintf(outputFile, "\x1B[%dm", levelColors[level]);
	std::fprintf(outputFile, "[%.03f %s %s:%d %s] ", nowTime, levelLabels[level], filename, line, func);
	if (outputFile == stderr)
		std::fprintf(outputFile, "\x1B[0m");
	std::vfprintf(outputFile, format, args);
	std::fprintf(outputFile, "\n");
	// Note: This adds around 10us, but it's important for logging to finish writing the file, and logging is not used in performance critical code.
	std::fflush(outputFile);
}

void log(Level level, const char* filename, int line, const char* func, const char* format, ...) {
	va_list args;
	va_start(args, format);
	logVa(level, filename, line, func, format, args);
	va_end(args);
}

bool wasTruncated() {
	return truncated;
}


} // namespace logger
} // namespace rack
