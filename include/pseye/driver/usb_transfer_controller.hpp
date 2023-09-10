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
#ifndef PSEYE_DRIVER_USBTRANSFERCONTROLLER_HPP
#define PSEYE_DRIVER_USBTRANSFERCONTROLLER_HPP

#include "pseye/detail/config.hpp"

#if PSEYE_HAS_PRAGMA_ONCE
# pragma once
#endif

#include <functional>
#include <mutex>
#include <span>

struct libusb_transfer;
typedef struct libusb_device_handle libusb_device_handle;

PSEYE_NS_BEGIN

struct free_libusb_transfer
{
  void operator()(libusb_transfer* transfer) const;
};

/// Helper class for raw bulk data transfers
// TODO: iso support?
class usb_transfer_controller
{
public:
  // TODO: std::invocable?
  template <typename Handler>
  usb_transfer_controller(Handler&& handler)
    : handler_(std::forward<Handler>(handler))
  {
  }

  ~usb_transfer_controller() { stop(); }

  bool start(libusb_device_handle* handle, std::uint8_t endpoint, std::size_t num_transfers, std::size_t transfer_size);
  void stop();

private:
  void free_transfer(libusb_transfer* transfer);
  void process_done(libusb_transfer* transfer);

  std::function<void(std::span<std::uint8_t> data)> handler_;

  std::unique_ptr<uint8_t[]> transfer_buffer_;
  std::vector<std::unique_ptr<libusb_transfer, free_libusb_transfer>> transfers_;

  std::mutex active_transfers_mutex_;
  std::condition_variable transfer_done_condition_;
  uint8_t num_active_transfers_ = 0;
};

PSEYE_NS_END

#endif
