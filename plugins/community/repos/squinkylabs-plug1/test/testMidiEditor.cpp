
#include "asserts.h"
#include "MidiEditor.h"
#include "MidiEvent.h"
#include "MidiSelectionModel.h"
#include "MidiSequencer.h"
#include "MidiTrack.h"
#include "MidiSong.h"

static int _trackNumber = 0;

// sequencer factory - helper function
MidiSequencerPtr makeTest(bool empty = false)
{
    MidiSongPtr song = empty ?
        MidiSong::MidiSong::makeTest(MidiTrack::TestContent::empty, _trackNumber) :
        MidiSong::MidiSong::makeTest(MidiTrack::TestContent::eightQNotes, _trackNumber);
    MidiSequencerPtr sequencer = std::make_shared<MidiSequencer>(song);
    sequencer->makeEditor();

    sequencer->context->setTrackNumber(_trackNumber);
    sequencer->context->setStartTime(0);
    sequencer->context->setEndTime(
        sequencer->context->startTime() + 8);
    sequencer->context->setPitchLow(PitchUtils::pitchToCV(3, 0));
    sequencer->context->setPitchHi(PitchUtils::pitchToCV(5, 0));

    
    sequencer->assertValid();
    return sequencer;
}

static bool cursorOnSelection(MidiSequencerPtr seq)
{
    if (seq->selection->empty()) {
        return true;
    }

    assert(seq->selection->size() == 1);    // haven't done multi yet
    MidiEventPtr sel = *seq->selection->begin();
    MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(sel);
    assert(note);

    // for now, do exact match
    if ((note->startTime == seq->context->cursorTime()) &&
        (note->pitchCV == seq->context->cursorPitch())) {
        return true;
    }
    return false;
}

// from a null selection, select next
static void testNext1()
{
    MidiSequencerPtr seq = makeTest();
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected

    // note should be first one
    MidiEventPtr firstEvent = seq->context->getTrack()->begin()->second;
    assert(seq->selection->isSelected(firstEvent));
    assert(cursorOnSelection(seq));
}

// from a null selection, select previous. should select last note
static void testNext1b()
{
    MidiSequencerPtr seq = makeTest();
    seq->editor->selectPrevNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected

    // note should be last one
    auto it = seq->context->getTrack()->end();
    it--;
    it--;
    MidiEventPtr lastEvent = it->second;

    assert(lastEvent->type == MidiEvent::Type::Note);
    assert(seq->selection->isSelected(lastEvent));
    assert(cursorOnSelection(seq));
}

// from a non-null selection, select next
static void testNext2()
{
    MidiSequencerPtr seq = makeTest();
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected

    MidiEventPtr firstEvent = seq->context->getTrack()->begin()->second;
    assert(seq->selection->isSelected(firstEvent));

    // Above is just test1, so now first event selected
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected

    auto iter = seq->context->getTrack()->begin();
    ++iter;
    MidiEventPtr secondEvent = iter->second;
    assert(seq->selection->isSelected(secondEvent));
}

// from a non-null selection, select previous
static void testNext2b()
{
    MidiSequencerPtr seq = makeTest();

    // Select the second note in the Seq
    seq->editor->selectNextNote();
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);

    // Verify that second note is selected
    auto iter = seq->context->getTrack()->begin();
    ++iter;
    MidiEventPtr secondEvent = iter->second;
    assert(seq->selection->isSelected(secondEvent));

    // Above is just test1, so now second event selected
    seq->editor->selectPrevNote();
    assertEQ(seq->selection->size(), 1);     // should be one note selected

    iter = seq->context->getTrack()->begin();
    MidiEventPtr firstEvent = iter->second;
    assert(seq->selection->isSelected(firstEvent));
}

// from a null selection, select next in null track
static void testNext3()
{
    MidiSequencerPtr seq = makeTest(true);
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 0);     // should be nothing selected
    assert(seq->selection->empty());
}

// from a null selection, select previous in null track
static void testNext3b()
{
    MidiSequencerPtr seq = makeTest(true);
    seq->editor->selectPrevNote();
    assertEQ(seq->selection->size(), 0);     // should be nothing selected
    assert(seq->selection->empty());
}

