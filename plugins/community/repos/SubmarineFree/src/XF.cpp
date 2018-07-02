#include "XF.hpp"

int XF_Correlator::correlate(float a, float b) {
	//Remove old samples
	if (n == frameSize) {
		covariance -= (samples_a[sp] * samples_b[sp]);
		sigma_a -= samples_a[sp];
		sigma_b -= samples_b[sp];
		sigma_a2 -= (samples_a[sp] * samples_a[sp]);
		sigma_b2 -= (samples_b[sp] * samples_b[sp]);
	}
	else {
		n++;
	}

	//Add new samples
	covariance += (a * b);
	sigma_a += samples_a[sp] = a;
	sigma_b += samples_b[sp] = b;
	sigma_a2 += (a * a);
	sigma_b2 += (b * b);
	sp++;
	if (sp > frameSize - 1) {
		sp -= frameSize;
	}
	float stdev_a = powf(sigma_a2 - (sigma_a * sigma_a / n), 0.5f);
	float stdev_b = powf(sigma_b2 - (sigma_b * sigma_b / n), 0.5f);
	if (stdev_a * stdev_b == 0.0f)
		correlation = (stdev_a == stdev_b);
	else
		correlation = covariance / (stdev_a * stdev_b);
	if (schmitt) {
		if (fabs(correlation) < 0.4)
			schmitt = 0;
	}
	else {
		if (fabs(correlation) > 0.6)
			schmitt = 1;
	}
	return schmitt;
}

void XF::crossFade(XF_Controls *controls) {
	float fade = clamp((inputs[controls->cv].active?params[controls->polar].value * 5.0f + inputs[controls->cv].value:params[controls->fader].value)/10.0f, 0.0f, 1.0f);
	int mode = 0;
	if (params[controls->mode].value > 1.5f) {
		mode = controls->correlator->correlate(inputs[controls->a].value, inputs[controls->b].value);
		if (controls->correlator->correlation < -0.1f) {
			lights[controls->light3].value = 0.0f;
			lights[controls->light3 + 1].value = 1.0f;
		}
		else {
			lights[controls->light3].value = 1.0f;
			lights[controls->light3 + 1].value = 0.0f;
		}
	}
	else if (params[controls->mode].value > 0.5f) {
		mode = 0;
		lights[controls->light3].value = 0.0f;
		lights[controls->light3 + 1].value = 0.0f;
	}
	else {
		mode = 1;
		lights[controls->light3].value = 0.0f;
		lights[controls->light3 + 1].value = 0.0f;
	}
	if (mode == 0) {
		outputs[controls->out].value = inputs[controls->a].value * powf(1.0f - fade, 0.5f) + inputs[controls->b].value * powf(fade, 0.5f);
		if (controls->outr)
			outputs[controls->outr].value = inputs[controls->ar].value * powf(1.0f - fade, 0.5f) + inputs[controls->br].value * powf(fade, 0.5f);
		lights[controls->light1].value = 0.0f;
		lights[controls->light2].value = 1.0f;
	}
	else {
		outputs[controls->out].value = inputs[controls->a].value * (1.0f - fade) + inputs[controls->b].value * fade;
		if (controls->outr)
			outputs[controls->outr].value = inputs[controls->ar].value * (1.0f - fade) + inputs[controls->br].value * fade;
		lights[controls->light1].value = 1.0f;
		lights[controls->light2].value = 0.0f;
	}
}

void XF_LightKnob::step() {
	if (link) {
		setEnabled(!module->inputs[cv].active && (module->params[link].value < 0.5f));
	}
	else {
		setEnabled(!module->inputs[cv].active);
	}
	Knob::step();
}
