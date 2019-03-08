
#include "MidiEvent.h"
#include "MidiSelectionModel.h"

#include <assert.h>
extern int _mdb;
MidiSelectionModel::MidiSelectionModel()
{
    ++_mdb;
}

MidiSelectionModel::~MidiSelectionModel()
{
    --_mdb;
}

void MidiSelectionModel::select(std::shared_ptr<MidiEvent> event)
{
    selection.clear();
    assert(selection.empty());
    selection.insert(event);
}

void MidiSelectionModel::extendSelection(std::shared_ptr<MidiEvent> event)
{
    selection.insert(event);
}

MidiSelectionModel::const_iterator MidiSelectionModel::begin() const
{
    return selection.begin();
}

MidiSelectionModel::const_iterator MidiSelectionModel::end() const
{
    return selection.end();
}

void MidiSelectionModel::clear()
{
    selection.clear();
}

bool MidiSelectionModel::isSelected(MidiEventPtr evt) const
{
    auto it = selection.find(evt);
    return it != selection.end();
}

MidiEventPtr MidiSelectionModel::getLast()
{
    MidiEventPtr ret;
    float lastTime = 0;
    for (auto it : selection) {
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(it);
        if (note) {
            float noteEnd = note->startTime + note->duration;
            if (noteEnd > lastTime) {
                ret = note;
                lastTime = noteEnd;
            }
        } else {
            float end = it->startTime;
            if (end > lastTime) {
                ret = it;
                lastTime = end;
            }
        }
    }
    return ret;
}

MidiSelectionModelPtr MidiSelectionModel::clone() const
{
    MidiSelectionModelPtr ret = std::make_shared<MidiSelectionModel>();
    for (auto it : selection) {
        MidiEventPtr clonedEvent = it->clone();
        ret->selection.insert(clonedEvent);
    }
    return ret;
}

bool MidiSelectionModel::isSelectedDeep(MidiEventPtr evt) const
{
    auto it = std::find_if(begin(), end(), [evt](MidiEventPtr ev) {
        return *ev == *evt;
    });

    return it != end();
}
