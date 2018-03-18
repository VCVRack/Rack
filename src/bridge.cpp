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


static const int RECV_BUFFER_SIZE = (1<<13);
static const int RECV_QUEUE_SIZE = (1<<17);

struct BridgeClientConnection;
static BridgeClientConnection *connections[BRIDGE_NUM_PORTS] = {};
static AudioIO *audioListeners[BRIDGE_NUM_PORTS] = {};
static std::thread serverThread;
static bool serverRunning;


struct BridgeClientConnection {
	int client;
	RingBuffer<uint8_t, RECV_QUEUE_SIZE> recvQueue;
	BridgeCommand currentCommand = START_COMMAND;
	bool closeRequested = false;
	int port = -1;
	int sampleRate = -1;
	int audioChannels = 0;
	int audioBufferLength = -1;
	bool audioActive = false;

	~BridgeClientConnection() {
		setPort(-1);
	}

	void send(const uint8_t *buffer, int length) {
		if (length <= 0)
			return;
#ifdef ARCH_LIN
		int sendFlags = MSG_NOSIGNAL;
#else
		int sendFlags = 0;
#endif
		ssize_t written = ::send(client, (const char*) buffer, length, sendFlags);
		// We must write the entire buffer
		if (written < length)
			closeRequested = true;
	}

	template <typename T>
	void send(T x) {
		send((uint8_t*) &x, sizeof(x));
	}

	/** Does not check if the queue has enough data.
	You must do that yourself before calling this method.
	*/
	template <typename T>
	T shift() {
		T x;
		recvQueue.shiftBuffer((uint8_t*) &x, sizeof(x));
		return x;
	}

	void run() {
		info("Bridge client connected");

		while (!closeRequested) {
			uint8_t buffer[RECV_BUFFER_SIZE];
#ifdef ARCH_LIN
			int recvFlags = MSG_NOSIGNAL;
#else
			int recvFlags = 0;
#endif
			ssize_t received = ::recv(client, (char*) buffer, sizeof(buffer), recvFlags);
			if (received <= 0)
				break;

			// Make sure we can fill the buffer
			if (recvQueue.capacity() < (size_t) received) {
				// If we can't accept it, future messages will be incomplete
				break;
			}

			recvQueue.pushBuffer(buffer, received);

			// Loop the state machine until it returns false
			while (step()) {}
		}

		info("Bridge client closed");
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

			case PORT_SET_COMMAND: {
				if (recvQueue.size() >= 1) {
					int port = shift<uint8_t>();
					setPort(port);
					debug("Set port %d", port);
					currentCommand = NO_COMMAND;
					return true;
				}
			} break;

			case MIDI_MESSAGE_SEND_COMMAND: {
				if (recvQueue.size() >= 3) {
					uint8_t midiBuffer[3];
					recvQueue.shiftBuffer(midiBuffer, 3);
					// debug("MIDI: %02x %02x %02x", midiBuffer[0], midiBuffer[1], midiBuffer[2]);
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
					debug("Set audio channels %d", audioChannels);
					currentCommand = NO_COMMAND;
					return true;
				}
			} break;

			case AUDIO_PROCESS_COMMAND: {
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
						// Get input buffer
						int frames = audioBufferLength / 2;
						float input[audioBufferLength];
						recvQueue.shiftBuffer((uint8_t*) input, sizeof(float) * audioBufferLength);
						// Process stream
						float output[audioBufferLength];
						processStream(input, output, frames);
						// Send output buffer
						send((uint8_t*) output, audioBufferLength * sizeof(float));

						audioBufferLength = -1;
						currentCommand = NO_COMMAND;
						return true;
					}
				}
			} break;

			case AUDIO_ACTIVATE: {
				audioActive = true;
				refreshAudioActive();
				currentCommand = NO_COMMAND;
				return true;
			} break;

			case AUDIO_DEACTIVATE: {
				audioActive = false;
				refreshAudioActive();
				currentCommand = NO_COMMAND;
				return true;
			} break;

			default: {
				warn("Bridge client: bad command detected, closing");
				closeRequested = true;
			} break;
		}

		// Stop looping the state machine
		return false;
	}

	void setPort(int newPort) {
		// Unbind from existing port
		if (port >= 0 && connections[port] == this) {
			if (audioListeners[port])
				audioListeners[port]->setChannels(0, 0);
			connections[port] = NULL;
		}

		// Bind to new port
		if (newPort >= 0 && !connections[newPort]) {
			connections[newPort] = this;
			refreshAudioActive();
			port = newPort;
		}
		else {
			port = -1;
		}
	}

	void processStream(const float *input, float *output, int frames) {
		if (!(0 <= port && port < BRIDGE_NUM_PORTS))
			return;
		if (!audioListeners[port])
			return;
		audioListeners[port]->processStream(input, output, frames);
		debug("%d frames", frames);
	}

	void refreshAudioActive() {
		if (!(0 <= port && port < BRIDGE_NUM_PORTS))
			return;
		if (!audioListeners[port])
			return;
		if (audioActive)
			audioListeners[port]->setChannels(2, 2);
		else
			audioListeners[port]->setChannels(0, 0);
	}
};


static void clientRun(int client) {
	defer({
		close(client);
	});
	int err;
	(void) err;

#ifdef ARCH_MAC
	// Avoid SIGPIPE
	int flag = 1;
	setsockopt(client, SOL_SOCKET, SO_NOSIGPIPE, &flag, sizeof(int));
#endif

	// Disable non-blocking
#ifdef ARCH_WIN
	unsigned long blockingMode = 0;
	ioctlsocket(client, FIONBIO, &blockingMode);
#else
	err = fcntl(client, F_SETFL, fcntl(client, F_GETFL, 0) & ~O_NONBLOCK);
#endif

	BridgeClientConnection connection;
	connection.client = client;
	connection.run();
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
	err = getaddrinfo("127.0.0.1", "5000", &hints, &result);
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

	// Enable non-blocking
#ifdef ARCH_WIN
	unsigned long blockingMode = 1;
	ioctlsocket(server, FIONBIO, &blockingMode);
#else
	int flags = fcntl(server, F_GETFL, 0);
	err = fcntl(server, F_SETFL, flags | O_NONBLOCK);
#endif

	// Accept clients
	serverRunning = true;
	while (serverRunning) {
		int client = accept(server, NULL, NULL);
		if (client < 0) {
			// Wait a bit before attempting to accept another client
			std::this_thread::sleep_for(std::chrono::duration<double>(0.1));
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
	serverRunning = false;
	serverThread.join();
}

void bridgeAudioSubscribe(int port, AudioIO *audio) {
	if (!(0 <= port && port < BRIDGE_NUM_PORTS))
		return;
	// Check if an Audio is already subscribed on the port
	if (audioListeners[port])
		return;
	audioListeners[port] = audio;
	if (connections[port])
		connections[port]->refreshAudioActive();
}

void bridgeAudioUnsubscribe(int port, AudioIO *audio) {
	if (!(0 <= port && port < BRIDGE_NUM_PORTS))
		return;
	if (audioListeners[port] != audio)
		return;
	audioListeners[port] = NULL;
	audio->setChannels(0, 0);
}


} // namespace rack
