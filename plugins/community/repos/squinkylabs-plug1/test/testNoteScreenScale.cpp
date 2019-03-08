#include "asserts.h"

#include "MidiEditorContext.h"
#include "NoteScreenScale.h"

// basic test of x coordinates
static void test0()
{
    // viewport holds single quarter note
    MidiEditorContextPtr vp = std::make_shared<MidiEditorContext>(nullptr);
    vp->setStartTime(0);
    vp->setEndTime(1);

    // let's make one quarter note fill the whole screen
    MidiNoteEvent note;
    note.setPitch(3, 0);
    vp->setPitchRange(note.pitchCV, note.pitchCV);

    vp->setCursorPitch(note.pitchCV);

    NoteScreenScale n(vp, 100, 100);
    float left = n.midiTimeToX(note);
    float right = left + n.midiTimeTodX(1.0f);
    assertEQ(left, 0);
    assertEQ(right, 100);

    float l2 = n.midiTimeToX(note.startTime);
    assertEQ(left, l2);

    auto bounds = n.midiTimeToHBounds(note);
    assertEQ(bounds.first, 0);
    assertEQ(bounds.second, 100);
}

// basic test of y coordinates
static void test1()
{
    // viewport holds single quarter note
    MidiEditorContextPtr vp = std::make_shared<MidiEditorContext>(nullptr);
    vp->setTimeRange(0, 1);

    // let's make one quarter note fill the whole screen
    MidiNoteEvent note;
    note.setPitch(3, 0);
    vp->setPitchRange(note.pitchCV, note.pitchCV);
    vp->setCursorPitch(note.pitchCV);

    NoteScreenScale n(vp, 100, 100);
    auto y = n.midiPitchToY(note);
    auto h = n.noteHeight();
    assertClose(y, 0, .001);
    assertClose(h, 100, .001);
}

// test of offset x coordinates
// viewport = 1 bar, have an eight not on beat 4
static void test2()
{
    printf("test2\n");
    // viewport holds one bar of 4/4
    MidiEditorContextPtr vp = std::make_shared<MidiEditorContext>(nullptr);
    vp->setTimeRange(0, 4);

    // let's make one eight note
    MidiNoteEvent note;
    note.startTime = 3.f;
    note.duration = .5f;
    note.setPitch(3, 0);
    vp->setPitchRange(note.pitchCV, note.pitchCV);
    vp->setCursorPitch(note.pitchCV);

    NoteScreenScale n(vp, 100, 100);

    auto bounds = n.midiTimeToHBounds(note);
    assertEQ(bounds.first, 75.f);
    assertEQ(bounds.second, 75.f + (100.0 / 8));

    float x = n.midiTimeToX(note);
    float x2 = n.midiTimeToX(note.startTime);
    assertEQ(x, x2);
}

// basic test of y coordinates
static void test3()
{
    // viewport holds two pitches
    MidiEditorContextPtr vp = std::make_shared<MidiEditorContext>(nullptr);
    vp->setTimeRange(0, 1);

    MidiNoteEvent note1, note2;
    note1.setPitch(3, 0);
    note2.setPitch(3, 1);
    vp->setPitchRange(note1.pitchCV, note2.pitchCV);
    vp->setCursorPitch(note1.pitchCV);

    NoteScreenScale n(vp, 100, 100);
    auto h = n.noteHeight();
    assertClose(h, 50, .001);

    // hight pitch should be at top
    auto y = n.midiPitchToY(note2);
    assertClose(y, 0, .001);

}


void testNoteScreenScale()
{
    test0();
    test1();
    test2();
    test3();
}