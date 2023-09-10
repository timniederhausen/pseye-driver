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
#ifndef PSEYE_DRIVER_USBCONTEXT_HPP
#define PSEYE_DRIVER_USBCONTEXT_HPP

#include "pseye/detail/config.hpp"

#if PSEYE_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <atomic>
#include <thread>

typedef struct libusb_context libusb_context;
typedef struct libusb_device libusb_device;
typedef struct libusb_device_handle libusb_device_handle;
struct libusb_device_descriptor;
struct libusb_endpoint_descriptor;
struct libusb_config_descriptor;
enum libusb_transfer_type;

PSEYE_NS_BEGIN

class usb_context
{
public:
  usb_context();
  ~usb_context();

  template <typename Handler>
  void for_each_device(Handler&& handler)
  {
    for_each_device_aux(
        [](void* ctx, libusb_device* dev, const libusb_device_descriptor& desc) {
          auto& handler = *static_cast<Handler*>(ctx);
          handler(dev, desc);
        },
        &handler);
  }
  libusb_device_handle* open_device(std::uint16_t vendor_id, std::uint16_t product_id);

private:
  void for_each_device_aux(void (*cb)(void* ctx, libusb_device* dev, const libusb_device_descriptor& desc), void* ctx);
  void run_event_loop() const;

  libusb_context* usb_context_ = nullptr;
  std::thread event_loop_thread_;
  std::atomic<bool> exit_requested_ = false;
};

const libusb_endpoint_descriptor* find_endpoint(libusb_config_descriptor* config,
                                                std::uint8_t interface_index,
                                                libusb_transfer_type type);

PSEYE_NS_END

#endif
