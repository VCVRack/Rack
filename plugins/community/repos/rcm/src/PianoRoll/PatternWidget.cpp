#include "../../include/PianoRoll/PatternWidget.hpp"
#include "../../include/PianoRoll/PianoRollModule.hpp"
#include "../../include/PianoRoll/PianoRollWidget.hpp"

namespace rack_plugin_rcm {

struct PatternItem : MenuItem {
	PatternWidget *widget = NULL;
	int pattern;
	void onAction(EventAction &e) override {
		widget->module->transport.setPattern(pattern);
	}
};

struct PatternChoice : LedDisplayChoice {
	PatternWidget *widget = NULL;

	void onAction(EventAction &e) override {
		if (widget->module->inputs[PianoRollModule::PATTERN_INPUT].active == false) {
			Vec pos = RACK_PLUGIN_UI_RACKWIDGET->lastMousePos.minus(widget->widget->box.pos).minus(widget->box.pos);

			if (pos.x < 20) {
				widget->module->transport.advancePattern(-1);
			} else if (pos.x > 67) {
				widget->module->transport.advancePattern(1);
			} else {
				Menu *menu = RACK_PLUGIN_UI_RACKSCENE->createMenu();
				menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Pattern"));

				for (int i = 0; i < 64; i++) {
					PatternItem *item = new PatternItem();
					item->widget = widget;
					item->pattern = i;
					item->text = stringf("%d/64", i+1);
					item->rightText = CHECKMARK(item->pattern == widget->module->transport.currentPattern());
					menu->addChild(item);
				}
			}
		}
	}
	void step() override {
		text = stringf("- Ptrn %02d +", widget->module->transport.currentPattern() + 1);
	}
};

struct MeasuresItem : MenuItem {
	PatternWidget *widget = NULL;
	int measures;
	void onAction(EventAction &e) override {
		widget->module->patternData.setMeasures(widget->module->transport.currentPattern(), measures);
	}
};

struct MeasuresChoice : LedDisplayChoice {
	PatternWidget *widget = NULL;

	void onAction(EventAction &e) override {
		Menu *menu = RACK_PLUGIN_UI_RACKSCENE->createMenu();
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Measures"));

		for (int i = 1; i <= 16; i++) {
			MeasuresItem *item = new MeasuresItem();
			item->widget = widget;
			item->measures = i;
			item->text = stringf("%d measures", i);
			item->rightText = CHECKMARK(item->measures == widget->module->patternData.getMeasures(widget->module->transport.currentPattern()));
			menu->addChild(item);
		}
	}
	void step() override {
		text = stringf("Measures %d", widget->module->patternData.getMeasures(widget->module->transport.currentPattern()));
	}
};

struct BeatsPerMeasureItem : MenuItem {
	PatternWidget *widget = NULL;
	int beatsPerMeasure;
	void onAction(EventAction &e) override {
		widget->module->patternData.setBeatsPerMeasure(widget->module->transport.currentPattern(), beatsPerMeasure);
	}
};

struct BeatsPerMeasureChoice : LedDisplayChoice {
	PatternWidget *widget = NULL;

	void onAction(EventAction &e) override {
		Menu *menu = RACK_PLUGIN_UI_RACKSCENE->createMenu();
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Beats Per Measure"));

		for (int i = 1; i <= 16; i++) {
			BeatsPerMeasureItem *item = new BeatsPerMeasureItem();
			item->widget = widget;
			item->beatsPerMeasure = i;
			item->text = stringf("%d beats", i);
			item->rightText = CHECKMARK(item->beatsPerMeasure == widget->module->patternData.getBeatsPerMeasure(widget->module->transport.currentPattern()));
			menu->addChild(item);
		}
	}
	void step() override {
		text = stringf("%d", widget->module->patternData.getBeatsPerMeasure(widget->module->transport.currentPattern()));
	}
};

struct DivisionsPerBeatItem : MenuItem {
	PatternWidget *widget = NULL;
	int divisionsPerBeat;
	void onAction(EventAction &e) override {
		widget->module->patternData.setDivisionsPerBeat(widget->module->transport.currentPattern(), divisionsPerBeat);
	}
};

struct DivisionsPerBeatChoice : LedDisplayChoice {
	PatternWidget *widget = NULL;

