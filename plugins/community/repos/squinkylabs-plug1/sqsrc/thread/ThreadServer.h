#pragma once

#include <memory>
#include <thread>

class ThreadSharedState;
class ThreadMessage;

/**
 * ThreadServer implements a worker thread that can do work
 * off of the audio thread in a plugin.
 * To do useful work with Thread server:
 *      Derive a class from ThreadServer, and override handleMessage.
 *      Define at least one message by deriving from ThreadMessage.
 *      Control ThreadServer with ThreadClient.
 * For more info, refer to ThreadSharedState
 */
class ThreadServer
{
public:
    ThreadServer(std::shared_ptr<ThreadSharedState> state);
    virtual ~ThreadServer();
    void start();

    const ThreadServer& operator= (const ThreadServer&) = delete;
    ThreadServer(const ThreadServer&) = delete;
    static int _count;
protected:
    /**
     * Derived thread servers must override this to get messages.
     * Message is not const, as server is allowed to modify it and
     * send it back.
     */
    virtual void handleMessage(ThreadMessage*);

    /**
     * Utility for sending replies back to the  client.
     * Will causes an error if there is a message in the mailbox already.
     */
    void sendMessageToClient(ThreadMessage*);

    std::shared_ptr<ThreadSharedState> sharedState;
    std::unique_ptr<std::thread> thread;
private:

    // Message based clients should not override this
    virtual void threadFunction();

    /**
     *
     * TODO: get rid of proc and handle, if possible
     */
    void procMessage(ThreadMessage*);
};