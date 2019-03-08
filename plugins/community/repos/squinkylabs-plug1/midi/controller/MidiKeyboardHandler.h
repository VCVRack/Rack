#pragma once

class MidiSequencer;

class MidiKeyboardHandler
{
public:
    static bool handle(MidiSequencer* sequencer, unsigned key, unsigned mods);
private:
    enum class ChangeType { lessThan, plus, bracket };
    static void handleNoteEditorChange(MidiSequencer* sequencer, ChangeType type, bool increase);
};
