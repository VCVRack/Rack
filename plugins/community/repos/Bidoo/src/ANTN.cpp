// #define USE_CURL defined
#include "Bidoo.hpp"
#include "dsp/digital.hpp"
#include "dsp/samplerate.hpp"
#include "BidooComponents.hpp"
#include "dsp/ringbuffer.hpp"
#include "dsp/frame.hpp"
#include <algorithm>
#include <cctype>
#include <atomic>
#include <sstream>
#include <thread>
#ifdef USE_CURL
#include "curl/curl.h"
#endif // USE_CURL
#define MINIMP3_IMPLEMENTATION
#include "dep/minimp3/minimp3.h"


using namespace std;

namespace rack_plugin_Bidoo {

struct threadReadData {
  DoubleRingBuffer<char,262144> *dataToDecodeRingBuffer;
  string url;
  string secUrl;
  std::atomic<bool> *dl;
  std::atomic<bool> *free;
};

struct threadDecodeData {
  DoubleRingBuffer<char,262144> *dataToDecodeRingBuffer;
  DoubleRingBuffer<Frame<2>,262144> *dataAudioRingBuffer;
  mp3dec_t mp3d;
  std::atomic<bool> *dc;
  std::atomic<bool> *free;
};

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  struct threadReadData *pData = (struct threadReadData *) userp;
  size_t realsize = size * nmemb;
  if ((pData->dl->load()) && (realsize < pData->dataToDecodeRingBuffer->capacity()))
  {
    memcpy(pData->dataToDecodeRingBuffer->endData(), contents, realsize);
    pData->dataToDecodeRingBuffer->endIncr(realsize);
    return realsize;
  }
  return 0;
}

size_t WriteUrlCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  struct threadReadData *pData = (struct threadReadData *) userp;
  size_t realsize = size * nmemb;
  pData->secUrl += (const char*) contents;
  return realsize;
}

void * threadDecodeTask(threadDecodeData data)
{
  data.free->store(false);

  mp3dec_frame_info_t info;
  DoubleRingBuffer<Frame<2>,4096> *tmpBuffer = new DoubleRingBuffer<Frame<2>,4096>();
  SampleRateConverter<2> conv;
  int inSize;
  int outSize;

  while (data.dc->load()) {

    short pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];

    if (data.dataToDecodeRingBuffer->size() > 64000) {

      int samples = mp3dec_decode_frame(&data.mp3d, (const uint8_t*)data.dataToDecodeRingBuffer->startData(), data.dataToDecodeRingBuffer->size(), pcm, &info);

      if (info.frame_bytes > 0) {
        if (samples > 0) {
          if (info.channels == 1) {
            for(int i = 0; i < samples; i++) {
              if (!data.dc->load()) break;
              Frame<2> newFrame;
              newFrame.samples[0]=(float)pcm[i] * 30517578125e-15f;
              newFrame.samples[1]=(float)pcm[i] * 30517578125e-15f;
              tmpBuffer->push(newFrame);
            }
          }
          else {
            for(int i = 0; i < 2 * samples; i=i+2) {
              if (!data.dc->load()) break;
              Frame<2> newFrame;
              newFrame.samples[0]=(float)pcm[i] * 30517578125e-15f;
              newFrame.samples[1]=(float)pcm[i+1] * 30517578125e-15f;
              tmpBuffer->push(newFrame);
            }
          }
        }
        if (samples == 0) {
          //printf("invalid data \n");
        }
        data.dataToDecodeRingBuffer->startIncr(info.frame_bytes);
        conv.setRates(info.hz, engineGetSampleRate());
        conv.setQuality(10);
        inSize = tmpBuffer->size();
        outSize = data.dataAudioRingBuffer->capacity();
        conv.process(tmpBuffer->startData(), &inSize,data.dataAudioRingBuffer->endData(), &outSize);
        tmpBuffer->startIncr(inSize);
        data.dataAudioRingBuffer->endIncr((size_t)outSize);
      }
    }
  }

  data.free->store(true);
  return 0;
}

#ifdef USE_CURL
void * threadReadTask(threadReadData data)
{
  data.free->store(false);

  CURL *curl;
  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  string zeUrl;
  data.secUrl == "" ? zeUrl = data.url : zeUrl = data.secUrl;
  zeUrl.erase(std::remove_if(zeUrl.begin(), zeUrl.end(), [](unsigned char x){return std::isspace(x);}), zeUrl.end());
  if (stringExtension(data.url) == "pls") {
    istringstream iss(zeUrl);
    for (std::string line; std::getline(iss, line); )
    {
      std::size_t found=line.find("http");
      if (found!=std::string::npos) {
        zeUrl = line.substr(found);
        break;
      }
    }
  }

  curl_easy_setopt(curl, CURLOPT_URL, zeUrl.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
  curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  data.free->store(true);

  return 0;
}
#endif // USE_CURL


#ifdef USE_CURL
void * urlTask(threadReadData data)
{
  data.free->store(false);

  CURL *curl;
  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_URL, data.url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteUrlCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
  data.secUrl = "";
  curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  data.free->store(true);

  thread iThread = thread(threadReadTask, data);
  iThread.detach();

  return 0;
}
#endif // USE_CURL

struct ANTN : Module {
	enum ParamIds {
		URL_PARAM,
		TRIG_PARAM,
    GAIN_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		OUTL_OUTPUT,
		OUTR_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
  string url;
	SchmittTrigger trigTrigger;
  bool read = false;
  DoubleRingBuffer<Frame<2>,262144> dataAudioRingBuffer;
  DoubleRingBuffer<char,262144> dataToDecodeRingBuffer;
  thread rThread, dThread;
  threadReadData rData;
  threadDecodeData dData;
  std::atomic<bool> tDl;
  std::atomic<bool> tDc;
  std::atomic<bool> trFree;
  std::atomic<bool> tdFree;

	ANTN() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    tDl.store(true);
    tDc.store(true);
    trFree.store(true);
    tdFree.store(true);

    rData.dataToDecodeRingBuffer = &dataToDecodeRingBuffer;
    rData.dl = &tDl;
    rData.free = &trFree;

    dData.dataToDecodeRingBuffer = &dataToDecodeRingBuffer;
    dData.dataAudioRingBuffer = &dataAudioRingBuffer;
    dData.dc = &tDc;
    dData.free = &tdFree;
    mp3dec_init(&dData.mp3d);
	}

  ~ANTN() {
    tDc.store(false);
    while(!tdFree) {
    }

    tDl.store(false);
    while(!trFree) {
    }
  }

  json_t *toJson() override {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "url", json_string(url.c_str()));
    return rootJ;
  }

  void fromJson(json_t *rootJ) override {
    json_t *urlJ = json_object_get(rootJ, "url");
  	if (urlJ)
  		url = json_string_value(urlJ);
  }

	void step() override;

  void onSampleRateChange() override;
};

