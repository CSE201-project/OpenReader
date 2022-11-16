cmake_minimum_required(VERSION 3.12)
project(Pica VERSION 0.1 DESCRIPTION "Lumedix Pics Analyzer Library")

set(CMAKE_CXX_STANDARD 17)

message("CMAKE_CXX_COMPILER_VERSION : ${CMAKE_CXX_COMPILER_VERSION}")
if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL "8.2.0")
    set(EXTRA_LIBS "stdc++fs")
    message("setting EXTRA_LIBS to ${EXTRA_LIBS}")
endif()

include(FetchContent)

get_filename_component(ThirdPartiesPath "../ThirdParties" ABSOLUTE)

message("CMAKE_LIBRARY_ARCHITECTURE ${CMAKE_LIBRARY_ARCHITECTURE}")



if ("${CMAKE_LIBRARY_ARCHITECTURE}" STREQUAL "armv7-linux-gnueabi")
    set(ENABLE_OPENCV_PLOT OFF)
else()
    set(ENABLE_OPENCV_PLOT ON)
    add_compile_definitions(ENABLE_OPENCV_PLOT)
endif()

IF(WIN32)
    set(OpenCVLibArchPath ${ThirdPartiesPath}/opencv-4.5.0/mingw-build)
    set(CERES_DIR ${ThirdPartiesPath}/ceres-windows/ceres-solver)
    set(EIGEN_INCLUDE_DIR ${ThirdPartiesPath}/ceres-windows/Eigen )
    set(EXTRA_LIBS "-municode")
    message("setting EXTRA_LIBS to ${EXTRA_LIBS}")

    set(OpenXLSX_INCLUDE_DIR ${ThirdPartiesPath}/OpenXLSX-master/library)
    set(OpenXLSX_INCLUDE_DIR_EXPORT ${ThirdPartiesPath}/OpenXLSX-master/OpenXLSX-build/library)
    set(OpenXLSX_LIB ${ThirdPartiesPath}/OpenXLSX-master/OpenXLSX-build/output/libOpenXLSX-shared.dll)

    set(LibBuildPath "build-win32")
ELSE()
    set(OpenCVLibArchPath ${ThirdPartiesPath}/opencv-4.3.0/build-${CMAKE_CXX_LIBRARY_ARCHITECTURE})
ENDIF()

## ----------------------- OpenCV ----------------------- ##
message("OpenCVLibArchPath: ${OpenCVLibArchPath}")
set(OpenCV_DIR ${OpenCVLibArchPath})
find_package(OpenCV REQUIRED PATHS ${OpenCvLibArchPath} NO_DEFAULT_PATH)
message("OpenCV_LIBS ${OpenCV_INCLUDE_DIRS}")
message("OpenCV_LIBS ${OpenCV_LIBS}")
set(OpenCV_LIBS_MINIMAL "opencv_core;opencv_imgcodecs;opencv_imgproc")
message("OpenCV_LIBS_MINIMAL ${OpenCV_LIBS_MINIMAL}")
if (ENABLE_OPENCV_PLOT)
    set(OpenCV_LIBS_MINIMAL "${OpenCV_LIBS_MINIMAL};opencv_plot")
endif()

## ----------------------- Poco ----------------------- ##
set(PocoLibArchPath ${ThirdPartiesPath}/poco-1.10.1/${LibBuildPath})
message("PocoLibArchPath: ${PocoLibArchPath}")
find_package(Poco REQUIRED COMPONENTS Foundation JSON HINTS ${PocoLibArchPath})
message("Poco_FOUND ${Poco_DIR}")

find_package(Ceres REQUIRED)

