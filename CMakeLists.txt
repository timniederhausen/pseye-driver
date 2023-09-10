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
cmake_minimum_required(VERSION 3.25)
project(pseye-driver VERSION 0.4.0 LANGUAGES CXX)

include(CMakeDependentOption)

# https://cmake.org/cmake/help/latest/policy/CMP0077.html
cmake_policy(SET CMP0077 NEW)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  add_compile_options(/W3 /wd5105 /utf-8
    /Brepro
    /MP
    /permissive-
    /Zc:__cplusplus
    /Zc:preprocessor
    "$<$<CONFIG:DEBUG>:/DDEBUG=1;/D_DEBUG=1>"
    "$<$<CONFIG:RELWITHDEBINFO>:/Ob2>"
    "$<$<CONFIG:RELWITHDEBINFO>:/GL>"
    "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/Zc:inline;/Zc:char8_t->")

  add_compile_definitions(UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS $<$<CONFIG:DEBUG>:DEBUG>
                          $<$<CONFIG:DEBUG>:_DEBUG>)

  add_link_options("$<$<NOT:$<CONFIG:Debug>>:/OPT\:REF>" "$<$<CONFIG:Debug>:/INCREMENTAL\:NO>"
                   "$<$<CONFIG:RelWithDebInfo>:/INCREMENTAL\:NO>" "$<$<CONFIG:RelWithDebInfo>:/OPT\:ICF>"
                   "$<$<CONFIG:RelWithDebInfo>:/LTCG\:INCREMENTAL>" /Brepro)
endif()

set(BUILD_SHARED_LIBS OFF)

add_subdirectory(external)
add_subdirectory(src)
