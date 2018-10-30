#include "torpedo.hpp"
using namespace Torpedo;

void BasePort::addCheckSum(unsigned int byte, unsigned int counter) {
	_checksum += ((byte & 0xff) << ((counter % 4) * 8));
	_checksum &= 0xffffffff;
}

void BasePort::raiseError(unsigned int errorType) {
	_state = STATE_QUIESCENT;
	_checksum = 0;
	switch (errorType) {
		case ERROR_STATE:
			if (dbg) debug("Torpedo Error: STATE");
			break;
		case ERROR_COUNTER:
			if (dbg) debug("Torpedo Error: COUNTER");
			break;
		case ERROR_LENGTH:
			if (dbg) debug("Torpedo Error: LENGTH");
			break;
		case ERROR_CHECKSUM:
			if (dbg) debug("Torpedo Error: CHECKSUM");
			break;
	}
	error(errorType);
}

void RawOutputPort::abort(void) {
	_state = STATE_ABORTING;
	_message.clear();
	_counter = 0;
}

void RawOutputPort::completed(void) {
	if (dbg) debug("Torpedo Completed:");
}

void RawOutputPort::process(void) {
	int portValue = 0;
	switch (_state) {
		case STATE_HEADER:
			switch (_counter) {
				case 0:
					_checksum = 0;
					portValue = 0x1000 | (_appId.length()?_appId[0]:0);
					_counter++;
					break;
				case 1:
				case 2:
				case 3:
					portValue = 0x1000 | (_counter * 0x100) | ((_appId.length() > _counter)?_appId[_counter]:0);
					_counter++;
					break;
				case 4:
				case 5:
				case 6:
				case 7:
					portValue = 0x1000 | (_counter * 0x100) | (_message.length() >> (8 * (_counter - 4)) & 0xff);
					_counter++;
					break;
				case 8:
				case 9:
				case 10:
				case 11:
				case 12:
				case 13:
				case 14:
					portValue = 0x1000 | (_counter * 0x100);
					_counter++;
					break;
				case 15:
					portValue = 0x1000 | (_counter * 0x100);
					_counter = 0;
					_state = STATE_BODY;
			}
			addCheckSum(portValue & 0xff, _counter + 3);
			break;
		case STATE_BODY:
			portValue = 0x2000 | ((_counter % 0x10) * 0x100) | _message[_counter];
			addCheckSum(portValue & 0xff, _counter);
			_counter++;
			if (_counter == _message.length()) {
				_counter = 0;
				_state = STATE_TRAILER;
			}
			break;
		case STATE_TRAILER:
			switch (_counter) {
				case 0:
				case 1:
				case 2:
					portValue = 0x3000 | (_counter * 0x100) | (_checksum & 0xff);
					_checksum >>= 8;
					_counter++;
					break;
				case 3:
					portValue = 0x3000 | (_counter * 0x100) | (_checksum & 0xff);
					_counter = 0;
					_state = STATE_QUIESCENT;
					completed();
					break;
			}
			break;
		case STATE_ABORTING:
			portValue = 0x3f00;
				_counter = 0;
			if (_message.length() > 0) {
				_state = STATE_HEADER;
			}
			else {
				_state = STATE_QUIESCENT;
			}
			break;
		case STATE_QUIESCENT:
			portValue = 0;
			break;
	}
	_port->value = 1.0f * portValue;
}

void RawOutputPort::send(std::string appId, std::string message) {
	_appId.assign(appId);
	send(message);
}

void RawOutputPort::send(std::string message) {
	if (!_port->active) return;
	if (!message.length()) {
		raiseError(ERROR_LENGTH);
		return;
	}
	if (dbg) debug("Torpedo Send:%s %s", _appId.c_str(), message.c_str());
	switch (_state) {
		case STATE_HEADER:
		case STATE_BODY:
		case STATE_TRAILER:
			abort();
			break;
		case STATE_ABORTING:
			break;
		case STATE_QUIESCENT:
			_state = STATE_HEADER;
			break;
	}
	_message.assign(message);
	_counter = 0;
}

void RawInputPort::process(void) {
	if (!_port->active) {
		_state = STATE_QUIESCENT;
		_checksum = 0;
		return;
	}
	unsigned int data = (unsigned int)(_port->value);
	if ((data & 0xff00) == 0x3f00) {
		_state = STATE_QUIESCENT;
		_checksum = 0;
		return;
	}
	unsigned int state = data >> 12;
	unsigned int counter = (data & 0x0f00) >> 8;
	data &= 0xff;
	switch (_state) {
		case STATE_QUIESCENT:
			if (!state)
				return;
			addCheckSum(data, counter);
			if (state == 1) {
				_counter = 0;
				_message.clear();
				_state = STATE_HEADER;
				if (counter != _counter) {
					raiseError(ERROR_COUNTER);
					return;
				}
				_appId.clear();
				_appId.push_back(data);
				_length = 0;
				return;
			}		
			raiseError(ERROR_STATE);
			return;
		case STATE_HEADER:
			addCheckSum(data, counter);
			if (state != _state) {
				raiseError(ERROR_STATE);
				return;
			}
			_counter++;
			if (counter != _counter) {
				raiseError(ERROR_COUNTER);
				return;
			}
			switch (counter) {
				case 0:
				case 1:
				case 2:
				case 3:
					_appId.push_back(data);
					break;
				case 4:
				case 5:
				case 6:
				case 7:
					_length >>= 8;
					_length += (data << 24);
					break;
				case 8:
				case 9:
				case 10:
				case 11:
				case 12:
				case 13:
				case 14:
					break;
				case 15:
					_state = STATE_BODY;
					_message.reserve(_length);
					_counter = 0;
					break;
			}
			break;
		case STATE_BODY:
			addCheckSum(data, counter);
			if (state != _state) {
				raiseError(ERROR_STATE);
				return;
			}
			if (counter != _counter++) {
				raiseError(ERROR_COUNTER);
				return;
			}
			_counter %= 16;
			_message.push_back(data);
			if (_message.length() >= _length) {
				_state = STATE_TRAILER;
				_counter = 0;
				return;
			}
			break;
		case STATE_TRAILER:
			if (state != _state) {
				raiseError(ERROR_STATE);
				return;
			}
			if (counter != _counter) {
				raiseError(ERROR_COUNTER);
				return;
			}
			if (_message.length() != _length) {
				raiseError(ERROR_LENGTH);
				return;
			}
			if (data != (_checksum & 0xff)) {
				raiseError(ERROR_CHECKSUM);
				return;
			}
			_checksum >>= 8;
			_counter++;
			if (_counter == 4) {
				_state = STATE_QUIESCENT;
				_checksum = 0;
				received(_appId, _message);
			}
			return;
	}
}

