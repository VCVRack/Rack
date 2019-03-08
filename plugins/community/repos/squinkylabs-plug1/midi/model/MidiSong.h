#pragma once

#include "MidiTrack.h"
#include <vector>
#include <memory>


class MidiSong;
class MidiLock;

using MidiSongPtr = std::shared_ptr<MidiSong>;

class MidiSong
{
public:
    MidiSong();
    ~MidiSong();
    std::shared_ptr<MidiTrack> getTrack(int index);
    std::shared_ptr<const MidiTrack> getTrackConst(int index) const;
    void createTrack(int index);

    void assertValid() const;

    /**
     * returns -1 if no tracks exist
     */
    int getHighestTrackNumber() const;

    /**
     * factory method to generate test content
     */
    static MidiSongPtr makeTest(MidiTrack::TestContent, int trackNumber);

    std::shared_ptr<MidiLock> lock;
private:
    std::vector<std::shared_ptr<MidiTrack>> tracks;

    /** like create track, but passes in the track
     */
    void addTrack(int index, std::shared_ptr<MidiTrack>);

   
};

