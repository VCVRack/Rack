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


struct BridgeClientConnection;
static BridgeClientConnection *connections[BRIDGE_NUM_PORTS] = {};
static AudioIO *audioListeners[BRIDGE_NUM_PORTS] = {};
static std::thread serverThread;
static bool serverRunning = false;


struct BridgeClientConnection {
	int client;
	bool ready = false;

	int port = -1;
	int sampleRate = 0;
	int audioChannels = 0;
	bool audioActive = false;

	~BridgeClientConnection() {
		setPort(-1);
	}

	/** Returns true if successful */
	bool send(const void *buffer, int length) {
		if (length <= 0)
			return false;

#ifdef ARCH_LIN
		int flags = MSG_NOSIGNAL;
#else
		int flags = 0;
#endif
		ssize_t actual = ::send(client, buffer, length, flags);
		if (actual != length) {
			ready = false;
			return false;
		}
		return true;
	}

	template <typename T>
	bool send(T x) {
		return send(&x, sizeof(x));
	}

	/** Returns true if successful */
	bool recv(void *buffer, int length) {
		if (length <= 0)
			return false;

#ifdef ARCH_LIN
		int flags = MSG_NOSIGNAL;
#else
		int flags = 0;
#endif
		ssize_t actual = ::recv(client, buffer, length, flags);
		if (actual != length) {
			ready = false;
			return false;
		}
		return true;
	}

	template <typename T>
	bool recv(T *x) {
		return recv(x, sizeof(*x));
	}

	void flush() {
		int err;
		// Turn off Nagle
		int flag = 1;
		err = setsockopt(client, IPPROTO_TCP, TCP_NODELAY, (char*) &flag, sizeof(int));
		// Turn on Nagle
		flag = 0;
		err = setsockopt(client, IPPROTO_TCP, TCP_NODELAY, (char*) &flag, sizeof(int));
		(void) err;
	}

	void run() {
		info("Bridge client connected");

		// Check hello key
		uint32_t hello = -1;
		recv<uint32_t>(&hello);
		if (hello != BRIDGE_HELLO) {
			info("Bridge client protocol mismatch %x %x", hello, BRIDGE_HELLO);
			return;
		}

		// Process commands until no longer ready
		ready = true;
		while (ready) {
			step();
		}

		info("Bridge client closed");
	}

	/** Accepts a command from the client */
	void step() {
		uint8_t command = NO_COMMAND;
		if (!recv<uint8_t>(&command)) {
			ready = false;
			return;
		}

		switch (command) {
			default:
			case NO_COMMAND: {
				warn("Bridge client: bad command %d detected, closing", command);
				ready = false;
			} break;

			case QUIT_COMMAND: {
				debug("Bridge client quitting");
				ready = false;
			} break;

			case PORT_SET_COMMAND: {
				uint8_t port = -1;
				recv<uint8_t>(&port);
				setPort(port);
			} break;

			case MIDI_MESSAGE_SEND_COMMAND: {
				uint8_t midiBuffer[3];
				recv(&midiBuffer, 3);
				debug("MIDI: %02x %02x %02x", midiBuffer[0], midiBuffer[1], midiBuffer[2]);
			} break;

			case AUDIO_SAMPLE_RATE_SET_COMMAND: {
				uint32_t sampleRate = 0;
				recv<uint32_t>(&sampleRate);
				setSampleRate(sampleRate);
			} break;

			case AUDIO_CHANNELS_SET_COMMAND: {
				uint8_t channels = 0;
				recv<uint8_t>(&channels);
				// TODO
			} break;

			case AUDIO_PROCESS_COMMAND: {
				uint32_t length = 0;
				recv<uint32_t>(&length);
				if (length == 0) {
					ready = false;
					return;
				}

				float input[length];
				if (!recv(&input, length * sizeof(float))) {
					ready = false;
					return;
				}
				float output[length];
				int frames = length / 2;
				memset(&output, 0, sizeof(output));
				processStream(input, output, frames);
				send(&output, length * sizeof(float));
				flush();
			} break;

			case AUDIO_ACTIVATE: {
				audioActive = true;
				refreshAudio();
			} break;

			case AUDIO_DEACTIVATE: {
				audioActive = false;
				refreshAudio();
			} break;
		}
	}

	void setPort(int port) {
		// Unbind from existing port
		if (this->port >= 0 && connections[this->port] == this) {
			if (audioListeners[this->port])
				audioListeners[this->port]->setChannels(0, 0);
			connections[this->port] = NULL;
		}

		// Bind to new port
		if ((0 <= port && port < BRIDGE_NUM_PORTS) && !connections[port]) {
			this->port = port;
			connections[this->port] = this;
			refreshAudio();
		}
		else {
			this->port = -1;
		}
	}

	void setSampleRate(int sampleRate) {
		this->sampleRate = sampleRate;
		refreshAudio();
	}

	void processStream(const float *input, float *output, int frames) {
		if (!(0 <= port && port < BRIDGE_NUM_PORTS))
			return;
		if (!audioListeners[port])
			return;
		audioListeners[port]->setBlockSize(frames);
		audioListeners[port]->processStream(input, output, frames);
	}

	void refreshAudio() {
		if (!(0 <= port && port < BRIDGE_NUM_PORTS))
			return;
		if (connections[port] != this)
			return;
		if (!audioListeners[port])
			return;
		if (audioActive)
			audioListeners[port]->setChannels(2, 2);
		else
			audioListeners[port]->setChannels(0, 0);
		audioListeners[port]->setSampleRate(sampleRate);
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


static void serverConnect() {
	int err;

	// Initialize sockets
#ifdef ARCH_WIN
	WSADATA wsaData;
	err = WSAStartup(MAKEWORD(2, 2), &wsaData);
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

static void serverRun() {
	while (serverRunning) {
		std::this_thread::sleep_for(std::chrono::duration<double>(0.1));
		serverConnect();
	}
}

void bridgeInit() {
	serverRunning = true;
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
		connections[port]->refreshAudio();
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
