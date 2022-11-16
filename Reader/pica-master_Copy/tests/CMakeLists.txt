cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v2.13.1
)
FetchContent_MakeAvailable(Catch2)

IF(WIN32)
    set(EXTRA_LIBS "-municode")
ELSE()
    set(EXTRA_LIBS "")
ENDIF()

add_executable(PicaTests
        TestTools.cpp TestTools.h
        common/imageProcessing/TestFindControlLine.cpp
        common/cropper/TestCropperRectangleDetection.cpp
        monoplex/algos/TestAlgoExponantialFit.cpp
        monoplex/algos/TestProxNoise.cpp
        monoplex/algos/TestAlgoPoly2.cpp
        monoplex/algos/TestTestMinusProxNoise.cpp
        common/imageProcessing/resources/Profiles.h
        common/imageProcessing/TestComputeProfile.cpp
        common/imageProcessing/TestRemoveLinesOnProfile.cpp
        multiplex/pattern/TestPatternManager.cpp
        common/fileProcessing/TestParser.cpp)

add_test(NAME PicaTests COMMAND PicaTests)

target_link_libraries(PicaTests PUBLIC Pica Catch2::Catch2 ${EXTRA_LIBS})
