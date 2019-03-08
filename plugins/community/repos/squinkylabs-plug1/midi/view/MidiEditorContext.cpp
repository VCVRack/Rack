
#include "MidiEditorContext.h"
#include "MidiSelectionModel.h"
#include "MidiSong.h"
#include "TimeUtils.h"

extern int _mdb;

MidiEditorContext::MidiEditorContext(MidiSongPtr song) : _song(song)
{
    ++_mdb;
}

MidiEditorContext::~MidiEditorContext()
{
    --_mdb;
}

void MidiEditorContext::scrollViewportToCursorPitch()
{
    while (m_cursorPitch < pitchLow()) {
        scrollVertically(-1 * PitchUtils::octave);
    }
    while (m_cursorPitch > pitchHi()) {
        scrollVertically(1 * PitchUtils::octave);
    }
}

void MidiEditorContext::assertCursorInViewport() const
{
    assertGE(m_cursorTime, m_startTime);
    assertLT(m_cursorTime, m_endTime);
    assertGE(m_cursorPitch, m_pitchLow);
    assertLE(m_cursorPitch, m_pitchHi);
}
 
void MidiEditorContext::assertValid() const
{
    assert(m_endTime > m_startTime);
    assert(m_pitchHi >= m_pitchLow);
    //assert(getSong());

    assertGE(m_cursorTime, 0);
    assertLT(m_cursorPitch, 10);      // just for now
    assertGT(m_cursorPitch, -10);

    assertCursorInViewport();
}

void MidiEditorContext::scrollVertically(float pitchCV)
{
    m_pitchHi += pitchCV;
    m_pitchLow += pitchCV;
}

MidiSongPtr MidiEditorContext::getSong() const
{
    return _song.lock();
}

MidiEditorContext::iterator_pair MidiEditorContext::getEvents() const
{

    iterator::filter_func lambda = [this](MidiTrack::const_iterator ii) {
        const MidiEventPtr me = ii->second;
        bool ret = false;
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(me);
        if (note) {
            ret = note->pitchCV >= m_pitchLow && note->pitchCV <= m_pitchHi;
        }
        if (ret) {
            ret = me->startTime < this->m_endTime;
        }
        return ret;
    };

    const auto song = getSong();
    const auto track = song->getTrack(this->track);

    // raw will be pair of track::const_iterator
    const auto rawIterators = track->timeRange(this->m_startTime, this->m_endTime);

    return iterator_pair(iterator(rawIterators.first, rawIterators.second, lambda),
        iterator(rawIterators.second, rawIterators.second, lambda));
}

bool MidiEditorContext::cursorInViewport() const
{
    if (m_cursorTime < m_startTime) {
        return false;
    }
    if (m_cursorTime >= m_endTime) {
        return false;
    }
    if (m_cursorPitch > m_pitchHi) {
        return false;
    }
    if (m_cursorPitch < m_pitchLow) {
        return false;
    }

    return true;
}

bool MidiEditorContext::cursorInViewportTime() const
{
    if (m_cursorTime < m_startTime) {
        return false;
    }
    if (m_cursorTime >= m_endTime) {
        return false;
    }

    return true;
}

void MidiEditorContext::adjustViewportForCursor()
{
    if (!cursorInViewportTime()) {

        float minimumAdvance = 0;
        if (m_cursorTime >= m_endTime) {
            minimumAdvance = m_cursorTime - m_endTime;
        } else if (m_cursorTime < m_startTime) {
            minimumAdvance = m_cursorTime - m_startTime;
        }

        // figure what fraction of 2 bars this is
        float advanceBars = minimumAdvance / TimeUtils::barToTime(2);
        advanceBars += (minimumAdvance < 0) ? -2 : 2;

        float x = std::round(advanceBars / 2.f);
        float finalAdvanceTime = x * TimeUtils::barToTime(2);

        m_startTime += finalAdvanceTime;
        m_endTime = m_startTime + TimeUtils::barToTime(2);
        assert(m_startTime >= 0);
    }

    // and to the pitch
    scrollViewportToCursorPitch();
}

MidiTrackPtr MidiEditorContext::getTrack()
{
    MidiSongPtr song = getSong();
    assert(song);
    return song->getTrack(trackNumber);
}

void MidiEditorContext::setCursorToNote(MidiNoteEventPtrC note)
{
    m_cursorTime = note->startTime;
    m_cursorPitch = note->pitchCV;
    adjustViewportForCursor();
}

void MidiEditorContext::setCursorToSelection(MidiSelectionModelPtr selection)
{
    // could be wrong for multi-select
    if (!selection->empty()) {
        MidiEventPtr ev = *selection->begin();
        MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(ev);
        assert(note);
        if (note) {
            setCursorToNote(note);
        }
    }
}