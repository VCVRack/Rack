#include "bridge.hpp"
#include "util/common.hpp"
#include "dsp/ringbuffer.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <fcntl.h>

#include <thread>


namespace rack {


enum BridgeCommand {
	NO_COMMAND = 0,
	START_COMMAND,
	QUIT_COMMAND,
	CHANNEL_SET_COMMAND,
	AUDIO_SAMPLE_RATE_SET_COMMAND,
	AUDIO_CHANNELS_SET_COMMAND,
	AUDIO_BUFFER_SEND_COMMAND,
	MIDI_MESSAGE_SEND_COMMAND,
	NUM_COMMANDS
};


static const int RECV_BUFFER_SIZE = (1<<13);
static const int RECV_QUEUE_SIZE = (1<<17);

static AudioIO *audioListeners[BRIDGE_CHANNELS];
static std::thread serverThread;
static bool serverQuit;


struct BridgeClientConnection {
	RingBuffer<uint8_t, RECV_QUEUE_SIZE> recvQueue;
	BridgeCommand currentCommand = START_COMMAND;
	bool closeRequested = false;
	int channel = -1;
	int sampleRate = -1;
	int audioChannels = 0;
	int audioBufferLength = -1;

	/** Does not check if the queue has enough data.
	You must do that yourself before calling this method.
	*/
	template <typename T>
	T shift() {
		T x;
		recvQueue.shiftBuffer((uint8_t*) &x, sizeof(x));
		return x;
	}

	/** Steps the state machine
	Returns true if step() should be called again
	*/
	bool step() {
		switch (currentCommand) {
			case NO_COMMAND: {
				if (recvQueue.size() >= 1) {
					// Read command type
					uint8_t c = shift<uint8_t>();
					currentCommand = (BridgeCommand) c;
					return true;
				}
			} break;

			case START_COMMAND: {
				// To prevent other TCP protocols from connecting, require a "password" on startup to continue the connection.
				const int password = 0xff00fefd;
				if (recvQueue.size() >= 4) {
					int p = shift<uint32_t>();
					if (p == password) {
						currentCommand = NO_COMMAND;
						return true;
					}
					else {
						closeRequested = true;
					}
				}
			} break;

			case QUIT_COMMAND: {
				closeRequested = true;
				currentCommand = NO_COMMAND;
				debug("Quitting!");
			} break;

			case CHANNEL_SET_COMMAND: {
				if (recvQueue.size() >= 1) {
					channel = shift<uint8_t>();
					debug("Set channel %d", channel);
					currentCommand = NO_COMMAND;
					return true;
				}
			} break;

			case AUDIO_SAMPLE_RATE_SET_COMMAND: {
				if (recvQueue.size() >= 4) {
					sampleRate = shift<uint32_t>();
					debug("Set sample rate %d", sampleRate);
					currentCommand = NO_COMMAND;
					return true;
				}
			} break;

			case AUDIO_CHANNELS_SET_COMMAND: {
				if (recvQueue.size() >= 1) {
					audioChannels = shift<uint8_t>();
					debug("Set audio channels %d", channel);
					currentCommand = NO_COMMAND;
					return true;
				}
			} break;

			case AUDIO_BUFFER_SEND_COMMAND: {
				if (audioBufferLength < 0) {
					// Get audio buffer size
					if (recvQueue.size() >= 4) {
						audioBufferLength = shift<uint32_t>();
						if (audioBufferLength <= RECV_QUEUE_SIZE) {
							return true;
						}
						else {
							// Audio buffer is too large
							closeRequested = true;
						}
					}
				}
				else {
					if (recvQueue.size() >= (size_t) audioBufferLength) {
						// TODO Do something with the data
						recvQueue.start += audioBufferLength;
						debug("Received %d audio samples", audioBufferLength);
						audioBufferLength = -1;
						currentCommand = NO_COMMAND;
						return true;
					}
				}
			} break;

			case MIDI_MESSAGE_SEND_COMMAND: {
				if (recvQueue.size() >= 3) {
					uint8_t midiBuffer[3];
					recvQueue.shiftBuffer(midiBuffer, 3);
					debug("MIDI: %02x %02x %02x", midiBuffer[0], midiBuffer[1], midiBuffer[2]);
					currentCommand = NO_COMMAND;
					return true;
				}
			} break;

			default: {
				warn("Bridge client: bad command detected, closing");
				closeRequested = true;
			} break;
		}
		return false;
	}

