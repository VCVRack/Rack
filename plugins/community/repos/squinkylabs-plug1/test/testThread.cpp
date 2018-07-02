
#include "asserts.h"
#include "ThreadSharedState.h"
#include "ThreadServer.h"
#include "ThreadClient.h"
#include "ThreadPriority.h"

#include <assert.h>
#include <memory>
#include <vector>



// test that we can build and tear down.
static void test0()
{
    assertEQ(ThreadSharedState::_dbgCount, 0);
    {
        std::shared_ptr<ThreadSharedState> noise = std::make_shared<ThreadSharedState>();
        std::unique_ptr<ThreadServer> server(new ThreadServer(noise));
        std::unique_ptr<ThreadClient> client(new ThreadClient(noise, std::move(server)));
    }
    assertEQ(ThreadSharedState::_dbgCount, 0);
}

static void test1()
{
    for (int i = 0; i < 200; ++i)
        test0();
}

/**************************************************************************/

// client will send to server
class Test1Message : public ThreadMessage
{
public:
    Test1Message() : ThreadMessage(Type::TEST1)
    {
    }
    int payload = 0;
};

// client will send to server
class Test2Message : public ThreadMessage
{
public:
    Test2Message() : ThreadMessage(Type::TEST2)
    {
    }
};

class TestServer : public ThreadServer
{
public:
    TestServer(std::shared_ptr<ThreadSharedState> state) : ThreadServer(state)
    {
    }
    void handleMessage(ThreadMessage* msg) override
    {
        switch (msg->type) {
            case ThreadMessage::Type::TEST1:
            {
                Test1Message * tstMsg = static_cast<Test1Message *>(msg);
                assertEQ(tstMsg->payload, nextExpectedPayload);
                ++nextExpectedPayload;
                tstMsg->payload += 1000;
                sendMessageToClient(tstMsg);        // send back the modified one
            }
            break;
            default:
                assert(false);
        }
    }
    int nextExpectedPayload = 100;
};

static void test2()
{
    // Set up all the objects
    std::unique_ptr<Test1Message> msg(new Test1Message());
    std::shared_ptr<ThreadSharedState> state = std::make_shared<ThreadSharedState>();
    std::unique_ptr<TestServer> server(new TestServer(state));
    std::unique_ptr<ThreadClient> client(new ThreadClient(state, std::move(server)));

    for (int count = 0; count < 50; ++count) {
        msg->payload = 100 + count;
        const int expectedPayload = msg->payload + 1000;
        for (bool done = false; !done; ) {
            bool b = client->sendMessage(msg.get());
            if (b) {
                done = true;
            }
        }

        for (bool done = false; !done; ) {
            auto rxmsg = client->getMessage();
            if (rxmsg) {
                done = true;
                assert(rxmsg->type == ThreadMessage::Type::TEST1);
                Test1Message* tmsg = reinterpret_cast<Test1Message *>(rxmsg);
                assertEQ(tmsg->payload, expectedPayload);
            }
        }
    }
}

// not a real test
static void test3()
{
    bool b = ThreadPriority::boostNormal();
    bool b2 = ThreadPriority::boostRealtime();
    printf("\nnormal boost: %d\n", b);
    printf("realtime boost: %d\n", b2);
    ThreadPriority::restore();
}

static std::atomic<bool> stopNow;
static std::atomic<int> count;
static double xxx, yyy;
static std::atomic<int> slow;
static std::atomic<int> fast;

//thread func
static void t4(bool iAmIt,int boost)
{
   // printf("t4 called with %d\n", iAmIt);

    if (iAmIt) {
        switch (boost) {
            case 0:
                printf("no boost\n");
                break;
            case 1:
                printf("boosting\n");
                ThreadPriority::boostNormal();
                break;
            case 2:
                printf("boosting RT\n");
                ThreadPriority::boostRealtime();
                break;
            default:
                assert(false);
        }
            
        fflush(stdout);
           
    }
    while (!stopNow) {
        for (int i = 0; i < 100000; ++i) {
            yyy = yyy + (double) rand();
        }

        if (iAmIt) {
            ++fast;
        } else {
            ++slow;
        }
    }
}

// runs all the test treads, returns ratio of work done in the default theads
// and work done in test thread.
static double test4sub(int boost)
{
    stopNow = false;
    count = 0;
    xxx = 0;
    yyy = 0;
    slow = 0;
    fast = 0;
    int numSlow = 0;
    std::vector< std::shared_ptr<std::thread>> threads;

    threads.push_back(std::make_shared<std::thread>(t4, true, boost));
    for (int i = 0; i < 9; ++i) {
        threads.push_back(std::make_shared<std::thread>(t4, false, 0));
        ++numSlow;
    }

    printf("started all\n");
    xxx = 0;
    yyy = 0;

    std::this_thread::sleep_for(std::chrono::seconds(20));
    stopNow = true;

    for (auto thread : threads) {
        thread->join();
    }

    ThreadPriority::restore();
    const double ret = (double) slow / (double) fast;
    printf("slow/fast was %f (%d) ratio=%d\n", ret, (int) slow, numSlow);
    return ret;
}

static void test4()
{
    printf("testing thread priorities, part1. will take a while\n"); fflush(stdout);
    const double ref = test4sub(0);
    printf("testing thread priorities, part2. will take a while\n"); fflush(stdout);
    const double boosted = test4sub(1);
    printf("testing thread priorities, part3. will take a while\n"); fflush(stdout);
    const double boostedRT = test4sub(2);
    printf("ref = %f, boosted = %f rt=%f\n", ref, boosted, boostedRT); fflush(stdout);
}

#ifdef ARCH_WIN
static void test5()
{
    ThreadPriority::boostRealtimeWindows();
}
#endif
/*****************************************************************/

void testThread(bool extended)
{

   assertEQ(ThreadSharedState::_dbgCount, 0);
   assertEQ(ThreadMessage::_dbgCount, 0);
   test0();
   test1();
   test2();
   test3();
   if (extended) {
       test4();
   }
#ifdef ARCH_WIN
   test5();
#endif
   assertEQ(ThreadSharedState::_dbgCount, 0);
   assertEQ(ThreadMessage::_dbgCount, 0);
}