	void onAction(EventAction &e) override {
		Menu *menu = RACK_PLUGIN_UI_RACKSCENE->createMenu();
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Divisions Per Beat"));

		for (int i = 1; i <= 16; i++) {
			DivisionsPerBeatItem *item = new DivisionsPerBeatItem();
			item->widget = widget;
			item->divisionsPerBeat = i;
			item->text = stringf("%d divisions", i);
			item->rightText = CHECKMARK(item->divisionsPerBeat == widget->module->patternData.getDivisionsPerBeat(widget->module->transport.currentPattern()));
			menu->addChild(item);
		}
	}
	void step() override {
		text = stringf("%d", widget->module->patternData.getDivisionsPerBeat(widget->module->transport.currentPattern()));
	}
};

struct SequenceRunningChoice : LedDisplayChoice {
	PatternWidget *widget = NULL;

	void onAction(EventAction &e) override {
		widget->module->transport.toggleRun();
	}
	void step() override {
		std::string displayText;
		if (widget->module->transport.isRunning()) {
			if (widget->module->transport.isRecording()) {
				displayText += "Recording";
			} else if (widget->module->transport.isPendingRecording()) {
				displayText += "Prerecord";
			} else {
				displayText += "Running";
			}
		} else {
			displayText += "Paused";

			if (widget->module->transport.isRecording()) {
				displayText += " (rec)";
			}

			if (widget->module->transport.isPendingRecording()) {
				displayText += " (pre)";
			}
		}

		text = displayText;
	}
};


PatternWidget::PatternWidget() {

	// measuresChoice
	// measuresSeparator
	// beatsPerMeasureChoice
	// beatsPerMeasureSeparator
	// divisionsPerBeatChoice
	// divisionsPerBeatSeparator
	// tripletsChoice

	Vec pos = Vec();

	PatternChoice *patternChoice = Widget::create<PatternChoice>(pos);
	patternChoice->widget = this;
	addChild(patternChoice);
	pos = patternChoice->box.getBottomLeft();
	this->patternChoice = patternChoice;

	this->patternSeparator = Widget::create<LedDisplaySeparator>(pos);
	addChild(this->patternSeparator);

	MeasuresChoice *measuresChoice = Widget::create<MeasuresChoice>(pos);
	measuresChoice->widget = this;
	addChild(measuresChoice);
	pos = measuresChoice->box.getBottomLeft();
	this->measuresChoice = measuresChoice;

	this->measuresSeparator = Widget::create<LedDisplaySeparator>(pos);
	addChild(this->measuresSeparator);

	BeatsPerMeasureChoice *beatsPerMeasureChoice = Widget::create<BeatsPerMeasureChoice>(pos);
	beatsPerMeasureChoice->widget = this;
	addChild(beatsPerMeasureChoice);
	this->beatsPerMeasureChoice = beatsPerMeasureChoice;

	this->beatsPerMeasureSeparator = Widget::create<LedDisplaySeparator>(pos);
	this->beatsPerMeasureSeparator->box.size.y = this->beatsPerMeasureChoice->box.size.y;
	addChild(this->beatsPerMeasureSeparator);

	DivisionsPerBeatChoice *divisionsPerBeatChoice = Widget::create<DivisionsPerBeatChoice>(pos);
	divisionsPerBeatChoice->widget = this;
	addChild(divisionsPerBeatChoice);
	this->divisionsPerBeatChoice = divisionsPerBeatChoice;
	pos = divisionsPerBeatChoice->box.getBottomLeft();

	this->divisionsPerBeatSeparator = Widget::create<LedDisplaySeparator>(pos);
	addChild(this->divisionsPerBeatSeparator);

	SequenceRunningChoice *sequenceRunningChoice = Widget::create<SequenceRunningChoice>(pos);
	sequenceRunningChoice->widget = this;
	addChild(sequenceRunningChoice);
	this->sequenceRunningChoice = sequenceRunningChoice;
	pos = sequenceRunningChoice->box.getBottomLeft();

	box.size = Vec(86.863, pos.y);

	this->patternChoice->box.size.x = box.size.x;
	this->patternSeparator->box.size.x = box.size.x;
	this->measuresChoice->box.size.x = box.size.x;
	this->measuresSeparator->box.size.x = box.size.x;
	this->beatsPerMeasureChoice->box.size.x = box.size.x / 2;
	this->beatsPerMeasureSeparator->box.pos.x = box.size.x / 2;
	this->divisionsPerBeatChoice->box.pos.x = box.size.x / 2;
	this->divisionsPerBeatChoice->box.size.x = box.size.x / 2;
	this->divisionsPerBeatSeparator->box.size.x = box.size.x;
	this->sequenceRunningChoice->box.size.x = box.size.x;
}

} // namespace rack_plugin_rcm
