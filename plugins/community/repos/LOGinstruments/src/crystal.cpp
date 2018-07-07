#include <math.h>
// #include <sys/time.h>
#include <dsp/digital.hpp>
#include "LOGinstruments.hpp"
#include "LOGgui.hpp"

namespace rack_plugin_LOGinstruments {

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define SMALL		0.999 // for LkInt and DCB
#define CIRCB_MAX	192000
#define PAMP_SAW	1.0
#define PAMP_TRI	0.2
#define PAMP_SQU	1.0
#define PAMP_BIP	1.0
#define PAMP_PUL	1.0
#define PAMP_SINE	100.0
#define MAX_OSC		32.0
#define PER_VOICE_ATT 0.6
#define RST_HYSTE_SMP 1024 // must be >= MAX_OSC

Vec gemcoor[(int)MAX_OSC+1] = {
		Vec(0.0, 0.3),
		Vec(0.5, 1.0),
		Vec(0.1, 0.3),
		Vec(0.5, 1.0),
		Vec(0.22, 0.3),
		Vec(0.5, 1.0),
		Vec(0.33, 0.3),
		Vec(0.5, 1.0),
		Vec(0.44, 0.3),
		Vec(0.5, 1.0),
		Vec(0.56, 0.3),
		Vec(0.5, 1.0),
		Vec(0.67, 0.3),
		Vec(0.5, 1.0),
		Vec(0.78, 0.3),
		Vec(0.5, 1.0),
		Vec(0.9, 0.3),
		Vec(0.5, 1.0),
		Vec(1.0, 0.3),
		//Vec(0.5, 1.0),
		//20
		Vec(0.2, 0.0),
		Vec(0.8, 0.0),
		Vec(0.0, 0.3),
		Vec(1.0, 0.3),
		Vec(0.0, 0.3),
		Vec(0.2, 0.0),
		Vec(0.2, 0.3),
		Vec(0.3, 0.0),
		Vec(0.8, 0.3),
		Vec(0.7, 0.0),
		Vec(1.0, 0.3),
		Vec(0.8, 0.0),
		Vec(0.3, 0.0),
		Vec(0.5, 0.3),
};

float dturandTbl[] = { 0.0,
		0.2, -0.2, 0.5, -0.5, 0.1, -0.1, 0.6, -0.6,
		0.7, -0.7, 0.156, -0.156, 0.199, -0.199, 0.686, -0.686,
		0.808, -0.808, 0.432, -0.432, 0.111, -0.111, 0.343, -0.343,
		0.252, -0.252, 0.577, -0.577, 0.159, -0.159, 0.666 };


/*
long int GetUsTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_usec;
}
*/

typedef enum {
		OSC_TYPE_SAW,
		OSC_TYPE_TRI,
		OSC_TYPE_SQU,
		OSC_TYPE_PUL,
		OSC_TYPE_BIP,
		OSC_TYPE_SINE,
		NUM_OSC_TYPES,
	} OSC_TYPE;

/* CIRCB */
template<typename T>
class circB {
public:
	circB(int iL, float pAmp) {
		int i;
		if (iL < 1 || iL > CIRCB_MAX) {
			iL = 100;
		}
		buffer = new T[CIRCB_MAX];
		for (i = 0; i < iL; i++) {
			buffer[i] = static_cast<T>(0.0);
		}
		bEnd = &buffer[iL];
		init(pAmp);
	}

	~circB() {
		delete[] buffer;
	}

	T process(T in) {
		incrpt(rp);
		T ret = *rp;
		incrpt(wp);
		*wp = in;

		return ret;
	}

	inline T read() {
		return *rp;
	}

	inline void inject(T in) {
		*wp += in;
	}

	void resize(int iL) {

		if (iL == bEnd - buffer) return; // same size
		if (iL < 1 || iL > CIRCB_MAX) return;

		while (bEnd < &buffer[iL]) {
			*(bEnd+1) = *bEnd; // replicate content
			bEnd++;
		}

		bEnd = &buffer[iL];
		if (rp > bEnd || wp > bEnd) init(0.0);

	}

