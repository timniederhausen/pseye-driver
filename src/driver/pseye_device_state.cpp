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
#include "pseye/driver/pseye_device_state.hpp"
#include "pseye/driver/pseye_device_controller_ops.hpp"
#include "pseye/exception.hpp"
#include "pseye/log.hpp"

#include <chrono>
#include <span>
#include <thread>

PSEYE_NS_BEGIN

namespace detail
{

struct pseye_frame_rate_info
{
  int fps;
  std::uint8_t clock_scale;
  std::uint8_t com4;
  std::uint8_t camera_clock_select;
};

// TODO: don't use this here!
using namespace ov7725;

inline constexpr pseye_frame_rate_info supported_rates_vga[] = {
    {83, 0x01,     com4_pll_8x | com4_aec_full_window, 0x02}, // 83 FPS+: video is partly corrupt
    {75, 0x01,     com4_pll_6x | com4_aec_full_window, 0x02}, // 75 FPS or below: video is valid
    {60, 0x00,     com4_pll_4x | com4_aec_full_window, 0x04},
    {50, 0x01,     com4_pll_4x | com4_aec_full_window, 0x02},
    {40, 0x02,     com4_pll_8x | com4_aec_full_window, 0x04},
    {30, 0x04,     com4_pll_6x | com4_aec_full_window, 0x02},
    {25, 0x00, com4_pll_bypass | com4_aec_full_window, 0x02},
    {20, 0x04,     com4_pll_4x | com4_aec_full_window, 0x02},
    {15, 0x09,     com4_pll_6x | com4_aec_full_window, 0x02},
    {10, 0x09,     com4_pll_4x | com4_aec_full_window, 0x02},
    { 8, 0x02, com4_pll_bypass | com4_aec_full_window, 0x02},
    { 5, 0x04, com4_pll_bypass | com4_aec_full_window, 0x02},
    { 3, 0x06, com4_pll_bypass | com4_aec_full_window, 0x02},
    { 2, 0x09, com4_pll_bypass | com4_aec_full_window, 0x02},
};

inline constexpr pseye_frame_rate_info supported_rates_qvga[] = {
    {290, 0x00,     com4_pll_8x | com4_aec_full_window, 0x04},
    {205, 0x01,     com4_pll_8x | com4_aec_full_window, 0x02}, // 205 FPS+: video is partly corrupt
    {187, 0x01,     com4_pll_6x | com4_aec_full_window, 0x02}, // 187 FPS or below: video is valid
    {150, 0x00,     com4_pll_4x | com4_aec_full_window, 0x04},
    {137, 0x02,     com4_pll_8x | com4_aec_full_window, 0x02},
    {125, 0x01,     com4_pll_4x | com4_aec_full_window, 0x02},
    {100, 0x02,     com4_pll_8x | com4_aec_full_window, 0x04},
    { 90, 0x03,     com4_pll_6x | com4_aec_full_window, 0x02},
    { 75, 0x04,     com4_pll_6x | com4_aec_full_window, 0x02},
    { 60, 0x04,     com4_pll_8x | com4_aec_full_window, 0x04},
    { 50, 0x04,     com4_pll_4x | com4_aec_full_window, 0x02},
    { 40, 0x06,     com4_pll_6x | com4_aec_full_window, 0x03},
    { 37, 0x00, com4_pll_bypass | com4_aec_full_window, 0x04},
    { 30, 0x04,     com4_pll_4x | com4_aec_full_window, 0x04},
    { 17, 0x18,     com4_pll_8x | com4_aec_full_window, 0x02},
    { 15, 0x18,     com4_pll_6x | com4_aec_full_window, 0x02},
    { 12, 0x02, com4_pll_bypass | com4_aec_full_window, 0x04},
    { 10, 0x18,     com4_pll_4x | com4_aec_full_window, 0x02},
    {  7, 0x04, com4_pll_bypass | com4_aec_full_window, 0x04},
    {  5, 0x06, com4_pll_bypass | com4_aec_full_window, 0x04},
    {  3, 0x09, com4_pll_bypass | com4_aec_full_window, 0x04},
    {  2, 0x18, com4_pll_bypass | com4_aec_full_window, 0x02},
};

} // namespace detail

