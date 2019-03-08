
#pragma once
#include "util/math.hpp"
#include "nanovg.h"
#include "window.hpp"
#include "MidiEditorContext.h"
#include "MidiSequencer.h"
#include <GLFW/glfw3.h>
#include "UIPrefs.h"
#include "MidiKeyboardHandler.h"
#include "NoteScreenScale.h"
#include "PitchUtils.h"
#include "../ctrl/SqHelper.h"
#include "TimeUtils.h"

#include <sstream>


/**
 * This class needs some refactoring and renaming.
 * It is really the entire sequencer UI, including the notes.
 * 
 * Pretty soon we should sepparate out the NoteEditor.
 */
struct NoteDisplay : OpaqueWidget
{
    NoteDisplay(const Vec& pos, const Vec& size, MidiSongPtr song)
    {
        this->box.pos = pos;
		box.size = size;
        sequencer = std::make_shared<MidiSequencer>(song);
        sequencer->makeEditor();
        
        
        assert(sequencer->context->getSong() == song);
   
        // hard code view range to our demo song
        sequencer->context->setStartTime(0);
        sequencer->context->setEndTime(8);
        sequencer->context->setPitchLow(PitchUtils::pitchToCV(3, 0));
        sequencer->context->setPitchHi(PitchUtils::pitchToCV(5, 0));

        //initScaleFuncs();
        scaler = std::make_shared<NoteScreenScale>(sequencer->context, size.x, size.y);
        
        focusLabel = new Label();
        focusLabel->box.pos = Vec(40, 40);
        focusLabel->text = "";
        focusLabel->color = SqHelper::COLOR_WHITE;
        addChild(focusLabel);
        updateFocus(false);

          
        editAttributeLabel = new Label();
        editAttributeLabel->box.pos = Vec(10, 10);
        editAttributeLabel->text = "";
        editAttributeLabel->color = SqHelper::COLOR_WHITE;
        addChild(editAttributeLabel);

        barRangeLabel = new Label();
        barRangeLabel->box.pos = Vec(100, 10);
        barRangeLabel->text = "";
        barRangeLabel->color = SqHelper::COLOR_WHITE;
        addChild(barRangeLabel);
    }

    Label* focusLabel=nullptr;
    Label* editAttributeLabel = nullptr;
    Label* barRangeLabel = nullptr;
    std::shared_ptr<NoteScreenScale> scaler;
    MidiSequencerPtr sequencer;
    bool cursorState = false;
    int cursorFrameCount = 0;
    bool haveFocus = true;
    MidiEditorContext::NoteAttribute curAttribute = MidiEditorContext::NoteAttribute::Duration;
    int curFirstBar = -1;

    void step() override {
        auto attr = sequencer->context->noteAttribute;
        if (curAttribute != attr) {
            curAttribute = attr;
            switch (attr) {
                case MidiEditorContext::NoteAttribute::Pitch:
                    editAttributeLabel->text = "Pitch";
                    break;
                 case MidiEditorContext::NoteAttribute::Duration:
                    editAttributeLabel->text = "Duration";
                    break;
                 case MidiEditorContext::NoteAttribute::StartTime:
                    editAttributeLabel->text = "Start Time";
                    break;
            }
        }

        int firstBar = 1 + TimeUtils::timeToBar(sequencer->context->startTime());
        if (firstBar != curFirstBar) {
            curFirstBar = firstBar;
            std::stringstream str;
            str << "First Bar: " << curFirstBar << " Last Bar: " << curFirstBar + 1;
            barRangeLabel->text = str.str();
        }
    }

    void updateFocus(bool focus) {
        if (focus != haveFocus) {
            haveFocus = focus;
            focusLabel->text = focus ? "" : "Click in editor to get focus";
        }
    }

