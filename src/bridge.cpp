#include "bridge.hpp"
#include "util/common.hpp"
#include "dsp/ringbuffer.hpp"

#include <unistd.h>
#ifdef ARCH_WIN
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netinet/tcp.h>
	#include <fcntl.h>
#endif


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
					if (recvQueue.size() >= (size_t) (sizeof(float) * audioBufferLength)) {
						float input[audioBufferLength];
						float output[audioBufferLength];
						memset(output, 0, sizeof(output));
						recvQueue.shiftBuffer((uint8_t*) input, sizeof(float) * audioBufferLength);
						int frames = audioBufferLength / 2;
						processStream(input, output, frames);
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

	void processStream(const float *input, float *output, int frames) {
		if (!(0 <= channel && channel < BRIDGE_CHANNELS))
			return;
		if (!audioListeners[channel])
			return;
		audioListeners[channel]->processStream(input, output, frames);
	}
};



static void clientRun(int client) {
	int err;
	BridgeClientConnection connection;

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

#ifdef ARCH_MAC
	// Avoid SIGPIPE
	int flag = 1;
	setsockopt(client, SOL_SOCKET, SO_NOSIGPIPE, &flag, sizeof(int));
#endif

#ifdef ARCH_WIN
	unsigned long blockingMode = 1;
	ioctlsocket(client, FIONBIO, &blockingMode);
#else
	err = fcntl(client, F_SETFL, fcntl(client, F_GETFL, 0) & ~O_NONBLOCK);
#endif

	while (!connection.closeRequested) {
		uint8_t buffer[RECV_BUFFER_SIZE];
#ifdef ARCH_LIN
		ssize_t received = recv(client, (char*) buffer, sizeof(buffer), MSG_NOSIGNAL);
#else
		ssize_t received = recv(client, (char*) buffer, sizeof(buffer), 0);
#endif
		if (received <= 0)
			break;

		connection.recv(buffer, received);
	}

	err = close(client);
	(void) err;
	info("Bridge client closed");
}


static void serverRun() {
	int err;

	// Initialize sockets
#ifdef ARCH_WIN
	WSADATA wsaData;
	err = WSAStartup(MAKEWORD(2,2), &wsaData);
	defer({
		WSACleanup();
	});
	if (err) {
		warn("Could not initialize Winsock");
		return;
	}
#endif

	// Get address
#ifdef ARCH_WIN
	struct addrinfo hints;
	struct addrinfo *result = NULL;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	err = getaddrinfo(NULL, "5000", &hints, &result);
	if (err) {
		warn("Could not get Bridge server address");
		return;
	}
	defer({
		freeaddrinfo(result);
	});
#else
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_port = htons(5000);
#endif

	// Open socket
#ifdef ARCH_WIN
	SOCKET server = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (server == INVALID_SOCKET) {
#else
	int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server < 0) {
#endif
		warn("Bridge server socket() failed");
		return;
	}
	defer({
		close(server);
	});


	// Bind socket to address
#ifdef ARCH_WIN
	err = bind(server, result->ai_addr, (int)result->ai_addrlen);
#else
	err = bind(server, (struct sockaddr*) &addr, sizeof(addr));
#endif
	if (err) {
		warn("Bridge server bind() failed");
		return;
	}

	// Listen for clients
	err = listen(server, 20);
	if (err) {
		warn("Bridge server listen() failed");
		return;
	}
	info("Bridge server started");

	// Make server non-blocking
#ifdef ARCH_WIN
	unsigned long blockingMode = 1;
	ioctlsocket(server, FIONBIO, &blockingMode);
#else
	err = fcntl(server, F_SETFL, fcntl(server, F_GETFL, 0) | O_NONBLOCK);
#endif

	// Accept clients
	serverQuit = false;
	while (!serverQuit) {
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
