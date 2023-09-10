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
#ifndef PSEYE_EXCEPTION_HPP
#define PSEYE_EXCEPTION_HPP

#include "pseye/detail/config.hpp"

#include <stdexcept>

#if PSEYE_HAS_PRAGMA_ONCE
#pragma once
#endif

PSEYE_NS_BEGIN

class usb_error : public std::runtime_error
{
public:
  using runtime_error::runtime_error;

  explicit usb_error(int libusb_error, const std::string& message)
    : runtime_error(message)
    , libusb_error_(libusb_error)
  {
  }

  int libusb_error() const noexcept { return libusb_error_; }

private:
  int libusb_error_ = 0 /*LIBUSB_SUCCESS*/;
};

/// USB-level Camera Bridge Processor error
class camera_bridge_error : public usb_error
{
public:
  using usb_error::usb_error;
};

/// USB-level Serial Camera Control Bus error
class sccb_error : public usb_error
{
public:
  using usb_error::usb_error;
};

PSEYE_NS_END

#endif
