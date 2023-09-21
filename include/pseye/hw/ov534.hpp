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
#ifndef PSEYE_HW_OV534_HPP
#define PSEYE_HW_OV534_HPP

#include "pseye/detail/config.hpp"

#if PSEYE_HAS_PRAGMA_ONCE
#pragma once
#endif

PSEYE_HARDWARE_NS_BEGIN

// OmniVision OV534-538
namespace ov534
{

enum class reg : std::uint8_t
{
  // SCCB & micro controller
  ms_speed = 0xf0,
  ms_id,
  ms_address,
  ms_data_out,
  ms_data_in,
  ms_ctrl,
  ms_status,
  reserved_f7,
  reserved_f8,
  mc_bist,
  mc_al,
  mc_ah,
  mc_d,
  sample,
  ac_bist,

  // system controller
  reset0 = 0xe0,
  reset1,
  clock0,
  clock1,
  reserved_e4,
  camera_clock,
  user,
  sys_ctrl,
  step_low,
  step_high,
  max_low,
  max_high,
  irq_m0,
  irq_m1,
  irq0,
  irq1,
  dif = 0x35,
  cif_frame = 0x3b,
  ipu_frame,
  phy_bist0,
  phy_bist1,
  phy_bist2,

  // GPIO
  gpio_n0 = 0x20,
  gpio_c0,
  gpio_i0,
  gpio_v0,
  gpio_n1,
  gpio_c1,
  gpio_i1,
  gpio_v1,
  sensor_s1,
  sensor_c1,
  sensor_i1,
  sensor_v1,
  sensor_s0,
  sensor_c0,
  sensor_i0,
  sensor_v0,
  regulator_c0,
  regulator_c1,
  sd_pga,
  sd_c0,
  sd_c1,
  gpio_s0 = 0x39,
  gpio_s1,

  // Video data
  video_data_addr = 0x1c,
  video_data_value,
  unknown_1e,
  unknown_1f,

