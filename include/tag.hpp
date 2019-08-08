#pragma once
#include <common.hpp>
#include <set>


namespace rack {
namespace tag {


extern const std::set<std::string> allowedTags;

std::string normalize(const std::string& tag);


} // namespace tag
} // namespace rack
