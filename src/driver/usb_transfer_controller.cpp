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
#include "pseye/driver/usb_transfer_controller.hpp"

#include "pseye/log.hpp"

#include <libusb.h>

PSEYE_NS_BEGIN

void free_libusb_transfer::operator()(libusb_transfer* transfer) const
{
  // if (transfer); libusb_free_transfer handles NULL values
  ::libusb_free_transfer(transfer);
}

bool usb_transfer_controller::start(libusb_device_handle* handle,
                                    std::uint8_t endpoint,
                                    std::size_t num_transfers,
                                    std::size_t transfer_size)
{
  ::libusb_clear_halt(handle, endpoint);

  std::lock_guard lock(active_transfers_mutex_);
  transfers_.resize(num_transfers);
  transfer_buffer_.reset(new std::uint8_t[transfer_size * num_transfers]);

  for (int index = 0; index < num_transfers; ++index) {
    transfers_[index].reset(::libusb_alloc_transfer(0));
    ::libusb_fill_bulk_transfer(
        transfers_[index].get(), handle, endpoint, &transfer_buffer_[index * transfer_size], transfer_size,
        [](libusb_transfer* transfer) {
          const auto self = static_cast<usb_transfer_controller*>(transfer->user_data);
          self->process_done(transfer);
        },
        this, 0);

    const auto res = ::libusb_submit_transfer(transfers_[index].get());
    if (res != LIBUSB_SUCCESS) {
      // We might just have exhausted the number of possible transfers, just stop and keep the existing ones going!
      PSEYE_LOG_WARNING("failed to submit transfer {}: {} {}", index, res, ::libusb_error_name(res));
      break; // don't submit more!
    }

    num_active_transfers_++;
  }

  PSEYE_LOG_DEBUG("started {} active transfers with {} bytes each on {}", num_active_transfers_, transfer_size,
                  endpoint);
  return num_active_transfers_ > 0; // we're good if we got one at least!
}

void usb_transfer_controller::stop()
{
  std::unique_lock lock(active_transfers_mutex_);
  if (num_active_transfers_ == 0)
    return;

  // Cancel any pending transfers
  for (const auto& transfer : transfers_) {
    if (transfer)
      ::libusb_cancel_transfer(transfer.get());
  }

  // Wait for cancellation to finish
  transfer_done_condition_.wait(lock, [this]() {
    return num_active_transfers_ == 0;
  });

  transfers_.clear();
}

void usb_transfer_controller::free_transfer(libusb_transfer* transfer)
{
  std::lock_guard lock(active_transfers_mutex_);
  --num_active_transfers_;

  const auto it = std::find_if(std::begin(transfers_), std::end(transfers_), [transfer](const auto& p) {
    return p.get() == transfer;
  });
  if (it != std::end(transfers_)) {
    it->reset();
  } else {
    PSEYE_LOG_ERROR("transfer {} not found", static_cast<void*>(transfer));
  }

  transfer_done_condition_.notify_one();
}

void usb_transfer_controller::process_done(libusb_transfer* transfer)
{
  bool resubmit = true;

  switch (transfer->status) {
    case LIBUSB_TRANSFER_COMPLETED:
      if (transfer->num_iso_packets == 0) {
        // Bulk transfers only have one payload transfer
        handler_(std::span(transfer->buffer, transfer->actual_length));
      } else {
        // TODO: test & implement iso transfer mode!
        PSEYE_LOG_ERROR("completed isochronous transfer with {} bytes", transfer->actual_length);

        for (int packet_id = 0; packet_id < transfer->num_iso_packets; ++packet_id) {
          const auto& pkt = transfer->iso_packet_desc[packet_id];
          if (pkt.status == LIBUSB_TRANSFER_COMPLETED) {
            const auto pkt_buf = libusb_get_iso_packet_buffer_simple(transfer, packet_id);
            handler_(std::span(transfer->buffer, transfer->actual_length));
            continue;
          }
          PSEYE_LOG_ERROR("bad iso packet: status {} {}", static_cast<int>(pkt.status),
                          ::libusb_error_name(pkt.status));
        }
      }
      break;
    case LIBUSB_TRANSFER_CANCELLED:
    case LIBUSB_TRANSFER_ERROR:
    case LIBUSB_TRANSFER_NO_DEVICE: {
      PSEYE_LOG_DEBUG("not retrying transfer: status {} {}", static_cast<int>(transfer->status),
                      ::libusb_error_name(transfer->status));
      free_transfer(transfer);
      resubmit = false;
      break;
    }
    case LIBUSB_TRANSFER_TIMED_OUT:
    case LIBUSB_TRANSFER_STALL:
    case LIBUSB_TRANSFER_OVERFLOW:
      PSEYE_LOG_DEBUG("retrying transfer with: status {} {}", static_cast<int>(transfer->status),
                      ::libusb_error_name(transfer->status));
      break;
  }

  if (resubmit) {
    const int ret = ::libusb_submit_transfer(transfer);
    if (ret != LIBUSB_SUCCESS) {
      free_transfer(transfer);
      PSEYE_LOG_ERROR("failed re-submit of transfer with: {} {}", ret, ::libusb_error_name(ret));
    }
  }
}

PSEYE_NS_END
