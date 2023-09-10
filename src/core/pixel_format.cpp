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
#include "pseye/pixel_format.hpp"

#include <Simd/SimdBase.h>
#include <Simd/SimdSse41.h>
#include <Simd/SimdBayer.h>
#include <Simd/SimdConversion.h>

PSEYE_NS_BEGIN

// TODO: this is quite unoptimized!
template <SimdPixelFormatType BayerFormat, pixel_format Output>
void demosaic(const uint8_t* src[6], std::size_t col0, std::size_t col2, std::size_t col4, uint8_t* dst0, std::size_t stride)
{
  if constexpr (Output == pixel_format::uyvy) {
    uint8_t bgr[2 * 2 * 3], y[2 * 2], u[2 * 1], v[2 * 1];
    Simd::Base::BayerToBgr<BayerFormat>(src, col0, col0 + 1, col2, col2 + 1, col4, col4 + 1, bgr, bgr + 3, bgr + 6,
                                        bgr + 9);
    Simd::Base::BgrToYuv420p(bgr, 2, 2, 2 * 3, y, 2, u, 2, v, 2);
    Simd::Base::Yuv420pToUyvy422(y, 2, u, 2, v, 2, 2, 2, dst0, stride);
    return;
  }

  uint8_t* dst1 = dst0 + stride;
  if constexpr (Output == pixel_format::gray) {
    uint8_t bgr00[2 * 3], bgr10[2 * 3];
    Simd::Base::BayerToBgr<BayerFormat>(src, col0, col0 + 1, col2, col2 + 1, col4, col4 + 1, bgr00, bgr00 + 3, bgr10,
                                        bgr10 + 3);
    dst0[0] = Simd::Base::BgrToGray(bgr00[0], bgr00[1], bgr00[2]);
    dst0[1] = Simd::Base::BgrToGray(bgr00[3], bgr00[4], bgr00[5]);
    dst1[0] = Simd::Base::BgrToGray(bgr10[0], bgr10[1], bgr10[2]);
    dst1[1] = Simd::Base::BgrToGray(bgr10[3], bgr10[4], bgr10[5]);
    return;
  }

  if constexpr (Output == pixel_format::bgra8888 || Output == pixel_format::rgba8888) {
    Simd::Base::BayerToBgr<BayerFormat>(src, col0, col0 + 1, col2, col2 + 1, col4, col4 + 1, dst0, dst0 + 4, dst1,
                                        dst1 + 4);
    dst0[3] = 255;
    dst0[7] = 255;
    dst1[3] = 255;
    dst1[7] = 255;
    if constexpr (Output == pixel_format::rgba8888) {
      std::swap(dst0[0], dst0[2]);
      std::swap(dst0[4], dst0[6]);
      std::swap(dst1[0], dst1[2]);
      std::swap(dst1[4], dst1[6]);
    }
  } else {
    Simd::Base::BayerToBgr<BayerFormat>(src, col0, col0 + 1, col2, col2 + 1, col4, col4 + 1, dst0, dst0 + 3, dst1,
                                        dst1 + 3);
    if constexpr (Output == pixel_format::rgb888) {
      std::swap(dst0[0], dst0[2]);
      std::swap(dst0[3], dst0[5]);
      std::swap(dst1[0], dst1[2]);
      std::swap(dst1[3], dst1[5]);
    }
  }
}

template <SimdPixelFormatType BayerFormat, pixel_format Output>
void demosaic(const uint8_t* bayer, std::size_t width, std::size_t height, std::size_t bayerStride, uint8_t* out, std::size_t outStride)
{
  const uint8_t* src[6];
  for (std::size_t row = 0; row < height; row += 2) {
    src[0] = (row == 0 ? bayer : bayer - 2 * bayerStride);
    src[1] = src[0] + bayerStride;
    src[2] = bayer;
    src[3] = src[2] + bayerStride;
    src[4] = (row == height - 2 ? bayer : bayer + 2 * bayerStride);
    src[5] = src[4] + bayerStride;

    demosaic<BayerFormat, Output>(src, 0, 0, 2, out, outStride);

    for (std::size_t col = 2; col < width - 2; col += 2)
      demosaic<BayerFormat, Output>(src, col - 2, col, col + 2, out + size_bytes(Output, col, 1), outStride);

    demosaic<BayerFormat, Output>(src, width - 4, width - 2, width - 2, out + size_bytes(Output, width - 2, 1),
                                  outStride);

    bayer += 2 * bayerStride;
    out += 2 * outStride;
  }
}

