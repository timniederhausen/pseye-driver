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
#ifndef PSEYE_DRIVER_PSEYEDEVICESTATE_HPP
#define PSEYE_DRIVER_PSEYEDEVICESTATE_HPP

#include "pseye/detail/config.hpp"

#if PSEYE_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "pseye/pixel_format.hpp"

#include <cstddef>
#include <cstdint>

PSEYE_NS_BEGIN

class pseye_device_controller;

enum class size_mode
{
  vga,  // 640x480
  qvga, // 320x240
};

/// Low-level state of the PlayStation Eye camera device.
struct pseye_device_state
{
  size_mode mode() const { return width != 320 ? size_mode::vga : size_mode::qvga; }

  std::uint32_t width = 0, height = 0;
  // see find_valid_frame_rate() for valid options
  std::uint32_t rate = 60;
  pixel_format format = pixel_format::grbg8;
  bool enable_auto_gain = true;
  bool enable_auto_white_balance = true;
  std::uint8_t gain = 0;              // [0, 255]
  std::uint8_t exposure = 64;         // [0, 255]
  std::uint8_t denoise_threshold = 0; // [0, 255]
  std::uint8_t hue = 143;             // [0, 255]
  std::uint8_t brightness = 0;        // [0, 255]
  std::uint8_t contrast = 64;         // [0, 255]
  std::uint8_t blue_blc = 128;        // [0, 255]
  std::uint8_t red_blc = 128;         // [0, 255]
  std::uint8_t green_blc = 128;       // [0, 255]
  bool flip_h = false, flip_v = false;
  bool enable_test_pattern = false;
};

int find_valid_frame_rate(size_mode mode, int desired_fps);
void set_frame_rate(pseye_device_controller& controller, size_mode mode, int desired_fps);
void set_camera_led_status(pseye_device_controller& controller, bool on);
void set_automatic_gain(pseye_device_controller& controller, bool val);
void set_auto_white_balance(pseye_device_controller& controller, bool val);
void set_gain(pseye_device_controller& controller, std::uint8_t val);
void set_exposure(pseye_device_controller& controller, std::uint16_t val);
void set_denoise_threshold(pseye_device_controller& controller, std::uint8_t val);
void set_contrast_gain(pseye_device_controller& controller, std::uint8_t val);
void set_brightness(pseye_device_controller& controller, std::uint8_t val);
void set_hue(pseye_device_controller& controller, uint8_t val);
void set_red_balance_target(pseye_device_controller& controller, std::uint8_t val);
void set_blue_balance_target(pseye_device_controller& controller, std::uint8_t val);
void set_green_balance_target(pseye_device_controller& controller, std::uint8_t val);
void set_flip_and_test_pattern(pseye_device_controller& controller, bool horizontal, bool vertical, bool test_pattern);
void apply_state(pseye_device_controller& controller, const pseye_device_state& state);

PSEYE_NS_END

#endif
