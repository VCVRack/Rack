#ifndef TSOSCCOMMUNICATOR_HPP
#define TSOSCCOMMUNICATOR_HPP

#include <thread> // std::thread
#include <mutex>
#include <string.h>
//#include <stdio.h>
#include <map>

#include "../lib/oscpack/osc/OscOutboundPacketStream.h"
#include "../lib/oscpack/ip/UdpSocket.h"
#include "../lib/oscpack/osc/OscReceivedElements.h"
#include "../lib/oscpack/osc/OscPacketListener.h"

#include "TSOSCSequencerListener.hpp"

// OSC connection information
typedef struct TSOSCInfo {	
	// OSC output IP address.
	std::string oscTxIpAddress;
	// OSC output port number.
	uint16_t oscTxPort;
	// OSC input port number.
	uint16_t oscRxPort;
} TSOSCConnectionInfo;


// OSC Connection / track ports used to try to auto-increment and probably eventually allow multiple guys to talk on the same
// ports just with namespace or id routing.
class TSOSCConnector
{
public:
	static TSOSCConnector* Connector();
	// Get an id for the module instance.
	int getId();
	// Register the usage of these ports.
	bool registerPorts(int id, uint16_t txPort, uint16_t rxPort);
	// Clear the usage of these ports.
	bool clearPorts(int id, uint16_t txPort, uint16_t rxPort);
	// Register the usage of these ports.
	bool registerPort(int id, uint16_t port);
	// Clear the usage of these ports.
	bool clearPort(int id, uint16_t port);
	// Get an available port.
	uint16_t getAvailablePort(int id, uint16_t desiredPort);
	// See if the port is in use (returns the id of the module using it or 0 if it is free).
	int portInUse(uint16_t port);

	// Get an id for the module instance.
	static int GetId() { return Connector()->getId(); }
	// Get an available port.
	static uint16_t GetAvailablePort(int id, uint16_t desiredPort) { return Connector()->getAvailablePort(id, desiredPort); }
	// Register the usage of these ports.
	static bool RegisterPorts(int id, uint16_t txPort, uint16_t rxPort) { return Connector()->registerPorts(id, txPort, rxPort); }
	// Clear the usage of these ports.
	static bool ClearPorts(int id, uint16_t txPort, uint16_t rxPort) { return Connector()->clearPorts(id, txPort, rxPort);}
	// Register the usage of these ports.
	static bool RegisterPort(int id, uint16_t port) { return Connector()->registerPort(id, port); }
	// Clear the usage of these ports.
	static bool ClearPort(int id, uint16_t port) { return Connector()->clearPort(id, port); }

	// See if the port is in use (returns the id of the module using it or 0 if it is free).
	static int PortInUse(uint16_t port) { return Connector()->portInUse(port); }
private:
	TSOSCConnector();
	//TSOSCConnector(TSOSCConnector const&) {};             // copy constructor is private
	//TSOSCConnector& operator=(TSOSCConnector const&) { return this; };  // assignment operator is private

	static TSOSCConnector* _instance;
	// The last id we gave out.
	int _lastId;
	// The ports that are in use by which id.
	std::map<uint16_t, int> _portMap;

	// Port mutex.
	std::mutex _mutex;
};

#endif // !TSOSCCOMMUNICATOR_HPP
