
#include "MidiSequencer.h"
#include "MidiEditor.h"
#include "UndoRedoStack.h"

int _mdb = 0;       // global instance counter

MidiSequencer::MidiSequencer(std::shared_ptr<MidiSong> sng) :
    selection(std::make_shared<MidiSelectionModel>()),
    song(sng),
    context(std::make_shared<MidiEditorContext>(sng)
    )
{
    undo = std::make_shared<UndoRedoStack>();
    ++_mdb;
}

void MidiSequencer::makeEditor()
{
    MidiSequencerPtr seq = shared_from_this();
    editor = std::make_shared<MidiEditor>(seq);
}

MidiSequencer::~MidiSequencer()
{
    --_mdb;
}


void MidiSequencer::assertValid() const
{
    assert(editor);
    assert(undo);
    assert(song);
    assert(context);
    assert(selection);
    context->assertValid();
    song->assertValid(); 
    assertSelectionInTrack();
}

void MidiSequencer::assertSelectionInTrack() const
{
    MidiTrackPtr track = context->getTrack();
    for (auto it : *selection) {
#if 1
        
        auto foundPtr = track->findEventPointer(it);
        assert(foundPtr != track->end());
        auto x = *foundPtr;
        MidiEventPtrC y = x.second;

       // MidiEventPtrC x = track->findEventPointer(it);;
#else
       MidiEventPtrC foundPtr = track->findEventPointer(it)->second;
       assert(foundPtr);
#endif
       
       
    }
}