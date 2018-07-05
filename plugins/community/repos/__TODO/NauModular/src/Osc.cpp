#include "NauModular.hpp"
#include <iostream>

#if ARCH_WIN
#include <Winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/select.h>
#endif

#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include "tinyosc.h"
#include "dsp/digital.hpp"

#define osc_strncpy(_dst, _src, _len) strncpy(_dst, _src, _len)

struct Osc : Module{
    enum ParamIds{
        NUM_PARAMS
    };

    enum InputIds{
        GATE_INPUT,
        TRIG_INPUT,
        NUM_INPUTS
    };

    enum OutputIds{
        CV_OUTPUT,
        GATE_OUTPUT,
        TRIG_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds{
        GATE_IN_LIGHT,
        TRIG_IN_LIGHT,
        CV_OUT_LIGHT,
        GATE_OUT_LIGHT,
        TRIG_OUT_LIGHT,
        NUM_LIGHTS
    };

    Osc();
    ~Osc();

    void makeUDPSocket();
    void checkIncomingOsc();
    int makeFloatMsg(char * _buf, int  _bufLen, std::string _addr, float _val);
    int makeGateMsg(char * _buf, int _bufLen, float _val);
    int makeTrigMsg(char * _buf, int _bufLen, float _val);
    void sendMsg(char * _buf, int _bufLen);
    void sendGateMsg(float _val);
    void sendTrigMsg(float _val=5.0);

    void step() override;
    void updateGateIn();
    void updateTrigIn();

    void openRemoteGate(float _val=5.0);
    void closeRemoteGate();
    void openLocalGate(float _val=5.0);
    void closeLocalGate();
    void setLocalCv(float _val);
    void setTrigNow();

    bool isAnyOutputActive();

    SchmittTrigger trigIn;
    float lastTrigInput;
    float lastTrigOutput;
    float thrTrig;
    float thrGateOpen;
    float thrGateClosed;
    float curTime;
    float stepTime;
    std::string ipOut;
    char * buffer;
    char * bufMsgOut;
    int portIn;
    int portOut;
    int s;
    int i;
    socklen_t slen;
    int recv_len;

    const int bufLen;
    const int msgBufLen;
    struct sockaddr_in si_me, si_other;
    bool bReady;
    bool bGateOpen;

    int n;
};

Osc::Osc() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS), bufLen(1024), msgBufLen(1024){
    portIn = 9000;
    portOut = 8000;
    ipOut = "127.0.0.1";
    slen = sizeof(si_other);
    buffer = new char[bufLen];
    bufMsgOut = new char[msgBufLen];
    bReady = false;
    n = 0;

    lastTrigInput = 0.0;
    bGateOpen = false;
    thrTrig = 5;
    thrGateOpen = 5;
    thrGateClosed = 0;
    curTime = 0;

#if ARCH_WIN
    WSADATA wsaData;
    WORD versionRequested = MAKEWORD(2,2);
    if(WSAStartup(versionRequested, &wsaData) != 0) {
        std::cout<<"OSC ERROR: Failed to start Windows networking"<<std::endl;
        return;
    }
#endif

    makeUDPSocket();
}

Osc::~Osc(){
#if ARCH_WIN
    if(bReady && (closesocket(s) != 0)) {
        std::cout << "OSC ERROR: Failed to close socket: " << WSAGetLastError() << std::endl;
    }
#else
    if(bReady)close(s);
#endif

    delete [] buffer;
    delete [] bufMsgOut;

#if ARCH_WIN
    WSACleanup();
#endif
}