	void reset() {
		T * pt = buffer;
		while(pt < bEnd)
			*(pt++) = static_cast<T>(0.0);
	}

	void init(float pAmp) {
		rp = buffer;
		wp = buffer;
		*rp += 1.0 * pAmp;
		decrpt(wp);
	}

	T* getBuffer() { return buffer; }

	int getSize() { return bEnd - buffer; }
	int getSizeBytes() { return sizeof(T) * (bEnd - buffer); }

private:
	T* buffer;
	T* bEnd;
	T* wp;
	T* rp;
	void incrpt(T*& pt) {
		pt++;
		if (pt > bEnd) pt = buffer;
	}
	void decrpt(T*& pt) {
		pt--;
		if (pt < buffer) pt = bEnd;
	}

};

/* SmCook Sine CLASS */
template<typename T>
class SCSine {
private:
	T k, kpl1, kmin1;
	T yz1, yz2;

public:

	SCSine(T f) {
		setf0(f);
		reset();
	}

	T process () {
		T y = kpl1 * yz1 - k * yz2;
		yz1 = - (kmin1 * yz2 + k * yz1);
		yz2 = y;
		return y;
	}

	void reset() {
		yz1 = PAMP_SINE;
		yz2 = 0.0;
	}

	void inject(float inj) {
		yz1 += inj;
		yz2 = 0.0;
	}

	void setf0(T f) {
		k = -cos(M_PI*f*engineGetSampleTime());
		kpl1 = k + 1; kmin1 = 1 - k;
	}

};

/* BQUAD CLASS */
template<typename T>
class BQuad {
private:
	T b0, b1, b2, a1, a2;
	T z1, z2;

public:

	BQuad(T D) {
		setD(D);
		z1 = z2 = 0.0;
	}

	BQuad(T b0, T b1, T b2, T a1, T a2) : b0(b0), b1(b1), b2(b2), a1(a1), a2(a2) {
		z1 = z2 = 0.0;
	}

	T process (T x) {
		T y = x * b0 + z1;
		z1 = x * b1 - y * a1 + z2;
		z1 = x * b2 - y * a2;
		return y;
	}

	void setD(T D) {
		a1 = b1 = -(D-2.0) / (D+1.0);
		a2 = b0 = (D-1.0)*(D-2.0) / ((D+1.0)*(D+2.0));
		b2 = 1.0;
	}

};

template <typename T>
class DCB {
	T dxn, dyn, dxn1, dyn1;

public:

	DCB() {
		reset();
	}

	void reset() {
		dxn = dyn = dxn1 = dyn1 = 0.0;
	}

	T process(T xn) {
		dxn = xn;
		T dyn = dxn - dxn1 + SMALL * dyn1;
		dxn1 = dxn;
		dyn1 = xn = dyn;
		return dyn;
	}
};


template <typename T>
class LkInt {
	T yn, ixn1;

public:

	LkInt() {
		reset();
	}

	void reset() {
		yn = ixn1 = 0.0;
	}

	T process(T xn) {
		yn = xn + SMALL * ixn1;
		ixn1 = yn;
		return yn;
	}
};


class aliasFreeOsc {

private:
	int oscID;
	float fs;
	float f0, basef0, basePK; // f0 has detuning applied
	float dtubnded, dturand, origBound;
	float L;
	int iL;
	float frac;
	circB<float> * dly;
	int ect, timeToCheck;
	float nrg, avgNrg;
	float pAmp;
	float makeupG;
	float nrgThr;
	float polarity;
	OSC_TYPE type;
	int integrate;
	bool dcblock;
	float kscale;
	float dxn, dxn1, ixn, ixn1, iixn1, dyn1;
	BQuad<float> * APThiran;
	float xgain, xgainStep;
	long int xfade;
	DCB<float> *dcb;
	LkInt<float> *lki[2];
	SCSine<float> *scs;

public:

