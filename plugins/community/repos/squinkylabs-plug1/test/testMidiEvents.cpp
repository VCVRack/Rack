

#include "MidiEvent.h"

#include "asserts.h"


static void testType()
{
    MidiNoteEvent note;
    assert(note.type == MidiEvent::Type::Note);
    MidiEndEvent end;
    assert(end.type == MidiEvent::Type::End);
}

static void testCast()
{
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    MidiEventPtr evn(note);

    MidiEndEventPtr end = std::make_shared<MidiEndEvent>();
    MidiEventPtr eve(end);

    assert(safe_cast<MidiNoteEvent>(evn));
    assert(safe_cast<MidiEndEvent>(eve));

    assert(!safe_cast<MidiNoteEvent>(eve));
    assert(!safe_cast<MidiEndEvent>(evn));

    assert(!safe_cast<MidiNoteEvent>(end));
    assert(!safe_cast<MidiEndEvent>(note));

    assert(safe_cast<MidiEvent>(note));
    assert(safe_cast<MidiEvent>(end));
}

static void testEqual()
{
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    MidiNoteEventPtr note2 = std::make_shared<MidiNoteEvent>();
    MidiEventPtr evn(note);

    MidiEndEventPtr end = std::make_shared<MidiEndEvent>();
    MidiEventPtr eve(end);

    assertEQ(note, evn);
    assertEQ(end, eve);
    assert (*note == *note2);
    assert(note != note2);

    assert(!(*note == *end));
    assert(*note != *end);
    assert(*note != *eve);
    assert(*evn != *end);
    assert(*note != *end);

    note2->pitchCV = 50;
    assert(*note != *note2);
}

static void testPitch()
{
    MidiNoteEvent n;
    n.pitchCV = 0;            // C4 in VCV
    auto pitch = n.getPitch();
    assert(pitch.first == 4);
    assert(pitch.second == 0);

    n.setPitch(pitch.first, pitch.second);
    assertEQ(n.pitchCV, 0);

    //************************************************

    n.pitchCV = 1.f / 12.f;
    pitch = n.getPitch();
    assert(pitch.first == 4);
    assert(pitch.second == 1);

    n.setPitch(pitch.first, pitch.second);
    assertClose(n.pitchCV, 1.f / 12.f, .0001);

    //********************************************************

    n.pitchCV = 3 + 1.f / 12.f;
    pitch = n.getPitch();
    assertEQ(pitch.first, 4.0f + 3);
    assertEQ(pitch.second, 1);

    n.setPitch(pitch.first, pitch.second);
    assertClose(n.pitchCV, 3 + 1.f / 12.f, .0001);

    //********************************************************

    n.pitchCV = 7.f / 12.f;
    pitch = n.getPitch();
    assertEQ(pitch.first, 4.0f);
    assertEQ(pitch.second, 7);

    n.setPitch(pitch.first, pitch.second);
    assertClose(n.pitchCV, 7.f / 12.f, .0001);

    //********************************************************

    n.pitchCV = 3 + 7.f / 12.f;
    pitch = n.getPitch();
    assertEQ(pitch.first, 4.0f+3);
    assertEQ(pitch.second, 7);

    n.setPitch(pitch.first, pitch.second);
    assertClose(n.pitchCV,3 +  7.f / 12.f, .0001);
}

static void testPitch2()
{
    assertNoMidi();     // check for leaks
    MidiNoteEvent n;
    n.pitchCV = -1;            // C3 in VCV
    auto pitch = n.getPitch();
    assert(pitch.first == 3);
    assert(pitch.second == 0);

    n.setPitch(pitch.first, pitch.second);
    assertClose(n.pitchCV, -1, .0001);

    //******************************************************************************
   
    n.pitchCV = 0 + 1.f / 12.f + 1.f / 25.f;      // D4 plus less than half semi
    pitch = n.getPitch();
    assertEQ(pitch.first, 4);
    assertEQ(pitch.second, 1);

    n.setPitch(pitch.first, pitch.second);
    assertClose(n.pitchCV, 1.f / 12.f, .0001);

    //**********************************************************
    n.pitchCV = 0 + 1.f / 12.f + 1.f / 23.f;      // D4 plus more than half semi
    pitch = n.getPitch();
    assertEQ(pitch.first, 4);
    assertEQ(pitch.second, 2);

    n.setPitch(pitch.first, pitch.second);
    assertClose(n.pitchCV, 2.f / 12.f, .0001);
}

static void testCopyCtor()
{
    assertNoMidi();     // check for leaks
    {
        MidiNoteEvent note;
        note.pitchCV = 1.12f;
        note.duration = 33.45f;
        note.startTime = 15.3f;
        MidiNoteEvent note2(note);
        assert(note == note2);

        MidiNoteEvent note3 = note;
        assert(note == note3);

        MidiEndEvent end;
        end.startTime = 234.5f;
        MidiEndEvent end2(end);
        assert(end == end2);

        MidiEndEvent end3 = end;
        assert(end == end3);
    }
    assertNoMidi();     // check for leaks

}


static void testEqualNote()
{
    MidiNoteEvent note1;
    MidiNoteEvent note2;
    assert(note1 == note2);
    note2.pitchCV = .5;
    assert(note1 != note2);
    note2 = note1;
    assert(note1 == note2);
    note2.duration = 5;
    assert(note1 != note2);
    MidiNoteEvent note3(note2);
    assert(note3 == note2);
}

static void testEqualEnd()
{
    MidiEndEvent end1;
    MidiEndEvent end2;
    assert(end1 == end2);
    end2.startTime = 55;
    assert(end1 != end2);
}


static void testClone()
{
    MidiNoteEvent note;
    note.pitchCV = 4.1f;
    note.duration = .6f;
    note.startTime = .22f;

    MidiEventPtr evt = note.clone();
    MidiNoteEventPtr note2 = note.clonen();
    assert(note == *evt);
    assert(note == *note2);

    MidiEndEvent end;
    end.startTime = 102345;
    MidiEventPtr end2 = end.clone();
    assert(end == *end2);
}


void  testMidiEvents()
{
    assertNoMidi();     // check for leaks
    testType();
    testCast();
    testEqual();
    testPitch();
    testPitch2();
    testCopyCtor();

    testEqualNote();
    testEqualEnd();
    testClone();
    

    assertNoMidi();     // check for leaks
}