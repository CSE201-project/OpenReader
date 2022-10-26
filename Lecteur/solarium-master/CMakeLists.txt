cmake_minimum_required(VERSION 3.12)

project(Solarium LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DENABLE_DEBUG_MACRO")

get_filename_component(SUBBUILD_DIRNAME ${CMAKE_CURRENT_BINARY_DIR} NAME)
string(REPLACE "-205" "" SUBBUILD_DIRNAME "${SUBBUILD_DIRNAME}")

message("CMAKE_BUILD_TYPE : ${CMAKE_BUILD_TYPE}")
message("SUBBUILD_DIRNAME : ${SUBBUILD_DIRNAME}")


message("CMAKE_CXX_COMPILER_VERSION : ${CMAKE_CXX_COMPILER_VERSION}")
message("CMAKE_C_LIBRARY_ARCHITECTURE : ${CMAKE_C_LIBRARY_ARCHITECTURE}")
message("CMAKE_CXX_LIBRARY_ARCHITECTURE : ${CMAKE_CXX_LIBRARY_ARCHITECTURE}")
if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL "8.2.0")
	set(EXTRA_LIBS "stdc++fs")
	message("setting EXTRA_LIBS to ${EXTRA_LIBS}")
endif()

if ("${CMAKE_LIBRARY_ARCHITECTURE}" STREQUAL "")
	set(CMAKE_LIBRARY_ARCHITECTURE "win32")
	set(CMAKE_C_LIBRARY_ARCHITECTURE "win32")
	set(CMAKE_CXX_LIBRARY_ARCHITECTURE "win32")
endif()

message("CMAKE_LIBRARY_ARCHITECTURE ${CMAKE_LIBRARY_ARCHITECTURE}")
get_filename_component(ThirdPartiesPath "../ThirdParties" ABSOLUTE)
set(LibBuildPath "build-${CMAKE_CXX_LIBRARY_ARCHITECTURE}")

set(USE_DUMMY_PERIPHERALS_DEFAULT OFF)
set(USE_UEYE_CAMERA_DEFAULT OFF)
set(USE_HW_LMX03_DEFAULT OFF)
set(USE_V4L2_CAMERA_DEFAULT OFF)

if ("${CMAKE_LIBRARY_ARCHITECTURE}" STREQUAL "x86_64-linux-gnu")
	set(USE_DUMMY_PERIPHERALS_DEFAULT ON)
	#[[set(USE_V4L2_CAMERA_DEFAULT ON)]]
elseif ("${CMAKE_LIBRARY_ARCHITECTURE}" STREQUAL "win32")
	set(USE_DUMMY_PERIPHERALS_DEFAULT ON)
elseif("${CMAKE_LIBRARY_ARCHITECTURE}" STREQUAL "armv7-linux-gnueabi")
	set(USE_UEYE_CAMERA_DEFAULT ON)
elseif("${CMAKE_LIBRARY_ARCHITECTURE}" STREQUAL "ccimx8x" OR "${CMAKE_LIBRARY_ARCHITECTURE}" STREQUAL "ccimx8mn")
	set(USE_HW_LMX03_DEFAULT ON)
	set(USE_V4L2_CAMERA_DEFAULT ON)
	set(EXTRA_LIBS "${EXTRA_LIBS};digiapix")
endif()

option(USE_DUMMY_PERIPHERALS "Using fake peripherals" ${USE_DUMMY_PERIPHERALS_DEFAULT})
option(USE_HW_LMX03 "Use LMX 03 Peripherals" ${USE_HW_LMX03_DEFAULT})
option(USE_UEYE_CAMERA "Using IDS uEye Camera" ${USE_UEYE_CAMERA_DEFAULT})
option(USE_V4L2_CAMERA "Using V4L2 Camera" ${USE_V4L2_CAMERA_DEFAULT})

include(FetchContent)

## ----------------------- HARU ----------------------- ##
set(HARU_PATH "/home/mario/DEV/ThirdParties/libharu-RELEASE_2_3_0/${LibBuildPath}")
message("searching for HARU in ${HARU_PATH}")

set(HARU_INCLUDE_DIRS ${HARU_PATH}/include)
set(HARU_LIBS ${HARU_PATH}/lib/libhpdf.so)


message("found HARU include ${HARU_INCLUDE_DIRS}")
message("found HARU libs ${HARU_LIBS}")

## ----------------------- SQLite ----------------------- ##
find_package (SQLite3)

