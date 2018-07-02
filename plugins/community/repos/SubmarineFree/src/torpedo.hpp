#pragma once
#include "rack.hpp"
#include "deque"
using namespace rack;

namespace Torpedo {
	// 
	// Basic shared functionality
	//

	struct BasePort {
		enum States {
			STATE_QUIESCENT,
			STATE_HEADER,
			STATE_BODY,
			STATE_TRAILER,
			STATE_ABORTING
		};
	
		enum Errors {
			ERROR_STATE,
			ERROR_COUNTER,
			ERROR_LENGTH,
			ERROR_CHECKSUM
		};
	
		unsigned int _checksum = 0;
		Module *_module;
		unsigned int _portNum;
		unsigned int _state = STATE_QUIESCENT;

		unsigned int dbg = 0;

		BasePort(Module *module, unsigned int portNum) {
			_module = module;
			_portNum = portNum;	
		}
		void addCheckSum(unsigned int byte, unsigned int counter);
		virtual int isBusy(void) {
			return (_state != STATE_QUIESCENT);
		}
		void raiseError(unsigned int errorType);
		virtual void error(unsigned int errorType) {};
		
	};
	
	//
	// Raw output port functionality. Encapsulating layers 2-5 of the OSI model
	//

	struct RawOutputPort : BasePort {
		std::string _appId;
		unsigned int _counter;
		std::string _message;
		Output *_port;

		RawOutputPort(Module *module, unsigned int portNum) : BasePort(module, portNum) {
			_port = &(_module->outputs[_portNum]);
		}

		virtual void abort();
		virtual void appId(std::string app) { _appId.assign(app); }
		virtual void completed();
		virtual void process();
		virtual void send(std::string appId, std::string message);
		virtual void send(std::string message);
	};

	//
	// Raw input port functionality. Encapsulating layers 2-5 of the OSI model
	//
	
	struct RawInputPort : BasePort {
		std::string _appId;
		unsigned int _counter;
		unsigned int _length;
		std::string _message;
		Input *_port;

		RawInputPort(Module *module, unsigned int portNum) : BasePort(module, portNum) { 
			_port = &(_module->inputs[_portNum]);
		}

		void process();
		virtual void received(std::string appId, std::string message);
	};

	//
	// Basic text sending.
	//

	struct TextInputPort : RawInputPort {
		TextInputPort(Module *module, unsigned int portNum) : RawInputPort(module, portNum) {}

		void received(std::string appId, std::string message) override;
		virtual void received(std::string message) {}
	};

	struct TextOutputPort : RawOutputPort {
		TextOutputPort(Module *module, unsigned int portNum) : RawOutputPort(module, portNum) {_appId.assign("TEXT");}
	};

	//
	// Queued sending.
	//

	struct QueuedOutputPort : RawOutputPort {
		std::vector<std::string *> _queue;
		unsigned int _replace = 0;
		unsigned int _size = 0;

		QueuedOutputPort(Module *module, unsigned int portNum) : RawOutputPort(module, portNum) {}
		virtual ~QueuedOutputPort() { for (auto i : _queue) delete i; }

		void abort() override;
		int isBusy() override { return (_state != STATE_QUIESCENT) || _queue.size(); }
		virtual int isFul() { return _queue.size() >= _size; }
		void process() override;
		void replace(unsigned int rep) { _replace = rep; }
		void send(std::string message) override;
		void size(unsigned int s);
	};

	//
	// Addressed Messages.
	//

	struct MessageOutputPort : QueuedOutputPort {
		MessageOutputPort(Module *module, unsigned int portNum) : QueuedOutputPort(module, portNum) {_appId.assign("MESG");}

		virtual void send(std::string pluginName, std::string moduleName, std::string message);
	};

	struct MessageInputPort : RawInputPort {
		MessageInputPort(Module *module, unsigned int portNum) : RawInputPort(module, portNum) {}

		void received(std::string appId, std::string message) override;
		virtual void received(std::string pluginName, std::string moduleName, std::string message) {}
	};

	//
	// Device Patches.
	//

	struct PatchOutputPort : QueuedOutputPort {
		PatchOutputPort(Module *module, unsigned int portNum) : QueuedOutputPort(module, portNum) {_appId.assign("PTCH");}

		virtual void send(std::string pluginName, std::string moduleName, json_t *rootJ);
	};

	struct PatchInputPort : RawInputPort {
		PatchInputPort(Module *module, unsigned int portNum) : RawInputPort(module, portNum) {}

		void received(std::string appId, std::string message) override;
		virtual void received(std::string pluginName, std::string moduleName, json_t *rootJ) {}
	};
		
}
	