file(GLOB MATHLAB_EXPORT
        mathlab_export/*.cpp
        mathlab_export/*.h
        )

# Add a custom command that produces version.cpp, plus
# a dummy output that's not actually produced, in order
# to force version.cmake to always be re-run before the build
ADD_CUSTOM_COMMAND(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/GitVersion.cpp ${CMAKE_CURRENT_BINARY_DIR}/_GitVersion.cpp
        COMMAND ${CMAKE_COMMAND} ARGS -DGIT_DIR="${CMAKE_CURRENT_SOURCE_DIR}" -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GitVersion.cmake
)

add_library(Pica STATIC
        ${MATHLAB_EXPORT}
        ${CMAKE_CURRENT_BINARY_DIR}/GitVersion.cpp
        GitVersion.h
        common/fileProcessing/Parser.cpp
        common/fileProcessing/Parser.h
        common/imageProcessing/ImagePreProcessing.cpp
        common/imageProcessing/ImagePreProcessing.h
        common/imageProcessing/ImageAnalyzer.cpp
        common/imageProcessing/ImageAnalyzer.h
        common/cropper/StripCropperRectangleDetection.cpp
        common/cropper/StripCropperRectangleDetection.h
        common/cropper/CassetteType.cpp
        common/cropper/CassetteType.h
        common/cropper/CropperStructsAndData.h
        monoplex/algos/CurveFitter.cpp
        monoplex/algos/CurveFitter.h
        monoplex/algos/IImageProcessor.cpp
        monoplex/algos/IImageProcessor.h
        monoplex/algos/AlgoProxNoise.cpp
        monoplex/algos/AlgoProxNoise.h
        monoplex/algos/AlgoExponantialFit.cpp
        monoplex/algos/AlgoExponantialFit.h
        monoplex/algos/AlgoPoly2.cpp
        monoplex/algos/AlgoPoly2.h
        monoplex/algos/AlgoTestMinusProxNoise.cpp
        monoplex/algos/AlgoTestMinusProxNoise.h
        multiplex/pattern/PatternManager.cpp
        multiplex/pattern/PatternManager.h
        multiplex/pattern/MultiplexStructs.h
        multiplex/algos/AlgoMultiplexConvolBrut.cpp
        multiplex/algos/AlgoMultiplexConvolBrut.h)

add_library(PicaUseTestMonoplexLib STATIC
        monoplex/useTestAnalyzer/ConcentrationPredictor.cpp
        monoplex/useTestAnalyzer/ConcentrationPredictor.h
        monoplex/useTestAnalyzer/ProxNoiseGrids.cpp
        monoplex/useTestAnalyzer/ProxNoiseGrids.h
        monoplex/useTestAnalyzer/ExcelWriter.cpp
        monoplex/useTestAnalyzer/ExcelWriter.h
        monoplex/useTestAnalyzer/ROCCurveManager.cpp
        monoplex/useTestAnalyzer/ROCCurveManager.h
        monoplex/useTestAnalyzer/MonoplexUseTestAnalyzer.cpp
        monoplex/useTestAnalyzer/MonoplexUseTestAnalyzer.h
        monoplex/useTestAnalyzer/RegulatoryDataManager.cpp
        monoplex/useTestAnalyzer/RegulatoryDataManager.h)

add_library(PicaUseTestMultiplexLib STATIC
        multiplex/useTestAnalyzer/ExcelWriter.cpp
        multiplex/useTestAnalyzer/ExcelWriter.h
        multiplex/useTestAnalyzer/MultiplexUseTestAnalyzer.cpp
        multiplex/useTestAnalyzer/MultiplexUseTestAnalyzer.h)

set_target_properties(Pica PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(Pica PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CERES_DIR}/include
        ${CERES_DIR}/internal/ceres/miniglog
        SYSTEM ${EIGEN_INCLUDE_DIRS}
        BEFORE ${CERES_DIR}/cmake-build-release/config)

target_include_directories(PicaUseTestMonoplexLib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OpenXLSX_INCLUDE_DIR}
        ${OpenXLSX_INCLUDE_DIR_EXPORT})

target_include_directories(PicaUseTestMultiplexLib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OpenXLSX_INCLUDE_DIR}
        ${OpenXLSX_INCLUDE_DIR_EXPORT})

target_link_libraries(Pica PUBLIC
        ${OpenCV_LIBS_MINIMAL}
        ceres
        Poco::Foundation Poco::JSON)

target_link_libraries(PicaUseTestMonoplexLib PUBLIC
        Pica
        ${OpenXLSX_LIB}
        ${OpenCV_LIBS_MINIMAL})

target_link_libraries(PicaUseTestMultiplexLib PUBLIC
        Pica
        ${OpenXLSX_LIB}
        ${OpenCV_LIBS_MINIMAL})

add_executable(PicaRunnerMassTest main_massTest.cpp)
target_link_libraries(PicaRunnerMassTest PUBLIC Pica opencv_highgui)

add_executable(PicaRunnerMatlab main_matlab.cpp)
target_link_libraries(PicaRunnerMatlab PUBLIC Pica
        ${EXTRA_LIBS} PicaUseTestMonoplexLib)

add_executable(PicaRunnerAnalyzeUseTest main_analyzeUseTest.cpp)
target_link_libraries(PicaRunnerAnalyzeUseTest PUBLIC Pica
        ${EXTRA_LIBS} PicaUseTestMonoplexLib)

add_executable(PicaRunnerAnalyzeMultiplex main_analyzeMultiplex.cpp)
target_link_libraries(PicaRunnerAnalyzeMultiplex PUBLIC Pica
        ${EXTRA_LIBS} PicaUseTestMultiplexLib)

add_executable(PicaRunnerCompileData main_compileData.cpp)
target_link_libraries(PicaRunnerCompileData PUBLIC Pica
        ${EXTRA_LIBS} PicaUseTestMonoplexLib)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