## ----------------------- Poco ----------------------- ##
set(PocoLibArchPath ${ThirdPartiesPath}/poco-1.10.1/${LibBuildPath})
message("PocoLibArchPath: ${PocoLibArchPath}")
find_package(Poco REQUIRED COMPONENTS Foundation Net Util HINTS ${PocoLibArchPath})
message("Poco_FOUND ${Poco_DIR}")

## ----------------------- OpenCV ----------------------- ##
if("${CMAKE_LIBRARY_ARCHITECTURE}" STREQUAL "ccimx8x")
	find_package(OpenCV REQUIRED)
else()
	if("${CMAKE_LIBRARY_ARCHITECTURE}" STREQUAL "win32")
		set(OpenCVLibArchPath ${ThirdPartiesPath}/opencv-4.5.0/mingw-build)
	else()
		set(OpenCVLibArchPath ${ThirdPartiesPath}/opencv-4.3.0/${LibBuildPath})
	endif()
	message("OpenCVLibArchPath: ${OpenCVLibArchPath}")
	set(OpenCV_DIR ${OpenCVLibArchPath})
	find_package(OpenCV REQUIRED PATHS ${OpenCvLibArchPath} NO_DEFAULT_PATH)
	set(OpenCV_LIBS_MINIMAL "opencv_core;opencv_imgcodecs;opencv_imgproc")
	if (NOT "${CMAKE_LIBRARY_ARCHITECTURE}" STREQUAL "armv7-linux-gnueabi")
		set(OpenCV_LIBS_MINIMAL "${OpenCV_LIBS_MINIMAL};opencv_plot")
	endif()
	message("OpenCV_LIBS_MINIMAL ${OpenCV_LIBS_MINIMAL}")
endif()
message("OpenCV_LIBS ${OpenCV_INCLUDE_DIRS}")
message("OpenCV_LIBS ${OpenCV_LIBS}")
set(OpenCV_LIBS_MINIMAL "opencv_core;opencv_imgcodecs;opencv_imgproc")
message("OpenCV_LIBS_MINIMAL ${OpenCV_LIBS_MINIMAL}")

## ----------------------- Camera ----------------------- ##
set(CAMERA_LIBRARIES "")
set(CAMERA_INCLUDE_DIRS "")
set(CAMERA_SRC "")

if(USE_UEYE_CAMERA)
	set(LIBUEYE_PATH ${ThirdPartiesPath}/ueye-sdk-4.93)
	set(CAMERA_LIBRARIES ${LIBUEYE_PATH}/lib/${CMAKE_CXX_LIBRARY_ARCHITECTURE}/libueye_api.so)
	set(CAMERA_INCLUDE_DIRS ${LIBUEYE_PATH}/include)
	set(CAMERA_SRC
			device/ueye/UEyeCamera.cpp device/ueye/UEyeCamera.h
			device/ueye/UEyeCameraParams.cpp device/ueye/UEyeCameraParams.h)
elseif(USE_V4L2_CAMERA)
	set(CAMERA_SRC
			device/v4l2camera/V4L2Camera.cpp device/v4l2camera/V4L2Camera.h
			)
endif()

message("CAMERA_LIBRARIES        : ${CAMERA_LIBRARIES}")
message("CAMERA_INCLUDE_DIRS     : ${CAMERA_INCLUDE_DIRS}")

## ----------------------- Ceres ----------------------- ##
IF(WIN32)
	set(EIGEN_INCLUDE_DIR ${ThirdPartiesPath}/ceres-windows/Eigen)
	set(CeresLibArchPath ${ThirdPartiesPath}/ceres-windows/ceres-solver)
ELSE()
	set(EIGEN_INCLUDE_DIR ${ThirdPartiesPath}/eigen-3.3.9)
	set(CeresLibArchPath ${ThirdPartiesPath}/ceres-solver-1.12.0/build-${CMAKE_CXX_LIBRARY_ARCHITECTURE})
ENDIF()
find_package(Ceres REQUIRED PATHS ${CeresLibArchPath})

## ----------------------- Pica ----------------------- ##
IF(WIN32)
	get_filename_component(LIBPICA_PATH "../pica" ABSOLUTE)
	set(LIBPICA_LIBRARIES ${LIBPICA_PATH}/cmake-build-release/libPica.a)
ELSE()
	get_filename_component(LIBPICA_PATH "../Pica" ABSOLUTE)
	set(LIBPICA_LIBRARIES ${LIBPICA_PATH}/${SUBBUILD_DIRNAME}/libPica.a)