void Osc::makeUDPSocket(){
    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(s==-1){
        std::cout<<"OSC ERROR: cannot create socket"<<std::endl;
        bReady = false;
#if ARCH_WIN
        WSACleanup();
#endif
        return;
    }

    //set local
    memset((char *)&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(portIn);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    //set remote
    memset((char *)&si_other, 0 , sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(portOut);

#ifdef ARCH_WIN
    const unsigned long addr = inet_addr(ipOut.c_str());
    if ((addr == INADDR_NONE) || (addr == INADDR_ANY)){
        std::cout<<"OSC ERROR: remote IP is not valid"<<std::endl;
        bReady = false;
        WSACleanup();
        return;
    }
    si_other.sin_addr.S_un.S_addr = addr;
#else
    if(inet_aton(ipOut.c_str(), &si_other.sin_addr)==0){
        std::cout<<"OSC ERROR: remote IP is not valid"<<std::endl;
        bReady = false;
        return;
    }
#endif

    if(bind(s, (struct sockaddr*)&si_me, sizeof(si_me) ) == -1){
        std::cout<<"OSC ERROR: cannot bind socket"<<std::endl;
        bReady = false;
#if ARCH_WIN
        WSACleanup();
#endif
        return;
    }

    bReady = true;
    std::cout<<"OSC listening on port "<<portIn<<" and sending at "<<ipOut<<":"<<portOut<<std::endl;
}

void Osc::checkIncomingOsc(){
    if(!isAnyOutputActive()){
        //lights[CV_OUT_LIGHT].value = 0;
        //lights[GATE_OUT_LIGHT].value = 0;
        lights[TRIG_OUT_LIGHT].value = 0;
        setLocalCv(0);
        closeLocalGate();
        return;
    }

    if(outputs[TRIG_OUTPUT].value>0){
        if(curTime > (lastTrigOutput+0.1)){
            lights[TRIG_OUT_LIGHT].value = 0;
            outputs[TRIG_OUTPUT].value = 0;
        }
    }

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(s, &readSet);
    int microSec = stepTime * 1000000/2;
    struct timeval timeout = {0, microSec};
    if(select(s+1, &readSet, NULL, NULL, &timeout)>0){
        recv_len = 0;
        while((recv_len=(int)recvfrom(s, buffer, bufLen, 0, (struct sockaddr *)&si_other,&slen))>0){
            if(tosc_isBundle(buffer)){
                std::cout<<"OSC ERROR: no bundles, pleeze"<<std::endl;
            }else{
                tosc_message msg;
                tosc_parseMessage(&msg, buffer, bufLen);
                //tosc_printMessage(&msg);
                std::string sAddr = tosc_getAddress(&msg);
                float ff;
                if(sAddr=="/gate"){
                    ff = tosc_getNextFloat(&msg);
                    if(ff>0){
                        openLocalGate(ff);
                    }else{
                        closeLocalGate();
                    }
                }else if(sAddr=="/trigger"){
                    ff = tosc_getNextFloat(&msg);
                    setTrigNow();
                }else if(sAddr=="/cv"){
                    ff = tosc_getNextFloat(&msg);
                    setLocalCv(ff);
                }
            }
        }
    }
}

int Osc::makeFloatMsg(char * _buf, int  _bufLen, std::string _addr, float _val){
    memset(_buf,0,_bufLen);
    uint32_t ii = (uint32_t)strlen(_addr.c_str());
    if(_addr.c_str()==NULL || _addr.size()==0 || ii>=_bufLen)return -1;
    osc_strncpy(_buf, _addr.c_str(), _bufLen);
    ii = (ii+4)& ~0x3;
    _buf[ii++] = ',';
    int s_len = 1;
    osc_strncpy(_buf+ii, "f", _bufLen-ii-s_len);
    ii = (ii+4+s_len)& ~0x03;

    if((ii+4)>_bufLen)return -3;
    *((uint32_t *)(_buf+ii)) = htonl(*((uint32_t *)&_val));
    ii+=4;

    return ii;
}

int Osc::makeGateMsg(char * _buf, int  _bufLen, float _val){
    return makeFloatMsg(_buf, _bufLen, "/gate", _val);
}

int Osc::makeTrigMsg(char * _buf, int _bufLen, float _val){
    return makeFloatMsg(_buf, _bufLen, "/trigger", _val);
}

void Osc::sendMsg(char * _buf, int _bufLen){
    if(bReady)
        sendto(s, _buf, _bufLen, 0, (struct sockaddr *)&si_other, slen);
}

void Osc::sendGateMsg(float _val){
    int ll = makeGateMsg(bufMsgOut, msgBufLen, _val);
    if(ll>0){
        sendMsg(bufMsgOut, ll);
    }else{
        std::cout<<"OSC ERROR: cannot send gate message"<<std::endl;
    }
}

void Osc::sendTrigMsg(float _val){
    int ll = makeTrigMsg(bufMsgOut, msgBufLen, _val);
    if(ll>0){
        sendMsg(bufMsgOut, ll);
    }else{
        std::cout<<"OSC ERROR: cannot send trigger message"<<std::endl;
    }
}

void Osc::step(){
    stepTime = 1.0/engineGetSampleRate();
    curTime += stepTime;

    updateGateIn();
    updateTrigIn();

    checkIncomingOsc();

}

void Osc::updateGateIn(){
    if(inputs[GATE_INPUT].active){
        float curVal = inputs[GATE_INPUT].value;
        if(curVal>=thrGateOpen){
            openRemoteGate(curVal);
        }else if(curVal<=thrGateClosed){
            closeRemoteGate();
        }
    }else{
        closeRemoteGate();
    }
}

void Osc::openRemoteGate(float _val){
    if(bGateOpen)return;
    lights[GATE_IN_LIGHT].value = 1.0;
    sendGateMsg(_val);
    bGateOpen = true;
}

void Osc::closeRemoteGate(){
    if(!bGateOpen)return;
    lights[GATE_IN_LIGHT].value = 0.0;
    sendGateMsg(0);
    bGateOpen = false;
}

void Osc::closeLocalGate(){
    lights[GATE_OUT_LIGHT].value = 0;
    outputs[GATE_OUTPUT].value = 0;
}

void Osc::openLocalGate(float _val){
    lights[GATE_OUT_LIGHT].value = 1.0;
    outputs[GATE_OUTPUT].value = _val;
}

void Osc::setLocalCv(float _val){
    lights[CV_OUT_LIGHT].value = fabs(_val)/12.0;
    outputs[CV_OUTPUT].value = _val;
}

void Osc::setTrigNow(){
    lights[TRIG_OUT_LIGHT].value = 1;
    lastTrigOutput = curTime;
    outputs[TRIG_OUTPUT].value = 5;
}

void Osc::updateTrigIn(){
    if(inputs[TRIG_INPUT].active){
        float curVal = inputs[TRIG_INPUT].value;
        if(trigIn.process(curVal)){
            lights[TRIG_IN_LIGHT].value = 1.0;
            sendTrigMsg();
            lastTrigInput = curTime;
        }

        if(lights[TRIG_IN_LIGHT].value>0){
            if(curTime>(lastTrigInput+0.1)){
                lights[TRIG_IN_LIGHT].value = 0;
            }
        }
    }else{
        trigIn.reset();
        lights[TRIG_IN_LIGHT].value = 0.0;
    }
}

bool Osc::isAnyOutputActive(){
    return (outputs[CV_OUTPUT].active ||
            outputs[GATE_OUTPUT].active ||
            outputs[TRIG_OUTPUT].active);
}

struct OscWidget : ModuleWidget{
    OscWidget(Osc *module);
};

OscWidget::OscWidget(Osc *module) : ModuleWidget(module){
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
    	SVGPanel *panel = new SVGPanel();
    	panel->box.size = box.size;
    	panel->setBackground(SVG::load(assetPlugin(plugin, "res/Osc.svg")));
    	addChild(panel);
    }

    addInput(Port::create<PJ301MPort>(Vec(10,75), Port::INPUT, module, Osc::GATE_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(55,75), Port::INPUT, module, Osc::TRIG_INPUT));

    addOutput(Port::create<PJ301MPort>(Vec(32, 170), Port::OUTPUT, module, Osc::CV_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(10, 270), Port::OUTPUT, module, Osc::GATE_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(55, 270), Port::OUTPUT, module, Osc::TRIG_OUTPUT));

    addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(16, 103), module, Osc::GATE_IN_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(62, 103), module, Osc::TRIG_IN_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(40, 200), module, Osc::CV_OUT_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(20, 250), module, Osc::GATE_OUT_LIGHT));
    addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(60, 250), module, Osc::TRIG_OUT_LIGHT));

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

}

Model *modelOsc = Model::create<Osc, OscWidget>("NauModular", "Osc", "Osc", CONTROLLER_TAG);