// select one after another until end
static void testNext4()
{
    MidiSequencerPtr seq = makeTest();
    int notes = 0;
    for (bool done = false; !done; ) {
        seq->editor->selectNextNote();
        if (seq->selection->empty()) {
            done = true;
        } else {
            ++notes;
        }
    }
    assertEQ(notes, 8);
}

// select next that off way out of viewport
static void testNext5()
{
    MidiSequencerPtr seq = makeTest();
    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);

    // Give the (first) note a pitch and start time that are
    // way outside viewport
    seq->editor->changePitch(50);

    //temporary kludge, because change pitch has a bug.
    // We need to re-select the note in question
    {
        printf("remove this hack\n");
        seq->selection->clear();
        seq->editor->selectNextNote();
    }

    seq->editor->changeStartTime(false, 50);


    assertEQ(seq->selection->size(), 1);
    seq->assertValid();
    seq->editor->assertCursorInSelection();

    seq->editor->selectPrevNote();
    assertEQ(seq->selection->size(), 1);
    seq->assertValid();
    seq->editor->assertCursorInSelection();

    seq->editor->selectNextNote();
    assertEQ(seq->selection->size(), 1);
    seq->assertValid();
    seq->editor->assertCursorInSelection();
}

static void testPrev1()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected
    assert(!seq->selection->empty());
    seq->editor->assertCursorInSelection();

    seq->editor->selectPrevNote();
    assert(seq->selection->empty());
}

// transpose one semi
static void testTrans1()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected

    const float firstNotePitch = PitchUtils::pitchToCV(3, 0);
    assertClose(seq->context->cursorPitch(), firstNotePitch, .0001);

    MidiEventPtr firstEvent = seq->context->getTrack()->begin()->second;
    MidiNoteEventPtr firstNote = safe_cast<MidiNoteEvent>(firstEvent);
    const float p0 = firstNote->pitchCV;
    seq->editor->changePitch(1);
    //seq->assertValid();

    // after transpose, need to find first note again.
    firstEvent = seq->context->getTrack()->begin()->second;
    firstNote = safe_cast<MidiNoteEvent>(firstEvent);

    const float p1 = firstNote->pitchCV;
    assertClose(p1 - p0, 1.f / 12.f, .000001);
    const float transposedPitch = PitchUtils::pitchToCV(3, 1);
    assertClose(seq->context->cursorPitch(), transposedPitch, .0001);
    seq->assertValid();
    seq->editor->assertCursorInSelection();
}

static void testTrans3()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected

    const float firstNotePitch = PitchUtils::pitchToCV(3, 0);
    assertClose(seq->context->cursorPitch(), firstNotePitch, .0001);

    MidiEventPtr firstEvent = seq->context->getTrack()->begin()->second;
    MidiNoteEventPtr firstNote = safe_cast<MidiNoteEvent>(firstEvent);
    const float p0 = firstNote->pitchCV;
    seq->editor->changePitch(50);       // transpose off screen
    seq->assertValid();
    seq->editor->assertCursorInSelection();
}

static void testShiftTime1()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected


    MidiNoteEventPtr firstNote = seq->context->getTrack()->getFirstNote();

    const float s0 = firstNote->startTime;
    seq->editor->changeStartTime(false, 1);     // delay one unit (1/16 6h)

    firstNote = seq->context->getTrack()->getFirstNote();
    const float s1 = firstNote->startTime;
    assertClose(s1 - s0, 1.f / 4.f, .000001);
    
    seq->editor->changeStartTime(false, -50);
    firstNote = seq->context->getTrack()->getFirstNote();
    const float s2 = firstNote->startTime;
    assertEQ(s2, 0);
    seq->assertValid();
    seq->editor->assertCursorInSelection();
}

static void testShiftTimex(int units)
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected

    MidiEventPtr firstEvent = seq->context->getTrack()->begin()->second;
    MidiNoteEventPtr firstNote = safe_cast<MidiNoteEvent>(firstEvent);
    const float s0 = firstNote->startTime;
    seq->editor->changeStartTime(false, units);     // delay n units
    seq->assertValid();

    assertEQ(seq->selection->size(), 1);
    seq->editor->assertCursorInSelection();
}

