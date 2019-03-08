
#include <assert.h>
#include "MidiEditor.h"
#include "MidiEditorContext.h"
#include "MidiLock.h"
#include "MidiSelectionModel.h"
#include "MidiSequencer.h"
#include "MidiSong.h"
#include "MidiTrack.h"
#include "ReplaceDataCommand.h"
#include "TimeUtils.h"

extern int _mdb;

MidiEditor::MidiEditor(std::shared_ptr<MidiSequencer> seq) :
    m_seq(seq)
{
    _mdb++;
}

MidiEditor::~MidiEditor()
{
    _mdb--;
}

MidiTrackPtr MidiEditor::getTrack()
{
    return seq()->song->getTrack(seq()->context->getTrackNumber());
}

/**
 * If iterator already points to a note, return it.
 * Otherwise search for next one
 */
static MidiTrack::const_iterator findNextNoteOrCurrent(
    MidiTrackPtr track,
    MidiTrack::const_iterator it)
{
    if (it == track->end()) {
        return it;                  // if we are at the end, give up
    }
    for (bool done = false; !done; ) {

        if (it == track->end()) {
            done = true;
        }
        MidiEventPtr evt = it->second;
        if (evt->type == MidiEvent::Type::Note) {
            done = true;
        } else if (evt->type == MidiEvent::Type::End) {
            done = true;
        } else {
            assert(false);
            ++it;
        }
    }
    return it;
}

/**
 * returns track.end if can't find a note
 */
static MidiTrack::const_iterator findPrevNoteOrCurrent(
    MidiTrackPtr track,
    MidiTrack::const_iterator it)
{

    for (bool done = false; !done; ) {

        MidiEventPtr evt = it->second;
        switch (evt->type) {
            case  MidiEvent::Type::Note:
                done = true;                    // if we are on a note, then we can accept that
                break;
            case MidiEvent::Type::End:
                if (it == track->begin()) {
                    return track->end();            // Empty track, can't dec end ptr, so return "fail"
                } else {
                    --it;                           // try prev
                }
                break;
            default:
                assert(false);
                if (it == track->begin()) {
                    return track->end();            // Empty track, can't dec end ptr, so return "fail"
                } else {
                    --it;                           // try prev
                }
        }

    }
    return it;
}

static void selectNextNoteOrCurrent(
    MidiTrackPtr track,
    MidiTrack::const_iterator it,
    MidiSelectionModelPtr selection)
{
    it = findNextNoteOrCurrent(track, it);
    if (it == track->end()) {
        selection->clear();
    } else {
        MidiEventPtr evt = it->second;
        if (evt->type == MidiEvent::Type::End) {
            selection->clear();
        } else {
            selection->select(evt);
        }
    }
}

static void selectPrevNoteOrCurrent(
    MidiTrackPtr track,
    MidiTrack::const_iterator it,
    MidiSelectionModelPtr selection)
{
    it = findPrevNoteOrCurrent(track, it);
    if (it == track->end()) {
        // If we can't find a good one, give up
        selection->clear();
    } else {
        MidiEventPtr evt = it->second;
        if (evt->type == MidiEvent::Type::End) {
            selection->clear();
        } else {
            selection->select(evt);
        }
    }
}

void MidiEditor::selectNextNote()
{
    seq()->assertValid();

    MidiTrackPtr track = getTrack();
    assert(track);
    if (seq()->selection->empty()) {
        selectNextNoteOrCurrent(track, track->begin(), seq()->selection);
    } else {
        assert(seq()->selection->size() == 1);         // can't handle multi select yet
        MidiEventPtr evt = *seq()->selection->begin();
        assert(evt->type == MidiEvent::Type::Note);

        // find the event in the track
        auto it = track->findEventDeep(*evt);
        if (it == track->end()) {
            assert(false);
        }
        ++it;
        selectNextNoteOrCurrent(track, it, seq()->selection);
    }
    updateCursor();
    seq()->context->adjustViewportForCursor();
}