  // Unknown
  unknown_50 = 0x50,
  unknown_76 = 0x76,
  unknown_88 = 0x88,
  unknown_89,
  unknown_8d = 0x8d,
  unknown_8e,
  unknown_90 = 0x90,
  unknown_91,
  unknown_92 = 0x92,
  unknown_93,
  unknown_94,
  unknown_95,
  unknown_indirect_addr,
  unknown_indirect_value,
  horizontal_blocks = 0xc0,
  vertical_blocks,
  unknown_c2,
  unknown_c3,
};
static_assert(static_cast<std::uint8_t>(reg::ac_bist) == 0xfe);
static_assert(static_cast<std::uint8_t>(reg::irq1) == 0xef);
static_assert(static_cast<std::uint8_t>(reg::phy_bist2) == 0x3f);
static_assert(static_cast<std::uint8_t>(reg::sd_c1) == 0x34);
static_assert(static_cast<std::uint8_t>(reg::gpio_s1) == 0x3a);
static_assert(sizeof(register_setting<reg>) == 2);

inline constexpr std::uint8_t ms_ctrl_2byte_write = 0x33;
inline constexpr std::uint8_t ms_ctrl_3byte_write = 0x37;
inline constexpr std::uint8_t ms_ctrl_2byte_read = 0xf9;

inline constexpr std::uint8_t ms_status_command_busy = bit_0;
inline constexpr std::uint8_t ms_status_cycle_complete = bit_1;
inline constexpr std::uint8_t ms_status_slave_ack = 0 << 2;
inline constexpr std::uint8_t ms_status_slave_nak = 1 << 2;

inline constexpr std::uint8_t sys_ctrl_camera_power_down = bit_0;
inline constexpr std::uint8_t sys_ctrl_suspend_enable = bit_1;
inline constexpr std::uint8_t sys_ctrl_wakeup_enable = bit_2;
inline constexpr std::uint8_t sys_ctrl_reset_3 = bit_3;
inline constexpr std::uint8_t sys_ctrl_mc_wakeup_reset_enable = bit_4;
inline constexpr std::uint8_t sys_ctrl_reset_5 = bit_5;
inline constexpr std::uint8_t sys_ctrl_launch_register_reset = bit_6;
inline constexpr std::uint8_t sys_ctrl_launch_suspend = bit_7;

inline constexpr std::uint8_t reset0_cif = bit_0; // Camera and ADC Interface
inline constexpr std::uint8_t reset0_isp = bit_1; // Image Signal Processing
inline constexpr std::uint8_t reset0_compression = bit_2;
inline constexpr std::uint8_t reset0_vfifo = bit_3;
inline constexpr std::uint8_t reset0_audio = bit_4;
inline constexpr std::uint8_t reset0_dif = bit_5;
inline constexpr std::uint8_t reset0_audio_interface = bit_6;
inline constexpr std::uint8_t reset0_sccb = bit_7;

namespace detail
{
inline constexpr std::uint8_t init_sys_ctrl =
    sys_ctrl_suspend_enable | sys_ctrl_wakeup_enable | sys_ctrl_reset_3 | sys_ctrl_reset_5;
}

// default includes sys_ctrl_camera_power
//{ reg::sys_ctrl, sys_ctrl_suspend_enable | sys_ctrl_reset_3 | sys_ctrl_mc_wakeup_reset_enable | sys_ctrl_reset_5 |
//  0b111010 },
inline constexpr register_setting<reg> initialization_data[] = {
    {            reg::unknown_92,                                                    0x01},
    {            reg::unknown_93,                                                    0x18},
    {            reg::unknown_94,                                                    0x10},
    {            reg::unknown_95,                                                    0x10},

    {                reg::clock0,                                                    0x00},
    {              reg::sys_ctrl, detail::init_sys_ctrl | sys_ctrl_mc_wakeup_reset_enable},

    { reg::unknown_indirect_addr,                                                    0x00},
    {reg::unknown_indirect_value,                                                    0x20},
    {reg::unknown_indirect_value,                                                    0x20},
    {reg::unknown_indirect_value,                                                    0x20},
    {reg::unknown_indirect_value,                                                    0x0A},
    {reg::unknown_indirect_value,                                                    0x3F},
    {reg::unknown_indirect_value,                                                    0x4A},
    {reg::unknown_indirect_value,                                                    0x20},
    {reg::unknown_indirect_value,                                                    0x15},
    {reg::unknown_indirect_value,                                                    0x0B},

    {                 reg::sd_c1,                                                   0b101},
    {                reg::clock1,                                                   0b100},
    {              reg::sys_ctrl,                                   detail::init_sys_ctrl},
    {          reg::regulator_c1,                                                    0xF9},
    {               reg::gpio_c1,                                                    0x42},
    {               reg::gpio_c0,                                                    0xF0},
    {          reg::camera_clock,                                                   0b100},

    {            reg::unknown_1f,                                                    0x81},

    {            reg::unknown_50,                                                    0x89},
    {            reg::unknown_76,                                                    0x00},
    {            reg::unknown_89,                                                    0x00},
    {            reg::unknown_8d,                                                    0x00},
    {            reg::unknown_8e,                                                    0x00},
};

inline constexpr register_setting<reg> start_yuv[] = {
    {reg::unknown_88,                0x00},
    {reg::unknown_8d, 0b11100 /*0b01000*/},
    {reg::unknown_8e,          0b10000000},

    {reg::unknown_c2,           0b0001100},
    {reg::unknown_c3,           0b1101001},
};

inline constexpr register_setting<reg> start_bayer[] = {
    {reg::unknown_88, 0x08},
    {reg::unknown_8d, 0x00},
    {reg::unknown_8e, 0x00},

    {reg::unknown_c2, 0x01},
    {reg::unknown_c3, 0x01},
};

inline constexpr register_setting<reg> start_vga[] = {
    {reg::horizontal_blocks, 640 / 8},
    {  reg::vertical_blocks, 480 / 8},
};

inline constexpr register_setting<reg> start_qvga[] = {
    {reg::horizontal_blocks, 320 / 8},
    {  reg::vertical_blocks, 240 / 8},
};

enum class video_format
{
  raw8,
  raw10,
  raw16,
  reserved_3,
  yuv422,
  yuv411_yyyy_yuyv,
  yuv411_yuyv_yyyy,
  yuv411_uyy_vyy,
};
enum class transfer
{
  bulk,
  iso
};

struct video_data_configuration
{
  static constexpr std::uint8_t enable_uvc_header_format = bit_3;
  static constexpr std::uint8_t auto_frame_size = bit_3;

  std::uint8_t format = 0;
  std::uint8_t payload_size[2] = {0x00, 0x80};     // high, low
  std::uint8_t frame_size[3] = {0x0a, 0x00, 0x00}; // high, mid, low
  // XXX: don't want to overwrite those usually(?)
  std::uint8_t internal_header_bytes[4] = {0xff, 0x5a, 0xa5, 0x00};
  std::uint8_t cntl[2] = {0, bit_1 | bit_2};
};
static_assert(sizeof(video_data_configuration) == 0xC);

constexpr video_data_configuration make_video_data_settings(video_format fmt,
                                                            transfer tf,
                                                            std::uint16_t payload_size,
                                                            std::uint32_t frame_size,
                                                            bool enable_still_image_header = false,
                                                            bool even_odd_byte_swap = false)
{
  video_data_configuration cfg;
  cfg.format = (static_cast<std::uint8_t>(fmt) << 4) | (static_cast<std::uint8_t>(tf) << 3) |
               (static_cast<std::uint8_t>(enable_still_image_header) << 2) |
               (static_cast<std::uint8_t>(even_odd_byte_swap) << 7);
  cfg.payload_size[0] = payload_size >> 8;
  cfg.payload_size[1] = payload_size & 0xff;
  // XXX: does this even matter with auto_frame_size? doesn't seem like it
  cfg.frame_size[0] = (frame_size >> 16) & 0xff;
  cfg.frame_size[1] = (frame_size >> 8) & 0xff;
  cfg.frame_size[2] = frame_size & 0xff;
  cfg.cntl[0] |= video_data_configuration::enable_uvc_header_format;
  // XXX: Without auto_frame_size we don't get anything
  cfg.cntl[1] |= video_data_configuration::auto_frame_size;
  return cfg;
}

} // namespace ov534

PSEYE_HARDWARE_NS_END

#endif
