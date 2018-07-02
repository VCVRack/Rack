#pragma once
#include <memory>

class ThreadSharedState;
class ThreadServer;
class ThreadMessage;

/**
 * This class is meant to be used directly (not sub-classed)
 * All APIs are non-blocking (polled), so that they may be called
 * from an audio render thread without fear of priority inversion.
 * See ThreadSharedState and ThreadSerever for more info.
 */
class ThreadClient
{
public:
    ThreadClient(std::shared_ptr<ThreadSharedState> state, std::unique_ptr<ThreadServer> server);
    ~ThreadClient();

    /**
     * Poll to see if a message has come back from the server.
     * Will return null if no message waiting.
     */
    ThreadMessage * getMessage();

    /**
     * Try to send a message.
     * Returns true if message sent.
     *
     * Message might not be sent for various reasons:
     *      - Mailbox still has previous message.
     *      - Unable to lock semaphore without blocking.
     */
    bool sendMessage(ThreadMessage *);



    const ThreadClient& operator= (const ThreadClient&) = delete;
    ThreadClient(const ThreadClient&) = delete;
private:
    std::shared_ptr<ThreadSharedState> sharedState;
    std::unique_ptr<ThreadServer> _server;
};