// Move to edit context?
void MidiEditor::updateCursor()
{
    if (seq()->selection->empty()) {
        return;
    }

    MidiNoteEventPtr firstNote;
    // If cursor is already in selection, leave it there.
    for (auto it : *seq()->selection) {
        MidiEventPtr ev = it;
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);
        if (note) {
            if (!firstNote) {
                firstNote = note;
            }
            if ((note->startTime == seq()->context->cursorTime()) &&
                (note->pitchCV == seq()->context->cursorPitch())) {
                return;
            }
        }
    }
    seq()->context->setCursorTime(firstNote->startTime);
    seq()->context->setCursorPitch(firstNote->pitchCV);
}

void MidiEditor::selectPrevNote()
{
    //assert(song);
    //assert(selection);
    seq()->assertValid();

    MidiTrackPtr track = getTrack();
    assert(track);
    if (seq()->selection->empty()) {
        // for prev, let's do same as next - if nothing selected, select first
        selectPrevNoteOrCurrent(track, --track->end(), seq()->selection);
    } else {
        // taken from next..
        assert(seq()->selection->size() == 1);         // can't handle multi select yet
        MidiEventPtr evt = *seq()->selection->begin();
        assert(evt->type == MidiEvent::Type::Note);

        // find the event in the track
        auto it = track->findEventDeep(*evt);
        if (it == track->begin()) {
            seq()->selection->clear();         // if we are at start, can't dec.unselect
            return;
        }
        --it;
        selectPrevNoteOrCurrent(track, it, seq()->selection);
    }
    updateCursor();
    seq()->context->adjustViewportForCursor();
}

void MidiEditor::changePitch(int semitones)
{
#if 1
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeChangePitchCommand(seq(), semitones);
    seq()->undo->execute(cmd);
    seq()->assertValid();
    float deltaCV = PitchUtils::semitone * semitones;
#else
    float deltaCV = PitchUtils::semitone * semitones;
    for (auto ev : *seq()->selection) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);       // for now selection is all notes
        note->pitchCV += deltaCV;
    }
#endif

    // Now fixup selection and viewport
    seq()->context->setCursorPitch(seq()->context->cursorPitch() + deltaCV);
    seq()->context->adjustViewportForCursor();
    seq()->context->assertCursorInViewport();
}

void MidiEditor::changeStartTime(bool ticks, int amount)
{
    MidiLocker l(seq()->song->lock);
    assert(!ticks);         // not implemented yet
    assert(amount != 0);
    float advanceAmount = amount * 1.f / 4.f;       // hard code units to 1/16th notes

#if 1
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeChangeStartTimeCommand(seq(), advanceAmount);
    seq()->undo->execute(cmd);
    seq()->assertValid();

    // after we change start times, we need to put the cursor on the moved notes
    seq()->context->setCursorToSelection(seq()->selection);
#else

    MidiNoteEventPtr lastNote = safe_cast<MidiNoteEvent>(seq()->selection->getLast());
    float lastTime = lastNote->startTime + lastNote->duration;
    lastTime += advanceAmount;
    extendTrackToMinDuration(lastTime);

    bool setCursor = false;
    MidiTrackPtr track = seq()->context->getTrack();

    for (auto ev : *seq()->selection) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);       // for now selection is all notes
        track->deleteEvent(*note);
        note->startTime += advanceAmount;
        note->startTime = std::max(0.f, note->startTime);
        track->insertEvent(note);
        if (!setCursor) {
            seq()->context->setCursorTime(note->startTime);
            setCursor = true;
        }
    }
#endif
    seq()->context->adjustViewportForCursor();
    seq()->context->assertCursorInViewport();
}

void MidiEditor::changeDuration(bool ticks, int amount)
{
    assert(!ticks);         // not implemented yet
    assert(amount != 0);

    float advanceAmount = amount * 1.f / 4.f;       // hard code units to 1/16th notes

#if 1
    ReplaceDataCommandPtr cmd = ReplaceDataCommand::makeChangeDurationCommand(seq(), advanceAmount);
    seq()->undo->execute(cmd);
    seq()->assertValid();
#else

    for (auto ev : *seq()->selection) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);       // for now selection is all notes
        note->duration += advanceAmount;

        // arbitrary min limit.
        note->duration = std::max(.001f, note->duration);
    }
