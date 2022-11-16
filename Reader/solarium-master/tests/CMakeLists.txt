cmake_minimum_required(VERSION 3.12)

project(Solarium LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v2.13.1
)
FetchContent_MakeAvailable(Catch2)

add_executable(SolariumTests main_tests.cpp
        Tools_Converter.cpp
        StudiesRepositoryTests.cpp
        DBTests.cpp
        TestTools.cpp TestTools.h
        AnalysisResult_JSONConversion.cpp
        ParamsFinderTests.cpp
        LEDParamsTest.cpp LensFocalTests.cpp ImgToolsTests.cpp)
add_test(NAME SolariumTests COMMAND SolariumTests)

target_link_libraries(SolariumTests PUBLIC SolariumLib Catch2::Catch2)
