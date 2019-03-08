#pragma once

#include "MidiTrack.h"
#include "FilteredIterator.h"
#include <memory>

class MidiSong;
class MidiSelectionModel;

class MidiEditorContext
{
public:
    MidiEditorContext(std::shared_ptr<MidiSong>);
    ~MidiEditorContext();

    float cursorPitch() const
    {
        return m_cursorPitch;
    }
    void setCursorPitch(float pitch)
    {
        m_cursorPitch = pitch;
    }
    float cursorTime() const
    {
        return m_cursorTime;
    }
    void setCursorTime(float time)
    {
        m_cursorTime = time;
    }

    MidiEvent::time_t startTime()
    {
        return m_startTime;
    }
    void setStartTime(MidiEvent::time_t t)
    {
        m_startTime = t;
    }
    void setEndTime(MidiEvent::time_t t)
    {
        m_endTime = t;
    }
    void setTimeRange(MidiEvent::time_t start, MidiEvent::time_t end)
    {
        m_startTime = start;
        m_endTime = end;
        assert(end > start);
    }

    MidiEvent::time_t endTime()
    {
        return m_endTime;
    }
    float pitchHi()
    {
        return m_pitchHi;
    }
    float pitchLow()
    {
        return m_pitchLow;
    }
    void setPitchLow(float p)
    {
        m_pitchLow = p;
    }
    void setPitchHi(float p)
    {
        m_pitchHi = p;
    }
    void setPitchRange(float l, float h)
    {
        assert(h >= l);
        m_pitchHi = h;
        m_pitchLow = l;
    }
    int getTrackNumber()
    {
        return trackNumber;
    }
    void setTrackNumber(int n)
    {
        trackNumber = n;
    }

    MidiTrackPtr getTrack();

    void setCursorToNote(MidiNoteEventPtrC note);
    void setCursorToSelection(std::shared_ptr<MidiSelectionModel> selection);

 
    // TODO: change to const_iterator
    using iterator = filtered_iterator<MidiEvent, MidiTrack::const_iterator>;
    using iterator_pair = std::pair<iterator, iterator>;
    iterator_pair getEvents() const;

    int track = 0;
    std::shared_ptr<MidiSong> getSong() const;

    void scrollVertically(float pitchCV);

    // Which field of note is being edited?
    enum class NoteAttribute
    {
        Pitch,
        Duration,
        StartTime
    };

    NoteAttribute noteAttribute;

    void assertValid() const;

    bool cursorInViewport() const;
    void assertCursorInViewport() const;
    void scrollViewportToCursorPitch();
    bool cursorInViewportTime() const;
    void adjustViewportForCursor();
private:
       // TODO: don't allow direct access?
    float m_cursorTime = 0;
    float m_cursorPitch = 0;

    // range will include t == start time, but will not
    // include t == endTime
    MidiEvent::time_t m_startTime = 0;
    MidiEvent::time_t m_endTime = 1;

    // pitch is inclusive: Low and Hi will be included
    float m_pitchLow = 0;
    float m_pitchHi = 0;

    int trackNumber = 0;

     // Below is not for clients to call. TODO: use private or something.
    // Definitely need some architecture here.
    std::weak_ptr<MidiSong> _song;

};

using MidiEditorContextPtr = std::shared_ptr<MidiEditorContext>;