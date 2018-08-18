#include "gui_components.hpp"
#include "rack.hpp"

namespace rack_plugin_CastleRocktronics {

ShadedKnob::ShadedKnob() {
  shadowWidget = new CircularShadow();
  shadowWidget->blurRadius = shadowBlur;
  addChild(shadowWidget);

  knobTransformWidget = new TransformWidget();
  addChild(knobTransformWidget);
  knobSVGWidget = new SVGWidget();
  knobTransformWidget->addChild(knobSVGWidget);

  highlightWidget = new SVGWidget();
  addChild(highlightWidget);

  markerTransformWidget = new TransformWidget();
  addChild(markerTransformWidget);
  markerSVGWidget = new SVGWidget();
  markerTransformWidget->addChild(markerSVGWidget);
}

void ShadedKnob::setKnob(std::shared_ptr<SVG> svg) {
  knobSVGWidget->svg = svg;

  knobSVGWidget->wrap();
  knobTransformWidget->box.size = knobSVGWidget->box.size;
  box.size = knobSVGWidget->box.size;

  shadowWidget->box.size = knobSVGWidget->box.size;
  shadowWidget->box.pos = (knobSVGWidget->box.pos).plus(shadowOffset);
}

void ShadedKnob::setHighlight(std::shared_ptr<SVG> svg) {
  highlightWidget->svg = svg;
  highlightWidget->wrap();
}

void ShadedKnob::setMarker(std::shared_ptr<SVG> svg) {
  markerSVGWidget->svg = svg;
  markerSVGWidget->wrap();
  markerTransformWidget->box.size = knobSVGWidget->box.size;
}

void ShadedKnob::step() {
  // Re-transform TransformWidget if dirty
  if (dirty) {
    knobTransformWidget->box.size = box.size;
    markerTransformWidget->box.size = box.size;

    knobTransformWidget->identity();
    markerTransformWidget->identity();

    knobTransformWidget->scale(box.size.div(knobSVGWidget->box.size));
    markerTransformWidget->scale(box.size.div(knobSVGWidget->box.size));

    float angle = rescalef(value, minValue, maxValue, minAngle, maxAngle);
    Vec center = knobSVGWidget->box.getCenter();

    knobTransformWidget->translate(center);
    markerTransformWidget->translate(center);
    knobTransformWidget->rotate(angle);
    markerTransformWidget->rotate(angle);
    knobTransformWidget->translate(center.neg());
    markerTransformWidget->translate(center.neg());
  }
  FramebufferWidget::step();
}

void ShadedKnob::onChange(EventChange &e) {
  dirty = true;
  Knob::onChange(e);
}

} // namespace rack_plugin_CastleRocktronics
