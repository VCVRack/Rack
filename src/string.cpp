#include "string.hpp"
#include <cctype> // for tolower and toupper
#include <algorithm> // for transform
#include <libgen.h> // for dirname and basename


namespace rack {
namespace string {


std::string f(const char *format, ...) {
	va_list args;
	va_start(args, format);
	// Compute size of required buffer
	int size = vsnprintf(NULL, 0, format, args);
	va_end(args);
	if (size < 0)
		return "";
	// Create buffer
	std::string s;
	s.resize(size);
	va_start(args, format);
	vsnprintf(&s[0], size + 1, format, args);
	va_end(args);
	return s;
}

std::string lowercase(const std::string &s) {
	std::string r = s;
	std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) { return std::tolower(c); });
	return r;
}

std::string uppercase(const std::string &s) {
	std::string r = s;
	std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) { return std::toupper(c); });
	return r;
}

std::string trim(const std::string &s) {
	const std::string whitespace = " \n\r\t";
	size_t first = s.find_first_not_of(whitespace);
	if (first == std::string::npos)
		return "";
	size_t last = s.find_last_not_of(whitespace);
	if (last == std::string::npos)
		return "";
	return s.substr(first, last - first + 1);
}

std::string ellipsize(const std::string &s, size_t len) {
	if (s.size() <= len)
		return s;
	else
		return s.substr(0, len - 3) + "...";
}

bool startsWith(const std::string &str, const std::string &prefix) {
	return str.substr(0, prefix.size()) == prefix;
}

bool endsWith(const std::string &str, const std::string &suffix) {
	return str.substr(str.size() - suffix.size(), suffix.size()) == suffix;
}

std::string directory(const std::string &path) {
	char *pathDup = strdup(path.c_str());
	std::string directory = dirname(pathDup);
	free(pathDup);
	return directory;
}

std::string filename(const std::string &path) {
	char *pathDup = strdup(path.c_str());
	std::string filename = basename(pathDup);
	free(pathDup);
	return filename;
}

// libgen.h defines a `basename` macro
#undef basename
std::string basename(const std::string &path) {
	size_t pos = path.rfind('.');
	if (pos == std::string::npos)
		return path;
	return std::string(path, 0, pos);
}

std::string extension(const std::string &path) {
	size_t pos = path.rfind('.');
	if (pos == std::string::npos)
		return "";
	return std::string(path, pos + 1);
}


/*
From https://github.com/forrestthewoods/lib_fts by Forrest Smith
License:

This software is dual-licensed to the public domain and under the following
license: you are granted a perpetual, irrevocable license to copy, modify,
publish, and distribute this file as you see fit.
*/
static bool fuzzy_match_recursive(const char *pattern, const char *str, int &outScore, const char *strBegin, uint8_t const *srcMatches, uint8_t *matches, int maxMatches, int nextMatch, int &recursionCount, int recursionLimit) {
	// Count recursions
	++recursionCount;
	if (recursionCount >= recursionLimit)
		return false;

	// Detect end of strings
	if (*pattern == '\0' || *str == '\0')
		return false;

	// Recursion params
	bool recursiveMatch = false;
	uint8_t bestRecursiveMatches[256];
	int bestRecursiveScore = 0;

	// Loop through pattern and str looking for a match
	bool first_match = true;
	while (*pattern != '\0' && *str != '\0') {

		// Found match
		if (std::tolower(*pattern) == std::tolower(*str)) {

			// Supplied matches buffer was too short
			if (nextMatch >= maxMatches)
				return false;

			// "Copy-on-Write" srcMatches into matches
			if (first_match && srcMatches) {
				std::memcpy(matches, srcMatches, nextMatch);
				first_match = false;
			}

			// Recursive call that "skips" this match
			uint8_t recursiveMatches[256];
			int recursiveScore;
			if (fuzzy_match_recursive(pattern, str + 1, recursiveScore, strBegin, matches, recursiveMatches, sizeof(recursiveMatches), nextMatch, recursionCount, recursionLimit)) {

				// Pick best recursive score
				if (!recursiveMatch || recursiveScore > bestRecursiveScore) {
					std::memcpy(bestRecursiveMatches, recursiveMatches, 256);
					bestRecursiveScore = recursiveScore;
				}
				recursiveMatch = true;
			}

			// Advance
			matches[nextMatch++] = (uint8_t)(str - strBegin);
			++pattern;
		}
		++str;
	}

	// Determine if full pattern was matched
	bool matched = *pattern == '\0' ? true : false;

	// Calculate score
	if (matched) {
		const int sequential_bonus = 5;            // bonus for adjacent matches
		const int separator_bonus = 30;             // bonus if match occurs after a separator
		const int camel_bonus = 0;                 // bonus if match is uppercase and prev is lower
		const int first_letter_bonus = 15;          // bonus if the first letter is matched

		const int leading_letter_penalty = 0;      // penalty applied for every letter in str before the first match
		const int max_leading_letter_penalty = 0; // maximum penalty for leading letters
		const int unmatched_letter_penalty = -1;    // penalty for every letter that doesn't matter

		// Iterate str to end
		while (*str != '\0')
			++str;

		// Initialize score
		outScore = 100;

		// Apply leading letter penalty
		int penalty = leading_letter_penalty * matches[0];
		if (penalty < max_leading_letter_penalty)
			penalty = max_leading_letter_penalty;
		outScore += penalty;

		// Apply unmatched penalty
		int unmatched = (int)(str - strBegin) - nextMatch;
		outScore += unmatched_letter_penalty * unmatched;

		// Apply ordering bonuses
		for (int i = 0; i < nextMatch; ++i) {
			uint8_t currIdx = matches[i];

			if (i > 0) {
				uint8_t prevIdx = matches[i - 1];

				// Sequential
				if (currIdx == (prevIdx + 1))
					outScore += sequential_bonus;
			}

			// Check for bonuses based on neighbor character value
			if (currIdx > 0) {
				// Camel case
				char neighbor = strBegin[currIdx - 1];
				char curr = strBegin[currIdx];
				if (std::islower(neighbor) && std::isupper(curr))
					outScore += camel_bonus;

				// Separator
				bool neighborSeparator = neighbor == '_' || neighbor == ' ';
				if (neighborSeparator)
					outScore += separator_bonus;
			}
			else {
				// First letter
				outScore += first_letter_bonus;
			}
		}
	}

	// Return best result
	if (recursiveMatch && (!matched || bestRecursiveScore > outScore)) {
		// Recursive score is better than "this"
		std::memcpy(matches, bestRecursiveMatches, maxMatches);
		outScore = bestRecursiveScore;
		return true;
	}
	else if (matched) {
		// "this" score is better than recursive
		return true;
	}
	else {
		// no match
		return false;
	}
}


float fuzzyScore(const std::string &s, const std::string &query) {
	uint8_t matches[256];
	int recursionCount = 0;
	int recursionLimit = 10;
	int score = 0;
	bool match = fuzzy_match_recursive(query.c_str(), s.c_str(), score, s.c_str(),
		NULL, matches, sizeof(matches),
		0, recursionCount, recursionLimit);
	return match ? score : 0.f;
}


} // namespace string
} // namespace rack
