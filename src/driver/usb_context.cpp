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
#include "pseye/driver/usb_context.hpp"

#include "pseye/exception.hpp"
#include "pseye/log.hpp"

#include <libusb.h>

#include <span>

PSEYE_NS_BEGIN

void LIBUSB_CALL libusb_log_cb(libusb_context*, libusb_log_level level, const char* str)
{
  std::string_view message(str);
  if (message.ends_with("\n"))
    message.remove_suffix(1);
  switch (level) {
    case LIBUSB_LOG_LEVEL_NONE: break;
    case LIBUSB_LOG_LEVEL_ERROR: PSEYE_LOG_ERROR("libusb: {}", message); break;
    case LIBUSB_LOG_LEVEL_WARNING: PSEYE_LOG_WARNING("libusb: {}", message); break;
    case LIBUSB_LOG_LEVEL_INFO: PSEYE_LOG_INFO("libusb: {}", message); break;
    case LIBUSB_LOG_LEVEL_DEBUG: PSEYE_LOG_DEBUG("libusb: {}", message); break;
  }
}

usb_context::usb_context()
{
  PSEYE_LOG_DEBUG("usb_context::usb_context");
  libusb_init_option log_cb_option;
  log_cb_option.option = LIBUSB_OPTION_LOG_CB;
  log_cb_option.value.log_cbval = libusb_log_cb;

  const libusb_init_option options[] = {
      log_cb_option, {LIBUSB_OPTION_LOG_LEVEL, {LIBUSB_LOG_LEVEL_INFO}},
 // { LIBUSB_OPTION_USE_USBDK, 1 /*unused*/ },
  };

  const auto ret = ::libusb_init_context(&usb_context_, options, std::size(options));
  if (0 != ret) {
    PSEYE_LOG_ERROR("failed to init libusb context: {} {}", ret, ::libusb_error_name(ret));
    throw usb_error(ret, "failed to init libusb context");
  }

  event_loop_thread_ = std::thread(&usb_context::run_event_loop, this);
}

usb_context::~usb_context()
{
  PSEYE_LOG_DEBUG("usb_context::~usb_context");
  exit_requested_ = true;
  event_loop_thread_.join();
  ::libusb_exit(usb_context_);
}

libusb_device_handle* usb_context::open_device(std::uint16_t vendor_id, std::uint16_t product_id)
{
  return ::libusb_open_device_with_vid_pid(usb_context_, vendor_id, product_id);
}

void usb_context::for_each_device_aux(void (*cb)(void* ctx, libusb_device* dev, const libusb_device_descriptor& desc),
                                      void* ctx)
{
  libusb_device** devs;
  const auto res = ::libusb_get_device_list(usb_context_, &devs);
  if (res < 0) {
    PSEYE_LOG_ERROR("failed to enumerate USB devices: {} {}", res, ::libusb_error_name(res));
    return;
  }

  for (libusb_device* dev : std::span(devs, res)) {
    libusb_device_descriptor desc;
    ::libusb_get_device_descriptor(dev, &desc);
    cb(ctx, dev, desc);
  }

  ::libusb_free_device_list(devs, true);
}

void usb_context::run_event_loop() const
{
  PSEYE_LOG_DEBUG("entering usb context event loop");

  timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 25 * 1000;
  while (!exit_requested_) {
    libusb_handle_events_timeout_completed(usb_context_, &tv, NULL);
  }

  PSEYE_LOG_DEBUG("exiting usb context event loop");
}

const libusb_endpoint_descriptor* find_endpoint(libusb_config_descriptor* config,
                                                std::uint8_t interface_index,
                                                libusb_transfer_type type)
{
  for (int iface_idx = 0; iface_idx < config->bNumInterfaces; iface_idx++) {
    const libusb_interface* iface = &config->interface[iface_idx];
    for (int altsetting_idx = 0; altsetting_idx < iface->num_altsetting; altsetting_idx++) {
      const libusb_interface_descriptor* altsetting = &iface->altsetting[altsetting_idx];
      if (altsetting->bInterfaceNumber != interface_index)
        continue;

      for (int ep_idx = 0; ep_idx < altsetting->bNumEndpoints; ep_idx++) {
        const libusb_endpoint_descriptor* ep = &altsetting->endpoint[ep_idx];
        if ((ep->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == type && ep->wMaxPacketSize != 0)
          return ep;
      }
    }
  }
  return nullptr;
}

PSEYE_NS_END