	aliasFreeOsc(float knob, OSC_TYPE type, int oscID) {
		this->oscID = oscID;
		dly = nullptr;
		fs = engineGetSampleRate();
		pAmp = 10.0;
		iL = 100;
		frac = 1.5;
		APThiran = new BQuad<float>(frac);
		dly = new circB<float>(iL, pAmp);
		dcb = new DCB<float>();
		lki[0] = new LkInt<float>();
		lki[1] = new LkInt<float>();
		scs = new SCSine<float>(440.0);
		dturand = dturandTbl[oscID] / 12.0;
		dtubnded = dturand; // 0 <= dtu < 1 semitone
		setFreq(knob);
		setType(type);
		nrg = 0.0;
		nrgThr = 0.5 * pAmp; // energy threshold for reinit is half the initial pulse energy
		dxn = dxn1 = ixn = ixn1 = dyn1 = 0.0;
	}

	~aliasFreeOsc() {
		delete dly;
	}

	void setKscale(float ks) {
		kscale = ks;
	}

	void setD(float d) {
		APThiran->setD(d);
	}

	float getKscale() { return kscale; }

	/* expects a deviation in semitones from C4 */
	void setFreq(float pitchKnob) {
		basePK = pitchKnob;
		basef0 = 261.626 * powf(2.0, pitchKnob / 12.0);
		f0 = powf(2.0, dtubnded) * basef0;
		float Lprev = L;
		L = fs / f0;
		if (L == Lprev) return;

		int iLprev = iL;
		iL = (int)floor(L) - 1; // make it always >1 (best: 1.5-2.5)
		iL = MAX(2, iL); // never go below this
		frac = L - (float)iL;
		if (iL != iLprev) {
			if (abs(iL-iLprev) > 1) reset();
			dbgPrint(("%d: new: %d, old: %d\n", oscID, (int)floor(L), iL));
		}
		dly->resize(iL);
		APThiran->setD(frac);
		scs->setf0(f0);
		ect = timeToCheck = 10 * iL; // check energy every 10 periods
	}

	float getFreq() { return basef0; }
	float getPitchKn() { return basePK; }

	void setDtuBound(float b) {
		//if (b > 1.0 || b < 0.0) return;
		origBound = b;
		dtubnded = dturand * b;
		setFreq(basePK);
	}

	float getDtuBound() { return origBound; }

	void setType(OSC_TYPE t) {
		type = t;
		switch(type) {
		case OSC_TYPE_SINE:
			makeupG = 1.0;
			break;
		case OSC_TYPE_SAW:
			polarity = +1.0;
			integrate = 1;
			dcblock = true;
			pAmp = PAMP_SAW;
			makeupG = 1.0;
			break;
		case OSC_TYPE_SQU:
			polarity = -1.0;
			integrate = 1;
			dcblock = false;
			pAmp = PAMP_SQU;
			makeupG = 1.0;
			break;
		case OSC_TYPE_TRI:
			polarity = -1.0;
			integrate = 2;
			dcblock = false;
			pAmp = PAMP_TRI;
			makeupG = 0.25;
			break;
		case OSC_TYPE_PUL:
			polarity = +1.0;
			integrate = 0;
			dcblock = false;
			pAmp = PAMP_PUL;
			makeupG = 2.0;
			break;
		case OSC_TYPE_BIP:
			polarity = -1.0;
			integrate = 0;
			dcblock = false;
			pAmp = PAMP_BIP;
			makeupG = 2.0;
			break;
		default:
			break;
		}
#ifndef CRAZY_BEHAVIOR
		nrgThr = 0.5 * pAmp;
#endif
		dxn = dxn1 = ixn = ixn1 = dyn1 = 0.0;
		reset();

	}

	void reset() {
		dly->reset();
		dly->inject(pAmp);
		dcb->reset();
		lki[0]->reset();
		lki[1]->reset();
		scs->reset();
	}

