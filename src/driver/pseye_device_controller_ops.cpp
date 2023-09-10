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
#include "pseye/driver/pseye_device_controller_ops.hpp"
#include "pseye/driver/pseye_device_controller.hpp"
#include "pseye/exception.hpp"
#include "pseye/log.hpp"

#include <libusb.h>

#include <chrono>
#include <thread>

PSEYE_NS_BEGIN

uint8_t read_register(pseye_device_controller& controller, ov534::reg reg)
{
  uint8_t buf[1];
  const auto ret = libusb_control_transfer(controller.get(),
                                           LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                           0x01, 0x00, static_cast<uint16_t>(reg), buf, sizeof(buf), 0);
  if (ret != 1) {
    throw camera_bridge_error(ret, "bridge register read failed");
  }
  PSEYE_LOG_DEBUG("GET 01 0000 {:04x} {:02x}", static_cast<std::uint16_t>(reg), buf[0]);
  return buf[0];
}

void write_register(pseye_device_controller& controller, ov534::reg reg, uint8_t val)
{
  PSEYE_LOG_DEBUG("SET 01 0000 {:04x} {:02x}", static_cast<std::uint16_t>(reg), val);
  const auto ret = libusb_control_transfer(controller.get(),
                                           LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE,
                                           0x01, 0x00, static_cast<std::uint16_t>(reg), &val, sizeof(val), 0);
  if (ret != 1) {
    throw camera_bridge_error(ret, "bridge register write failed");
  }
}

uint8_t read_sccb_register(pseye_device_controller& controller, ov7725::reg reg)
{
  // First send the register we want to read
  write_register(controller, ov534::reg::ms_address, static_cast<std::uint8_t>(reg));
  write_register(controller, ov534::reg::ms_ctrl, ov534::ms_ctrl_2byte_write);
  if (!check_sccb_status(controller)) {
    PSEYE_LOG_ERROR("setup SCCB register read target failed: reg {:#x}", static_cast<std::uint8_t>(reg));
    throw sccb_error("setup SCCB register read target failed");
  }

  // Then send the request to actually read it
  write_register(controller, ov534::reg::ms_ctrl, ov534::ms_ctrl_2byte_read);
  if (!check_sccb_status(controller)) {
    PSEYE_LOG_ERROR("submit SCCB register read failed: reg {:#x}", static_cast<std::uint8_t>(reg));
    throw sccb_error("submit SCCB register read failed");
  }

  return read_register(controller, ov534::reg::ms_data_in);
}

void write_sccb_register(pseye_device_controller& controller, ov7725::reg reg, uint8_t val)
{
  write_register(controller, ov534::reg::ms_address, static_cast<std::uint8_t>(reg));
  write_register(controller, ov534::reg::ms_data_out, val);
  write_register(controller, ov534::reg::ms_ctrl, ov534::ms_ctrl_3byte_write);
  if (!check_sccb_status(controller)) {
    PSEYE_LOG_ERROR("SCCB register write failed: reg {:#x} value {:#x}", static_cast<std::uint8_t>(reg), val);
    throw sccb_error("SCCB register write failed");
  }
}

bool check_sccb_status(pseye_device_controller& controller)
{
  constexpr int num_sccb_status_retries = 15; // ~150ms max

  for (int i = 0; i < num_sccb_status_retries; i++) {
    const auto data = read_register(controller, ov534::reg::ms_status);

    if (0 != (data & ov534::ms_status_slave_nak)) {
      PSEYE_LOG_ERROR("NACK in status {:#08b} for SCCB i/o command in retry {}", data, i + 1);
      return false;
    }

    if (data == ov534::ms_status_slave_ack)
      return true;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // retry for all other statuses
    if (0 != (data & (ov534::ms_status_command_busy | ov534::ms_status_cycle_complete))) {
      continue;
    }

    PSEYE_LOG_INFO("unexpected status {:#08b} in retry {}", data, i + 1);
  }
  return false;
}

void write_video_data(pseye_device_controller& controller, const void* data, int len, std::uint8_t offset)
{
  write_register(controller, ov534::reg::video_data_addr, offset);
  for (auto p = static_cast<const uint8_t*>(data); --len >= 0; ++p) {
    write_register(controller, ov534::reg::video_data_value, *p);
  }
}

PSEYE_NS_END