const detail::pseye_frame_rate_info& find_closest(std::span<const detail::pseye_frame_rate_info> supported_rates,
                                                  int desired)
{
  // TODO: lower_bound()?
  for (const auto& info : supported_rates) {
    if (info.fps <= desired)
      return info;
  }
  return supported_rates.back();
}

int find_valid_frame_rate(size_mode mode, int desired_fps)
{
  switch (mode) {
    case size_mode::vga: return find_closest(detail::supported_rates_vga, desired_fps).fps;
    case size_mode::qvga: return find_closest(detail::supported_rates_qvga, desired_fps).fps;
  }
  throw std::runtime_error("invalid mode");
}

void set_frame_rate(pseye_device_controller& controller, size_mode mode, int desired_fps)
{
  const detail::pseye_frame_rate_info& info = mode == size_mode::vga
                                                  ? find_closest(detail::supported_rates_vga, desired_fps)
                                                  : find_closest(detail::supported_rates_qvga, desired_fps);

  PSEYE_LOG_DEBUG("setting frame rate to {}: {} {} {}", info.fps, info.clock_scale, info.com4,
                  info.camera_clock_select);
  write_sccb_register(controller, ov7725::reg::clock_control, info.clock_scale);
  write_sccb_register(controller, ov7725::reg::com4, info.com4);
  write_register(controller, ov534::reg::camera_clock, info.camera_clock_select);
}

void set_camera_led_status(pseye_device_controller& controller, bool on)
{
  // Bit 7 seems to control the LED, so tell the bridge via |c0| that we'd like to change that
  // and submit the actual new value in |v0|
  write_register(controller, ov534::reg::gpio_c0, bit_7);
  write_register(controller, ov534::reg::gpio_v0, on ? bit_7 : 0);
  // TODO: do we need to reset |c0|? doesn't seem like it, but the other drivers do
}

void set_automatic_gain(pseye_device_controller& controller, bool val)
{
  constexpr std::uint8_t default_value = ov7725::com8_enable_fast_agc_aec | ov7725::com8_aec_step_size_vblank |
                                         ov7725::com8_enable_banding_filter | ov7725::com8_enable_aec_below_banding;
  constexpr std::uint8_t defect_correction =
      ov7725::dsp_ctrl1_auto_correct_black_defect | ov7725::dsp_ctrl1_auto_correct_white_defect;
  if (val) {
    write_sccb_register(controller, ov7725::reg::com8,
                        default_value | ov7725::com8_enable_agc | ov7725::com8_enable_awb | ov7725::com8_enable_aec);
    write_sccb_register(controller, ov7725::reg::dsp_ctrl1,
                        read_sccb_register(controller, ov7725::reg::dsp_ctrl1) | defect_correction);
  } else {
    write_sccb_register(controller, ov7725::reg::com8, default_value);
    write_sccb_register(controller, ov7725::reg::dsp_ctrl1,
                        read_sccb_register(controller, ov7725::reg::dsp_ctrl1) & ~defect_correction);
  }
}

void set_auto_white_balance(pseye_device_controller& controller, bool val)
{
  constexpr std::uint8_t wbc_threshold_2 = 10;
  // TODO: what is bit 5 for?
  if (val) {
    write_sccb_register(controller, ov7725::reg::awb_ctrl0,
                        ov7725::awb_ctrl0_enable_awb_gain | ov7725::awb_ctrl0_calculate_awb | bit_5);
  } else {
    write_sccb_register(controller, ov7725::reg::awb_ctrl0,
                        ov7725::awb_ctrl0_enable_awb_gain | bit_5 | wbc_threshold_2);
  }
}

void set_gain(pseye_device_controller& controller, std::uint8_t val)
{
  write_sccb_register(controller, ov7725::reg::gain, val);
}

