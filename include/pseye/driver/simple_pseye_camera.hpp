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
#ifndef PSEYE_DRIVER_SIMPLEPSEYECAMERA_HPP
#define PSEYE_DRIVER_SIMPLEPSEYECAMERA_HPP

#include "pseye/detail/config.hpp"

#if PSEYE_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "pseye/driver/pseye_device_controller.hpp"
#include "pseye/driver/pseye_device_state.hpp"
#include "pseye/driver/usb_transfer_controller.hpp"
#include "pseye/driver/uvc_frame_processor.hpp"
#include "pseye/pixel_format.hpp"

#include <memory>

PSEYE_NS_BEGIN

class spsc_frame_buffer;

class simple_pseye_camera
{
public:
  simple_pseye_camera(libusb_device* device, const pseye_device_state& initial_state);
  simple_pseye_camera(libusb_device_handle* device, const pseye_device_state& initial_state);
  ~simple_pseye_camera();

  simple_pseye_camera(const simple_pseye_camera&) = delete;
  simple_pseye_camera& operator=(const simple_pseye_camera&) = delete;

  void start(size_mode mode, int frame_rate = 75, pixel_format internal_format = pixel_format::grbg8);
  void stop();

  const pseye_device_state& state() const { return state_; }
  spsc_frame_buffer& frame_buffer() { return *frame_buffer_; }
  bool is_active() const { return is_active_; }

private:
  void initialize();
  void process_transfer_data(std::span<uint8_t> data);

  pseye_device_controller handle_;
  pseye_device_state state_;
  usb_transfer_controller transfer_;
  uvc_frame_processor processor_;
  std::uint32_t payload_size_ = 0;
  std::unique_ptr<spsc_frame_buffer> frame_buffer_;
  bool is_active_ = false;
};

PSEYE_NS_END

#endif
