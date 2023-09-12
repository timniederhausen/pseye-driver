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
#include "pseye/driver/uvc_frame_processor.hpp"

#include "pseye/log.hpp"

PSEYE_NS_BEGIN

inline constexpr std::uint8_t UVC_STREAM_FID = 1 << 0;
inline constexpr std::uint8_t UVC_STREAM_EOF = 1 << 1;
inline constexpr std::uint8_t UVC_STREAM_PTS = 1 << 2;
inline constexpr std::uint8_t UVC_STREAM_SCR = 1 << 3;
inline constexpr std::uint8_t UVC_STREAM_RES = 1 << 4;
inline constexpr std::uint8_t UVC_STREAM_STI = 1 << 5;
inline constexpr std::uint8_t UVC_STREAM_ERR = 1 << 6;
inline constexpr std::uint8_t UVC_STREAM_EOH = 1 << 7;

uvc_frame_processor::status uvc_frame_processor::put(std::span<const std::uint8_t> data)
{
  if (data.empty())
    return status::need_data;

  const auto header_len = data[0];
  if (header_len < 2 || data.size() < header_len) {
    PSEYE_LOG_ERROR("bad header: {} {}", header_len, data.size());
    discard_frame_ = true;
    return status::need_data;
  }

  if (0 != (data[1] & UVC_STREAM_ERR)) {
    PSEYE_LOG_ERROR("ERR bit in header: {:#02x}", data[1]);
    discard_frame_ = true;
    return status::need_data;
  }

  if (0 == (data[1] & UVC_STREAM_PTS)) {
    PSEYE_LOG_ERROR("no PTS in header: {:#02x}", data[1]);
    discard_frame_ = true;
    return status::need_data;
  }

  const std::uint32_t this_pts = (data[5] << 24) | (data[4] << 16) | (data[3] << 8) | data[2];
  const std::uint8_t this_fid = (data[1] & UVC_STREAM_FID) ? 1 : 0;

  if (this_pts != last_pts_ || this_fid != last_fid_) {
    // Changed PTS or toggled frame ID bit means new frame!
    frame_len_ = 0;
    discard_frame_ = false;
    last_pts_ = this_pts;
    last_fid_ = this_fid;

    if (current_frame_.empty())
      return status::need_buffer;
  } else if (0 != (data[1] & UVC_STREAM_EOF)) {
    // After an EOF packet, we always begin a new frame.
    last_pts_ = 0;

    // If this frame doesn't have the correct size, just drop it entirely!
    if (frame_len_ + (data.size() - header_len) != current_frame_.size()) {
      PSEYE_LOG_DEBUG("incorrect final frame size: {} + {} != {}", frame_len_, (data.size() - header_len),
                      current_frame_.size());
      discard_frame_ = true;
    }
  }

  // no progress to be made now, just ask for more data
  if (discard_frame_)
    return status::need_data;

  const auto to_copy = data.size() - header_len;
  if (frame_len_ + to_copy <= current_frame_.size()) {
    std::memcpy(&current_frame_[frame_len_], &data[header_len], to_copy);
    frame_len_ += to_copy;

    if (0 != (data[1] & UVC_STREAM_EOF)) {
      frame_len_ = 0;
      return status::frame_complete;
    }
  } else {
    PSEYE_LOG_DEBUG("frame overflow: {} + {} > {}", frame_len_, to_copy, current_frame_.size());
    discard_frame_ = true;
  }

  return status::need_data;
}

PSEYE_NS_END