	OSC_TYPE getType() {
		return type;
	}

	float process() {
		float xn;
		if (type == OSC_TYPE_SINE) {
			xn = scs->process();
			xn = 0.5 * ( (1-kscale)*xn + (kscale)*(xn*basef0/261.626) ); // keyboard scaling
		} else {
			xn = makeupG * dly->process(polarity * APThiran->process(dly->read()));

			if (integrate) {
				if (dcblock) {
					xn = dcb->process(xn);
				}
				for (int i = 0; i < integrate; i++) {
					xn = lki[i]->process(xn);
					xn = 0.5 * ( (1-kscale)*xn + (kscale)*(xn*basef0/261.626) ); // keyboard scaling
				}
			}
		}

		nrg += fabs(xn);
		ect--;
		if (ect == 0) {
			checkEnergy();
			ect = timeToCheck;
			nrg = 0.0;
		}
		return xn;
	}

	float getNrg() { return avgNrg; }

private:
	void checkEnergy() {
		avgNrg = nrg / timeToCheck;
		if (avgNrg < nrgThr) {
			reinit();
		}
	}

	void reinit() {
		if (type != OSC_TYPE_SINE) {
			dly->inject(pAmp * 0.5);
		} else {
			scs->inject(pAmp);
		}
	}

};


/* MODULE */
struct CrystalModule: Module {
	enum ParamIds {
		PARAM_F0,
		PARAM_VOICES,
		PARAM_OSC_TYPE,
		PARAM_DETUNE,
		PARAM_VOLUME,
		PARAM_PITCH_IN_AMT,
		PARAM_KEYB_SCALE,
		PARAM_MANUAL_RST,
		PARAM_ILIKETHIS,
		NUM_PARAMS
	};

	enum InputIds {
		IN_CV1,
		IN_CV2,
		IN_CV3,
		NUM_INPUTS
	};

	enum OutputIds {
		OUT_MAIN,
		OUT_LOW_DET,
		OUT_HIGH_DET,
		NUM_OUTPUTS
	};

	aliasFreeOsc * voices[32];
	int allocd_voices;
	DCB<float>* dcblockL, *dcblockR;
	SchmittTrigger M_RST, CV_RST;
	int progrReset;

	int algo;
	float pitch;
	int rst;
	int vcycl;

	CrystalModule() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, MAX_OSC) {
		dcblockL = new DCB<float>();
		dcblockR = new DCB<float>();
		algo = OSC_TYPE_SQU;
		pitch = 440.0;
		allocd_voices = 0;
		progrReset = 0;
		rst = 0;
		vcycl = 0;
	};
	void step() override;

};

