#include "rack.hpp"


using namespace rack;


extern Model *modelAudioInterface;
extern Model *modelMIDIToCVInterface;
extern Model *modelQuadMIDIToCVInterface;
extern Model *modelMIDICCToCVInterface;
extern Model *modelMIDITriggerToCVInterface;
extern Model *modelBlank;
extern Model *modelNotes;



struct GridChoice : LedDisplayChoice {
	virtual void setId(int id) {}
};


struct Grid16MidiWidget : MidiWidget {
	LedDisplaySeparator *hSeparators[4];
	LedDisplaySeparator *vSeparators[4];
	GridChoice *gridChoices[4][4];

	void createGridChoices() {
		Vec pos = channelChoice->box.getBottomLeft();
		for (int x = 1; x < 4; x++) {
			vSeparators[x] = Widget::create<LedDisplaySeparator>(pos);
			addChild(vSeparators[x]);
		}
		for (int y = 0; y < 4; y++) {
			hSeparators[y] = Widget::create<LedDisplaySeparator>(pos);
			addChild(hSeparators[y]);
			for (int x = 0; x < 4; x++) {
				GridChoice *gridChoice = createGridChoice();
				assert(gridChoice);
				gridChoice->box.pos = pos;
				gridChoice->setId(4*y + x);
				gridChoices[x][y] = gridChoice;
				addChild(gridChoice);
			}
			pos = gridChoices[0][y]->box.getBottomLeft();
		}
		for (int x = 1; x < 4; x++) {
			vSeparators[x]->box.size.y = pos.y - vSeparators[x]->box.pos.y;
		}
	}
	void step() override {
		MidiWidget::step();
		for (int x = 1; x < 4; x++) {
			vSeparators[x]->box.pos.x = box.size.x / 4 * x;
		}
		for (int y = 0; y < 4; y++) {
			hSeparators[y]->box.size.x = box.size.x;
			for (int x = 0; x < 4; x++) {
				gridChoices[x][y]->box.size.x = box.size.x / 4;
				gridChoices[x][y]->box.pos.x = box.size.x / 4 * x;
			}
		}
	}
	virtual GridChoice *createGridChoice() {return NULL;}
};