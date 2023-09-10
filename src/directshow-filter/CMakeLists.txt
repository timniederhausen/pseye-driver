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
project(pseye-directshow-filter)

add_library(${PROJECT_NAME} SHARED
  camera-filter.cpp
  camera-filter.hpp
  module.cpp
  module.def
  module.hpp
  module-guid.hpp
  registry.cpp
  registry.hpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC pseye::core pseye::driver libdshowcapture)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  # XXX: The utf8 overloads are fine and that's all we use!
  "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING>"
)
