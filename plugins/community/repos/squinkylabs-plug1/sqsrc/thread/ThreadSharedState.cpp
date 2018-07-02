
#include <assert.h>
#include "ThreadSharedState.h"

std::atomic<int> ThreadSharedState::_dbgCount;
std::atomic<int> ThreadMessage::_dbgCount;

#include <iostream>
#include <chrono>
#include <thread>

ThreadMessage*  ThreadSharedState::server_waitForMessageOrShutdown()
{
    // printf("wait\n"); fflush(stdout);

    std::unique_lock<std::mutex> guard(mailboxMutex);           // grab the mutex that protects condition
    ThreadMessage* returnMessage = nullptr;
    for (bool done = false; !done; ) {
        if (serverStopRequested.load()) {
            done = true;
        } else {
            returnMessage = mailboxClient2Server.load();
            if (returnMessage) {
                done = true;
            }
        }
        if (!done) {
            mailboxCondition.wait(guard);
        }
    }
    mailboxClient2Server.store(nullptr);
    return returnMessage;
}

void ThreadSharedState::client_askServerToStop()
{
    serverStopRequested.store(true);                        // ask server to stop
    std::unique_lock<std::mutex> guard(mailboxMutex);       // grab the mutex
    mailboxCondition.notify_all();                          // wake up server
}

ThreadMessage* ThreadSharedState::client_pollMessage()
{
    ThreadMessage* msg = nullptr;

    // grab lock
    std::unique_lock<std::mutex> guard(mailboxMutex);
    msg = mailboxServer2Client.load();
    if (msg) {
        mailboxServer2Client.store(nullptr);
    }
    return msg;
}

// signal in lock
bool ThreadSharedState::client_trySendMessage(ThreadMessage* msg)
{
    assert(serverRunning.load());
    // If the client tries to send a message before the previous one is read, the
    // call will fail and the client must try again.
    if (mailboxClient2Server.load()) {
        return false;
    }

    // Write to mailbox (condition) in lock

    std::unique_lock<std::mutex> guard(mailboxMutex, std::defer_lock);
    // We must use a try_lock here, as calling regular lock() could cause a priority inversion.
    bool didLock = guard.try_lock();
    if (!didLock) {
        return false;
    }
    assert(guard.owns_lock());

    assert(!mailboxClient2Server.load());               // if there is still a message there we are out of sync
    mailboxClient2Server.store(msg);

    mailboxCondition.notify_all();
    return true;
}

void ThreadSharedState::server_sendMessage(ThreadMessage* msg)
{
    std::unique_lock<std::mutex> guard(mailboxMutex);
    assert(mailboxServer2Client.load() == nullptr);
    mailboxServer2Client.store(msg);  
}