static void testShiftTime2()
{
    testShiftTimex(20);
}


static void testShiftTime3()
{
    testShiftTimex(50);
}

static void testChangeDuration1()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected

    MidiNoteEventPtr firstNote = seq->context->getTrack()->getFirstNote();
    const float d0 = firstNote->duration;
    seq->editor->changeDuration(false, 1);     // lengthen one unit

    firstNote = seq->context->getTrack()->getFirstNote();
    const float d1 = firstNote->duration;
    assertClose(d1 - d0, 1.f / 4.f, .000001);
    seq->assertValid();

    // try to make negative, should not go below 1
    seq->editor->changeDuration(false, -50);
    firstNote = seq->context->getTrack()->getFirstNote();
    const float d2 = firstNote->duration;
    assertGT(d2, 0);
    seq->assertValid();
}

// transpose multi
static void testTrans2()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();          // now first is selected

    MidiEventPtr firstEvent = seq->context->getTrack()->begin()->second;
    MidiNoteEventPtr firstNote = safe_cast<MidiNoteEvent>(firstEvent);
    const float p0 = firstNote->pitchCV;

    seq->editor->changePitch(1);
    firstEvent = seq->context->getTrack()->begin()->second;
    firstNote = safe_cast<MidiNoteEvent>(firstEvent);


    const float p1 = firstNote->pitchCV;
    assertClose(p1 - p0, 1.f / 12.f, .000001);
    seq->assertValid();

    assert(seq->undo->canUndo());
    seq->undo->undo();
    MidiNoteEventPtr firstNoteAfterUndo = safe_cast<MidiNoteEvent>(seq->context->getTrack()->begin()->second);
    const float p3 = firstNoteAfterUndo->pitchCV;
    assertClose(p3, p0, .000001);
    seq->undo->redo();
    MidiNoteEventPtr firstNoteAfterRedo = safe_cast<MidiNoteEvent>(seq->context->getTrack()->begin()->second);
    const float p4 = firstNoteAfterRedo->pitchCV;
    assertClose(p4, p1, .000001);


}

static void testCursor1()
{
    MidiSequencerPtr seq = makeTest(false);
    assertEQ(seq->context->cursorTime(), 0);
    assertEQ(seq->context->cursorPitch(), 0)
    assertEQ(seq->context->startTime(), 0);
}

static void testCursor2()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->advanceCursor(false, 1);
    assertEQ(seq->context->cursorTime(), 1.f / 4.f);

    seq->editor->advanceCursor(false, -4);
    assertEQ(seq->context->cursorTime(), 0);
    assertEQ(seq->context->startTime(), 0);
}

static void testCursor3()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();

    // Select first note to put cursor in it
    assertEQ(seq->context->cursorTime(), 0);
    MidiNoteEvent note;
    note.setPitch(3, 0);
    assertEQ(seq->context->cursorPitch(), note.pitchCV);

    // Now advance a 1/4 note
    seq->editor->advanceCursor(false, 4);
    assertEQ(seq->context->cursorTime(), 1.f);
    assert(seq->selection->empty());
    assertEQ(seq->context->startTime(), 0);

}

// move multiple times in two directions
static void testCursor4()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();

    // Select first note to put cursor in it
    assertEQ(seq->context->cursorTime(), 0);
    MidiNoteEvent note;
    note.setPitch(3, 0);
    assertEQ(seq->context->cursorPitch(), note.pitchCV);

    // Now advance up 3
    seq->editor->changeCursorPitch(1);
    seq->editor->changeCursorPitch(1);
    seq->editor->changeCursorPitch(1);

    for (int i = 0; i < 12; ++i) {
        seq->editor->advanceCursor(false, 1);
    }

    assert(!seq->selection->empty());
    assertEQ(seq->context->startTime(), 0);
}


// move up to scroll viewport
static void testCursor4b()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();

    // Select first note to put cursor in it
    assertEQ(seq->context->cursorTime(), 0);
    MidiNoteEvent note;
    note.setPitch(3, 0);
    assertEQ(seq->context->cursorPitch(), note.pitchCV);

    // Now advance up 3 octaves
    seq->editor->changeCursorPitch(3 * 12);

    assert(seq->selection->empty());
    seq->assertValid();
}


