cmake_minimum_required(VERSION 3.8)
project(LRT)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        src/LindenbergResearch.cpp
        src/LindenbergResearch.hpp
        src/AlmaFilter.cpp
        src/SimpleFilter.cpp
        src/ReShaper.cpp
        src/BlankPanel.cpp
        src/BlankPanelM1.cpp
        src/VCO.cpp
        src/dsp/DSPMath.cpp
        src/dsp/DSPMath.hpp
        src/Release.h
        src/dsp/Oscillator.cpp
        src/dsp/Oscillator.hpp
        src/dsp/DSPSystem.hpp
        src/dsp/LadderFilter.hpp
        src/dsp/LadderFilter.cpp
        src/dsp/MS20zdf.hpp src/dsp/MS20zdf.cpp src/MS20Filter.cpp src/LRComponents.cpp src/LRComponents.hpp)

include_directories(.)
include_directories(src)
include_directories(src/dsp)
include_directories(../../include)
include_directories(../../include)
include_directories(../../include/dsp)
include_directories(../../dep/include)

add_executable(LRT ${SOURCE_FILES} src/dsp/DSPMath.cpp src/dsp/DSPMath.hpp src/Release.h)