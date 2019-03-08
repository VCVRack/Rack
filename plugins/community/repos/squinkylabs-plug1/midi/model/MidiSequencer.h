#pragma once

#include "MidiEditor.h"
#include "MidiSelectionModel.h"
#include "MidiSong.h"
#include "MidiEditorContext.h"
#include "UndoRedoStack.h"
#include <memory>

class MidiSong;
class MidiSequencer;
using MidiSequencerPtr = std::shared_ptr<MidiSequencer>;

/**
 * Despite the fancy name, this class doesn't do much.
 * It holds all the other modules that make up a sequencer.
 * It knows how to create all the objects and hook them up.
 */
class MidiSequencer : public std::enable_shared_from_this<MidiSequencer>
{
public:
    /** constructor takes a song to edit
    */
    MidiSequencer(std::shared_ptr<MidiSong>);

    /**
     * must be called to make constructor
     * todo: memory leak for circular ref
     */
    void makeEditor();

    ~MidiSequencer();

    void assertValid() const;

    /** The various classes we collect
     */
    MidiSelectionModelPtr const selection;
    MidiSongPtr const song;
    MidiEditorContextPtr context;
    MidiEditorPtr editor;
    UndoRedoStackPtr undo;

private:
    void assertSelectionInTrack() const;
};

