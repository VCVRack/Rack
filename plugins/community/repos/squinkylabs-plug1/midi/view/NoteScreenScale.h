#pragma once

class MidiEditorContext;



#include "MidiEvent.h"
/**
 * This class know how to map between pitch, time, and screen coordinates.
 * notes on the screen have:
 *      height in pixels - determined by vertical zoom
 *      width in pixels - determined by duration and horizontal zoom
 *      x position where the note starts.
 *      y position of the upper edge of the notes.
 *
 * Coordinate conventions:
 *      if viewport hi and low pitches are the same, it maps a note of that pitch to full screen.
 *      y==0 it the top edge, increasing y goes down the screen
 */

class NoteScreenScale
{
public:
    NoteScreenScale(std::shared_ptr<MidiEditorContext> vp, float screenWidth, float screenHeight);
    float midiTimeToX(const MidiEvent& ev);
    float midiTimeToX(MidiEvent::time_t ev);
    float midiTimeTodX(MidiEvent::time_t dt);

    std::pair<float, float> midiTimeToHBounds(const MidiNoteEvent& note);



    float midiPitchToY(const MidiNoteEvent& note);
    float midiCvToY(float cv);

    float noteHeight();
private:
    float ax = 0;
    float ay = 0;
    std::shared_ptr<MidiEditorContext> viewport;
};