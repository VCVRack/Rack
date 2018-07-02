#include "ValleyWidgets.hpp"
#include "global_ui.hpp"

DynamicItem::DynamicItem(unsigned long itemNumber) {
    _itemNumber = itemNumber;
    _choice = nullptr;
}

void DynamicItem::onAction(EventAction &e) {
    if(_choice != nullptr) {
        *_choice = _itemNumber;
    }
}

DynamicChoice::DynamicChoice() {
    _choice = nullptr;
    _visibility = nullptr;
    _viewMode = ACTIVE_HIGH_VIEW;
    _font = Font::load(assetPlugin(plugin, "res/din1451alt.ttf"));
    _text = std::make_shared<std::string>("");
    _textSize = 14;
}

void DynamicChoice::onAction(EventAction &e) {
#ifdef USE_VST2
    Menu* menu = rack::global_ui->ui.gScene->createMenu();
#else
    Menu* menu = gScene->createMenu();
#endif // USE_VST2
    menu->box.pos = getAbsoluteOffset(Vec(0, box.size.y)).round();
	menu->box.size.x = box.size.x;
    for(unsigned long i = 0; i < _items.size(); ++i){
        DynamicItem *item = new DynamicItem(i);
        item->_choice = _choice;
        item->_itemNumber = i;
        item->text = _items[i];
        menu->addChild(item);
    }
}

void DynamicChoice::step() {
    if(_visibility != nullptr) {
        if(*_visibility) {
            visible = true;
        }
        else {
            visible = false;
        }
        if(_viewMode == ACTIVE_LOW_VIEW) {
            visible = !visible;
        }
    }
    else {
        visible = true;
    }
    if(_choice != nullptr) {
        *_text = _items[*_choice];
    }
}

void DynamicChoice::draw(NVGcontext *vg) {
    nvgBeginPath(vg);
    NVGcolor bgColor = nvgRGB(0x1A, 0x1A, 0x1A);
    nvgFillColor(vg, bgColor);
    nvgStrokeWidth(vg, 0.f);
    nvgRect(vg, 0, 0, this->box.size.x, this->box.size.y - 3);
    nvgFill(vg);
    nvgClosePath(vg);

    nvgBeginPath(vg);
    NVGcolor outlineColor = nvgRGB(0xF9, 0xF9, 0xF9);
    nvgStrokeColor(vg, outlineColor);
    nvgStrokeWidth(vg, 1.f);
    nvgMoveTo(vg, 0.f, 0.f);
    nvgLineTo(vg, this->box.size.x, 0.f);
    nvgLineTo(vg, this->box.size.x, this->box.size.y - 3);
    nvgLineTo(vg, 0.f, this->box.size.y -3);
    nvgLineTo(vg, 0.f, 0.f);
    nvgStroke(vg);
    nvgClosePath(vg);

    if(_choice != nullptr) {
        *_text = _items[*_choice];
    }
    nvgFontSize(vg, _textSize);
    nvgFontFaceId(vg, _font->handle);
    nvgTextLetterSpacing(vg, 0.f);
    Vec textPos = Vec(this->box.size.x / 2.f, this->box.size.y / 2.f - 2.f);
    NVGcolor textColor = nvgRGB(0xFF,0xFF,0xFF);
    nvgFillColor(vg, textColor);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgText(vg, textPos.x, textPos.y, _text->c_str(), NULL);
}

DynamicChoice* createDynamicChoice(const Vec& pos,
                                   float width,
                                   const std::vector<std::string>& items,
                                   unsigned long* choiceHandle,
                                   int* visibilityHandle,
                                   DynamicViewMode viewMode) {
    DynamicChoice* choice = new DynamicChoice;
    choice->_choice = choiceHandle;
    choice->box.pos = pos;
    choice->box.size.x = width;
    choice->_items = items;
    choice->_visibility = visibilityHandle;
    choice->_viewMode = viewMode;
    return choice;
}
