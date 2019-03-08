#pragma once
#include <memory>
#include <set>

class MidiEvent;
class MidiSelectionModel;

using MidiSelectionModelPtr = std::shared_ptr<MidiSelectionModel>;

/**
 * Central manager for tracking selections in the MidiSong being edited.
 */
class MidiSelectionModel
{
public:
    MidiSelectionModel();
    ~MidiSelectionModel();
    /**
     * replace the current selection with a single event
     */
    void select(std::shared_ptr<MidiEvent>);

    void extendSelection(std::shared_ptr<MidiEvent>);

    /**
     * select nothing
     */
    void clear();

    using container = std::set<std::shared_ptr<MidiEvent>>;
    using const_iterator = container::const_iterator;

    const_iterator begin() const;
    const_iterator end() const;

    int size() const
    {
        return (int) selection.size();
    }
    bool empty() const
    {
        return selection.empty();
    }

    MidiSelectionModelPtr clone() const;


    std::shared_ptr<MidiEvent> getLast();

    /** Returns true is this object instance is in selection.
     * i.e. changes on pointer value.
     * O(1)
     */
    bool isSelected(std::shared_ptr<MidiEvent>) const;

    /** Returns true is there is an object in selection equivalent
     * to 'event'. i.e.  selection contains entry == *event.
     * O(n), where n is the number of items in selection
     */
    bool isSelectedDeep(std::shared_ptr<MidiEvent> event) const;
private:


    container selection;
};
