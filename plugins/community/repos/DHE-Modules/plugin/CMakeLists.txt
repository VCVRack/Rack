cmake_minimum_required(VERSION 3.13)
project(DHE-Modules
        VERSION 0.6.4
        LANGUAGES CXX)

add_library(plugin)

target_sources(plugin
        PRIVATE
        include/display/controls.h
        include/display/panel.h
        include/util/duration.h
        include/util/gate.h
        include/util/mode.h
        include/util/phase-accumulator.h
        include/util/range.h
        include/util/rotation.h
        include/util/selector.h
        include/util/sigmoid.h
        include/util/signal.h
        include/util/stage-components.h
        include/util/trigger.h
        include/dhe-modules.h
        src/booster-stage.cpp
        src/cubic.cpp
        src/dhe-modules.cpp
        src/func.cpp
        src/hostage.cpp
        src/ranger.cpp
        src/stage.cpp
        src/swave.cpp
        src/tapers.cpp
        src/upstage.cpp
        src/xycloid.cpp)
set_target_properties(plugin
        PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        PREFIX ""
        )
target_compile_options(plugin
        PRIVATE
        -stdlib=libc++
        -march=nocona
        -MMD
        -MP
        -O3
        -ffast-math
        -fno-finite-math-only
        -mmacosx-version-min=10.7
        -Wall
        -Wextra
        -Wno-unused-parameter
        )
target_compile_definitions(plugin
        PRIVATE
        ARCH_MAC
        SLUG=DHE-Modules
        VERSION=0.6.4
        )
target_include_directories(plugin PRIVATE include)
target_link_libraries(plugin PRIVATE racksdk)
target_link_options(plugin
        PRIVATE
        "LINKER:-undefined,dynamic_lookup"
        )

add_library(racksdk INTERFACE)
target_include_directories(racksdk
        INTERFACE
        ../../Rack-SDK/include
        ../../Rack-SDK/dep/include)
target_compile_features(racksdk INTERFACE
        cxx_std_11)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/rack)
set(PLUGINS_INSTALL_DIR plugins)
set(PLUGIN_INSTALL_DIR plugins/${PROJECT_NAME})

install(TARGETS plugin
        LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
        ARCHIVE DESTINATION ${PLUGIN_INSTALL_DIR})
install(DIRECTORY ../svg DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES ../LICENSE.txt DESTINATION ${PLUGIN_INSTALL_DIR})
install(DIRECTORY /Users/Dale/Documents/Rack/plugins/Fundamental DESTINATION ${PLUGINS_INSTALL_DIR})
