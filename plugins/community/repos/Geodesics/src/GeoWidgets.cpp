//***********************************************************************************************
//Geodesics: A modular collection for VCV Rack by Pierre Collard and Marc Boulé
//
//Based on code from Valley Rack Free by Dale Johnson
//See ./LICENSE.txt for all licenses
//***********************************************************************************************


#include "GeoWidgets.hpp"

namespace rack_plugin_Geodesics {

// Dynamic SVGPanel

void PanelBorderWidget::draw(NVGcontext *vg) {  // carbon copy from SVGPanel.cpp
    NVGcolor borderColor = nvgRGBAf(0.5, 0.5, 0.5, 0.5);
    nvgBeginPath(vg);
    nvgRect(vg, 0.5, 0.5, box.size.x - 1.0, box.size.y - 1.0);// full rect of module (including expansion area if a module has one)
    nvgStrokeColor(vg, borderColor);
    nvgStrokeWidth(vg, 1.0);
    nvgStroke(vg);
	if (expWidth != nullptr && *expWidth != nullptr) {// add expansion division when pannel uses expansion area
		int expW = **expWidth;
		nvgBeginPath(vg);
		nvgMoveTo(vg, box.size.x - expW, 1);
		nvgLineTo(vg, box.size.x - expW, box.size.y - 1.0);
		nvgStrokeWidth(vg, 2.0);
		nvgStroke(vg);
	}
}

DynamicSVGPanel::DynamicSVGPanel() {
    mode = nullptr;
    oldMode = -1;
	expWidth = nullptr;
    visiblePanel = new SVGWidget();
    addChild(visiblePanel);
    border = new PanelBorderWidget();
	border->expWidth = &expWidth;
    addChild(border);
}

void DynamicSVGPanel::addPanel(std::shared_ptr<SVG> svg) {
    panels.push_back(svg);
    if(!visiblePanel->svg) {
        visiblePanel->setSVG(svg);
        box.size = visiblePanel->box.size.div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE);
        border->box.size = box.size;
    }
}

void DynamicSVGPanel::step() { // all code except middle if() from SVGPanel::step() in SVGPanel.cpp
    if (isNear(rack::global_ui->window.gPixelRatio, 1.0)) {
		// Small details draw poorly at low DPI, so oversample when drawing to the framebuffer
        oversample = 2.f;
    }
    if(mode != nullptr && *mode != oldMode) {
        if ((unsigned)(*mode) < panels.size()) {
			visiblePanel->setSVG(panels[*mode]);
			dirty = true;
		}
		oldMode = *mode;
   }
	FramebufferWidget::step();
}



// Dynamic SVGPort

DynamicSVGPort::DynamicSVGPort() {
    mode = nullptr;
    oldMode = -1;
	//SVGPort constructor automatically called
}

void DynamicSVGPort::addFrame(std::shared_ptr<SVG> svg) {
    frames.push_back(svg);
    if(!background->svg)
        SVGPort::setSVG(svg);
}

void DynamicSVGPort::step() {
    if (isNear(rack::global_ui->window.gPixelRatio, 1.0)) {
		// Small details draw poorly at low DPI, so oversample when drawing to the framebuffer
        oversample = 2.f;
    }
    if(mode != nullptr && *mode != oldMode) {
		if ((unsigned)(*mode) < frames.size()) {
			background->setSVG(frames[*mode]);
			dirty = true;
		}
        oldMode = *mode;
    }
	Port::step();
}



// Dynamic SVGSwitch

DynamicSVGSwitch::DynamicSVGSwitch() {
    mode = nullptr;
    oldMode = -1;
	//SVGSwitch constructor automatically called
}

void DynamicSVGSwitch::addFrameAll(std::shared_ptr<SVG> svg) {
    framesAll.push_back(svg);
	if (framesAll.size() == 2) {
		addFrame(framesAll[0]);
		addFrame(framesAll[1]);
	}
}

void DynamicSVGSwitch::step() {
    if (isNear(rack::global_ui->window.gPixelRatio, 1.0)) {
		// Small details draw poorly at low DPI, so oversample when drawing to the framebuffer
        oversample = 2.f;
    }
    if(mode != nullptr && *mode != oldMode) {
		if ((unsigned)(*mode) * 2 + 1 < framesAll.size()) {
			if ((*mode) == 0) {
				frames[0]=framesAll[0];
				frames[1]=framesAll[1];
			}
			else {
				frames[0]=framesAll[2];
				frames[1]=framesAll[3];
			}
			onChange(*(new EventChange()));// required because of the way SVGSwitch changes images, we only change the frames above.
			//dirty = true;// dirty is not sufficient when changing via frames assignments above (i.e. onChange() is required)
		}
        oldMode = *mode;
    }
}



// Dynamic SVGKnob

DynamicSVGKnob::DynamicSVGKnob() {
	//SVGKnob constructor automatically called first 
    mode = nullptr;
    oldMode = -1;
	effect = nullptr;
	orientationAngle = 0.0f;
}

void DynamicSVGKnob::addFrameAll(std::shared_ptr<SVG> svg) {
    framesAll.push_back(svg);
	if (framesAll.size() == 1) {
		setSVG(svg);
	}
}

void DynamicSVGKnob::addEffect(std::shared_ptr<SVG> svg) {
	effect = new SVGWidget();    
	effect->setSVG(svg);
	addChild(effect);
}

void DynamicSVGKnob::step() {
    if (isNear(rack::global_ui->window.gPixelRatio, 1.0)) {
		// Small details draw poorly at low DPI, so oversample when drawing to the framebuffer
        oversample = 2.f;
    }
    if(mode != nullptr && *mode != oldMode) {
        if ((unsigned)(*mode) < framesAll.size()) {
			if ((*mode) == 0) {
				setSVG(framesAll[0]);
				if (effect != nullptr)
					effect->visible = false;
			}
			else {
				setSVG(framesAll[1]);
				if (effect != nullptr)
					effect->visible = true;
			}
			dirty = true;
		}
        oldMode = *mode;
    }
	
	//SVGKnob::step();
	// do code here because handle orientationAngle
	
	// Re-transform TransformWidget if dirty
	if (dirty) {
		float angle;
		if (isfinite(minValue) && isfinite(maxValue)) {
			angle = rescale(value, minValue, maxValue, minAngle, maxAngle);
		}
		else {
			angle = rescale(value, -1.0, 1.0, minAngle, maxAngle);
			angle = fmodf(angle, 2*M_PI);
		}
		angle += orientationAngle;
		tw->identity();
		// Rotate SVG
		Vec center = sw->box.getCenter();
		tw->translate(center);
		tw->rotate(angle);
		tw->translate(center.neg());
	}
	FramebufferWidget::step();	
	
}

} // namespace rack_plugin_Geodesics
