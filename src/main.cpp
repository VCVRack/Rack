#include "5V.hpp"
#include <time.h>
#include <unistd.h>


Scene *gScene = NULL;
RackWidget *gRackWidget = NULL;


int main() {
	pluginInit();
	rackInit();
	guiInit();
	gScene = new Scene();
	// audioInit();
	// audioDeviceOpen();
	// midiInit();
	rackStart();

	// Blocks until user exits
	guiRun();

	// Cleanup
	// midiDestroy();
	// audioDeviceClose();
	// audioDestroy();
	delete gScene;
	guiDestroy();
	rackStop();
	rackDestroy();
	pluginDestroy();
  return 0;
}