	void recv(uint8_t *buffer, int length) {
		// Make sure we can fill the buffer
		if (recvQueue.capacity() < (size_t) length) {
			// If we can't accept it, future messages will be incomplete
			closeRequested = true;
			return;
		}

		recvQueue.pushBuffer(buffer, length);

		// Loop the state machine until it returns false
		while (step()) {}
	}
};



static void clientRun(int client) {
	int err;

	// // Get client address
	// struct sockaddr_in addr;
	// socklen_t clientAddrLen = sizeof(addr);
	// err = getpeername(client, (struct sockaddr*) &addr, &clientAddrLen);
	// assert(!err);

	// // Get client IP address
	// struct in_addr ipAddr = addr.sin_addr;
	// char ipBuffer[INET_ADDRSTRLEN];
	// inet_ntop(AF_INET, &ipAddr, ipBuffer, INET_ADDRSTRLEN);

	// info("Bridge client %s connected", ipBuffer);
	info("Bridge client connected");

#ifndef ARCH_LIN
	// Avoid SIGPIPE
	int flag = 1;
	setsockopt(client, SOL_SOCKET, SO_NOSIGPIPE, &flag, sizeof(int));
#endif

	BridgeClientConnection connection;

	while (!connection.closeRequested) {
		uint8_t buffer[RECV_BUFFER_SIZE];
#ifdef ARCH_LIN
		ssize_t received = recv(client, buffer, sizeof(buffer), MSG_NOSIGNAL);
#else
		ssize_t received = recv(client, buffer, sizeof(buffer), 0);
#endif
		if (received <= 0)
			break;

		connection.recv(buffer, received);
	}

	info("Bridge client closed");
	err = close(client);
	(void) err;
}

static void serverRun() {
	int err;

	// Open socket
	int server = socket(AF_INET, SOCK_STREAM, 0);
	if (server < 0)
		return;

	// Bind to 127.0.0.1
	const std::string host = "127.0.0.1";
	const int port = 5000;
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);
	serverAddr.sin_port = htons(port);
	err = bind(server, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
	if (err) {
		warn("Bridge server bind() failed");
		goto serverRun_cleanup;
	}

	// Listen for clients
	err = listen(server, 20);
	if (err) {
		warn("Bridge server listen() failed");
		goto serverRun_cleanup;
	}
	info("Bridge server started");

	// Make server non-blocking
	err = fcntl(server, F_SETFL, fcntl(server, F_GETFL, 0) | O_NONBLOCK);
	serverQuit = false;

	while (!serverQuit) {
		// Accept client socket

		int client = accept(server, NULL, NULL);
		if (client < 0) {
			// Wait a bit before attempting to accept another client
			std::this_thread::sleep_for(std::chrono::duration<float>(0.1));
			continue;
		}

		// Launch client thread
		std::thread clientThread(clientRun, client);
		clientThread.detach();
	}

	// Cleanup
serverRun_cleanup:
	err = close(server);
	(void) err;
	info("Bridge server closed");
}


void bridgeInit() {
	serverThread = std::thread(serverRun);
}

void bridgeDestroy() {
	serverQuit = true;
	serverThread.join();
}

void bridgeAudioSubscribe(int channel, AudioIO *audio) {
	if (!(0 <= channel && channel < BRIDGE_CHANNELS))
		return;
	if (audioListeners[channel])
		return;
	audioListeners[channel] = audio;
}

void bridgeAudioUnsubscribe(int channel, AudioIO *audio) {
	if (!(0 <= channel && channel < BRIDGE_CHANNELS))
		return;
	if (audioListeners[channel] != audio)
		return;
	audioListeners[channel] = NULL;
}

bool bridgeAudioIsSubscribed(int channel, AudioIO *audio) {
	if (!(0 <= channel && channel < BRIDGE_CHANNELS))
		return false;
	return (audioListeners[channel] == audio);
}


} // namespace rack
