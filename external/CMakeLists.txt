# Copyright (c) Tim Niederhausen (tim@rnc-ag.de)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

set(LIBUSB_INSTALL_TARGETS OFF)
add_subdirectory(libusb)

add_subdirectory(fmt)

# TODO: don't want to patch that!
# add_subdirectory(simd/prj/cmake)
set(SIMD_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/simd)
file(GLOB_RECURSE SIMD_LIB_SRC ${SIMD_ROOT}/src/Simd/SimdLib.cpp ${SIMD_ROOT}/src/Simd/*.h)
file(GLOB_RECURSE SIMD_BASE_SRC ${SIMD_ROOT}/src/Simd/SimdBaseBgr*.cpp ${SIMD_ROOT}/src/Simd/SimdBaseBayer*.cpp ${SIMD_ROOT}/src/Simd/SimdBaseUy*.cpp ${SIMD_ROOT}/src/Simd/SimdBaseYuv*.cpp)
file(GLOB_RECURSE SIMD_SSE41_SRC ${SIMD_ROOT}/src/Simd/SimdSse41Bgr*.cpp ${SIMD_ROOT}/src/Simd/SimdSse41Bayer*.cpp  ${SIMD_ROOT}/src/Simd/SimdSse41Uy*.cpp  ${SIMD_ROOT}/src/Simd/SimdSse41Yuv*.cpp)
#file(GLOB_RECURSE SIMD_AVX1_SRC ${SIMD_ROOT}/src/Simd/SimdAvx1*.cpp)
#file(GLOB_RECURSE SIMD_AVX2_SRC ${SIMD_ROOT}/src/Simd/SimdAvx2*.cpp)
add_library(minisimd STATIC ${SIMD_LIB_SRC} ${SIMD_BASE_SRC} ${SIMD_SSE41_SRC} ${SIMD_AVX1_SRC} ${SIMD_AVX2_SRC})
target_compile_definitions(minisimd PUBLIC SIMD_AVX512BW_DISABLE SIMD_AVX512VNNI_DISABLE SIMD_AVX512BF16_DISABLE SIMD_AMXBF16_DISABLE PRIVATE SIMD_VERSION="unknown")
target_include_directories(minisimd PUBLIC ${SIMD_ROOT}/src)

# TODO: don't want to patch that!
# add_subdirectory(libdshowcapture)
set(libdshowcapture_SOURCES
    libdshowcapture/external/capture-device-support/Library/EGAVResult.cpp
    libdshowcapture/external/capture-device-support/Library/ElgatoUVCDevice.cpp
    libdshowcapture/external/capture-device-support/Library/win/EGAVHIDImplementation.cpp
    libdshowcapture/external/capture-device-support/SampleCode/DriverInterface.cpp
    libdshowcapture/source/capture-filter.cpp
    libdshowcapture/source/output-filter.cpp
    libdshowcapture/source/dshowcapture.cpp
    libdshowcapture/source/dshowencode.cpp
    libdshowcapture/source/device.cpp
    libdshowcapture/source/device-vendor.cpp
    libdshowcapture/source/encoder.cpp
    libdshowcapture/source/dshow-base.cpp
    libdshowcapture/source/dshow-demux.cpp
    libdshowcapture/source/dshow-enum.cpp
    libdshowcapture/source/dshow-formats.cpp
    libdshowcapture/source/dshow-media-type.cpp
    libdshowcapture/source/dshow-encoded-device.cpp
    libdshowcapture/source/log.cpp)
set(libdshowcapture_HEADERS
    libdshowcapture/dshowcapture.hpp
    libdshowcapture/source/external/IVideoCaptureFilter.h
    libdshowcapture/source/capture-filter.hpp
    libdshowcapture/source/output-filter.hpp
    libdshowcapture/source/device.hpp
    libdshowcapture/source/encoder.hpp
    libdshowcapture/source/dshow-base.hpp
    libdshowcapture/source/dshow-demux.hpp
    libdshowcapture/source/dshow-device-defs.hpp
    libdshowcapture/source/dshow-enum.hpp
    libdshowcapture/source/dshow-formats.hpp
    libdshowcapture/source/dshow-media-type.hpp
    libdshowcapture/source/log.hpp)

add_library(libdshowcapture STATIC ${libdshowcapture_SOURCES} ${libdshowcapture_HEADERS})
target_include_directories(libdshowcapture INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libdshowcapture)
target_include_directories(libdshowcapture PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libdshowcapture/external/capture-device-support/Library)
target_compile_definitions(libdshowcapture PRIVATE _UP_WINDOWS=1 UNICODE)
target_link_libraries(libdshowcapture PRIVATE setupapi strmiids ksuser winmm wmcodecdspuuid)
