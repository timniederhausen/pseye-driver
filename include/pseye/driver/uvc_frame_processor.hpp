/// @copyright Copyright (c) Tim Niederhausen (tim@rnc-ag.de)
///
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#ifndef PSEYE_DRIVER_UVCFRAMEPROCESSOR_HPP
#define PSEYE_DRIVER_UVCFRAMEPROCESSOR_HPP

#include "pseye/detail/config.hpp"

#if PSEYE_HAS_PRAGMA_ONCE
# pragma once
#endif

#include <span>

PSEYE_NS_BEGIN

// Class for unpacking a UVC-wrapped frame data stream
class uvc_frame_processor
{
public:
  enum class status
  {
    need_data,
    need_buffer,
    frame_complete,
  };

  void set_frame(std::span<std::uint8_t> frame) { current_frame_ = frame; }
  status put(std::span<const std::uint8_t> data);

private:
  std::span<std::uint8_t> current_frame_;
  std::size_t frame_len_ = 0;
  bool discard_frame_ = false;
  std::uint32_t last_pts_ = 0;
  std::uint8_t last_fid_ = 0;
};

PSEYE_NS_END

#endif
