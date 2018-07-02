#pragma once

#include <atomic>
#include <mutex>
#include <condition_variable>

/**
 * Messaging protocol between client and server.
 *
 * This protocol has the following goals:
 *      It is easy to implement.
 *      It is easy to understand.
 *      It is thread safe.
 *      The client's thread will never block.
 *
 * Basics of the protocol:
 *      Client initiates all communication.
 *      For every message sent client -> server, the server will send once back.
 *          The message objects are owned by whoever created them. Passing
 *          a message does not transfer ownership.
 *      Only one message may be "in play" at a time. Until the client
 *          receives a reply from the server, it may not send another message.
 */


/**
 * Base class for messages passed between client and server threads.
 * Receivers of message will typically examine the "type", and down-cast
 * based on that.
 */
class ThreadMessage
{
public:
    enum class Type
    {
        TEST1,
        TEST2,
        NOISE     // used by ColoredNoise
    };
    ThreadMessage(Type t) : type(t)
    {
        ++_dbgCount;
    }
    virtual ~ThreadMessage()
    {
        --_dbgCount;
    }

    const Type type;
    static std::atomic<int> _dbgCount;
};

/**
 * ThreadServer and ThreadClient do not refer to each other directly.
 * Instead, they both maintain pointers to ThreadSharedState.
 * All communication between thread goes through here.
 */
class ThreadSharedState
{
public:
    ThreadSharedState()
    {
        ++_dbgCount;
        serverRunning.store(false);
        serverStopRequested.store(false);
        mailboxClient2Server.store(nullptr);
        mailboxServer2Client.store(nullptr);
    }
    ~ThreadSharedState()
    {
        --_dbgCount;
    }
    std::atomic<bool> serverRunning;
    std::atomic<bool> serverStopRequested;
    static std::atomic<int> _dbgCount;

    /**
     * If return false, message not sent.
     * otherwise message send, and msg may be reused.
     */
    bool client_trySendMessage(ThreadMessage* msg);

    ThreadMessage* client_pollMessage();
    void client_askServerToStop();

    void server_sendMessage(ThreadMessage* msg);

    /**
     * returned message is a pointer to a message that we "own"
     * temporarily (sender may modify it, but won't delete it).
     *
     * if null returned, a shutdown has been requested
     */
    ThreadMessage* server_waitForMessageOrShutdown();


private:

    /**
     * This mutex protects all the private state
     */
    std::mutex mailboxMutex;

    /** The message in the mailbox.
     * This is an object by whoever created it. Ownership of message
     * is not passed.
     * TODO: given that a mutex protects us, we have no reason to use atomics here
     */
    std::atomic<ThreadMessage*> mailboxClient2Server;
    std::atomic<ThreadMessage*> mailboxServer2Client;

    std::condition_variable mailboxCondition;
};