// just past end of note
static void testCursor5()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();

    // Select first note to put cursor in it
    assertEQ(seq->context->cursorTime(), 0);
    MidiNoteEvent note;
    note.setPitch(3, 0);
    assertEQ(seq->context->cursorPitch(), note.pitchCV);

    // Now advance two units right, to end of note
    seq->editor->advanceCursor(false, 1);
    seq->editor->advanceCursor(false, 1);

    assert(seq->selection->empty());
    assertEQ(seq->context->startTime(), 0);
}

// move past the end of the second bar
static void testCursor6()
{
    MidiSequencerPtr seq = makeTest(false);

    assertEQ(seq->context->startTime(), 0);
    seq->assertValid();

    // go up two bars and 1/16
    seq->editor->advanceCursor(false, 16 * 2 + 1);

    // bar 2 should be new start time
    assertEQ(seq->context->startTime(), 2 * 4);

}

static void testInsertSub(int advancUnits)
{
    MidiSequencerPtr seq = makeTest(true);
    assert(seq->selection->empty());
    const int initialSize = seq->context->getTrack()->size();

    seq->editor->advanceCursor(false, advancUnits);       // move up a half note
    float pitch = seq->context->cursorPitch();

    seq->editor->insertNote();

    auto it = seq->context->getTrack()->begin();
    assert(it != seq->context->getTrack()->end());
    MidiEventPtr ev = it->second;
    MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);
    assert(note);

    assertEQ(note->pitchCV, pitch);
    assertEQ(seq->selection->size(), 1);

    assert(seq->selection->isSelected(note));
    seq->assertValid();
    const int insertSize = seq->context->getTrack()->size();
    assertGT(insertSize, initialSize);

    printf("finish undo for insert note\n");
#if 0
    assert(seq->undo->canUndo());
    seq->undo->undo();
    const int undoSize = seq->context->getTrack()->size();
    assert(undoSize == initialSize);
#endif
}

static void testInsert()
{
    testInsertSub(8);
}

static void testInsert2()
{
    testInsertSub(34);      //middle of second bar
}

static void testDelete()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();

    auto it = seq->context->getTrack()->begin();
    assert(it != seq->context->getTrack()->end());
    MidiEventPtr ev = it->second;
    MidiNoteEventPtr firstNote = safe_cast<MidiNoteEvent>(ev);
    assert(firstNote);
    assertEQ(firstNote->startTime, 0);

    assertEQ(seq->selection->size(), 1);
    seq->editor->deleteNote();
    assert(seq->selection->empty());

    it = seq->context->getTrack()->begin();
    assert(it != seq->context->getTrack()->end());
    ev = it->second;
    MidiNoteEventPtr secondNote = safe_cast<MidiNoteEvent>(ev);
    assert(secondNote);
    assertEQ(secondNote->startTime, 1.f);
    seq->assertValid();
}

// delete a note with undo/redo
static void testDelete2()
{
    MidiSequencerPtr seq = makeTest(false);
    seq->editor->selectNextNote();
    const int trackSizeBefore = seq->context->getTrack()->size();
    seq->editor->deleteNote();
    const int trackSizeAfter = seq->context->getTrack()->size();
    assertLT(trackSizeAfter, trackSizeBefore);

    assert(seq->undo->canUndo());
    seq->undo->undo();
    const int trackSizeAfterUndo = seq->context->getTrack()->size();
    assertEQ(trackSizeAfterUndo, trackSizeBefore);
}

void testMidiEditorSub(int trackNumber)
{
    _trackNumber = trackNumber;
    testNext1();
    testNext1b();
    testNext2();
    testNext2b();
    testNext3();
    testNext3b();
    testPrev1();
    testNext4();
  

    testTrans1();
    testShiftTime1();
    testShiftTime2();
    testShiftTime3();
    testChangeDuration1();

    testTrans2();
    testTrans3();

    testCursor1();
    testCursor2();
    testCursor3();
    testCursor4();
    testCursor4b();
    testCursor5();
    testCursor6();

    testInsert();
    testInsert2();
    testDelete();
    testDelete2();

    testNext5();
}

void testMidiEditor()
{
    testMidiEditorSub(0);
    testMidiEditorSub(2);
}