    void drawNotes(NVGcontext *vg)
    {
        MidiEditorContext::iterator_pair it = sequencer->context->getEvents();
        const int noteHeight = scaler->noteHeight();
        for ( ; it.first != it.second; ++it.first) {
            auto temp = *(it.first);
            MidiEventPtr evn = temp.second;
            MidiNoteEventPtr ev = safe_cast<MidiNoteEvent>(evn);

            const float x = scaler->midiTimeToX(*ev);
            const float y = scaler->midiPitchToY(*ev);
            const float width = scaler->midiTimeTodX(ev->duration);

          //  printf("draw note x=%f y=%f vs =%f\n", x, y, sequencer->context->viewport->startTime);
            fflush(stdout);

            const bool selected = sequencer->selection->isSelected(ev);
            filledRect(
                vg,
                selected ? UIPrefs::SELECTED_NOTE_COLOR : UIPrefs::NOTE_COLOR,
                x, y, width, noteHeight);
        }
    }

    void drawCursor(NVGcontext *vg) {
        cursorFrameCount--;
        if (cursorFrameCount < 0) {
            cursorFrameCount = 10;
            cursorState = !cursorState;
        }

        if (true) {
            auto color = cursorState ? 
                nvgRGB(0xff, 0xff, 0xff) :
                nvgRGB(0, 0, 0);
               
            const float x = scaler->midiTimeToX(sequencer->context->cursorTime());        
            const float y = scaler->midiCvToY(sequencer->context->cursorPitch()) + 
                 scaler->noteHeight() / 2.f;  
            filledRect(vg, color, x, y, 10, 3);
        }
    }

    void draw(NVGcontext *vg) override
    {   
        drawBackground(vg);
        drawNotes(vg);
        drawCursor(vg);
        OpaqueWidget::draw(vg);
    }

    void drawBackground(NVGcontext *vg) {
        filledRect(vg, UIPrefs::NOTE_EDIT_BACKGROUND, 0, 0, box.size.x, box.size.y);
        const int noteHeight = scaler->noteHeight();
        for (float cv = sequencer->context->pitchLow();
            cv <= sequencer->context->pitchHi();
            cv += PitchUtils::semitone) {
                const float y = scaler->midiCvToY(cv);
                const float width = box.size.x;
                bool accidental = PitchUtils::isAccidental(cv);
                if (accidental) {
                    filledRect(
                        vg,
                        UIPrefs::NOTE_EDIT_ACCIDENTAL_BACKGROUND,
                        0, y, width, noteHeight);
                }
        }
    }

   void strokedRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h)
    {
        nvgStrokeColor(vg, color);
        nvgBeginPath(vg);
        nvgRect(vg, x, y, w, h);
        nvgStroke(vg);
    }

    void filledRect(NVGcontext *vg, NVGcolor color, float x, float y, float w, float h)
    {
        nvgFillColor(vg, color);
        nvgBeginPath(vg);
        nvgRect(vg, x, y, w, h);
        nvgFill(vg);
    }
  
  //************************** These overrides are just to test even handling
    void onMouseDown(EventMouseDown &e) override
    {
        OpaqueWidget::onMouseDown(e);
      //  std::cout << "onMouseDown " << e.button << std::flush << std::endl;
    }
    void onMouseMove(EventMouseMove &e) override
    {
       //  std::cout << "onMouseMove " << std::flush << std::endl;
        OpaqueWidget::onMouseMove(e);
    }
    void onFocus(EventFocus &e) override
    {
        updateFocus(true);
        e.consumed = true;
    }
    void onDefocus(EventDefocus &e) override
    {
        updateFocus(false);
        e.consumed = true;
    }
    void onKey(EventKey &e) override
    {
        const unsigned key = e.key;
        unsigned mods = 0;
        if (rack::windowIsShiftPressed()) {
            mods |= GLFW_MOD_SHIFT;
        }
        if ( windowIsModPressed()) {
            mods |= GLFW_MOD_CONTROL;
        }

        bool handled =MidiKeyboardHandler::handle(sequencer.get(), key, mods);
        if (!handled) {
            OpaqueWidget::onKey(e);
        }
    }
    void onMouseEnter(EventMouseEnter &e) override
    {
        //std::cout << "nmouseenger " << std::flush << std::endl;
    }
   /** Called when another widget begins responding to `onMouseMove` events */
//	virtual void onMouseLeave(EventMouseLeave &e) {}

};