void set_exposure(pseye_device_controller& controller, std::uint16_t val)
{
  // TODO: what's right?
  // write_sccb_register(controller, ov7725::reg::aec_high, val >> 8);
  // write_sccb_register(controller, ov7725::reg::aec_low, val & 0xff);
  write_sccb_register(controller, ov7725::reg::aec_high, val >> 7);
  write_sccb_register(controller, ov7725::reg::aec_low, val << 1);
}

void set_denoise_threshold(pseye_device_controller& controller, std::uint8_t val)
{
  write_sccb_register(controller, ov7725::reg::denoise_thresh_ctrl, val);
  write_sccb_register(controller, ov7725::reg::denoise_thresh, val);
}

void set_contrast_gain(pseye_device_controller& controller, std::uint8_t val)
{
  write_sccb_register(controller, ov7725::reg::contrast_gain, val);
}

void set_saturation_gain(pseye_device_controller& controller, std::uint8_t val)
{
  write_sccb_register(controller, ov7725::reg::u_sat, val);
  write_sccb_register(controller, ov7725::reg::v_sat, val);
}

void set_brightness(pseye_device_controller& controller, std::uint8_t val)
{
  write_sccb_register(controller, ov7725::reg::brightness, val);
}

void set_hue(pseye_device_controller& controller, float val)
{
  int hue_cos = static_cast<int>(std::cosf(val) * 0x80);
  int hue_sin = static_cast<int>(std::sinf(val) * 0x80);

  const auto sign_bits = read_sccb_register(controller, ov7725::reg::hue_sign);
  if (hue_sin < 0) {
    write_sccb_register(controller, ov7725::reg::hue_sign, sign_bits | bit_1);
    hue_sin = -hue_sin;
  } else {
    write_sccb_register(controller, ov7725::reg::hue_sign, sign_bits & ~bit_1);
  }

  write_sccb_register(controller, ov7725::reg::hue_cos, hue_cos);
  write_sccb_register(controller, ov7725::reg::hue_sin, hue_sin);
}

void set_red_balance_target(pseye_device_controller& controller, std::uint8_t val)
{
  write_sccb_register(controller, ov7725::reg::tgt_r, val);
}

void set_blue_balance_target(pseye_device_controller& controller, std::uint8_t val)
{
  write_sccb_register(controller, ov7725::reg::tgt_b, val);
}

void set_green_balance_target(pseye_device_controller& controller, std::uint8_t val)
{
  // actually Gb
  write_sccb_register(controller, ov7725::reg::tgt_gb, val);
}

void set_flip_and_test_pattern(pseye_device_controller& controller, bool horizontal, bool vertical, bool test_pattern)
{
  auto val = read_sccb_register(controller, ov7725::reg::com3);
  val &= ~(ov7725::com3_vflip_image | ov7725::com3_hflip_image | ov7725::com3_enable_test_pattern);
  if (vertical)
    val |= ov7725::com3_vflip_image;
  if (horizontal)
    val |= ov7725::com3_hflip_image;
  if (test_pattern)
    val |= ov7725::com3_enable_test_pattern;
  write_sccb_register(controller, ov7725::reg::com3, val);
}

void apply_state(pseye_device_controller& controller, const pseye_device_state& state)
{
  set_frame_rate(controller, state.mode(), state.rate);
  set_automatic_gain(controller, state.enable_auto_gain);
  set_auto_white_balance(controller, state.enable_auto_white_balance);
  set_gain(controller, state.gain);
  set_exposure(controller, state.exposure);
  set_denoise_threshold(controller, state.denoise_threshold);
  set_hue(controller, state.hue);
  set_brightness(controller, state.brightness);
  set_contrast_gain(controller, state.contrast);
  set_saturation_gain(controller, state.saturation);
  set_blue_balance_target(controller, state.blue_blc);
  set_red_balance_target(controller, state.red_blc);
  set_green_balance_target(controller, state.green_blc);
  set_flip_and_test_pattern(controller, state.flip_h, state.flip_v, state.enable_test_pattern);
}

PSEYE_NS_END
