#pragma once
#include <vector>
#include <common.hpp>
#include <midi.hpp>


namespace rack {
namespace midiloopback {


struct Device;


struct Context {
	std::vector<Device*> devices;

	Context();
	~Context();
};


PRIVATE void init();


} // namespace midiloopback
} // namespace rack
