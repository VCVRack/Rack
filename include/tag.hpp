#pragma once
#include <vector>

#include <common.hpp>


namespace rack {
/** Tags describing the function(s) of modules */
namespace tag {


/** List of tags and their aliases.
The first alias of each tag `tag[tagId][0]` is the "canonical" alias.
It is guaranteed to exist and should be used as the human-readable form.

This list is manually alphabetized by the canonical alias.
*/
extern const std::vector<std::vector<std::string>> tagAliases;


/** Searches for a tag ID.
Searches tag aliases.
Case-insensitive.
Returns -1 if not found.
*/
int findId(const std::string& tag);

/** Returns the main tag name by tag ID. */
std::string getTag(int tagId);


} // namespace tag
} // namespace rack
