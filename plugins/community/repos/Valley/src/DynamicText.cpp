#include "ValleyWidgets.hpp"

 DynamicText::DynamicText() {
    font = Font::load(assetPlugin(plugin, "res/din1451alt.ttf"));
    size = 16;
    visibility = nullptr;
    colorHandle = nullptr;
    viewMode = ACTIVE_HIGH_VIEW;
}

void DynamicText::draw(NVGcontext* vg) {
    nvgFontSize(vg, size);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 0.f);
    Vec textPos = Vec(0.f, 0.f);
    if(colorHandle != nullptr) {
        switch((ColorMode)*colorHandle) {
            case COLOR_MODE_WHITE: textColor = nvgRGB(0xFF,0xFF,0xFF); break;
            case COLOR_MODE_BLACK: textColor = nvgRGB(0x14,0x14, 0x14); break;
            default: textColor = nvgRGB(0xFF,0xFF,0xFF);
        }
    }
    else {
        textColor = nvgRGB(0xFF,0xFF,0xFF);
    }

    nvgFillColor(vg, textColor);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
    nvgText(vg, textPos.x, textPos.y, text->c_str(), NULL);
}

void DynamicText::step() {
    if(visibility != nullptr) {
        if(*visibility) {
            visible = true;
        }
        else {
            visible = false;
        }
        if(viewMode == ACTIVE_LOW_VIEW) {
            visible = !visible;
        }
    }
}

DynamicText* createDynamicText(const Vec& pos, int size, std::string text,
                               int* visibilityHandle, DynamicViewMode viewMode) {
    DynamicText* dynText = new DynamicText();
    dynText->size = size;
    dynText->text = std::make_shared<std::string>(text);
    dynText->box.pos = pos;
    dynText->box.size = Vec(82,14);
    dynText->visibility = visibilityHandle;
    dynText->viewMode = viewMode;
    return dynText;
}

DynamicText* createDynamicText(const Vec& pos, int size, std::string text,
                               int* visibilityHandle, int* colorHandle, DynamicViewMode viewMode) {
    DynamicText* dynText = new DynamicText();
    dynText->size = size;
    dynText->text = std::make_shared<std::string>(text);
    dynText->box.pos = pos;
    dynText->box.size = Vec(82,14);
    dynText->visibility = visibilityHandle;
    dynText->viewMode = viewMode;
    dynText->colorHandle = colorHandle;
    return dynText;
}

DynamicText* createDynamicText(const Vec& pos, int size, std::shared_ptr<std::string> text,
                               int* visibilityHandle, DynamicViewMode viewMode) {
    DynamicText* dynText = new DynamicText();
    dynText->size = size;
    dynText->text = text;
    dynText->box.pos = pos;
    dynText->box.size = Vec(82,14);
    dynText->visibility = visibilityHandle;
    dynText->viewMode = viewMode;
    return dynText;
}

DynamicText* createDynamicText(const Vec& pos, int size, std::shared_ptr<std::string> text,
                               int* visibilityHandle, int* colorHandle, DynamicViewMode viewMode) {
    DynamicText* dynText = new DynamicText();
    dynText->size = size;
    dynText->text = text;
    dynText->box.pos = pos;
    dynText->box.size = Vec(82,14);
    dynText->visibility = visibilityHandle;
    dynText->viewMode = viewMode;
    dynText->colorHandle = colorHandle;
    return dynText;
}

DynamicFrameText::DynamicFrameText() {
    itemHandle = nullptr;
}

void DynamicFrameText::addItem(const std::string& item) {
    textItem.push_back(item);
}

void DynamicFrameText::draw(NVGcontext* vg) {
    int item = -1;
    if(itemHandle != nullptr) {
        item = *itemHandle;
    }
    if((int)textItem.size() && item >= 0 && item < (int)textItem.size()) {
        nvgFontSize(vg, size);
        nvgFontFaceId(vg, font->handle);
        nvgTextLetterSpacing(vg, 0.f);
        Vec textPos = Vec(0.f, 0.f);

        if(colorHandle != nullptr) {
            switch((ColorMode)*colorHandle) {
                case COLOR_MODE_WHITE: textColor = nvgRGB(0xFF,0xFF,0xFF); break;
                case COLOR_MODE_BLACK: textColor = nvgRGB(0x14,0x14, 0x14); break;
                default: textColor = nvgRGB(0xFF,0xFF,0xFF);
            }
        }
        else {
            textColor = nvgRGB(0xFF,0xFF,0xFF);
        }

        nvgFillColor(vg, textColor);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
        nvgText(vg, textPos.x, textPos.y, textItem[item].c_str(), NULL);
    }
}
