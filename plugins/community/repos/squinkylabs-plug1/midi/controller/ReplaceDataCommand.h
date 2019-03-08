#pragma once

#include <vector>

#include "MidiEvent.h"
#include "SqCommand.h"

class MidiEditorContext;
class MidiEvent;
class MidiNoteEvent;
class MidiSong;
class MidiSequencer;
class MidiSelectionModel;
class ReplaceDataCommand;

using ReplaceDataCommandPtr = std::shared_ptr<ReplaceDataCommand>;

class ReplaceDataCommand : public SqCommand
{
public:
    virtual void execute() override;
    virtual void undo() override;

    // TODO: rvalue
    ReplaceDataCommand(
        std::shared_ptr<MidiSong> song,
        std::shared_ptr<MidiSelectionModel>,
        std::shared_ptr<MidiEditorContext>,
        int trackNumber,
        const std::vector<MidiEventPtr>& inRemove,
        const std::vector<MidiEventPtr>& inAdd);

    /**
     * static factories for replace commands
     */
    static ReplaceDataCommandPtr makeDeleteCommand(std::shared_ptr<MidiSequencer> seq);
    static ReplaceDataCommandPtr makeInsertNoteCommand(std::shared_ptr<MidiSequencer> seq, std::shared_ptr<const MidiNoteEvent>);
    static ReplaceDataCommandPtr makeChangePitchCommand(std::shared_ptr<MidiSequencer> seq, int semitones);
    static ReplaceDataCommandPtr makeChangeStartTimeCommand(std::shared_ptr<MidiSequencer> seq, float delta);
    static ReplaceDataCommandPtr makeChangeDurationCommand(std::shared_ptr<MidiSequencer> seq, float delta);


private:
    std::shared_ptr<MidiSong> song;
    int trackNumber;
    std::shared_ptr<MidiSelectionModel> selection;

    std::vector<MidiEventPtr> removeData;
    std::vector<MidiEventPtr> addData;

    static void extendTrackToMinDuration(
        std::shared_ptr<MidiSequencer> seq,
        float neededLength,
        std::vector<MidiEventPtr>& toAdd,
        std::vector<MidiEventPtr>& toDelete);

    // base for change pitch, start time, duration
    enum class Ops {Pitch, Start, Duration };
    using Xform = std::function<void(MidiEventPtr)>;
    static ReplaceDataCommandPtr makeChangeNoteCommand(
        Ops,
        std::shared_ptr<MidiSequencer> seq,
        Xform xform,
        bool canChangeLength);
};

