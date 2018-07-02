
#include "ThreadClient.h"
#include "ThreadServer.h"
#include "ThreadSharedState.h"

#include <assert.h>

ThreadClient::ThreadClient(std::shared_ptr<ThreadSharedState> state,
    std::unique_ptr<ThreadServer> server) : sharedState(state), _server(std::move(server))
{
    assert(!sharedState->serverRunning);
    _server->start();
    while (!sharedState->serverRunning) {
    }
}

ThreadClient::~ThreadClient()
{
    sharedState->client_askServerToStop();
    sharedState->serverStopRequested.store(true);               // ask server to stop

    while (sharedState->serverRunning) {
        static bool did = false;
        if (!did) {
            did = true;
        }
    }
}

ThreadMessage * ThreadClient::getMessage()
{
    return sharedState->client_pollMessage();
}


bool ThreadClient::sendMessage(ThreadMessage * message)
{
    return sharedState->client_trySendMessage(message);
}