#include "plugin/Plugin.hpp"
#include "plugin/Model.hpp"


namespace rack {


Plugin::~Plugin() {
	for (Model *model : models) {
		delete model;
	}
}

void Plugin::addModel(Model *model) {
	assert(!model->plugin);
	model->plugin = this;
	models.push_back(model);
}


} // namespace rack
