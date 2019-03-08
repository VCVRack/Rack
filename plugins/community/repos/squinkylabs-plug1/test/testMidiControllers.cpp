
#include "MidiEvent.h"
#include "MidiSelectionModel.h"
#include "MidiSequencer.h"
#include "MidiSong.h"
#include "MidiTrack.h"

// selects a note and verifies that one event is selected
static void testSelectionModel1()
{
    MidiSelectionModelPtr sel = std::make_shared<MidiSelectionModel>();
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    MidiEventPtr evt = song->getTrack(0)->begin()->second;
    assert(evt);
    sel->select(evt);

    int ct = 0;

    for (auto it = sel->begin(); it != sel->end(); ++it) {
        ++ct;
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(*it);
        assert(note);
    }
    assertEQ(ct, 1);
    assertEQ(sel->size(), 1);
    assert(_mdb > 1);
}

// selects a note and verifies that it is selected
static void testSelectionModel2()
{
    MidiSelectionModelPtr sel = std::make_shared<MidiSelectionModel>();
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    MidiEventPtr evt = song->getTrack(0)->begin()->second;
    assert(evt);
    sel->select(evt);
    assert(sel->isSelected(evt));
}

// selects a note and verifies that its clone is not selected
static void testSelectionModel3()
{
    MidiSelectionModelPtr sel = std::make_shared<MidiSelectionModel>();
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    MidiEventPtr evt = song->getTrack(0)->begin()->second;

    // make a clone of note in note2
    MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(evt);
    MidiNoteEventPtr note2 = std::make_shared<MidiNoteEvent>();
    *note2 = *note;

    sel->select(evt);
    assert(!sel->isSelected(note2));
}

// selects two notes and verifies that it is selected
static void testSelectionModel4()
{
    MidiSelectionModelPtr sel = std::make_shared<MidiSelectionModel>();
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);

    auto it = song->getTrack(0)->begin();
    MidiEventPtr evt = it->second;
    assert(evt);
    sel->select(evt);

    ++it;
    MidiEventPtr evt2 = it->second;
    sel->extendSelection(evt2);

    assertEQ(sel->size(), 2);
    assert(sel->isSelected(evt));
    assert(sel->isSelected(evt2));
}

static void testMidiSequencer1()
{
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0);
    MidiSequencerPtr seq = std::make_shared<MidiSequencer>(song);
    seq->makeEditor();

    assert(seq->selection);
    auto sel = seq->selection;
    assert(sel->begin() == sel->end());
    assert(_mdb > 1);
    assert( seq->editor);
   
}

static void testMidiSequencer2()
{
    MidiSongPtr song = MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, 0); 
    MidiSequencerPtr seq = std::make_shared<MidiSequencer>(song);
    seq->makeEditor();

    assert(seq->editor);
    assert(seq->selection);
    assert(seq->song);
    assert(seq->context);
    assert(seq->song->getTrack(0));
}

void testMidiControllers()
{
    assertNoMidi();     // check for leaks
    testSelectionModel1();
    testSelectionModel2();
    testSelectionModel3();
    testSelectionModel4();
    testMidiSequencer1();
    testMidiSequencer2();
    assertNoMidi();     // check for leaks
}
