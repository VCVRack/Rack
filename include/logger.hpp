#pragma once


/** Example usage:

	DEBUG("error: %d", errno);

will print something like

	[0.123 debug myfile.cpp:45] error: 67
*/
#define DEBUG(format, ...) logger::log(rack::logger::DEBUG_LEVEL, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define INFO(format, ...) logger::log(rack::logger::INFO_LEVEL, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define WARN(format, ...) logger::log(rack::logger::WARN_LEVEL, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define FATAL(format, ...) logger::log(rack::logger::FATAL_LEVEL, __FILE__, __LINE__, format, ##__VA_ARGS__)


namespace rack {


/** Logs messages to a file or the console with decoration
*/
namespace logger {


enum Level {
	DEBUG_LEVEL,
	INFO_LEVEL,
	WARN_LEVEL,
	FATAL_LEVEL
};

void init();
void destroy();
/** Do not use this function directly. Use the macros above.
Thread-safe, meaning messages cannot overlap each other in the log.
*/
void log(Level level, const char *filename, int line, const char *format, ...);


} // namespace logger
} // namespace rack
