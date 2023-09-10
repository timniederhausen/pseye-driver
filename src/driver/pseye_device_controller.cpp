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
#include "pseye/driver/pseye_device_controller.hpp"
#include "pseye/exception.hpp"
#include "pseye/log.hpp"
#include "pseye/driver/usb_context.hpp"

#include <libusb.h>

PSEYE_NS_BEGIN

static libusb_device_handle* open_device_simple(libusb_device* device)
{
  libusb_device_handle* h;
  const auto ret = ::libusb_open(device, &h);
  if (LIBUSB_SUCCESS != ret) {
    PSEYE_LOG_ERROR("failed to open device: {} {}", ret, ::libusb_error_name(ret));
    throw usb_error(ret, "failed to open USB device");
  }
  return h;
}

pseye_device_controller::pseye_device_controller(libusb_device* device, std::uint8_t interface_index)
  : pseye_device_controller(open_device_simple(device), interface_index)
{
}

pseye_device_controller::pseye_device_controller(libusb_device_handle* h, std::uint8_t interface_index)
  : interface_index_(interface_index)
{
  // tell libusb to detach any active kernel drivers.
  auto ret = ::libusb_detach_kernel_driver(h, interface_index);
  if (ret == LIBUSB_SUCCESS || ret == LIBUSB_ERROR_NOT_FOUND || ret == LIBUSB_ERROR_NOT_SUPPORTED) {
    PSEYE_LOG_DEBUG("claiming interface {}", interface_index);
    ret = ::libusb_claim_interface(h, interface_index);
    if (ret != LIBUSB_SUCCESS) {
      ::libusb_attach_kernel_driver(h, interface_index);
      ::libusb_close(h);
      PSEYE_LOG_ERROR("failed to claim interface {}: {} {}", interface_index, ret, ::libusb_error_name(ret));
      throw usb_error(ret, "failed to claim interface");
    }
  } else {
    ::libusb_close(h);
    PSEYE_LOG_ERROR("failed to detach drivers for interface {}: {} {}", interface_index, ret, ::libusb_error_name(ret));
    throw usb_error(ret, "failed to detach kernel drivers");
  }

  device_handle_.reset(h);
  device_handle_.get_deleter().interface_index = interface_index;

  libusb_config_descriptor* config = nullptr;
  ret = ::libusb_get_active_config_descriptor(::libusb_get_device(h), &config);
  if (ret == LIBUSB_SUCCESS) {
    const auto bulk_endpoint = find_endpoint(config, interface_index, LIBUSB_TRANSFER_TYPE_BULK);
    const auto iso_endpoint = find_endpoint(config, interface_index, LIBUSB_TRANSFER_TYPE_ISOCHRONOUS);
    PSEYE_LOG_DEBUG("active endpoints: iso {} bulk {}", !iso_endpoint ? -1 : iso_endpoint->bEndpointAddress,
                    !bulk_endpoint ? -1 : bulk_endpoint->bEndpointAddress);
    if (!bulk_endpoint) {
      ::libusb_free_config_descriptor(config);
      throw usb_error(ret, "failed to find required bulk endpoint");
    }
    bulk_endpoint_ = bulk_endpoint->bEndpointAddress;
    ::libusb_free_config_descriptor(config);
  } else {
    PSEYE_LOG_ERROR("failed to query active config: {} {}", ret, ::libusb_error_name(ret));
    throw usb_error(ret, "failed to query active config");
  }
}

void pseye_device_controller::free_device::operator()(libusb_device_handle* h) const
{
  if (!h)
    return;

  PSEYE_LOG_DEBUG("freeing device {} {}", static_cast<void*>(h), interface_index);

  auto ret = ::libusb_release_interface(h, interface_index);
  if (LIBUSB_SUCCESS == ret) {
    // re-attach any kernel drivers that were disabled when we claimed this interface
    ret = ::libusb_attach_kernel_driver(h, interface_index);
    if (ret == LIBUSB_SUCCESS) {
      PSEYE_LOG_DEBUG("re-attached kernel driver to interface {}", interface_index);
    } else if (ret != LIBUSB_ERROR_NOT_FOUND && ret != LIBUSB_ERROR_NOT_SUPPORTED) {
      // don't fail noisily if we didn't have to attach anything!
      PSEYE_LOG_ERROR("failed to re-attach kernel driver to interface {}: {} {}", interface_index, ret,
                      ::libusb_error_name(ret));
    }
  } else {
    PSEYE_LOG_ERROR("failed to unclaim interface {}: {} {}", interface_index, ret, ::libusb_error_name(ret));
  }

  ::libusb_close(h);
}

PSEYE_NS_END
