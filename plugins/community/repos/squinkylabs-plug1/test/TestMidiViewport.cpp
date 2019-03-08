
#include "asserts.h"
#include "MidiEditorContext.h"
#include "MidiLock.h"
#include "MidiSong.h"


static void testReleaseSong()
{
    MidiSongPtr song(std::make_shared<MidiSong>());
    MidiLocker l(song->lock);
    MidiEditorContext vp(song);
    {
       
       // vp._song = song;

        song->createTrack(0);
        auto track = song->getTrack(0);

        MidiNoteEventPtr ev = std::make_shared<MidiNoteEvent>();
        track->insertEvent(ev);
        assertEvCount(1);       // one obj with two refs
    }
    song.reset();       // give up the one strong ref
    assertEvCount(0);
}

static void testEventAccess()
{
    MidiSongPtr song(std::make_shared<MidiSong>());
    MidiLocker l(song->lock);
    song->createTrack(0);
    auto track = song->getTrack(0);

    MidiNoteEventPtr ev = std::make_shared<MidiNoteEvent>();
    ev->startTime = 100;
    ev->pitchCV = 40;
    track->insertEvent(ev);

    MidiEditorContext vp(song);
    vp.setStartTime(90);
    vp.setEndTime(110);
    vp.setPitchLow(0);
    vp.setPitchHi(80);

    auto its = vp.getEvents();
    assertEQ(std::distance(its.first, its.second), 1);

    assert(its.first != its.second);

    auto i = its.first;
    auto x = i->second->startTime;
    its.first++;
}


static void testEventFilter()
{
    MidiSongPtr song(std::make_shared<MidiSong>());
    MidiLocker l(song->lock);

    song->createTrack(0);
    auto track = song->getTrack(0);

    MidiNoteEventPtr ev = std::make_shared<MidiNoteEvent>();
    ev->startTime = 100;
    ev->pitchCV = 40;
    track->insertEvent(ev);

    MidiNoteEventPtr ev2 = std::make_shared<MidiNoteEvent>();
    ev2->startTime = 102;
    ev2->pitchCV = 50;
    ev2->startTime = 100;
    track->insertEvent(ev2);
    assertEQ(track->size(), 2);

    MidiEditorContext vp(song);
    vp.setStartTime(90);
    vp.setEndTime(110);
    vp.setPitchLow(3);
    vp.setPitchHi(45);
    auto its = vp.getEvents();
    assertEQ(std::distance(its.first, its.second), 1);
}

static void testDemoSong()
{
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    MidiTrackPtr track = song->getTrack(0);

    const int numNotes = 8;             // track0 of test song has 8 notes
    const int numEvents = 8 + 1;        // got an end event
    assertEQ(std::distance(track->begin(), track->end()), numEvents);


    MidiEditorContext viewport(song);
    viewport.setTimeRange(0, 8);   // two measures

    // try a crazy wide range
    MidiNoteEvent note1;
    note1.setPitch(-10, 0);
    MidiNoteEvent note2;
    note2.setPitch(10, 0);

    viewport.setPitchRange(note1.pitchCV, note2.pitchCV);
 
    MidiEditorContext::iterator_pair it = viewport.getEvents();
    assertEQ(std::distance(it.first, it.second), numNotes);

    // try inclusive pitch range
    viewport.setPitchRange(PitchUtils::pitchToCV(3, 0),     // pitch of first note
        PitchUtils::pitchToCV(3, 7));
    it = viewport.getEvents();
    assertEQ(std::distance(it.first, it.second), numNotes);

    // reduce the pitch range to lose the highest note.
    viewport.setPitchHi(PitchUtils::pitchToCV(3, 7) - .01f);
    it = viewport.getEvents();
    assertEQ(std::distance(it.first, it.second), numNotes-1);

     // reduce the pitch range to lose the lowest note.
    viewport.setPitchLow(PitchUtils::pitchToCV(3, 0) + .01f);
    it = viewport.getEvents();
    assertEQ(std::distance(it.first, it.second), numNotes - 2);

    // try inclusive pitch range, but only half the time
    viewport.setPitchRange(PitchUtils::pitchToCV(3, 0),     // pitch of first note
                    PitchUtils::pitchToCV(3, 7));
    viewport.setEndTime(viewport.startTime() + 4);   // one measures
    it = viewport.getEvents();
    assertEQ(std::distance(it.first, it.second), numNotes /2);



}

void testMidiViewport()
{
    assertEvCount(0);
    testReleaseSong();
    testEventAccess();
    testEventFilter();
    testDemoSong();

    assertEvCount(0);
}