void CrystalModule::step() {

    if (params[PARAM_VOICES].value < 1.0) params[PARAM_VOICES].value = 1.0;

	if ( allocd_voices < floor(params[PARAM_VOICES].value) ) {
		voices[allocd_voices] = new
         aliasFreeOsc(params[PARAM_F0].value, (OSC_TYPE)int(floor(params[PARAM_OSC_TYPE].value)), allocd_voices); // allocate one at a time
		allocd_voices++;
	} else if ( allocd_voices > floor(params[PARAM_VOICES].value) ) {
#if DEBUG_LIGHTS
		lights[allocd_voices-1].value = 0.0;
#endif
		delete voices[allocd_voices-1];
		allocd_voices--;
	}

    vcycl = (vcycl+1) % allocd_voices;

	float cv1 = inputs[IN_CV1].value;
	float cv2 = inputs[IN_CV2].value;
	float cv3 = inputs[IN_CV3].value;

	algo = (int)round(cv2) % (int)NUM_OSC_TYPES;

	pitch = clamp(12.0f*(cv3), -32.0f, 54.0f) ;

	if (params[PARAM_ILIKETHIS].value != 1.0) {
		if (M_RST.process(params[PARAM_MANUAL_RST].value) || CV_RST.process(cv1)) {
			rst = RST_HYSTE_SMP;
			progrReset = allocd_voices-1;
		} else if (rst == 0) {
			rst = RST_HYSTE_SMP * ((cv1 > cv2) > cv3);
			if (rst) progrReset = allocd_voices-1;
		} else {
			rst--;
		}
	}
#if DEBUG_LIGHTS
	lights[MAX_OSC-1].value = rst;
#endif
	if (rst) {
		if (progrReset >= 0) {
			voices[progrReset]->setFreq(pitch);
			voices[progrReset]->setType((OSC_TYPE)algo);
			voices[progrReset]->reset();
			progrReset--;
		}
	}


	if ( fabs(voices[0]->getDtuBound() - params[PARAM_DETUNE].value) > 0.001 ) {
		for (int i = 0; i < allocd_voices; i++) {
			voices[i]->setDtuBound(params[PARAM_DETUNE].value);
		}
	}


	if (outputs[OUT_MAIN].active || outputs[OUT_LOW_DET].active || outputs[OUT_HIGH_DET].active) {
		float resultL = 0.0, resultH = 0.0, result = 0.0;
		for (int i = 1; i < allocd_voices; i+=2) {
			resultL += PER_VOICE_ATT * voices[i]->process();
#if DEBUG_LIGHTS
			lights[i].value = (voices[i]->getNrg(), 0.0f, 10.0f);
#endif
		}
		for (int i = 2; i < allocd_voices; i+=2) {
			resultH += PER_VOICE_ATT * voices[i]->process();
#if DEBUG_LIGHTS
			lights[i].value = clamp(voices[i]->getNrg(), 0.0f, 10.0f);
#endif
		}
		float vol = powf(10, params[PARAM_VOLUME].value/10.0);
		resultL += dcblockL->process(vol * PER_VOICE_ATT * voices[0]->process());
		resultH += dcblockR->process(vol * PER_VOICE_ATT * voices[0]->process());
		result = resultL+resultH;
		if (std::isnan(result) || result > 15.0) {
			for (int i = 0; i < allocd_voices; i++) {
				voices[i]->reset();
				dcblockL->reset();
				dcblockR->reset();
			}
		}
		outputs[OUT_LOW_DET].value = clamp(2.0f* vol * resultL, -15.0f, 15.0f);
		outputs[OUT_HIGH_DET].value = clamp(2.0f* vol * resultH, -15.0f, 15.0f);
		outputs[OUT_MAIN].value = clamp(vol * result, -15.0f, 15.0f);
	}

}


struct CrystalDisplay : TransparentWidget {
	CrystalModule *module;
	int frame = 0;
	int randLight[(int)MAX_OSC+1];
	NVGpaint pnt[(int)MAX_OSC+1];

	CrystalDisplay(CrystalModule * m) {
		module = m;
		memset(randLight, 0, sizeof(int)*(int)MAX_OSC);
	}

