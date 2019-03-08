#include "MidiLock.h"
#include "MidiSong.h"
#include "MidiTrack.h"

#include <assert.h>

MidiSong::MidiSong() : lock(std::make_shared<MidiLock>())
{
    ++_mdb;
}
MidiSong::~MidiSong()
{
    --_mdb;
}

int MidiSong::getHighestTrackNumber() const
{
    int numTracks = int(tracks.size());
    return numTracks - 1;
}


void MidiSong::addTrack(int index, std::shared_ptr<MidiTrack> track)
{
    if (index >= (int) tracks.size()) {
        tracks.resize(index + 1);
    }
    assert(!tracks[index]);         // can only create at empty loc

    tracks[index] = track;
}

void MidiSong::createTrack(int index)
{
    assert(lock);
    addTrack(index, std::make_shared<MidiTrack>(lock));
}


MidiTrackPtr MidiSong::getTrack(int index)
{
    assert(index < (int) tracks.size());
    assert(index >= 0);
    assert(tracks[index]);
    return tracks[index];
}


MidiTrackConstPtr MidiSong::getTrackConst(int index) const
{
    assert(index < (int) tracks.size());
    assert(index >= 0);
    assert(tracks[index]);
    return tracks[index];
}


MidiSongPtr MidiSong::makeTest(MidiTrack::TestContent content, int trackNumber)
{
    MidiSongPtr song = std::make_shared<MidiSong>();
    MidiLocker l(song->lock);
    auto track = MidiTrack::makeTest(content, song->lock);
    song->addTrack(trackNumber, track);
    song->assertValid();
    return song;
}
#if 0
MidiSongPtr MidiSong::makeTest1()
{
    MidiSongPtr song = std::make_shared<MidiSong>();
    auto track = MidiTrack::makeTest1();
    song->addTrack(0, track);
    song->assertValid();
    return song;
}

MidiSongPtr MidiSong::makeTestEmpty()
{
    MidiSongPtr song = std::make_shared<MidiSong>();
    auto track = MidiTrack::makeTestEmpty();
    song->addTrack(0, track);
    song->assertValid();
    return song;
}
#endif
void MidiSong::assertValid() const
{
    for (auto track : tracks) {
        if (track) {
            track->assertValid();
        }
    }
}