template <SimdPixelFormatType BayerFormat, pixel_format Output>
void demosaic(std::span<const std::uint8_t> bayer, std::size_t width, std::size_t height, std::span<std::uint8_t> out)
{
  // TODO: if we add SSE4.1 overloads for the generic converter, we might be able to move the conditions there!
  if constexpr (Output == pixel_format::bgr888 || Output == pixel_format::rgb888) {
    Simd::Sse41::BayerToBgr(bayer.data(), width, height, width * 1, BayerFormat, out.data(), size_bytes(Output, width, 1));
    if constexpr (Output == pixel_format::rgb888) {
      Simd::Sse41::BgrToRgb(out.data(), width, height, size_bytes(Output, width, 1), out.data(), size_bytes(Output, width, 1));
    }
    return;
  }
  if constexpr (Output == pixel_format::bgra8888 || Output == pixel_format::rgba8888) {
    Simd::Sse41::BayerToBgra(bayer.data(), width, height, width * 1, BayerFormat, out.data(), size_bytes(Output, width, 1), 255);
    if constexpr (Output == pixel_format::rgba8888) {
      Simd::Sse41::BgraToRgba(out.data(), width, height, size_bytes(Output, width, 1), out.data(), size_bytes(Output, width, 1));
    }
    return;
  }
  demosaic<BayerFormat, Output>(bayer.data(), width, height, width * 1, out.data(), size_bytes(Output, width, 1));
}

void convert_frame_from_bayer8(pixel_format to,
                              std::span<const std::uint8_t> input_frame,
                              std::span<std::uint8_t> output_frame,
                              std::uint32_t width,
                              std::uint32_t height)
{
  switch (to) {
    case pixel_format::bayer8: std::memcpy(output_frame.data(), input_frame.data(), width * height); break;
    case pixel_format::bgr888:
      demosaic<SimdPixelFormatBayerGrbg, pixel_format::bgr888>(input_frame, width, height, output_frame);
      break;
    case pixel_format::rgb888:
      demosaic<SimdPixelFormatBayerGrbg, pixel_format::rgb888>(input_frame, width, height, output_frame);
      break;
    case pixel_format::bgra8888:
      demosaic<SimdPixelFormatBayerGrbg, pixel_format::bgra8888>(input_frame, width, height, output_frame);
      break;
    case pixel_format::rgba8888:
      demosaic<SimdPixelFormatBayerGrbg, pixel_format::rgba8888>(input_frame, width, height, output_frame);
      break;
    case pixel_format::gray:
      demosaic<SimdPixelFormatBayerGrbg, pixel_format::gray>(input_frame, width, height, output_frame);
      break;
    case pixel_format::uyvy:
      demosaic<SimdPixelFormatBayerGrbg, pixel_format::uyvy>(input_frame, width, height, output_frame);
      break;
    default: throw std::runtime_error("unimplemented");
  }
}

void convert_frame(pixel_format from,
                   pixel_format to,
                   std::span<const std::uint8_t> input_frame,
                   std::span<std::uint8_t> output_frame,
                   std::uint32_t width,
                   std::uint32_t height)
{
  if (from == pixel_format::bayer8)
    return convert_frame_from_bayer8(to, input_frame, output_frame, width, height);

  if (from == pixel_format::bgr888 && to == pixel_format::bgra8888)
    return Simd::Sse41::BgrToBgra(input_frame.data(), width, height, size_bytes(pixel_format::bgr888, width, 1),
                                 output_frame.data(), size_bytes(pixel_format::bgra8888, width, 1), 255);

  // TODO: more formats!
  throw std::runtime_error("unimplemented");
}

PSEYE_NS_END
