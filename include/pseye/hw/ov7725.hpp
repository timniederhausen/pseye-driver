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
#ifndef PSEYE_HW_OV7725_HPP
#define PSEYE_HW_OV7725_HPP

#include "pseye/detail/config.hpp"

#if PSEYE_HAS_PRAGMA_ONCE
#pragma once
#endif

PSEYE_HARDWARE_NS_BEGIN

// OmniVision OV7725
namespace ov7725
{

enum class reg : std::uint8_t
{
  gain,
  gain_blue,
  gain_red,
  gain_green,
  reserved_04,
  u_b_avg,
  y_gb_avg,
  v_r_avg,
  aec_high,
  com2,
  pid,
  ver,
  com3,
  com4,
  com5,
  com6,
  aec_low,
  clock_control,
  com7,
  com8,
  com9,
  com10,
  reg16,
  horizontal_start,
  horizontal_size,
  vertical_start,
  vertical_size,
  pshift,
  manufacturer_id_high,
  manufacturer_id_low,
  reserved_1e,
  fine_aec,
  com11,
  reserved_21,
  bd_base,
  bd_max_step,
  aew,
  aeb,
  vpt,
  reserved_27,
  reg28,
  houtsize,
  exhch,
  exhcl,
  voutsize,
  advfl,
  advfh,
  yave,
  lum_hth,
  lum_lth,
  href,
  dm_lnl,
  dm_lnh,
  ad_off_b,
  ad_off_r,
  ad_off_gb,
  ad_off_gr,
  off_b,
  off_r,
  off_gb,
  off_gr,
  com12,
  com13,
  com14,
  com15,
  com16,
  tgt_b,
  tgt_r,
  tgt_gb,
  tgt_gr,
  lc_ctr,
  lc_xc,
  lc_yc,
  lc_coef,
  lc_radi,
  lc_coefb,
  lc_coefr,
  fix_gain,
  aref0,
  aref1,
  aref2,
  aref3,
  aref4,
  aref5,
  aref6,
  aref7,
  // reserved
  u_fix = 0x60,
  v_fix,
  awb_b_blk,
  awb_ctrl0,
  dsp_ctrl1,
  dsp_ctrl2,
  dsp_ctrl3,
  dsp_ctrl4,
  awb_bias,
  awb_ctrl1,
  awb_ctrl2,
  awb_ctrl3,
  awb_ctrl4,
  awb_ctrl5,
  awb_ctrl6,
  awb_ctrl7,
  awb_ctrl8,
  awb_ctrl9,
  awb_ctrl10,
  awb_ctrl11,
  awb_ctrl12,
  awb_ctrl13,
  awb_ctrl14,
  awb_ctrl15,
  awb_ctrl16,
  awb_ctrl17,
  awb_ctrl18,
  awb_r_gain_range,
  awb_g_gain_range,
  awb_b_gain_range,
  gamma1,
  gamma2,
  gamma3,
  gamma4,
  gamma5,
  gamma6,
  gamma7,
  gamma8,
  gamma9,
  gamma10,
  gamma11,
  gamma12,
  gamma13,
  gamma14,
  gamma15,
  gamma_slope,
  denoise_thresh,
  edge0,
  edge1,
  denoise_thresh_ctrl,
  edge2,
  edge3,
  matrix_coeff1,
  matrix_coeff2,
  matrix_coeff3,
  matrix_coeff4,
  matrix_coeff5,
  matrix_coeff6,
  matrix_ctrl,
  brightness,
  contrast_gain,
  reserved_9d,
  uv_adj0,
  uv_adj1,
  scale0,
  scale1,
  scale2,
  fifo_delay_ctrl_manual,
  fifo_delay_ctrl_auto,
  reserved_a5,
  sde,
  u_sat,
  v_sat,
  hue_cos,
  hue_sin,
  hue_sign,
  dsp_auto,
};
static_assert(static_cast<std::uint8_t>(reg::aref7) == 0x55);
static_assert(static_cast<std::uint8_t>(reg::dsp_auto) == 0xAC);
static_assert(sizeof(register_setting<reg>) == 2);

// COM3 values
inline constexpr std::uint8_t com3_vflip_image = bit_7;
inline constexpr std::uint8_t com3_hflip_image = bit_6;
inline constexpr std::uint8_t com3_swap_br = bit_5;
inline constexpr std::uint8_t com3_swap_yuv = bit_4;
inline constexpr std::uint8_t com3_swap_output_msb = bit_3;
inline constexpr std::uint8_t com3_power_down_clock_tristate = bit_2;
inline constexpr std::uint8_t com3_power_down_output_tristate = bit_1;
inline constexpr std::uint8_t com3_enable_test_pattern = bit_0;

// COM4 values
inline constexpr std::uint8_t com4_pll_bypass = 0 << 6;
inline constexpr std::uint8_t com4_pll_4x = 1 << 6;
inline constexpr std::uint8_t com4_pll_6x = 2 << 6;
inline constexpr std::uint8_t com4_pll_8x = 3 << 6;
inline constexpr std::uint8_t com4_aec_full_window = 0 << 4;
inline constexpr std::uint8_t com4_aec_half_window = 1 << 4;
inline constexpr std::uint8_t com4_aec_quarter_window = 2 << 4;
inline constexpr std::uint8_t com4_aec_low_twothird_window = 3 << 4;

// COM7 values
inline constexpr std::uint8_t com7_sccb_reset = bit_7;
inline constexpr std::uint8_t com7_res_vga = 0 << 6;
inline constexpr std::uint8_t com7_res_qvga = 1 << 6;
inline constexpr std::uint8_t com7_fmt_gbr422 = 0 << 2;
inline constexpr std::uint8_t com7_fmt_rgb565 = 1 << 2;
inline constexpr std::uint8_t com7_fmt_rgb555 = 2 << 2;
inline constexpr std::uint8_t com7_fmt_gbr444 = 3 << 2;
inline constexpr std::uint8_t com7_ofmt_yuv = 0 << 0;
inline constexpr std::uint8_t com7_ofmt_processed_bayer = 1 << 0;
inline constexpr std::uint8_t com7_ofmt_rgb = 2 << 0;
inline constexpr std::uint8_t com7_ofmt_bayer_raw = 3 << 0;

// COM8 values
inline constexpr std::uint8_t com8_enable_fast_agc_aec = bit_7;
inline constexpr std::uint8_t com8_aec_step_size_vblank = 1 << 6;
inline constexpr std::uint8_t com8_aec_step_size_unlimited = 0 << 6;
inline constexpr std::uint8_t com8_enable_banding_filter = bit_5;
inline constexpr std::uint8_t com8_enable_aec_below_banding = bit_4;
inline constexpr std::uint8_t com8_enable_fine_aec = bit_3;
inline constexpr std::uint8_t com8_enable_agc = bit_2;
inline constexpr std::uint8_t com8_enable_awb = bit_1;
inline constexpr std::uint8_t com8_enable_aec = bit_0;

// COM11 values
inline constexpr std::uint8_t com11_enable_single_frame_transfer_trigger = bit_0;
inline constexpr std::uint8_t com11_enable_single_frame = bit_1;

// COM13 values
inline constexpr std::uint8_t com13_enable_ablc_gain_trigger = bit_2;
inline constexpr std::uint8_t com13_enable_analog_blc = bit_5;
inline constexpr std::uint8_t com13_enable_adc_blc = bit_6;
inline constexpr std::uint8_t com13_enable_blc = bit_7;

// DSP_CTRL1 values
inline constexpr std::uint8_t dsp_ctrl1_enable_fifo_selection = bit_7;
inline constexpr std::uint8_t dsp_ctrl1_enable_uv_adjust = bit_6;
inline constexpr std::uint8_t dsp_ctrl1_enable_sde = bit_5;
inline constexpr std::uint8_t dsp_ctrl1_enable_color_matrix = bit_4;
inline constexpr std::uint8_t dsp_ctrl1_enable_interpolation = bit_3;
inline constexpr std::uint8_t dsp_ctrl1_enable_gamma_function = bit_2;
inline constexpr std::uint8_t dsp_ctrl1_auto_correct_black_defect = bit_1;
inline constexpr std::uint8_t dsp_ctrl1_auto_correct_white_defect = bit_0;

// DSP_CTRL2 values
inline constexpr std::uint8_t dsp_ctrl2_enable_horz_zoom = bit_0;
inline constexpr std::uint8_t dsp_ctrl2_enable_vert_zoom = bit_1;
inline constexpr std::uint8_t dsp_ctrl2_enable_horz_dcw = bit_2;
inline constexpr std::uint8_t dsp_ctrl2_enable_vert_dcw = bit_3;

// DSP_CTRL2 values
// TODO: rgb/yuv formats?
inline constexpr std::uint8_t dsp_ctrl4_output_yuv = 0 << 0;
inline constexpr std::uint8_t dsp_ctrl4_output_raw8 = 2 << 0;
inline constexpr std::uint8_t dsp_ctrl4_output_raw10 = 3 << 0;

// AWB_CTRL0 values
inline constexpr std::uint8_t awb_ctrl0_enable_awb_gain = bit_7;
inline constexpr std::uint8_t awb_ctrl0_calculate_awb = bit_6;

namespace detail
{
inline constexpr std::uint8_t init_com13 =
    bit_0 | bit_1 | bit_4 | com13_enable_analog_blc | com13_enable_adc_blc | com13_enable_blc;
inline constexpr std::uint8_t init_com8 = com8_enable_aec | com8_enable_awb | com8_enable_agc | com8_enable_fine_aec |
                                          com8_enable_aec_below_banding | com8_aec_step_size_vblank |
                                          com8_enable_fast_agc_aec;
} // namespace detail

inline constexpr register_setting<reg> initialization_data[] = {
    {         reg::com12,                0x00},

    { reg::clock_control,                0x00},

    {          reg::com9,                0x40},
    {         reg::com10,                0x00},

    {     reg::awb_ctrl0,                0xAA},
    {     reg::awb_ctrl1,                0x87},
    {     reg::dsp_ctrl3,                0x00},

    {         reg::com11,               bit_4},
    {         reg::aref0,                0x0F},
    {         reg::com13,  detail::init_com13},
    {          reg::com4, bit_0 | com4_pll_4x},
    {          reg::href,                0x00},
    {       reg::bd_base,                0x7F},
    {   reg::bd_max_step,                0x03},
    {           reg::aew,                0x40},
    {           reg::aeb,                0x30},
    {           reg::vpt,                0xA1},
    {         reg::exhch,                0x00},
    {         reg::exhcl,                0x00},
    {          reg::com8,   detail::init_com8},
    {          reg::com3,                0xC0},

    {reg::denoise_thresh,                0x00},
};

inline constexpr register_setting<reg> sensor_start_vga[] = {
    {reg::horizontal_start,  0x26},
    { reg::horizontal_size,  0xA0},
    {  reg::vertical_start,  0x07},
    {   reg::vertical_size,  0xF0},
    {        reg::houtsize,  0xA0},
    {        reg::voutsize,  0xF0},
    {       reg::dsp_ctrl2, bit_5},
};

inline constexpr std::uint8_t dsp_ctrl2_qvga_zoom =
    dsp_ctrl2_enable_horz_zoom | dsp_ctrl2_enable_vert_zoom | dsp_ctrl2_enable_horz_dcw | dsp_ctrl2_enable_vert_dcw;
inline constexpr register_setting<reg> sensor_start_qvga[] = {
    {reg::horizontal_start,                        0x3F},
    { reg::horizontal_size,                        0x50},
    {  reg::vertical_start,                        0x03},
    {   reg::vertical_size,                        0x78},
    {        reg::houtsize,                        0x50},
    {        reg::voutsize,                        0x78},
    {       reg::dsp_ctrl2, bit_5 | dsp_ctrl2_qvga_zoom},
};

} // namespace ov7725

PSEYE_HARDWARE_NS_END

#endif