ENDIF()
set(LIBPICA_INCLUDE_DIRS ${LIBPICA_PATH})
message("LIBPICA_LIBRARIES        : ${LIBPICA_LIBRARIES}")
message("LIBPICA_INCLUDE_DIRS     : ${LIBPICA_INCLUDE_DIRS}")


## ----------------------- Peripherals ----------------------- ##

set(PERIPHERALS_SRC device/hal/CameraWrapper.cpp device/hal/CameraWrapper.h)
set(PERIPHERALS_INCLUDE_DIRS)
if (USE_HW_LMX03)
	set(PERIPHERALS_SRC
			${PERIPHERALS_SRC}
			device/peripherals-lmx3/i2c_drv.cpp
			device/peripherals-lmx3/imx8_defs.h
			device/peripherals-lmx3/inc/i2c_drv.h
			device/peripherals-lmx3/inc/lp5562_drv.h
			device/peripherals-lmx3/inc/pca9624_drv.h
			device/peripherals-lmx3/inc/psy_drv.h
			device/peripherals-lmx3/inc/thm_drv.h
			device/peripherals-lmx3/inc/uvs_drv.h
			device/peripherals-lmx3/Lmx3Battery.cpp
			device/peripherals-lmx3/Lmx3Battery.h
			device/peripherals-lmx3/Lmx3LED.cpp
			device/peripherals-lmx3/Lmx3LED.h
			device/peripherals-lmx3/Lmx3LightSensor.cpp
			device/peripherals-lmx3/Lmx3LightSensor.h
			device/peripherals-lmx3/Lmx3TemperatureSensor.cpp
			device/peripherals-lmx3/Lmx3TemperatureSensor.h
			device/peripherals-lmx3/lp5562_drv.cpp
			device/peripherals-lmx3/pca9624_drv.cpp
			device/peripherals-lmx3/psy_drv.cpp
			device/peripherals-lmx3/thm_drv.cpp
			device/peripherals-lmx3/uvs_drv.cpp
			)
	set(PERIPHERALS_INCLUDE_DIRS device/peripherals-lmx3/inc)
elseif(NOT USE_DUMMY_PERIPHERALS)
	file(GLOB PERIPHERALS_SRC_ORIG "device/peripherals/*")
	set(PERIPHERALS_SRC ${PERIPHERALS_SRC_ORIG} ${PERIPHERALS_SRC}
			device/hal/BatteryWrapper.cpp device/hal/BatteryWrapper.h
			device/hal/CameraWrapper.cpp device/hal/CameraWrapper.h
			device/hal/LedWrapper.cpp device/hal/LedWrapper.h
			device/hal/LightSensorWrapper.cpp device/hal/LightSensorWrapper.h
			device/hal/TemperatureSensorWrapper.cpp device/hal/TemperatureSensorWrapper.h
			)
endif()
message("PERIPHERALS_SRC : ${PERIPHERALS_SRC}")

## ----------------------------------------------------------- ##

# Add a custom command that produces version.cpp, plus
# a dummy output that's not actually produced, in order
# to force version.cmake to always be re-run before the build
ADD_CUSTOM_COMMAND(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/GitVersion.cpp ${CMAKE_CURRENT_BINARY_DIR}/_GitVersion.cpp
		COMMAND ${CMAKE_COMMAND} ARGS -DGIT_DIR="${CMAKE_CURRENT_SOURCE_DIR}" -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GitVersion.cmake
		)

