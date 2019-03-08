#include "MidiEvent.h"
#include "MidiLock.h"
#include "MidiPlayer.h"
#include "MidiSong.h"
#include "MidiTrack.h"

#include "asserts.h"

static void testLock()
{
    MidiLock l;
    assert(!l.locked());
    bool b = l.playerTryLock();
    assert(b);
    assert(l.locked());

    l.playerUnlock();
    assert(!l.locked());
    l.editorLock();
    assert(l.locked());
    
    b = l.playerTryLock();
    assert(!b);
    l.editorUnlock();
    assert(!l.locked());
    b = l.playerTryLock();
    assert(b);
    assert(l.locked());

    l.playerUnlock();
    assert(!l.locked());
}


static void testLock2()
{
    MidiLockPtr l = MidiLock::make();
    assert(!l->locked());
    bool b = l->playerTryLock();
    assert(b);
    assert(l->locked());

    l->playerUnlock();
    assert(!l->locked());

    {
        MidiLocker _(l);
        assert(l->locked());
        b = l->playerTryLock();
        assert(!b);
    }
    assert(!l->locked());
    b = l->playerTryLock();
    assert(b);
    assert(l->locked());

    l->playerUnlock();
    assert(!l->locked());
}

static void testLock3()
{
    MidiLockPtr l = MidiLock::make();
    l->editorLock();
    l->editorLock();
    assert(l->locked());

    l->editorUnlock();
    assert(l->locked());

    l->editorUnlock();
    assert(!l->locked());
}

static void testLock4()
{
    MidiLockPtr l = MidiLock::make();
    assert(!l->dataModelDirty());
    l->editorLock();
    l->editorUnlock();
    assert(l->dataModelDirty());
    assert(!l->dataModelDirty());
}

class TestHost : public IPlayerHost
{
public:
    void setGate(bool g) override
    {
        if (g != gateState) {
            ++gateChangeCount;
            gateState = g;
        }
    }
    void setCV(float cv) override
    {
        if (cv != cvState) {
            ++cvChangeCount;
            cvState = cv;
        }
    }
    void onLockFailed() override
    {
        ++lockConflicts;
    }

    int cvChangeCount = 0;
    int gateChangeCount = 0;
    bool gateState = false;

    float cvState = -100;

    int lockConflicts = 0;
};


// test that apis can be called
static void test0()
{
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    std::shared_ptr<TestHost> host = std::make_shared<TestHost>();
    MidiPlayer pl(host, song);
    pl.timeElapsed(.01f);
}


/**
 * Makes a one-track song. 
 * Track has one quarter note at t=0, duration = eighth.
 * End event at quarter note end.
 *
 * noteOnTime = 0 * .5;
 * noteOffTime = .5 * .5;
 */
static MidiSongPtr makeSongOneQ()
{
    MidiSongPtr song = std::make_shared<MidiSong>();
    MidiLocker l(song->lock);
    song->createTrack(0);
    MidiTrackPtr track = song->getTrack(0);

    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->startTime = 0;
    note->duration = .5;
    note->pitchCV = 2.f;
    track->insertEvent(note);
    track->insertEnd(1);

    return song;
}


std::shared_ptr<TestHost> makeSongOneQandRun(float time)
{
    MidiSongPtr song = makeSongOneQ();
    std::shared_ptr<TestHost> host = std::make_shared<TestHost>();
    MidiPlayer pl(host, song);
    pl.timeElapsed(time);
    return host;
}

std::shared_ptr<TestHost> makeSongOneQandRun2(float timeBeforeLock, float timeDuringLock, float timeAfterLock)
{
   
    MidiSongPtr song = makeSongOneQ();
    std::shared_ptr<TestHost> host = std::make_shared<TestHost>();
    MidiPlayer pl(host, song);
    pl.timeElapsed(timeBeforeLock);
    {
        MidiLocker l(song->lock);
        pl.timeElapsed(timeDuringLock);
    }

    pl.timeElapsed(timeAfterLock);
    return host;
}

// just play the first note on
static void test1()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun(.24f);

    assertEQ(host->lockConflicts, 0);
    assertEQ(host->gateChangeCount, 1);
    assertEQ(host->gateState, true);
    assertEQ(host->cvChangeCount, 1);
    assertEQ(host->cvState, 2);
    assertEQ(host->lockConflicts, 0);
}

// same as test1, but with a lock contention
static void test1L()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun2(.20f, .01f, .03f);

    assertEQ(host->gateChangeCount, 1);
    assertEQ(host->gateState, true);
    assertEQ(host->cvChangeCount, 1);
    assertEQ(host->cvState, 2);
    assertEQ(host->lockConflicts, 1);
}



// play the first note on and off
static void test2()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun(.25f);

    assertEQ(host->lockConflicts, 0);
    assertEQ(host->gateChangeCount, 2);
    assertEQ(host->gateState, false);
    assertEQ(host->cvChangeCount, 1);
    assertEQ(host->cvState, 2);
}

// play the first note on and off
static void test2L()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun2(.20f, .01f, .04f);

    assertEQ(host->lockConflicts, 1);
    assertEQ(host->gateChangeCount, 2);
    assertEQ(host->gateState, false);
    assertEQ(host->cvChangeCount, 1);
    assertEQ(host->cvState, 2);
}
// loop around to first note on second time
static void test3()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun(.51f);

    assertEQ(host->gateChangeCount, 3);
    assertEQ(host->gateState, true);
    assertEQ(host->cvChangeCount, 1);       // only changes once because it's one note loop
    assertEQ(host->cvState, 2);
}

// loop around to first note on second time
static void test3L()
{
    std::shared_ptr<TestHost> host = makeSongOneQandRun2(.4f, .7f, .4f );

    assertGE(host->gateChangeCount, 3);
    assertEQ(host->gateState, true);
    assertGE(host->cvChangeCount, 1);       // only changes once because it's one note loop
    assertEQ(host->cvState, 2);
}

void testMidiPlayer()
{
    assertNoMidi();
    testLock();
    testLock2();
    testLock3();
    testLock4();

    test0();
    test1();
    test2();
    test3();

    test1L();
    test2L();
    test3L();
    assertNoMidi();     // check for leaks
}