void ANTN::onSampleRateChange() {
  read = false;
  dataAudioRingBuffer.clear();
}

void ANTN::step() {

	if (trigTrigger.process(params[TRIG_PARAM].value)) {

      tDc.store(false);
      while(!tdFree) {
      }

      tDl.store(false);
      while(!trFree) {
      }
      read = false;
      dataToDecodeRingBuffer.clear();
      dataAudioRingBuffer.clear();

      tDl.store(true);
#ifdef USE_CURL
      rData.url = url;
      if ((stringExtension(rData.url) == "m3u") || (stringExtension(rData.url) == "pls")) {
         rThread = thread(urlTask, std::ref(rData));
      }
      else {
         rThread = thread(threadReadTask, std::ref(rData));
      }
      rThread.detach();
#endif // USE_CURL

      tDc.store(true);
      mp3dec_init(&dData.mp3d);
      dThread = thread(threadDecodeTask, std::ref(dData));
      dThread.detach();
	}

   if ((dataAudioRingBuffer.size()>64000) && (engineGetSampleRate()<96000)) {
      read = true;
   }
   if ((dataAudioRingBuffer.size()>128000) && (engineGetSampleRate()>=96000)) {
      read = true;
   }

  if (read) {
    Frame<2> currentFrame = *dataAudioRingBuffer.startData();
    outputs[OUTL_OUTPUT].value = 5.0f*currentFrame.samples[0]*params[GAIN_PARAM].value;
    outputs[OUTR_OUTPUT].value = 5.0f*currentFrame.samples[1]*params[GAIN_PARAM].value;
    dataAudioRingBuffer.startIncr(1);
  }
}

struct ANTNTextField : LedDisplayTextField {
  ANTNTextField(ANTN *mod) {
    module = mod;
    font = Font::load(assetPlugin(plugin, "res/DejaVuSansMono.ttf"));
  	color = YELLOW_BIDOO;
  	textOffset = Vec(3, 3);
    text = module->url;
  }
	void onTextChange() override;
	ANTN *module;
};
void ANTNTextField::onTextChange() {
	if (text.size() > 0) {
      string tText = text;
      tText.erase(std::remove_if(tText.begin(), tText.end(), [](unsigned char x){return std::isspace(x);}), tText.end());
      module->url = tText;
	}
}

struct ANTNWidget : ModuleWidget {
  TextField *textField;
	json_t *toJson() override;
	void fromJson(json_t *rootJ) override;

	ANTNWidget(ANTN *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/ANTN.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

  	textField = new ANTNTextField(module);
  	textField->box.pos = Vec(5, 25);
  	textField->box.size = Vec(125, 100);
  	textField->multiline = true;
  	addChild(textField);

    addParam(ParamWidget::create<BidooBlueKnob>(Vec(54, 183), module, ANTN::GAIN_PARAM, 0.5f, 3.0f, 1.0f));

  	addParam(ParamWidget::create<BlueCKD6>(Vec(54, 245), module, ANTN::TRIG_PARAM, 0.0f, 1.0f, 0.0f));

  	static const float portX0[4] = {34, 67, 101};

  	addOutput(Port::create<TinyPJ301MPort>(Vec(portX0[1]-17, 334),Port::OUTPUT, module, ANTN::OUTL_OUTPUT));
  	addOutput(Port::create<TinyPJ301MPort>(Vec(portX0[1]+4, 334),Port::OUTPUT, module, ANTN::OUTR_OUTPUT));
  }
};

json_t *ANTNWidget::toJson() {
	json_t *rootJ = ModuleWidget::toJson();

	// text
	json_object_set_new(rootJ, "text", json_string(textField->text.c_str()));

	return rootJ;
}

void ANTNWidget::fromJson(json_t *rootJ) {
	ModuleWidget::fromJson(rootJ);

	// text
	json_t *textJ = json_object_get(rootJ, "text");
	if (textJ)
		textField->text = json_string_value(textJ);
}

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, ANTN) {
   Model *modelANTN = Model::create<ANTN, ANTNWidget>("Bidoo", "antN", "antN oscillator", OSCILLATOR_TAG);
   return modelANTN;
}