void RawInputPort::received(std::string appId, std::string message) {
	if (dbg) debug("Torpedo Received:%s %s", appId.c_str(), message.c_str());
}

void TextInputPort::received(std::string appId, std::string message) {
	if (!appId.compare("TEXT"))
		received(message);
}

void QueuedOutputPort::abort() {
	RawOutputPort::abort();
	for (auto i : _queue) delete i;
	_queue.clear();
}

void QueuedOutputPort::process() {
	if (!RawOutputPort::isBusy()) {
		if (_queue.size()) {
			std::string *s = _queue.front();
			_queue.erase(_queue.begin());
			RawOutputPort::send(std::string(*s));
			delete s;
		}
	}
	RawOutputPort::process();
}

void QueuedOutputPort::send(std::string message) {
	if (QueuedOutputPort::isBusy()) {
		if (_queue.size() >= _size) {
			if (!_replace)
				return;
			std::string *s = _queue.back();
			_queue.pop_back();
			delete s;
			if (dbg) debug("Torpedo Replaced:");
		}
		{
			std::string *s = new std::string(message);
			_queue.push_back(s);
			if (dbg) debug("Torpedo Queued:");
		}
		return;
	}
	RawOutputPort::send(message);
}

void QueuedOutputPort::size(unsigned int s) {
	if (s < 1) {
		return;
	}
	_size = s;
}

void MessageOutputPort::send(std::string pluginName, std::string moduleName, std::string message) {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "plugin", json_string(pluginName.c_str()));
	json_object_set_new(rootJ, "module", json_string(moduleName.c_str()));
	json_object_set_new(rootJ, "message", json_string(message.c_str()));
	char *msg = json_dumps(rootJ, 0);
	json_decref(rootJ);
	QueuedOutputPort::send(std::string(msg));
	free(msg);
}

void MessageInputPort::received(std::string appId, std::string message) {
	if (dbg) debug("Torpedo Received: %s", message.c_str());
	std::string pluginName;
	std::string moduleName;
	std::string messageText;

	if (appId.compare("MESG"))
		return;
	json_error_t error;
	json_t *rootJ = json_loads(message.c_str(), 0, &error);
	if (!rootJ) {
		debug("Torpedo MESG Error: %s", error.text);
		return;
	} 
	json_t *jp = json_object_get(rootJ, "plugin");
	if (json_is_string(jp)) 
		pluginName.assign(json_string_value(jp));
	json_t *jm = json_object_get(rootJ, "module");
	if (json_is_string(jm))
		moduleName.assign(json_string_value(jm));
	json_t *jt = json_object_get(rootJ, "message");
	if (json_is_string(jt))
		messageText.assign(json_string_value(jt));
	json_decref(rootJ);
	received(pluginName, moduleName, messageText);
}

void PatchOutputPort::send(std::string pluginName, std::string moduleName, json_t *rootJ) {
	json_t *wrapper = json_object();
	json_object_set_new(wrapper, "plugin", json_string(pluginName.c_str()));
	json_object_set_new(wrapper, "module", json_string(moduleName.c_str()));
	json_object_set_new(wrapper, "patch", rootJ);
	char *msg = json_dumps(wrapper, 0);
	json_decref(wrapper);
	QueuedOutputPort::send(std::string(msg));
	free(msg);
}

void PatchInputPort::received(std::string appId, std::string message) {
	if (dbg) debug("Torpedo Received: %s", message.c_str());
	std::string pluginName;
	std::string moduleName;

	if (appId.compare("PTCH"))
		return;
	json_error_t error;
	json_t *rootJ = json_loads(message.c_str(), 0, &error);
	if (!rootJ) {
		debug("Torpedo MESG Error: %s", error.text);
		return;
	} 
	json_t *jp = json_object_get(rootJ, "plugin");
	if (json_is_string(jp)) 
		pluginName.assign(json_string_value(jp));
	json_t *jm = json_object_get(rootJ, "module");
	if (json_is_string(jm))
		moduleName.assign(json_string_value(jm));
	json_t *jt = json_object_get(rootJ, "patch");
	if (jt)
		received(pluginName, moduleName, jt);
	json_decref(rootJ);
}
