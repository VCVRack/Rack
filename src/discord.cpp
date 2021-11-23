#include <discord.hpp>
#include <system.hpp>
#include <random.hpp>
#include <settings.hpp>

#include <thread>
#include <mutex>
#include <condition_variable>

#if defined ARCH_LIN || defined ARCH_MAC
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/un.h>
#endif
#if defined ARCH_WIN
	#include <fcntl.h>
#endif

#include <jansson.h>


namespace rack {
namespace discord {


static const char* CLIENT_ID = "878351961274060861";

static bool running = false;
static std::thread thread;
static std::mutex mutex;
static std::condition_variable cv;


static int sendJson(int fd, int32_t opcode, json_t* j) {
	// Encode payload
	char* json = json_dumps(j, 0);
	if (!json)
		return 1;
	DEFER({free(json);});
	size_t len = strlen(json);

	// Send header
	int32_t header[2] = {opcode, int32_t(len)};
	if (write(fd, header, sizeof(header)) != sizeof(header))
		return 1;

	// Send payload
	if (write(fd, json, len) != ssize_t(len))
		return 1;

	return 0;
}


static json_t* receiveJson(int fd) {
	// Receive header
	int32_t header[2];
	if (read(fd, header, sizeof(header)) != sizeof(header))
		return NULL;

	// Receive payload
	size_t len = header[1];
	char json[len];
	if (read(fd, json, len) != ssize_t(len))
		return NULL;
	// DEBUG("Payload: %.*s", int(len), json);

	// Parse payload
	json_t* j = json_loadb(json, len, 0, NULL);
	return j;
}


static void run() {
	system::setThreadName("Discord IPC");
	random::init();

	// Open socket
#if defined ARCH_LIN || defined ARCH_MAC
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		// WARN("Could not open Discord socket");
		return;
	}
	DEFER({close(fd);});

	// Get socket path
	const char* dir = getenv("XDG_RUNTIME_DIR");
	if (!dir)
		dir = getenv("TMPDIR");
	if (!dir)
		dir = getenv("TMP");
	if (!dir)
		dir = getenv("TEMP");
	if (!dir)
		dir = "/tmp";

	// Connect to socket
	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/discord-ipc-0", dir);
	if (connect(fd, (struct sockaddr*) &addr, sizeof(addr))) {
		// Fail silently since this just means Discord isn't open.
		// WARN("Could not bind Discord socket");
		return;
	}
#endif
#if defined ARCH_WIN
	const char* path = "\\\\?\\pipe\\discord-ipc-0";
	int fd = open(path, O_RDWR | O_APPEND);
	if (fd < 0) {
		// Fail silently since this just means Discord isn't open.
		// WARN("Could not open Discord socket");
		return;
	}
	DEFER({close(fd);});
#endif

	// Send handshake
	json_t* handshakeJ = json_object();
	json_object_set(handshakeJ, "v", json_integer(1));
	json_object_set(handshakeJ, "client_id", json_string(CLIENT_ID));
	DEFER({json_decref(handshakeJ);});
	if (sendJson(fd, 0, handshakeJ)) {
		// WARN("Could not request Discord handshake");
		return;
	}

	// Receive handshake response
	json_t* handshakeResJ = receiveJson(fd);
	if (!handshakeResJ) {
		// WARN("Could not receive Discord handshake response");
		return;
	}
	DEFER({json_decref(handshakeResJ);});

	// Send activity
	json_t* payloadJ = json_object();
	json_object_set(payloadJ, "cmd", json_string("SET_ACTIVITY"));
	json_object_set(payloadJ, "nonce", json_string(std::to_string(random::u64()).c_str()));
	{
		json_t* argsJ = json_object();
		json_object_set(argsJ, "pid", json_integer(getpid()));
		{
			json_t* activityJ = json_object();
			{
				json_t* timestampsJ = json_object();
				json_object_set(timestampsJ, "start", json_integer(system::getUnixTime()));
				json_object_set(activityJ, "timestamps", timestampsJ);
			}
			json_object_set(argsJ, "activity", activityJ);
		}
		json_object_set(payloadJ, "args", argsJ);
	}
	DEFER({json_decref(payloadJ);});
	if (sendJson(fd, 1, payloadJ)) {
		// WARN("Could not set activity on Discord");
		return;
	}

	// Receive activity response
	json_t* payloadResJ = receiveJson(fd);
	if (!payloadResJ) {
		// WARN("Could not receive Discord activity response");
		return;
	}
	DEFER({json_decref(payloadResJ);});

	// Wait for destroy()
	std::unique_lock<std::mutex> lock(mutex);
	cv.wait(lock, []() {return !running;});

	// Ask Discord to disconnect
	// json_t* disconnectJ = json_object();
	// DEFER({json_decref(disconnectJ);});
	// if (sendJson(fd, 2, disconnectJ)) {
	// 	WARN("Could not disconnect from Discord");
	// 	return;
	// }
}


void init() {
	if (!settings::discordUpdateActivity)
		return;
	running = true;
	thread = std::thread(run);
}


void destroy() {
	{
		std::lock_guard<std::mutex> lock(mutex);
		running = false;
		cv.notify_all();
	}
	if (thread.joinable())
		thread.join();
}


} // namespace discord
} // namespace rack
