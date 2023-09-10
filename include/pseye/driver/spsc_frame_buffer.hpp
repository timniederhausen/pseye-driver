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
#ifndef PSEYE_DRIVER_SPSCFRAMEQUEUE_HPP
#define PSEYE_DRIVER_SPSCFRAMEQUEUE_HPP

#include "pseye/detail/config.hpp"

#if PSEYE_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <chrono>
#include <memory>
#include <mutex>
#include <span>

PSEYE_NS_BEGIN

class spsc_frame_buffer
{
public:
  spsc_frame_buffer(std::uint32_t num_frames, std::size_t frame_size);
  ~spsc_frame_buffer() = default;

  std::span<uint8_t> writable_frame();
  void finish_writing();

  std::span<std::uint8_t> readable_frame_wait();

  template <class Rep, class Period>
  std::span<std::uint8_t> readable_frame_wait_for(const std::chrono::duration<Rep, Period>& rel_time)
  {
    std::unique_lock lock(mutex_);
    if (new_frame_condition_.wait_for(lock, rel_time, [this]() {
          return used_ != 0;
        }))
      return readable_frame_nowait_locked();
    return {};
  }

  template <class Clock, class Duration>
  std::span<std::uint8_t> readable_frame_wait_until(const std::chrono::time_point<Clock, Duration>& abs_time)
  {
    std::unique_lock lock(mutex_);
    if (new_frame_condition_.wait_until(lock, abs_time, [this]() {
          return used_ != 0;
        }))
      return readable_frame_nowait_locked();
    return {};
  }
  void finish_reading();

private:
  std::span<std::uint8_t> readable_frame_nowait_locked();

  std::size_t frame_size_;
  std::unique_ptr<uint8_t[]> frames_;
  std::uint32_t num_frames_;

  std::uint32_t head_ = 0;
  std::uint32_t tail_ = 0;
  std::uint32_t used_ = 0;

  std::mutex mutex_;
  std::condition_variable new_frame_condition_;
};

PSEYE_NS_END

#endif
