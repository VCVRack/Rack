#include "TSOSCCommunicator.hpp"

#include <thread> // std::thread
#include <string.h>
#include <stdio.h>
#include <map>
#include <mutex>
//#include "util.hpp"

#define MIN_PORT	1000
#define MAX_PORT	0xFFFF

// The connector
TSOSCConnector* TSOSCConnector::_instance = NULL;

TSOSCConnector::TSOSCConnector()
{
	_lastId = 0;
	return;
}

TSOSCConnector* TSOSCConnector::Connector()
{
	if (!_instance)
		_instance = new TSOSCConnector();
	return _instance;
}
// Get an id for the module instance.
int TSOSCConnector::getId()
{
	std::lock_guard<std::mutex> lock(_mutex);
	return ++_lastId;
}
// Register the usage of these ports.
bool TSOSCConnector::registerPorts(int id, uint16_t txPort, uint16_t rxPort)
{
	std::lock_guard<std::mutex> lock(_mutex);
	int tx = -1;
	int rx = -1;
	if (_portMap.count(txPort) < 1 || _portMap[txPort] == id)
	{
		tx = txPort;
	}
	if (_portMap.count(rxPort) < 1 || _portMap[rxPort] == id)
	{
		rx = rxPort;
	}
	if (tx > -1 && rx > -1)
	{
		_portMap[txPort] = id;
		_portMap[rxPort] = id;
		return true;
	}
	else
	{
		return false; // Do nothing
	}
}
// Register the usage of these ports.
bool TSOSCConnector::registerPort(int id, uint16_t port)
{
	std::lock_guard<std::mutex> lock(_mutex);
	int tx = -1;
	if (_portMap.count(port) < 1 || _portMap[port] == id)
	{
		tx = port;
	}
	if (tx > -1)
	{
		_portMap[port] = id;
		return true;
	}
	else
	{
		return false; // Do nothing
	}
}
// Clear the usage of these ports.
bool TSOSCConnector::clearPorts(int id, uint16_t txPort, uint16_t rxPort)
{
	std::lock_guard<std::mutex> lock(_mutex);
	std::map<uint16_t, int>::iterator it;
	int nErased = 0;
	it = _portMap.find(txPort);
	if (it != _portMap.end() && _portMap[txPort] == id)
	{
		_portMap.erase(it);
		nErased++;
	}
	it = _portMap.find(rxPort);
	if (it != _portMap.end() && _portMap[rxPort] == id)
	{
		_portMap.erase(it);
		nErased++;
	}
	return nErased == 2;
}
// Clear the usage of these ports.
bool TSOSCConnector::clearPort(int id, uint16_t port)
{
	std::lock_guard<std::mutex> lock(_mutex);
	std::map<uint16_t, int>::iterator it;
	it = _portMap.find(port);
	if (it != _portMap.end() && _portMap[port] == id)
	{
		_portMap.erase(it);
		return true;
	}
	return false;
}

// See if the port is in use (returns the id of the module using it or 0 if it is free).
int TSOSCConnector::portInUse(uint16_t port)
{
	std::lock_guard<std::mutex> lock(_mutex);
	std::map<uint16_t, int>::iterator it;
	int id = 0;
	it = _portMap.find(port);
	if (it != _portMap.end())
	{
		id = _portMap[port];
	}
	return id;
}

// Get an available port.
uint16_t TSOSCConnector::getAvailablePort(int id, uint16_t desiredPort)
{
	std::lock_guard<std::mutex> lock(_mutex);
	bool portFound = false;
	uint16_t port = desiredPort;
	uint16_t portA, portB;
	if (_portMap.count(port) < 1 || _portMap[port] == id)
	{
		portFound = true;
	}
	else
	{
		portA = port;
		portB = port;
		while (!portFound && (portA < MAX_PORT || portB > MIN_PORT))
		{
			if (portA < MAX_PORT)
			{
				portA += 2;
				portFound = _portMap.count(portA) < 1 || _portMap[portA] == id;
				if (portFound)
					port = portA;
			}
			if (!portFound && portB > MIN_PORT)
			{
				portB -= 2;
				portFound = _portMap.count(portB) < 1 || _portMap[portB] == id;
				if (portFound)
					port = portB;
			}
		} // end while
	}
	return (portFound) ? port : 0;
}
