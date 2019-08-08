#pragma once
#include <common.hpp>
#include <vector>


namespace rack {
namespace tag {


/** Searches for a tag ID.
Searches tag aliases.
Case-insensitive.
Returns -1 if not found.
*/
int findId(const std::string& tag);


/** List of tags and their aliases.
The first alias of each tag `tag[tagId][0]` is the "canonical" alias.
It is guaranteed to exist and should be used as the human-readable form.

This list is manually alphabetized by the canonical alias.
*/
extern const std::vector<std::vector<std::string>> tagAliases;


} // namespace tag
} // namespace rack
