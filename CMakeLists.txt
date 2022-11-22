cmake_minimum_required (VERSION 3.8)

project ("Direct2D")

find_library(DIRECT2D d2d1)
if (NOT DIRECT2D)
    message(FATAL_ERROR "Could not find Direct2D.")
endif ()

set (CMAKE_WIN32_EXECUTABLE "True")
set (CMAKE_CXX_FLAGS 
"/Wall /std:c++20 /DUNICODE /TP /Zc:__cplusplus /EHs /MT")

set (SOURCE_FILES
    src/clock.h src/clock.cpp src/D2DApp.h src/D2DApp.cpp src/WinMain.h src/WinMain.cpp)

add_executable (Window ${SOURCE_FILES})
target_link_libraries(Window ${DIRECT2D})