#endif
}

void MidiEditor::assertCursorInSelection()
{
    bool foundIt = false;
    assert(!seq()->selection->empty());
    for (auto it : *seq()->selection) {
        if (seq()->context->cursorTime() == it->startTime) {
            foundIt = true;
        }
    }
    assert(foundIt);
}

void MidiEditor::advanceCursor(bool ticks, int amount)
{
    assert(!ticks);         // not implemented yet
    assert(amount != 0);

    seq()->context->assertCursorInViewport();

    float advanceAmount = amount * 1.f / 4.f;       // hard code units to 1/16th notes
    seq()->context->setCursorTime(seq()->context->cursorTime() + advanceAmount);
    seq()->context->setCursorTime(std::max(0.f, seq()->context->cursorTime()));
    updateSelectionForCursor();
    seq()->context->adjustViewportForCursor();
    seq()->context->assertCursorInViewport();
    seq()->assertValid();
}

void MidiEditor::extendTrackToMinDuration(float neededLength)
{
    auto track = seq()->context->getTrack();
    float curLength = track->getLength();

    if (neededLength > curLength) {
        float need = neededLength;
        float needBars = need / 4.f;
        float roundedBars = std::round(needBars + 1.f);
        float duration = roundedBars * 4;
        std::shared_ptr<MidiEndEvent> end = track->getEndEvent();
        track->deleteEvent(*end);
        track->insertEnd(duration);
    }
}

void MidiEditor::insertNote()
{
    MidiLocker l(seq()->song->lock);
#if 1
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->startTime = seq()->context->cursorTime();
    note->pitchCV = seq()->context->cursorPitch();
    note->duration = 1;  // for now, fixed to quarter
    auto cmd = ReplaceDataCommand::makeInsertNoteCommand(seq(), note);

    seq()->undo->execute(cmd);
#else
     // for now, fixed to quarter
    extendTrackToMinDuration(seq()->context->cursorTime() + 1.f);

    auto track = seq()->context->getTrack();
    // for now, assume no note there
    MidiNoteEventPtr note = std::make_shared<MidiNoteEvent>();
    note->pitchCV = seq()->context->cursorPitch();
    note->startTime = seq()->context->cursorTime();
    note->duration = 1.0f;          // for now, fixed to quarter
    track->insertEvent(note);
#endif
    updateSelectionForCursor();
    seq()->assertValid();
}

void MidiEditor::deleteNote()
{

    if (seq()->selection->empty()) {
        return;
    }

    auto cmd = ReplaceDataCommand::makeDeleteCommand(seq());

    seq()->undo->execute(cmd);
    // TODO: move selection into undo
    seq()->selection->clear();
}

void MidiEditor::updateSelectionForCursor()
{
    seq()->selection->clear();
    const int cursorSemi = PitchUtils::cvToSemitone(seq()->context->cursorPitch());

    // iterator over all the notes that are in the edit context
    auto start = seq()->context->startTime();
    auto end = seq()->context->endTime();
    MidiTrack::note_iterator_pair notes = getTrack()->timeRangeNotes(start, end);
    for (auto it = notes.first; it != notes.second; ++it) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(it->second);
        const auto startTime = note->startTime;
        const auto endTime = note->startTime + note->duration;

        if ((PitchUtils::cvToSemitone(note->pitchCV) == cursorSemi) &&
            (startTime <= seq()->context->cursorTime()) &&
            (endTime > seq()->context->cursorTime())) {
            seq()->selection->select(note);
            return;
        }
    }
}

void MidiEditor::changeCursorPitch(int semitones)
{
    float pitch = seq()->context->cursorPitch() + (semitones * PitchUtils::semitone);
    pitch = std::max(pitch, -5.f);
    pitch = std::min(pitch, 5.f);
    seq()->context->setCursorPitch(pitch);
    seq()->context->scrollViewportToCursorPitch();
    updateSelectionForCursor();
}

void MidiEditor::setNoteEditorAttribute(MidiEditorContext::NoteAttribute attr)
{
    seq()->context->noteAttribute = attr;
}