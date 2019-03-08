
#include "MidiLock.h"
#include "MidiPlayer.h"
#include "MidiSong.h"


MidiPlayer::MidiPlayer(std::shared_ptr<IPlayerHost> host, std::shared_ptr<MidiSong> song) :
    host(host), song(song), trackPlayer(song->getTrack(0))
{
    ++_mdb;
}

void MidiPlayer::timeElapsed(float seconds)
{
   
    curMetricTime += seconds * 120.0f / 60.0f;        // fixed at 120 bpm for now
    if (!isPlaying) {
        return;
    }
    bool locked = song->lock->playerTryLock();
    if (locked) {
        if (song->lock->dataModelDirty()) {
            trackPlayer.reset();
        }
        trackPlayer.updateToMetricTime(curMetricTime, host.get());
        song->lock->playerUnlock();
    } else {
        trackPlayer.reset();
        host->onLockFailed();
    }
}

TrackPlayer::TrackPlayer(MidiTrackPtr track) : track(track)
{
}

TrackPlayer::~TrackPlayer()
{
   //curEvent = nullptr;
    
}

void TrackPlayer::updateToMetricTime(double time, IPlayerHost* host)
{
    // If we had a conflict and needed to reset, then
    // start all over from beginning
    if (isReset) {
        curEvent = curEvent = track->begin();
        noteOffTime = -1;
        isReset = false;
        loopStart = 0;
    }
    // keep processing events until we are caught up
    while (playOnce(time, host)) {

    }
}

bool TrackPlayer::playOnce(double metricTime, IPlayerHost* host)
{
    bool didSomething = false;

    if (noteOffTime >= 0 && noteOffTime <= metricTime) {
        host->setGate(false);
        noteOffTime = -1;
        didSomething = true;
    }

    const double eventStart = (loopStart + curEvent->first);
    if (eventStart <= metricTime) {
        MidiEventPtr event = curEvent->second;
        switch (event->type) {
            case MidiEvent::Type::Note:
            {
                MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
                // should now output the note.
                host->setGate(true);
                host->setCV(note->pitchCV);

                // and save off the note-off time.
                noteOffTime = note->duration + eventStart;
                ++curEvent;
            }
            break;
            case MidiEvent::Type::End:
                // for now, should loop.
                // uh oh!
               // assert(false);
               // curMetricTime = 0;
              //  trackPlayStatus.curEvent = song->getTrack(0)->begin();
                loopStart += curEvent->first;
                curEvent = track->begin();
                break;
            default:
                assert(false);
        }

        didSomething = true;
    }
    return didSomething;
}

#if 0
void MidiPlayer::timeElapsed(float seconds)
{
    curMetricTime += seconds * 120.0f / 60.0f;        // fixed at 120 bpm for now
    bool locked = song->lock->playerTryLock();
    if (locked) {
        while (playOnce()) {
        }
        song->lock->playerUnlock();
    } else {
        trackPlayStatus.reset();
        host->onLockFailed();
    }
}


bool MidiPlayer::playOnce()
{
    if (!isPlaying) {
        return false;
    }
    bool didSomething = false;

    // If we had a conflict and needed to reset, then
    // seek from the start to where we should be.
    if (trackPlayStatus.isReset) {
        trackPlayStatus.seekTo(song.get(), curMetricTime, host.get());
        return true;
    }

    if (trackPlayStatus.noteOffTime >= 0 && trackPlayStatus.noteOffTime <= curMetricTime) {
        host->setGate(false);
        trackPlayStatus.noteOffTime = -1;
        didSomething = true;
    }

    if (trackPlayStatus.curEvent->first <= curMetricTime) {
        MidiEventPtr event = trackPlayStatus.curEvent->second;
        switch (event->type) {
            case MidiEvent::Type::Note:
            {
                MidiNoteEventPtr note = safe_cast<MidiNoteEvent>(event);
                // should now output the note.
                host->setGate(true);
                host->setCV(note->pitchCV);

                // and save off the note-off time.
                trackPlayStatus.noteOffTime = note->duration + note->startTime;
                ++trackPlayStatus.curEvent;
            }
            break;
            case MidiEvent::Type::End:
                // for now, should loop.
                curMetricTime = 0;
                trackPlayStatus.curEvent = song->getTrack(0)->begin();
                break;
            default:
                assert(false);
        }

        didSomething = true;
    }
    return didSomething;
}


void TrackPlayStatus::seekTo(MidiSong* song, float time, IPlayerHost* host)
{
    isReset = false;
    curEvent = song->getTrack(0)->begin();
    while (curEvent->second->startTime < time) {
        ++curEvent;
        if (curEvent == song->getTrack(0)->end()) {
            assert(false);
            return;
        }
        MidiEventPtr evt = curEvent->second;
        MidiNoteEventPtr note = safe_cast< MidiNoteEvent>(evt);
        assert(note);
        noteOffTime = note->startTime + note->duration;
    }
}
#endif