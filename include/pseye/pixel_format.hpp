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
#ifndef PSEYE_PIXELFORMAT_HPP
#define PSEYE_PIXELFORMAT_HPP

#include "pseye/detail/config.hpp"

#if PSEYE_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <stdexcept>
#include <span>
#include <cstdint>

PSEYE_NS_BEGIN

// NOTE: not all of these formats are supported by the hardware directly
// and it's not possible to arbitrarily convert between them (yet) either!
enum class pixel_format
{
  grbg8,  // width * height bytes
  grbg10, // 5 * width * height / 4 bytes
  grbg16, // width * height * 2 bytes
  bgr24,  // width * height * 3 bytes
  rgb24,  // width * height * 3 bytes
  bgra32, // width * height * 4 bytes
  rgba32, // width * height * 4 bytes
  gray,   // width * height bytes
  yuyv,   // width * height * 2 bytes
  uyvy,   // width * height * 2 bytes
};

constexpr std::size_t size_bytes(pixel_format output, std::size_t width, std::size_t height)
{
  switch (output) {
    case pixel_format::grbg8: return width * height;
    case pixel_format::grbg10: return 5 * width * height / 4;
    case pixel_format::grbg16: return width * height * 2;
    case pixel_format::bgr24: return width * height * 3;
    case pixel_format::rgb24: return width * height * 3;
    case pixel_format::bgra32: return width * height * 4;
    case pixel_format::rgba32: return width * height * 4;
    case pixel_format::gray: return width * height;
    case pixel_format::yuyv: return width * height * 2;
    case pixel_format::uyvy: return width * height * 2;
  }
  // TODO: revisit this, I don't actually plan on throwing that
  // but I don't want to drag in <stdexcept>
  throw 0;
}

void convert_frame(pixel_format from,
                   pixel_format to,
                   std::span<const std::uint8_t> input_frame,
                   std::span<std::uint8_t> output_frame,
                   std::size_t width,
                   std::size_t height,
                   bool flip_v = false);

PSEYE_NS_END

#endif
