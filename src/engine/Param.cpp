#include <engine/Param.hpp>


namespace rack {
namespace engine {


json_t* Param::toJson() {
	json_t* rootJ = json_object();
	json_object_set_new(rootJ, "value", json_real(value));
	return rootJ;
}


void Param::fromJson(json_t* rootJ) {
	json_t* valueJ = json_object_get(rootJ, "value");
	if (valueJ)
		value = json_number_value(valueJ);
}


} // namespace engine
} // namespace rack
