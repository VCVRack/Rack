#include "DS.hpp"

float DS_Module::midpoint() {
	return (voltage0 * 0.5f + voltage1 * 0.5f);
}

json_t *DS_Module::toJson() {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "voltage0", json_real(voltage0));
	json_object_set_new(rootJ, "voltage1", json_real(voltage1));
	return rootJ;
}

void DS_Module::fromJson(json_t *rootJ) {
	json_t *j0 = json_object_get(rootJ, "voltage0");
	if (j0)
		voltage0 = json_number_value(j0);
	json_t *j1 = json_object_get(rootJ, "voltage1");
	if (j1)
		voltage1 = json_number_value(j1);
}

void DS_Module::onReset() {
	voltage0 = 0.0f;
	voltage1 = 10.0f;
}

float DS_Module::output(int state) {
	return state?voltage1:voltage0;
} 

void DS_Module::appendContextMenu(Menu *menu) {
	menu->addChild(MenuEntry::create());
	DS_MenuItem *m = MenuItem::create<DS_MenuItem>("Range 0V - 1V");
	m->module = this;
	m->vl = 0.0f;
	m->vh = 1.0f;
	menu->addChild(m);
	m = MenuItem::create<DS_MenuItem>("Range 0V - 5V");
	m->module = this;
	m->vl = 0.0f;
	m->vh = 5.0f;
	menu->addChild(m);
	m = MenuItem::create<DS_MenuItem>("Range 0V - 10V");
	m->module = this;
	m->vl = 0.0f;
	m->vh = 10.0f;
	menu->addChild(m);
	m = MenuItem::create<DS_MenuItem>("Range -5V - 5V");
	m->module = this;
	m->vl = -5.0f;
	m->vh = 5.0f;
	menu->addChild(m);
	m = MenuItem::create<DS_MenuItem>("Range -10V - 10V");
	m->module = this;
	m->vl = -10.0f;
	m->vh = 10.0f;
	menu->addChild(m);
}

void DS_MenuItem::onAction(EventAction &e) {
	module->voltage0 = vl;
	module->voltage1 = vh;
}

void DS_MenuItem::step() {
	rightText = CHECKMARK((module->voltage0 == vl) && (module->voltage1 == vh));
}

float DS_Schmitt::high(float v0, float v1) {
	return (v0 * 0.4f + v1 * 0.6f);
}

float DS_Schmitt::low(float v0, float v1) {
	return (v0 * 0.6f + v1 * 0.4f);
}

void DS_Schmitt::reset() {
	_state = 0;
}

void DS_Schmitt::set() {
	_state = 1;
}

void DS_Schmitt::set(int state) {
	_state = state;
}

int DS_Schmitt::state(float vl, float vh, float v) {
	if (_state) {
		if (v < vl)
			_state = 0;
	}
	else {
		if (v > vh)
			_state = 1;
	}
	return _state;
}

int DS_Schmitt::state(DS_Module *module, float v) {
	return state(low(module->voltage0, module->voltage1), high(module->voltage0, module->voltage1), v);
}

int DS_Schmitt::edge(float vl, float vh, float v) {
	int old = _state;
	return (state(vl, vh, v) != old);
}
	
int DS_Schmitt::edge(DS_Module *module, float v) {
	int old = _state;
	return (state(module, v) != old);
}

int DS_Schmitt::edge(float vl, float vh, float v, int falling) {
	return falling?fedge(vl, vh, v):redge(vl, vh, v);
}

int DS_Schmitt::edge(DS_Module *module, float v, int falling) {
	return falling?fedge(module, v):redge(module, v);
}

int DS_Schmitt::redge(float vl, float vh, float v) {
	int old = _state;
	return (state(vl, vh, v) && !old);
}

int DS_Schmitt::redge(DS_Module *module, float v) {
	int old = _state;
	return (state(module, v) && !old);
}

int DS_Schmitt::fedge(float vl, float vh, float v) {
	int old = _state;
	return (!state(vl, vh, v) && old);
}

int DS_Schmitt::fedge(DS_Module *module, float v) {
	int old = _state;
	return (!state(module, v) && old);
}
