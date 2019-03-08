cmake_minimum_required(VERSION 3.8)
project(LRT)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        src/LindenbergResearch.cpp
        src/LindenbergResearch.hpp
        src/modules/AlmaFilter.cpp
        src/modules/SimpleFilter.cpp
        src/modules/ReShaper.cpp
        src/modules/BlankPanel.cpp
        src/modules/BlankPanelEmpty.cpp
        src/modules/VCO.cpp
        src/dsp/DSPMath.cpp
        src/dsp/DSPMath.hpp
        src/dsp/Oscillator.cpp
        src/dsp/Oscillator.hpp
        src/dsp/DSPSystem.hpp
        src/dsp/LadderFilter.hpp
        src/dsp/LadderFilter.cpp
        src/dsp/MS20zdf.hpp
        src/dsp/MS20zdf.cpp
        src/modules/MS20Filter.cpp
        src/LRComponents.hpp
        src/dsp/WaveShaper.cpp
        src/dsp/WaveShaper.hpp
        src/dsp/Lockhart.cpp
        src/dsp/Lockhart.hpp
        src/modules/Westcoast.cpp
        src/dsp/HQTrig.hpp
        src/dsp/Serge.cpp
        src/dsp/Serge.hpp
        src/dsp/Horner.h
        src/dsp/LambertW.h
        src/dsp/LambertW.cpp
        src/widgets/LRKnob.cpp
        src/widgets/LRShadow.cpp
        src/widgets/LRCVIndicator.cpp
        src/widgets/LRCVIndicator.cpp
        src/widgets/LRLCDWidget.cpp
        src/widgets/LRLight.cpp
        src/widgets/LRPanel.cpp
        src/widgets/LRSVGRotator.cpp
        src/dsp/Saturator.cpp
        src/dsp/Saturator.hpp
        src/dsp/Hardclip.cpp
        src/dsp/Hardclip.hpp
        src/dsp/RShaper.cpp
        src/dsp/RShaper.hpp
        src/dsp/Overdrive.cpp
        src/dsp/Overdrive.hpp
        src/modules/QuickMix.cpp
        src/dsp/FastTanWF.cpp
        src/dsp/FastTanWF.hpp
        src/modules/BlankPanelWood.cpp
        src/dsp/DiodeLadder.cpp
        src/dsp/DiodeLadder.hpp
        src/modules/DiodeVCF.cpp
        src/widgets/FontIconWidget.cpp
        src/modules/BlankPanelSmall.cpp
        src/modules/Speck.cpp
        src/widgets/LRGradientWidget.cpp src/LRModule.cpp src/LRModuleWidget.cpp src/LRModel.hpp src/widgets/LRPatinaWidget.cpp src/LRGestalt.hpp src/modules/Korg35.cpp src/dsp/Korg35Filter.cpp src/dsp/Korg35Filter.hpp)

include_directories(.)
include_directories(src)
include_directories(src/dsp)
include_directories(src/dsp/kiss_fft)
include_directories(../../include)
include_directories(../../include)
include_directories(../../include/dsp)
include_directories(../../dep/include)

add_executable(LRT ${SOURCE_FILES} src/dsp/DSPMath.cpp src/dsp/DSPMath.hpp)