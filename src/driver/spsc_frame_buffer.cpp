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
#include "pseye/driver/spsc_frame_buffer.hpp"

PSEYE_NS_BEGIN

spsc_frame_buffer::spsc_frame_buffer(std::uint32_t num_frames, std::size_t frame_size)
  : frame_size_(frame_size)
  , frames_(new uint8_t[frame_size * num_frames])
  , num_frames_(num_frames)
{
}

std::span<uint8_t> spsc_frame_buffer::writable_frame()
{
  std::lock_guard lock(mutex_);
  return {&frames_[head_ * frame_size_], frame_size_};
}

void spsc_frame_buffer::finish_writing()
{
  std::unique_lock lock(mutex_);
  // Unlike other SPSC queues we simply keep overwriting the last frame we wrote if we end up full.
  // That is, we do nothing here and let the producer just keep writing to the frame buffer it has.
  const bool is_full = used_ == num_frames_ - 1;
  if (!is_full) {
    ++used_;
    head_ = (head_ + 1) % num_frames_;
    new_frame_condition_.notify_one();
  }
}

std::span<std::uint8_t> spsc_frame_buffer::readable_frame_wait()
{
  std::unique_lock lock(mutex_);
  new_frame_condition_.wait(lock, [this]() {
    return used_ != 0;
  });
  return readable_frame_nowait_locked();
}

void spsc_frame_buffer::finish_reading()
{
  std::unique_lock lock(mutex_);
  used_--;
  tail_ = (tail_ + 1) % num_frames_;
}

std::span<std::uint8_t> spsc_frame_buffer::readable_frame_nowait_locked()
{
  return std::span(&frames_[frame_size_ * tail_], frame_size_);
}

PSEYE_NS_END
