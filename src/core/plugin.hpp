#include <rack.hpp>


namespace rack {
namespace core {


extern Model* modelAudioInterface;
extern Model* modelAudioInterface16;
extern Model* modelMIDI_CV;
extern Model* modelMIDI_CC;
extern Model* modelMIDI_Gate;
extern Model* modelMIDI_Map;
extern Model* modelCV_MIDI;
extern Model* modelCV_CC;
extern Model* modelCV_Gate;
extern Model* modelBlank;
extern Model* modelNotes;


template <class TChoice>
struct Grid16MidiWidget : MidiWidget {
	LedDisplaySeparator* hSeparators[4];
	LedDisplaySeparator* vSeparators[4];
	TChoice* choices[4][4];

	template <class TModule>
	void setModule(TModule* module) {
		Vec pos = channelChoice->box.getBottomLeft();
		// Add vSeparators
		for (int x = 1; x < 4; x++) {
			vSeparators[x] = createWidget<LedDisplaySeparator>(pos);
			vSeparators[x]->box.pos.x = box.size.x / 4 * x;
			addChild(vSeparators[x]);
		}
		// Add hSeparators and choice widgets
		for (int y = 0; y < 4; y++) {
			hSeparators[y] = createWidget<LedDisplaySeparator>(pos);
			hSeparators[y]->box.size.x = box.size.x;
			addChild(hSeparators[y]);
			for (int x = 0; x < 4; x++) {
				choices[x][y] = new TChoice;
				choices[x][y]->box.pos = pos;
				choices[x][y]->setId(4 * y + x);
				choices[x][y]->box.size.x = box.size.x / 4;
				choices[x][y]->box.pos.x = box.size.x / 4 * x;
				choices[x][y]->setModule(module);
				addChild(choices[x][y]);
			}
			pos = choices[0][y]->box.getBottomLeft();
		}
		for (int x = 1; x < 4; x++) {
			vSeparators[x]->box.size.y = pos.y - vSeparators[x]->box.pos.y;
		}
	}
};


template <class TModule>
struct CcChoice : LedDisplayChoice {
	TModule* module;
	int id;
	int focusCc;

	CcChoice() {
		box.size.y = mm2px(6.666);
		textOffset.y -= 4;
	}

	void setModule(TModule* module) {
		this->module = module;
	}

	void setId(int id) {
		this->id = id;
	}

	void step() override {
		if (!module) {
			text = string::f("%d", id);
			return;
		}
		if (module->learningId == id) {
			if (0 <= focusCc)
				text = string::f("%d", focusCc);
			else
				text = "--";
			color.a = 0.5;
		}
		else {
			text = string::f("%d", module->learnedCcs[id]);
			color.a = 1.0;

			// HACK
			if (APP->event->selectedWidget == this)
				APP->event->setSelected(NULL);
		}
	}

	void onSelect(const event::Select& e) override {
		if (!module)
			return;
		module->learningId = id;
		focusCc = -1;
		e.consume(this);
	}

	void onDeselect(const event::Deselect& e) override {
		if (!module)
			return;
		if (module->learningId == id) {
			if (0 <= focusCc && focusCc < 128) {
				module->learnedCcs[id] = focusCc;
			}
			module->learningId = -1;
		}
	}

	void onSelectText(const event::SelectText& e) override {
		int c = e.codepoint - '0';
		if (0 <= c && c <= 9) {
			if (focusCc < 0)
				focusCc = 0;
			focusCc = focusCc * 10 + c;
		}
		if (focusCc >= 128)
			focusCc = 0;
		e.consume(this);
	}

	void onSelectKey(const event::SelectKey& e) override {
		if ((e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER) && e.action == GLFW_PRESS && (e.mods & RACK_MOD_MASK) == 0) {
			event::Deselect eDeselect;
			onDeselect(eDeselect);
			APP->event->selectedWidget = NULL;
			e.consume(this);
		}
	}
};


template <class TModule>
struct NoteChoice : LedDisplayChoice {
	TModule* module;
	int id;

	NoteChoice() {
		box.size.y = mm2px(6.666);
		textOffset.y -= 4;
		textOffset.x -= 4;
	}

	void setId(int id) {
		this->id = id;
	}

	void setModule(TModule* module) {
		this->module = module;
	}

	void step() override {
		if (!module) {
			text = "C4";
			return;
		}
		if (module->learningId == id) {
			text = "--";
			color.a = 0.5;
		}
		else {
			uint8_t note = module->learnedNotes[id];
			static const char* noteNames[] = {
				"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
			};
			int oct = note / 12 - 1;
			int semi = note % 12;
			text = string::f("%s%d", noteNames[semi], oct);
			color.a = 1.0;

			// HACK
			if (APP->event->selectedWidget == this)
				APP->event->setSelected(NULL);
		}
	}

	void onSelect(const event::Select& e) override {
		if (!module)
			return;
		module->learningId = id;
		e.consume(this);
	}

	void onDeselect(const event::Deselect& e) override {
		if (!module)
			return;
		if (module->learningId == id) {
			module->learningId = -1;
		}
	}
};


} // namespace core
} // namespace rack
