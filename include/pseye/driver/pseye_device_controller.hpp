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
#ifndef PSEYE_DRIVER_PSEYEDEVICECONTROLLER_HPP
#define PSEYE_DRIVER_PSEYEDEVICECONTROLLER_HPP

#include "pseye/detail/config.hpp"

#if PSEYE_HAS_PRAGMA_ONCE
# pragma once
#endif

#include <memory>
#include <cstdint>

typedef struct libusb_device libusb_device;
typedef struct libusb_device_handle libusb_device_handle;

PSEYE_NS_BEGIN

inline constexpr std::uint16_t pseye_vendor_id = 0x1415;
inline constexpr std::uint16_t pseye_product_id = 0x2000;
inline constexpr std::uint8_t pseye_interface_number = 0;

inline constexpr std::uint32_t vga_frame_rates[] = {75, 60, 50, 40, 30, 15};
inline constexpr std::uint32_t qvga_frame_rates[] = {187, 150, 137, 125, 100, 75, 60, 50, 37, 30};

/// Low-level control over the PlayStation Eye camera device.
class pseye_device_controller
{
public:
  pseye_device_controller(libusb_device* device, std::uint8_t interface_index);
  pseye_device_controller(libusb_device_handle* device, std::uint8_t interface_index);

  libusb_device_handle* get() const { return device_handle_.get(); }
  std::uint8_t bulk_endpoint() const { return bulk_endpoint_; }

private:
  struct free_device
  {
    void operator()(libusb_device_handle* h) const;
    std::uint8_t interface_index; // XXX: not too great to duplicate it here :(
  };
  std::unique_ptr<libusb_device_handle, free_device> device_handle_;
  std::uint8_t interface_index_;
  std::uint8_t bulk_endpoint_;
};

PSEYE_NS_END

#endif