configure_file(configure.h.in configure.h @ONLY)
set(Solarium_SOURCES
		GitVersion.cpp GitVersion.h
		SolariumApp.cpp SolariumApp.h
		httpServer/HTTPRouting.cpp httpServer/HTTPRouting.h
		httpServer/HTTPDefaultRoute.cpp httpServer/HTTPDefaultRoute.h
		httpServer/HTTPRouteInfo.cpp httpServer/HTTPRouteInfo.h
		httpServer/HTTPNotFound.cpp httpServer/HTTPNotFound.h
		httpServer/HTTPRouteSnapshot.cpp httpServer/HTTPRouteSnapshot.h
		httpServer/HTTPRouteFiles.cpp httpServer/HTTPRouteFiles.h
		httpServer/HTTPRouteLeds.cpp httpServer/HTTPRouteLeds.h
		httpServer/HTTPRouteBrowsePatient.cpp httpServer/HTTPRouteBrowsePatient.h
		httpServer/HTTPRouteBrowseAnalysis.cpp httpServer/HTTPRouteBrowseAnalysis.h
		httpServer/HTTPRouteStudyComments.cpp httpServer/HTTPRouteStudyComments.h
		device/SolariumDevice.cpp device/SolariumDevice.h
		device/ILightSensor.h
		device/ITemperatureSensor.h
		device/ICamera.h
		device/IBattery.h
		device/ILedDriver.h
		device/IPeripheralStatus.cpp device/IPeripheralStatus.h
		device/IIHMLeds.h
		device/DeviceInfoFormatter.cpp device/DeviceInfoFormatter.h
		device/dummyDevice/DummyLightSensor.cpp device/dummyDevice/DummyLightSensor.h
		device/dummyDevice/DummyTemperatureSensor.cpp device/dummyDevice/DummyTemperatureSensor.h
		device/dummyDevice/DummyBattery.cpp device/dummyDevice/DummyBattery.h
		device/dummyDevice/DummyCamera.cpp device/dummyDevice/DummyCamera.h
		device/dummyDevice/DummyLed.cpp device/dummyDevice/DummyLed.h
		device/dummyDevice/DummyOutput.h
		${PERIPHERALS_SRC}
		${CAMERA_SRC}
		${V4L2CAMERA_SRC}
		tools/simple_logging.h
		tools/logging.cpp tools/logging.h
		tools/sync_stream.cpp tools/sync_stream.h
		tools/Exception.h
		tools/DebugInfo.h
		tools/Converter.cpp tools/Converter.h
		httpServer/HTTPRouteTakeMeasure.cpp httpServer/HTTPRouteTakeMeasure.h
		measures/MeasureConfig.cpp measures/MeasureConfig.h
		measures/MeasuresManager.cpp measures/MeasuresManager.h
		measures/StripAnalysis.cpp measures/StripAnalysis.h
		measures/AnalysisResult.cpp measures/AnalysisResult.h
		measures/PatientInfo.cpp measures/PatientInfo.h
		measures/CalibrationResult.cpp measures/CalibrationResult.h
		measures/StudySnapshotResult.cpp measures/StudySnapshotResult.h
		repository/AnalysisRepository.cpp repository/AnalysisRepository.h
		repository/ReaderDB.cpp repository/ReaderDB.h
		tools/FSTools.cpp tools/FSTools.h
		repository/StudiesRepository.cpp repository/StudiesRepository.h
		httpServer/HTTPRouteDiagnostics.cpp httpServer/HTTPRouteDiagnostics.h
		httpServer/HTTPMeasureCalibration.cpp httpServer/HTTPMeasureCalibration.h
		httpServer/HTTPRouteStudies.cpp httpServer/HTTPRouteStudies.h
		httpServer/HTTPTools.cpp httpServer/HTTPTools.h
		httpServer/HTTPRouteStudySequence.cpp httpServer/HTTPRouteStudySequence.h
		httpServer/HTTPParamsFinder.cpp httpServer/HTTPParamsFinder.h
		measures/ParamsFinder.cpp measures/ParamsFinder.h
		reports/Reports.cpp reports/Reports.h
		httpServer/HTTPReports.cpp httpServer/HTTPReports.h
		models/LEDParams.cpp models/LEDParams.h
		models/LensFocal.cpp models/LensFocal.h
		tools/ImgTools.cpp tools/ImgTools.h
		repository/IRepository.cpp repository/IRepository.h
		models/SpaceInfo.cpp models/SpaceInfo.h)

add_library(SolariumLib STATIC ${Solarium_SOURCES})

## ----------------------- Solarium Lib ----------------------- ##
target_include_directories(SolariumLib PUBLIC
		${CAMERA_INCLUDE_DIRS}
		${PERIPHERALS_INCLUDE_DIRS}
		${OpenCV_INCLUDE_DIRS}
		${LIBPICA_INCLUDE_DIRS}
		${HARU_INCLUDE_DIRS}
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(SolariumLib
		Poco::Foundation Poco::Net Poco::Util
		SQLite::SQLite3
		${CAMERA_LIBRARIES}
		${OpenCV_LIBS_MINIMAL}
		${LIBPICA_LIBRARIES}
		${HARU_LIBS}
		ceres
		${EXTRA_LIBS})

target_compile_definitions(SolariumLib PRIVATE DLIB_DISABLE_ASSERTS)

## ----------------------- Solarium ----------------------- ##
add_executable(Solarium main.cpp)
target_link_libraries(Solarium SolariumLib ${OpenCV_LIBS_MINIMAL})

## ----------------------- Tests ----------------------- ##
include(CTest)
if(BUILD_TESTING)
	add_subdirectory(tests)
endif()
