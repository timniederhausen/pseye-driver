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
project(pseye-driver)

add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME}
  PUBLIC
  ${CMAKE_SOURCE_DIR}/include/pseye/driver/spsc_frame_buffer.hpp
  ${CMAKE_SOURCE_DIR}/include/pseye/driver/pseye_device_controller.hpp
  ${CMAKE_SOURCE_DIR}/include/pseye/driver/pseye_device_controller_ops.hpp
  ${CMAKE_SOURCE_DIR}/include/pseye/driver/pseye_device_state.hpp
  ${CMAKE_SOURCE_DIR}/include/pseye/driver/simple_pseye_camera.hpp
  ${CMAKE_SOURCE_DIR}/include/pseye/driver/usb_context.hpp
  ${CMAKE_SOURCE_DIR}/include/pseye/driver/usb_transfer_controller.hpp
  ${CMAKE_SOURCE_DIR}/include/pseye/driver/uvc_frame_processor.hpp
  ${CMAKE_SOURCE_DIR}/include/pseye/hw/ov534.hpp
  ${CMAKE_SOURCE_DIR}/include/pseye/hw/ov7725.hpp
  PRIVATE
  spsc_frame_buffer.cpp
  pseye_device_controller.cpp
  pseye_device_controller_ops.cpp
  pseye_device_state.cpp
  simple_pseye_camera.cpp
  usb_context.cpp
  usb_transfer_controller.cpp
  uvc_frame_processor.cpp)
add_library(pseye::driver ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} PUBLIC pseye::core PRIVATE usb-1.0)
