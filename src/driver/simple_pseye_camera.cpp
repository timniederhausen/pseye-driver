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
#include "pseye/driver/simple_pseye_camera.hpp"
#include "pseye/driver/pseye_device_controller_ops.hpp"
#include "pseye/driver/usb_transfer_controller.hpp"
#include "pseye/driver/uvc_frame_processor.hpp"
#include "pseye/driver/spsc_frame_buffer.hpp"
#include "pseye/exception.hpp"
#include "pseye/log.hpp"

#include <libusb.h>

#include <chrono>
#include <thread>

PSEYE_NS_BEGIN

inline constexpr std::size_t transfer_count = 5;
inline constexpr std::size_t transfer_size = 0x10000;

simple_pseye_camera::simple_pseye_camera(libusb_device* device, const pseye_device_state& initial_state)
  : handle_(device, pseye_interface_number)
  , state_(initial_state)
  , transfer_([this](std::span<std::uint8_t> data) {
    process_transfer_data(data);
  })
{
  initialize();
}

simple_pseye_camera::simple_pseye_camera(libusb_device_handle* device, const pseye_device_state& initial_state)
  : handle_(device, pseye_interface_number)
  , state_(initial_state)
  , transfer_([this](std::span<std::uint8_t> data) {
    process_transfer_data(data);
  })
{
  initialize();
}

simple_pseye_camera::~simple_pseye_camera()
{
  stop();
}

void simple_pseye_camera::start(size_mode mode, int frame_rate, pixel_format internal_format)
{
  switch (mode) {
    case size_mode::vga:
      state_.width = 640;
      state_.height = 480;
      break;
    case size_mode::qvga:
      state_.width = 320;
      state_.height = 240;
      break;
  }
  state_.rate = find_valid_frame_rate(mode, frame_rate);
  state_.format = internal_format;

  const std::uint32_t frame_size = size_bytes(state_.format, state_.width, state_.height);
  const std::uint32_t payload_size = 2 * 1024;

  ov534::video_data_configuration video_cfg;
  std::uint8_t com7_value = 0;
  std::uint8_t dsp_ctrl4_value = 0;

  switch (state_.format) {
    case pixel_format::bayer8:
      video_cfg = ov534::make_video_data_settings(ov534::video_format::raw8, ov534::transfer::bulk, payload_size / 4,
                                                  frame_size / 4);
      com7_value |= ov7725::com7_ofmt_processed_bayer;
      dsp_ctrl4_value |= ov7725::dsp_ctrl4_output_raw8;
      break;
    case pixel_format::bayer10:
      video_cfg = ov534::make_video_data_settings(ov534::video_format::raw10, ov534::transfer::bulk, payload_size / 4,
                                                  frame_size / 4);
      com7_value |= ov7725::com7_ofmt_processed_bayer;
      dsp_ctrl4_value |= ov7725::dsp_ctrl4_output_raw10;
      break;
    default: throw std::runtime_error("unsupported transfer pixel format");
  }

  PSEYE_LOG_DEBUG("setting: payload size {} frame size {}", payload_size, frame_size);
  write_video_data(handle_, video_cfg);

  switch (mode) {
    case size_mode::vga:
      write_register(handle_, ov534::bridge_start_vga);
      write_sccb_register(handle_, ov7725::sensor_start_vga);
      com7_value |= ov7725::com7_res_vga;
      break;
    case size_mode::qvga:
      write_register(handle_, ov534::bridge_start_qvga);
      write_sccb_register(handle_, ov7725::sensor_start_qvga);
      com7_value |= ov7725::com7_res_qvga;
      break;
  }
  write_sccb_register(handle_, ov7725::reg::dsp_ctrl4, dsp_ctrl4_value);
  write_sccb_register(handle_, ov7725::reg::com7, com7_value);
  apply_state(handle_, state_);

  set_camera_led_status(handle_, true);
  write_register(handle_, ov534::reg::reset0, 0x00);

  payload_size_ = payload_size;
  frame_buffer_ = std::make_unique<spsc_frame_buffer>(2, frame_size);
  transfer_.start(handle_.get(), handle_.bulk_endpoint(), transfer_count, transfer_size);
  is_active_ = true;
}

void simple_pseye_camera::stop()
{
  if (!is_active_)
    return;

  write_register(handle_, ov534::reg::reset0, ov534::reset0_cif | ov534::reset0_vfifo);
  set_camera_led_status(handle_, false);

  transfer_.stop();
  is_active_ = false;
}

void simple_pseye_camera::initialize()
{
  // reset camera bridge
  write_register(handle_, ov534::reg::sys_ctrl,
                 ov534::sys_ctrl_suspend_enable | ov534::sys_ctrl_mc_wakeup_reset_enable | ov534::sys_ctrl_reset_3 |
                     ov534::sys_ctrl_reset_5);
  write_register(handle_, ov534::reg::reset0, ov534::reset0_vfifo);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // set the SCCB target sensor to our OV772x
  write_register(handle_, ov534::reg::ms_id, 0x42);

  // reset sensor
  write_sccb_register(handle_, ov7725::reg::com7, ov7725::com7_sccb_reset);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  uint16_t product_id = read_sccb_register(handle_, ov7725::reg::pid) << 8;
  product_id |= read_sccb_register(handle_, ov7725::reg::ver);
  PSEYE_LOG_INFO("Sensor ID: OV{:04x}", product_id);

  write_register(handle_, ov534::initialization_data);
  write_sccb_register(handle_, ov7725::initialization_data);
  write_register(handle_, ov534::reg::reset0, ov534::reset0_cif | ov534::reset0_vfifo);
}

void simple_pseye_camera::process_transfer_data(std::span<uint8_t> data)
{
  // Process the input data in |payload_size_|-sized chunks
  do {
    const auto payload = data.subspan(0, std::min<std::size_t>(payload_size_, data.size()));
    switch (processor_.put(payload)) {
      case uvc_frame_processor::status::need_data:
        // we successfully read that payload block
        data = data.subspan(payload.size());
        break;
      case uvc_frame_processor::status::need_buffer:
        // we might've been reset - just give it the current frame
        processor_.set_frame(frame_buffer_->writable_frame());
        break;
      case uvc_frame_processor::status::frame_complete:
        frame_buffer_->finish_writing();
        processor_.set_frame(frame_buffer_->writable_frame());
        data = data.subspan(payload.size());
        break;
    }
  } while (!data.empty());
}

PSEYE_NS_END
