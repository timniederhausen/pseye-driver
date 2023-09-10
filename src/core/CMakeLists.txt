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
project(pseye-core)

add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME}
  PUBLIC
  ${CMAKE_SOURCE_DIR}/include/pseye/detail/config.hpp
  ${CMAKE_SOURCE_DIR}/include/pseye/detail/hardware.hpp
  ${CMAKE_SOURCE_DIR}/include/pseye/log.hpp
  ${CMAKE_SOURCE_DIR}/include/pseye/exception.hpp
  ${CMAKE_SOURCE_DIR}/include/pseye/pixel_format.hpp
  PRIVATE
  log.cpp
  pixel_format.cpp
)
add_library(pseye::core ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt PRIVATE minisimd)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/external/wil/include)