	void drawCrystal(NVGcontext *vg) {

		Vec p;
		nvgSave(vg);
		Rect b = Rect(Vec(0, 15), box.size.minus(Vec(0, 15*2)));

#ifndef CA
		for (int i = 0; i < module->allocd_voices; i++) {

			nvgBeginPath(vg);

			p = gemcoor[i].mult(b.size);
			nvgMoveTo(vg, p.x, p.y);

			randLight[i+1] +=  (int)(128 *(randomUniform()-0.5));
			unsigned char nrg = (unsigned char) clamp(module->voices[i]->getNrg()*256.0f, 20.0f, 256.0f);
			pnt[i+1] = nvgLinearGradient(vg, b.pos.x, b.pos.y, b.pos.x+b.size.x, b.pos.y+b.size.y,
					nvgRGBA(0x40, 0x75, 0xDB, nrg), nvgRGBA(0x8C, 0xAC, 0xEA, nrg+randLight[i+1]));
			nvgStrokePaint(vg, pnt[i+1]);
			p = gemcoor[i+1].mult(b.size);
			nvgLineTo(vg, p.x, p.y);
			nvgLineCap(vg, NVG_ROUND);
			nvgMiterLimit(vg, 2.0);
			nvgStrokeWidth(vg, 1.3);
			nvgStroke(vg);

		}
#else
		for (int i = 0; i < module->allocd_voices; i++) {
			randLight[i+1] +=  (int)(256 *(randomUniform()-0.5));
			NVGpaint pnt = nvgLinearGradient(vg, b.pos.x, b.pos.y, b.pos.x+b.size.x, b.pos.y+b.size.y,
					nvgRGBA(0x20, 0x60, 0xD0, randLight[i]), nvgRGBA(0x20, 0x60, 0xD0, randLight[i+1]));
			nvgStrokePaint(vg, pnt);
			p = gemcoor[i+1].mult(b.size);
			nvgLineTo(vg, p.x, p.y);
		}
#endif
		nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
		nvgResetScissor(vg);
		nvgRestore(vg);

	}



	void draw(NVGcontext *vg) override {

		drawCrystal(vg);

	}
};


struct CrystalWidget : ModuleWidget {
	CrystalWidget(CrystalModule *module);
};

CrystalWidget::CrystalWidget(CrystalModule *module) : ModuleWidget(module) {
	box.size = Vec(15*13, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/crystal-nofonts.svg")));
		addChild(panel);
	}

	addParam(ParamWidget::create<RoundBlackKnob>(Vec(140, 210), module, CrystalModule::PARAM_VOICES, 1.0, MAX_OSC, 1.0));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(20, 210), module, CrystalModule::PARAM_DETUNE, 0.0, 1.0, 0.1));
	addParam(ParamWidget::create<RoundBlackKnob>(Vec(20, 280), module, CrystalModule::PARAM_VOLUME, -30, 6.0, 0.0));

	addInput(Port::create<PJ301MPort>(Vec(0, 80), Port::INPUT, module, CrystalModule::IN_CV1));
	addInput(Port::create<PJ301MPort>(Vec(box.size.x-24, 80), Port::INPUT, module, CrystalModule::IN_CV2));
	addInput(Port::create<PJ301MPort>(Vec(box.size.x/2-12, 160), Port::INPUT, module, CrystalModule::IN_CV3));

	addParam(ParamWidget::create<CKD6>(Vec(85, 284), module, CrystalModule::PARAM_MANUAL_RST, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<CKSS>(Vec(150, 285), module, CrystalModule::PARAM_ILIKETHIS, 0.0, 1.0, 0.0));

	addOutput(Port::create<PJ301MPort>(Vec(80, 345), Port::OUTPUT, module, CrystalModule::OUT_MAIN));
	addOutput(Port::create<PJ301MPort>(Vec(10, 325), Port::OUTPUT, module, CrystalModule::OUT_LOW_DET));
	addOutput(Port::create<PJ301MPort>(Vec(160, 325), Port::OUTPUT, module, CrystalModule::OUT_HIGH_DET));


	{
		CrystalDisplay *display = new CrystalDisplay(module);
		display->box.pos = Vec(10, 60);
		display->box.size = Vec(box.size.x-20, 140);
		addChild(display);
	}

#ifdef DEBUG_LIGHTS
	int i;
	for (i = 0; i < (int)MAX_OSC-1; i++) {
		addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(5 + (i*5), 360), module, i));
	}
	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(50, 30), module, i));
#endif
}

} // namespace rack_plugin_LOGinstruments

using namespace rack_plugin_LOGinstruments;

RACK_PLUGIN_MODEL_INIT(LOGinstruments, Crystal) {
   Model *modelCrystal = Model::create<CrystalModule, CrystalWidget>("LOGinstruments", "Crystal", "Crystal", OSCILLATOR_TAG);
   return modelCrystal;
}
