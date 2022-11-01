#pragma once
#include <common.hpp>
#include <midi.hpp>


namespace rack {
namespace midiloopback {


struct Device;


struct Context {
	Device* devices[1] = {};

	Context();
	~Context();
};


PRIVATE void init();


} // namespace midiloopback
} // namespace rack
