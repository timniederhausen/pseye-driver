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
#include <Simd/SimdYuvToBgr.h>

PSEYE_NS_BEGIN

// This tends to emit a 16-bit `rol` or `ror` instruction
SIMD_INLINE constexpr std::uint16_t swap16(std::uint16_t x)
{
  return static_cast<std::uint16_t>(((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8));
}

template <typename YuvType, std::size_t Y0, std::size_t Y1, std::size_t U0, std::size_t V0>
SIMD_INLINE void bgr_to_yuv(const std::uint8_t* bgr, std::uint8_t* out)
{
  out[Y0] = Simd::Base::BgrToY<YuvType>(bgr[0], bgr[1], bgr[2]);
  out[Y1] = Simd::Base::BgrToY<YuvType>(bgr[3], bgr[4], bgr[5]);

  const int blue = Simd::Base::Average(bgr[0], bgr[3]);
  const int green = Simd::Base::Average(bgr[1], bgr[4]);
  const int red = Simd::Base::Average(bgr[2], bgr[5]);

  out[U0] = Simd::Base::BgrToU<YuvType>(blue, green, red);
  out[V0] = Simd::Base::BgrToV<YuvType>(blue, green, red);
}

// simple YUYV <-> UYVY swap
void swap_yuyv_uyvy(const std::uint8_t* in,
                    std::size_t width,
                    std::size_t height,
                    std::size_t in_stride,
                    std::uint8_t* out,
                    std::size_t out_stride)
{
  for (std::size_t row = 0; row < height; ++row) {
    for (std::size_t col = 0, off = 0; col < width; col += 2, off += 4) {
      std::uint16_t block[2];
      std::memcpy(block, in + off, sizeof(block));
      block[0] = swap16(block[0]);
      block[1] = swap16(block[1]);
      std::memcpy(out + off, block, sizeof(block));
    }
    in += in_stride;
    out += out_stride;
  }
}

// TODO: this is quite unoptimized!
template <SimdPixelFormatType BayerFormat, pixel_format Output>
void demosaic(const uint8_t* src[6],
              std::size_t col0,
              std::size_t col2,
              std::size_t col4,
              uint8_t* dst0,
              std::size_t stride)
{
  uint8_t* dst1 = dst0 + stride;

  // Handle the simple BGR/RGB cases first:
  if constexpr (Output == pixel_format::bgra32 || Output == pixel_format::rgba32) {
    Simd::Base::BayerToBgr<BayerFormat>(src, col0, col0 + 1, col2, col2 + 1, col4, col4 + 1, dst0, dst0 + 4, dst1,
                                        dst1 + 4);
    dst0[3] = 255;
    dst0[7] = 255;
    dst1[3] = 255;
    dst1[7] = 255;
    if constexpr (Output == pixel_format::rgba32) {
      std::swap(dst0[0], dst0[2]);
      std::swap(dst0[4], dst0[6]);
      std::swap(dst1[0], dst1[2]);
      std::swap(dst1[4], dst1[6]);
    }
  } else if (Output == pixel_format::bgr24 || Output == pixel_format::rgb24) {
    Simd::Base::BayerToBgr<BayerFormat>(src, col0, col0 + 1, col2, col2 + 1, col4, col4 + 1, dst0, dst0 + 3, dst1,
                                        dst1 + 3);
    if constexpr (Output == pixel_format::rgb24) {
      std::swap(dst0[0], dst0[2]);
      std::swap(dst0[3], dst0[5]);
      std::swap(dst1[0], dst1[2]);
      std::swap(dst1[3], dst1[5]);
    }
  }

  // Now we need to temporarily convert to BGR
  uint8_t bgr[(2 * 2) * 3]; // 2x2 BGR block
  Simd::Base::BayerToBgr<BayerFormat>(src, col0, col0 + 1, col2, col2 + 1, col4, col4 + 1, bgr, bgr + 3, bgr + 6,
                                      bgr + 9);

  if constexpr (Output == pixel_format::yuyv) {
    bgr_to_yuv<Simd::Base::Bt601, 0, 2, 1, 3>(bgr, dst0);
    bgr_to_yuv<Simd::Base::Bt601, 0, 2, 1, 3>(bgr + 6, dst1);
    return;
  }

  if constexpr (Output == pixel_format::uyvy) {
    bgr_to_yuv<Simd::Base::Bt601, 1, 3, 0, 2>(bgr, dst0);
    bgr_to_yuv<Simd::Base::Bt601, 1, 3, 0, 2>(bgr + 6, dst1);
    return;
  }

  if constexpr (Output == pixel_format::gray) {
    dst0[0] = Simd::Base::BgrToGray(bgr[0], bgr[1], bgr[2]);
    dst0[1] = Simd::Base::BgrToGray(bgr[3], bgr[4], bgr[5]);
    dst1[0] = Simd::Base::BgrToGray(bgr[6 + 0], bgr[6 + 1], bgr[6 + 2]);
    dst1[1] = Simd::Base::BgrToGray(bgr[6 + 3], bgr[6 + 4], bgr[6 + 5]);
    return;
  }
}

template <SimdPixelFormatType BayerFormat, pixel_format Output>
void demosaic(const uint8_t* bayer,
              std::size_t width,
              std::size_t height,
              std::size_t bayer_stride,
              uint8_t* out,
              std::size_t out_stride)
{
  const uint8_t* src[6];
  for (std::size_t row = 0; row < height; row += 2) {
    src[0] = (row == 0 ? bayer : bayer - 2 * bayer_stride);
    src[1] = src[0] + bayer_stride;
    src[2] = bayer;
    src[3] = src[2] + bayer_stride;
    src[4] = (row == height - 2 ? bayer : bayer + 2 * bayer_stride);
    src[5] = src[4] + bayer_stride;

    demosaic<BayerFormat, Output>(src, 0, 0, 2, out, out_stride);

    for (std::size_t col = 2; col < width - 2; col += 2)
      demosaic<BayerFormat, Output>(src, col - 2, col, col + 2, out + size_bytes(Output, col, 1), out_stride);

    demosaic<BayerFormat, Output>(src, width - 4, width - 2, width - 2, out + size_bytes(Output, width - 2, 1),
                                  out_stride);

    bayer += 2 * bayer_stride;
    out += 2 * out_stride;
  }
}

inline constexpr std::size_t minus_one = static_cast<std::size_t>(-1);

template <pixel_format Output>
struct output_buffer_adapter
{
  output_buffer_adapter(std::span<std::uint8_t> out, std::size_t width, bool flip_v)
    : stride(flip_v ? minus_one * size_bytes(Output, width, 1) : size_bytes(Output, width, 1))
    , ptr(flip_v ? out.data() + out.size() + stride : out.data())
  {
  }

  operator std::uint8_t*() const { return ptr; }

  std::size_t stride;
  std::uint8_t* ptr;
};

template <SimdPixelFormatType BayerFormat, pixel_format Output>
SIMD_INLINE void demosaic(std::span<const std::uint8_t> bayer,
                          std::size_t width,
                          std::size_t height,
                          std::span<std::uint8_t> out,
                          bool flip_v = false)
{
  output_buffer_adapter<Output> output(out, width, flip_v);

  // TODO: if we add SSE4.1 overloads for the generic converter, we might be able to move the conditions there!
  if constexpr (Output == pixel_format::bgr24 || Output == pixel_format::rgb24) {
    Simd::Sse41::BayerToBgr(bayer.data(), width, height, width * 1, BayerFormat, output.ptr, output.stride);
    if constexpr (Output == pixel_format::rgb24) {
      Simd::Sse41::BgrToRgb(out.data(), width, height, size_bytes(Output, width, 1), out.data(),
                            size_bytes(Output, width, 1));
    }
    return;
  }
  if constexpr (Output == pixel_format::bgra32 || Output == pixel_format::rgba32) {
    Simd::Sse41::BayerToBgra(bayer.data(), width, height, width * 1, BayerFormat, output.ptr, output.stride, 255);
    if constexpr (Output == pixel_format::rgba32) {
      Simd::Sse41::BgraToRgba(out.data(), width, height, size_bytes(Output, width, 1), out.data(),
                              size_bytes(Output, width, 1));
    }
    return;
  }
  demosaic<BayerFormat, Output>(bayer.data(), width, height, width * 1, output.ptr, output.stride);
}

template <pixel_format Input, pixel_format Output>
void convert_rgb(std::span<const std::uint8_t> in,
                 std::size_t width,
                 std::size_t height,
                 std::span<std::uint8_t> out,
                 bool flip_v = false)
{
  output_buffer_adapter<Output> output(out, width, flip_v);

  // BGR
  if constexpr (Input == pixel_format::bgr24 && Output == pixel_format::bgra32)
    return Simd::Sse41::BgrToBgra(in.data(), width, height, size_bytes(Input, width, 1), output.ptr, output.stride,
                                  255);

  if constexpr (Input == pixel_format::bgr24 && Output == pixel_format::rgb24)
    return Simd::Sse41::BgrToRgb(in.data(), width, height, size_bytes(Input, width, 1), output.ptr, output.stride);

  if constexpr (Input == pixel_format::bgr24 && Output == pixel_format::gray)
    return Simd::Sse41::BgrToGray(in.data(), width, height, size_bytes(Input, width, 1), output.ptr, output.stride);

  // BGRA
  if constexpr (Input == pixel_format::bgra32 && Output == pixel_format::rgba32)
    return Simd::Sse41::BgraToRgba(in.data(), width, height, size_bytes(Input, width, 1), output.ptr, output.stride);

  if constexpr (Input == pixel_format::bgra32 && Output == pixel_format::gray)
    return Simd::Sse41::BgraToGray(in.data(), width, height, size_bytes(Input, width, 1), output.ptr, output.stride);

  throw std::runtime_error("unimplemented");
}

template <pixel_format Input, pixel_format Output>
void convert_yuv(std::span<const std::uint8_t> in,
                 std::size_t width,
                 std::size_t height,
                 std::span<std::uint8_t> out,
                 bool flip_v = false)
{
  output_buffer_adapter<Output> output(out, width, flip_v);

  // XXX: only thing we support so far!
  return swap_yuyv_uyvy(in.data(), width, height, size_bytes(pixel_format::yuyv, width, 1), output.ptr, output.stride);
}

template <SimdPixelFormatType BayerFormat>
void demosaic(pixel_format to,
              std::span<const std::uint8_t> input_frame,
              std::span<std::uint8_t> output_frame,
              std::size_t width,
              std::size_t height,
              bool flip_v)
{
  switch (to) {
    case pixel_format::bgr24:
      demosaic<BayerFormat, pixel_format::bgr24>(input_frame, width, height, output_frame, flip_v);
      break;
    case pixel_format::rgb24:
      demosaic<BayerFormat, pixel_format::rgb24>(input_frame, width, height, output_frame, flip_v);
      break;
    case pixel_format::bgra32:
      demosaic<BayerFormat, pixel_format::bgra32>(input_frame, width, height, output_frame, flip_v);
      break;
    case pixel_format::rgba32:
      demosaic<BayerFormat, pixel_format::rgba32>(input_frame, width, height, output_frame, flip_v);
      break;
    case pixel_format::gray:
      demosaic<BayerFormat, pixel_format::gray>(input_frame, width, height, output_frame, flip_v);
      break;
    case pixel_format::uyvy:
      demosaic<BayerFormat, pixel_format::uyvy>(input_frame, width, height, output_frame, flip_v);
      break;
    case pixel_format::yuyv:
      demosaic<BayerFormat, pixel_format::yuyv>(input_frame, width, height, output_frame, flip_v);
      break;
    default: throw std::runtime_error("unimplemented");
  }
}

template <pixel_format Input>
void convert_rgb(pixel_format to,
                 std::span<const std::uint8_t> input_frame,
                 std::span<std::uint8_t> output_frame,
                 std::size_t width,
                 std::size_t height,
                 bool flip_v)
{
  switch (to) {
    case pixel_format::bgr24:
      return convert_rgb<Input, pixel_format::bgr24>(input_frame, width, height, output_frame, flip_v);
    case pixel_format::rgb24:
      return convert_rgb<Input, pixel_format::rgb24>(input_frame, width, height, output_frame, flip_v);
    case pixel_format::bgra32:
      return convert_rgb<Input, pixel_format::bgra32>(input_frame, width, height, output_frame, flip_v);
    case pixel_format::rgba32:
      return convert_rgb<Input, pixel_format::rgba32>(input_frame, width, height, output_frame, flip_v);
    case pixel_format::gray:
      return convert_rgb<Input, pixel_format::gray>(input_frame, width, height, output_frame, flip_v);
    default: throw std::runtime_error("unimplemented");
  }
}

template <pixel_format Input>
void convert_yuv(pixel_format to,
                 std::span<const std::uint8_t> input_frame,
                 std::span<std::uint8_t> output_frame,
                 std::size_t width,
                 std::size_t height,
                 bool flip_v)
{
  switch (to) {
    case pixel_format::yuyv:
      return convert_yuv<Input, pixel_format::yuyv>(input_frame, width, height, output_frame, flip_v);
    case pixel_format::uyvy:
      return convert_yuv<Input, pixel_format::uyvy>(input_frame, width, height, output_frame, flip_v);
    default: throw std::runtime_error("unimplemented");
  }
}

void convert_frame(pixel_format from,
                   pixel_format to,
                   std::span<const std::uint8_t> input_frame,
                   std::span<std::uint8_t> output_frame,
                   std::size_t width,
                   std::size_t height,
                   bool flip_v)
{
  if (from == to) {
    // assert(output_frame.size() == input_frame.size());
    std::memcpy(output_frame.data(), input_frame.data(), output_frame.size());
    return;
  }

  switch (from) {
    case pixel_format::grbg8:
      return demosaic<SimdPixelFormatBayerGrbg>(to, input_frame, output_frame, width, height, flip_v);
    case pixel_format::bgr24:
      return convert_rgb<pixel_format::bgr24>(to, input_frame, output_frame, width, height, flip_v);
    case pixel_format::rgb24:
      return convert_rgb<pixel_format::rgb24>(to, input_frame, output_frame, width, height, flip_v);
    case pixel_format::bgra32:
      return convert_rgb<pixel_format::bgra32>(to, input_frame, output_frame, width, height, flip_v);
    case pixel_format::rgba32:
      return convert_rgb<pixel_format::rgba32>(to, input_frame, output_frame, width, height, flip_v);
    case pixel_format::yuyv:
      return convert_yuv<pixel_format::yuyv>(to, input_frame, output_frame, width, height, flip_v);
    case pixel_format::uyvy:
      return convert_yuv<pixel_format::uyvy>(to, input_frame, output_frame, width, height, flip_v);
    default:
      // TODO: more formats!
      throw std::runtime_error("unimplemented");
  }
}

PSEYE_NS_END
