#include "ReplaceDataCommand.h"
#include "MidiLock.h"
#include "MidiSequencer.h"
#include "MidiSong.h"
#include "MidiTrack.h"

#include <assert.h>

ReplaceDataCommand::ReplaceDataCommand(
    MidiSongPtr song,
    MidiSelectionModelPtr selection,
    std::shared_ptr<MidiEditorContext> unused,
    int trackNumber,
    const std::vector<MidiEventPtr>& inRemove,
    const std::vector<MidiEventPtr>& inAdd)
    : song(song), trackNumber(trackNumber), selection(selection), removeData(inRemove), addData(inAdd)
{
    assert(song->getTrack(trackNumber));
}

void ReplaceDataCommand::execute()
{
    MidiTrackPtr mt = song->getTrack(trackNumber);
    assert(mt);
    MidiLocker l(mt->lock);

    for (auto it : addData) {
        mt->insertEvent(it);
    }

    for (auto it : removeData) {
        mt->deleteEvent(*it);
    }

    // clone the selection, clear real selection, add stuff back correctly
    // at the very least we must clear the selection, as those notes are no
    // longer in the track.
    MidiSelectionModelPtr reference = selection->clone();
    selection->clear();
    for (auto it : addData) {
        auto foundIter = mt->findEventDeep(*it);      // find an event in the track that matches the one we just inserted
        assert(foundIter != mt->end());
        MidiEventPtr evt = foundIter->second;
        selection->extendSelection(evt);
    }
}

void ReplaceDataCommand::undo()
{
    MidiTrackPtr mt = song->getTrack(trackNumber);
    assert(mt);
    MidiLocker l(mt->lock);

    // to undo the insertion, delete all of them
    for (auto it : addData) {
        mt->deleteEvent(*it);
    }
    for (auto it : removeData) {
        mt->insertEvent(it);
    }

    selection->clear();
    for (auto it : removeData) {
        auto foundIter = mt->findEventDeep(*it);      // find an event in the track that matches the one we just inserted
        assert(foundIter != mt->end());
        MidiEventPtr evt = foundIter->second;
        selection->extendSelection(evt);
    }

    // TODO: move cursor
}


ReplaceDataCommandPtr ReplaceDataCommand::makeDeleteCommand(MidiSequencerPtr seq)
{
    std::vector<MidiEventPtr> toRemove;
    std::vector<MidiEventPtr> toAdd;
    auto track = seq->context->getTrack();
    for (auto it : *seq->selection) {
        MidiEventPtr ev = it;
        toRemove.push_back(ev);
    }
    ReplaceDataCommandPtr ret = std::make_shared<ReplaceDataCommand>(
        seq->song,
        seq->selection,
        seq->context,
        seq->context->getTrackNumber(),
        toRemove,
        toAdd);
    return ret;
}

ReplaceDataCommandPtr ReplaceDataCommand::makeChangeNoteCommand(
    Ops op,
    std::shared_ptr<MidiSequencer> seq,
    Xform xform,
    bool canChangeLength)
{
    std::vector<MidiEventPtr> toAdd;
    std::vector<MidiEventPtr> toRemove;

    if (canChangeLength) {
        // Figure out the duration of the track after xforming the notes
        MidiSelectionModelPtr clonedSelection = seq->selection->clone();
        // find required length
        MidiEndEventPtr end = seq->context->getTrack()->getEndEvent();
        float endTime = end->startTime;

        for (auto it : *clonedSelection) {
            MidiEventPtr ev = it;
            xform(ev);
            float t = ev->startTime;
            MidiNoteEventPtrC note = safe_cast<MidiNoteEvent>(ev);
            if (note) {
                t += note->duration;
                //printf("note extends to %f\n", t);
            }
            endTime = std::max(endTime, t);
        }
       // printf("when done, end Time = %f\n", endTime);
        // set up events to extend to that length
        if (endTime > end->startTime) {
            extendTrackToMinDuration(seq, endTime, toAdd, toRemove);
        }
    }

    MidiSelectionModelPtr clonedSelection = seq->selection->clone();

    // will remove existing selection
    for (auto it : *seq->selection) {
        auto note = safe_cast<MidiNoteEvent>(it);
        if (note) {
            toRemove.push_back(note);
        }
    }

    // and add back the transformed notes
    for (auto it : *clonedSelection) {
        MidiEventPtr event = it;
        xform(event);
        toAdd.push_back(event);
    }

    ReplaceDataCommandPtr ret = std::make_shared<ReplaceDataCommand>(
        seq->song,
        seq->selection,
        seq->context,
        seq->context->getTrackNumber(),
        toRemove,
        toAdd);
    return ret;
}

ReplaceDataCommandPtr ReplaceDataCommand::makeChangePitchCommand(MidiSequencerPtr seq, int semitones)
{
    const float deltaCV = PitchUtils::semitone * semitones;
    Xform xform = [deltaCV](MidiEventPtr event) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            note->pitchCV += deltaCV;
        }

    };
    return makeChangeNoteCommand(Ops::Pitch, seq, xform, false);
}

ReplaceDataCommandPtr ReplaceDataCommand::makeChangeStartTimeCommand(MidiSequencerPtr seq, float delta)
{
    Xform xform = [delta](MidiEventPtr event) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            note->startTime += delta;
            note->startTime = std::max(0.f, note->startTime);
        }
    };
    return makeChangeNoteCommand(Ops::Start, seq, xform, true);
}

ReplaceDataCommandPtr ReplaceDataCommand::makeChangeDurationCommand(MidiSequencerPtr seq, float delta)
{
    Xform xform = [delta](MidiEventPtr event) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
        if (note) {
            note->duration += delta;
             // arbitrary min limit.
            note->duration = std::max(.001f, note->duration);
        }
    };
    return makeChangeNoteCommand(Ops::Duration, seq, xform, true);
}


ReplaceDataCommandPtr ReplaceDataCommand::makeInsertNoteCommand(MidiSequencerPtr seq, MidiNoteEventPtrC origNote)
{
    MidiNoteEventPtr note = origNote->clonen();

    // Make the delete end / inserts end to extend track.
    // Make it long enough to hold insert note.
    std::vector<MidiEventPtr> toRemove;
    std::vector<MidiEventPtr> toAdd;
    extendTrackToMinDuration(seq, note->startTime + note->duration, toAdd, toRemove);

    toAdd.push_back(note);

    ReplaceDataCommandPtr ret = std::make_shared<ReplaceDataCommand>(
        seq->song,
        seq->selection,
        seq->context,
        seq->context->getTrackNumber(),
        toRemove,
        toAdd);
    return ret;
}

void ReplaceDataCommand::extendTrackToMinDuration(
    MidiSequencerPtr seq,
    float neededLength,
    std::vector<MidiEventPtr>& toAdd,
    std::vector<MidiEventPtr>& toDelete)
{
    auto track = seq->context->getTrack();
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