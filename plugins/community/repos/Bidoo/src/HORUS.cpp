// #include "Bidoo.hpp"
// #include "BidooComponents.hpp"
// #include "dsp/samplerate.hpp"
// #include "dsp/decimator.hpp"
// #include "dsp/filter.hpp"
// #include "dsp/ringbuffer.hpp"
// #include "dsp/digital.hpp"
// #include <math.h>
//
// #define BOTTOM_FREQ 100
// #define	BSZ 65536
// #define ROUND(n)		((int)((double)(n)+0.5))
// #define PIN(n,min,max) ((n) > (max) ? max : ((n) < (min) ? (min) : (n)))
// #define	MODF(n,i,f) ((i) = (int)(n), (f) = (n) - (double)(i))
//
// enum
// {
// 	kMixMono,
// 	kMixWetOnly,
// 	kMixWetLeft,
// 	kMixWetRight,
// 	kMixWetLeft75,
// 	kMixWetRight75,
// 	kMixStereo
// };
//
// #define	NUM_MIX_MODES	7
// #define	NUM_DELAYS	11
//
// using namespace std;
//
// struct HORUS : Module {
// 	enum ParamIds {
// 		RATE_PARAM,
//     WIDTH_PARAM,
// 		FEEDBACK_PARAM,
// 		DELAY_PARAM,
// 		MIXMODE_PARAM,
// 		NUM_PARAMS
// 	};
// 	enum InputIds {
// 		L_INPUT,
//     R_INPUT,
// 		NUM_INPUTS
// 	};
// 	enum OutputIds {
// 		L_OUTPUT,
//     R_OUTPUT,
// 		NUM_OUTPUTS
// 	};
// 	enum LightIds {
// 		NUM_LIGHTS
// 	};
//
// 	void setRate (float v);
// 	void setWidth (float v);
// 	void setFeedback (float v);
// 	void setDelay (float v);
// 	void setMixMode (float v);
// 	void setSweep(void);
//
// 	float _paramSweepRate = 0.2f;
// 	float _paramWidth = 0.3f;
// 	float _paramFeedback = 0.0f;
// 	float _paramDelay = 0.2f;
// 	float _paramMixMode = 0.0f;
// 	double _sweepRate = 0.2;
// 	double _feedback = 0.0;
// 	double _feedbackPhase = 1.0;
// 	int _sweepSamples = 0;
// 	int	   _delaySamples = 22;
// 	double _minSweepSamples;
// 	double _maxSweepSamples;
// 	int	  _mixMode = 0;
// 	double *_buf  = new double[BSZ];
// 	int	   _fp = 0;
// 	double _step;
// 	double _sweep = 0.0;
//
//
// 	double _mixLeftWet =  0.5f;
// 	double _mixLeftDry =  0.5f;
// 	double _mixRightWet =  0.5f;
// 	double _mixRightDry =  0.5f;
//
//
// 	HORUS() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
// 	}
//
// 	~HORUS() {
// 		if( _buf )
// 			delete[] _buf;
//   }
//
// 	void step() override;
// };
//
// void HORUS::setSweep()
// {
// 	_step = (double)(_sweepSamples * 2.0 * _sweepRate) / (double)engineGetSampleRate();
// 	if( _step <= 1.0 )
// 	{
// 		printf( "_sweepSamples: %i\n", _sweepSamples );
// 		printf( "_sweepRate: %f\n", _sweepRate );
// 		printf( "engineGetSampleRate: %f\n", engineGetSampleRate() );
// 		printf( "_step out of range: %f\n", _step );
// 	}
//
// 	_minSweepSamples = _delaySamples;
// 	_maxSweepSamples = _delaySamples + _sweepSamples;
//
// 	_sweep = (_minSweepSamples + _maxSweepSamples) / 2;
// }
//
// void HORUS::setRate (float rate)
// {
// 	_paramSweepRate = rate;
// 	_sweepRate = pow(10.0,(double)_paramSweepRate);
// 	_sweepRate  -= 1.0;
// 	_sweepRate  *= 1.1f;
// 	_sweepRate  += 0.1f;
// }
//
// void HORUS::setWidth (float v)
// {
// 	_paramWidth = v;
// 	_sweepSamples = ROUND(v * 0.05 * engineGetSampleRate());
// }
//
// void HORUS::setDelay (float v)
// {
// 	_paramDelay = v;
// 	double delay = pow(10.0, (double)v * 2.0)/1000.0;
// 	_delaySamples = ROUND(delay * engineGetSampleRate());
// }
//
// void HORUS::setFeedback(float v)
// {
// 	_paramFeedback = v;
// 	_feedback = v;
// }
//
// void HORUS::setMixMode (float v)
// {
// 	_paramMixMode = v;
// 	_mixMode = (int)(v * NUM_MIX_MODES);
// 	switch(_mixMode)
// 	{
// 	case kMixMono:
// 	default:
// 		_mixLeftWet = _mixRightWet = 1.0;
// 		_mixLeftDry = _mixRightDry = 1.0f;
// 		_feedbackPhase = 1.0;
// 		break;
// 	case kMixWetOnly:
// 		_mixLeftWet = _mixRightWet = 1.0f;
// 		_mixLeftDry = _mixRightDry = 1.0;
// 		_feedbackPhase = -1.0;
// 		break;
// 	case kMixWetLeft:
// 		_mixLeftWet = 1.0f;
// 		_mixLeftDry = 0.0f;
// 		_mixRightWet = 0.0f;
// 		_mixRightDry = 1.0f;
// 		break;
// 	case kMixWetRight:
// 		_mixLeftWet = 0.0f;
// 		_mixLeftDry = 1.0f;
// 		_mixRightWet = 1.0f;
// 		_mixRightDry = 0.0f;
// 		break;
// 	case kMixStereo:
// 		_mixLeftWet = 1.0f;
// 		_mixLeftDry = 1.0f;
// 		_mixRightWet = -1.0f;
// 		_mixRightDry = 1.0f;
// 		break;
// 	}
// }
//
//
// void HORUS::step() {
// 	setRate(params[RATE_PARAM].value);
// 	setWidth(params[WIDTH_PARAM].value);
// 	setFeedback(params[FEEDBACK_PARAM].value);
// 	setDelay(params[DELAY_PARAM].value);
// 	setMixMode(params[MIXMODE_PARAM].value);
// 	setSweep();
//
// 	float inval = (inputs[L_INPUT].value + inputs[R_INPUT].value) / 2.0f;
// 	_buf[_fp] = inval;
// 	_fp = (_fp + 1) & (BSZ-1);
//
// 	int ep1, ep2;
// 	double w1, w2;
// 	double ep = _fp - _sweep;
// 	MODF(ep, ep1, w2);
// 	ep1 &= (BSZ-1);
// 	ep2 = ep1 + 1;
// 	ep2 &= (BSZ-1);
// 	w1 = 1.0 - w2;
// 	double outval = _buf[ep1] * w1 + _buf[ep2] * w2;
//
// 	outputs[L_OUTPUT].value = (float)PIN(_mixLeftDry * inval + _mixLeftWet * outval, -10, 10);
// 	outputs[R_OUTPUT].value = (float)PIN(_mixRightDry * inval + _mixRightWet * outval,-10, 10);
//
// 	_sweep += _step;
// 	if( _sweep >= _maxSweepSamples || _sweep <= _minSweepSamples )
// 	{
// 		_step = -_step;
// 	}
// }
//
// struct HORUSWidget : ModuleWidget {
// 	HORUSWidget(HORUS *module) : ModuleWidget(module) {
// 		setPanel(SVG::load(assetPlugin(plugin, "res/HORUS.svg")));
//
// 		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
// 		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
// 		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
// 		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
//
// 		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 50), module, HORUS::RATE_PARAM, 0.0f, 150.0f, 0.0f));
// 		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 100), module, HORUS::WIDTH_PARAM, 0.0f, 5.0f, 0.0f));
// 		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 150), module, HORUS::DELAY_PARAM, 0.0f, 1.0f, 0.0f));
// 		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 200), module, HORUS::FEEDBACK_PARAM, 0.0f, 1.0f, 0.0f));
// 		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 250), module, HORUS::MIXMODE_PARAM, 0.0f, 1.0f, 0.0f));
//
// 		addInput(Port::create<TinyPJ301MPort>(Vec(24.0f, 319.0f), Port::INPUT, module, HORUS::L_INPUT));
// 		addInput(Port::create<TinyPJ301MPort>(Vec(24.0f, 339.0f), Port::INPUT, module, HORUS::R_INPUT));
// 		addOutput(Port::create<TinyPJ301MPort>(Vec(78.0f, 319.0f),Port::OUTPUT, module, HORUS::L_OUTPUT));
// 		addOutput(Port::create<TinyPJ301MPort>(Vec(78.0f, 339.0f),Port::OUTPUT, module, HORUS::R_OUTPUT));
// 	}
// };
//
//
//
// Model *modelHORUS = Model::create<HORUS, HORUSWidget>("Bidoo", "horUS", "horUS chorus", CHORUS_TAG);
