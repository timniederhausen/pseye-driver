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
#include "camera-filter.hpp"
#include "module.hpp"

#include "pseye/driver/simple_pseye_camera.hpp"
#include "pseye/driver/spsc_frame_buffer.hpp"
#include "pseye/driver/usb_context.hpp"
#include "pseye/log.hpp"

#include <source/dshow-formats.hpp>

#include <shlobj_core.h>
#include <strsafe.h>
#include <inttypes.h>

PSEYE_NS_BEGIN

/* ========================================================================= */

using hundred_ns = std::chrono::duration<long long, std::ratio_multiply<std::ratio<100>, std::nano>>;

inline constexpr std::uint64_t one_second_in100_ns = 100 * 100000;
inline constexpr std::chrono::milliseconds frame_wait_time(10);

inline constexpr bool enable_vga = true;
inline constexpr bool enable_qvga = true;

// first format is default!
inline constexpr DShow::VideoFormat supported_formats[] = {
    DShow::VideoFormat::XRGB, // 32bit RGB, alpha always 255
    DShow::VideoFormat::YUY2, // YUYV 4:2:2
    DShow::VideoFormat::UYVY  // UYVY 4:2:2
};

pseye_camera_filter::pseye_camera_filter()
{
  PSEYE_LOG_DEBUG("pseye_camera_filter::pseye_camera_filter");

  if (enable_vga) {
    for (const auto fps : vga_frame_rates) {
      for (const auto fmt : supported_formats) {
        AddVideoFormat(fmt, 640, 480, one_second_in100_ns / fps);
      }
    }
  }

  if (enable_qvga) {
    for (const auto fps : qvga_frame_rates) {
      for (const auto fmt : supported_formats) {
        AddVideoFormat(fmt, 320, 240, one_second_in100_ns / fps);
      }
    }
  }

  // default
  if (enable_vga)
    SetVideoFormat(supported_formats[0], 640, 480, one_second_in100_ns / vga_frame_rates[0]);
  else if (enable_qvga)
    SetVideoFormat(supported_formats[0], 320, 240, one_second_in100_ns / qvga_frame_rates[0]);

  worker_thread_ = std::thread(&pseye_camera_filter::run, this);

  AddRef();
  ++locks;
}

pseye_camera_filter::~pseye_camera_filter()
{
  PSEYE_LOG_DEBUG("pseye_camera_filter::~pseye_camera_filter");

  is_active_ = false;
  is_stopped_ = true;

  if (worker_thread_.joinable())
    worker_thread_.join();

  --locks;
}

STDMETHODIMP pseye_camera_filter::Pause()
{
  PSEYE_LOG_DEBUG("camera input paused");
  is_active_ = false;
  return OutputFilter::Pause();
}

STDMETHODIMP pseye_camera_filter::Run(REFERENCE_TIME tStart)
{
  PSEYE_LOG_DEBUG("camera input started at {}", tStart);
  is_active_ = true;
  return OutputFilter::Run(tStart);
}

STDMETHODIMP pseye_camera_filter::Stop()
{
  PSEYE_LOG_DEBUG("camera input stopped");
  is_active_ = false;
  return OutputFilter::Stop();
}

void pseye_camera_filter::run()
{
  uint64_t filter_time = get_time();
  bool active_now = is_active_;
  interval_ = GetInterval();

  PSEYE_LOG_DEBUG("entering camera loop at {} interval {} active {}", filter_time, interval_, active_now);

  // if (!ensure_device_exists(active_now))
  //  return;

  bool active_previously = active_now;
  while (!is_stopped_) {
    const auto now = std::chrono::steady_clock::now();

    active_now = is_active_; // load atomic state
    process_frame(active_previously, active_now, filter_time);
    active_previously = active_now;

    std::this_thread::sleep_until(now + hundred_ns(interval_));
    filter_time += interval_;
  }

  PSEYE_LOG_DEBUG("exiting camera loop at {}", filter_time);

  if (device_)
    device_->stop();
}

pixel_format convert_video_format(DShow::VideoFormat format)
{
  switch (format) {
    case DShow::VideoFormat::Any: break;
    case DShow::VideoFormat::Unknown: break;
    case DShow::VideoFormat::ARGB: return pixel_format::bgra32;
    case DShow::VideoFormat::XRGB: return pixel_format::bgra32;
    case DShow::VideoFormat::RGB24: return pixel_format::bgr24;
    case DShow::VideoFormat::I420: break;
    case DShow::VideoFormat::NV12: break;
    case DShow::VideoFormat::YV12: break;
    case DShow::VideoFormat::Y800: break;
    case DShow::VideoFormat::P010: break;
    case DShow::VideoFormat::YVYU: break;
    case DShow::VideoFormat::YUY2: return pixel_format::yuyv;
    case DShow::VideoFormat::UYVY: return pixel_format::uyvy;
    case DShow::VideoFormat::HDYC: break;
    case DShow::VideoFormat::MJPEG: break;
    case DShow::VideoFormat::H264: break;
    case DShow::VideoFormat::HEVC: break;
  }
  // shouldn't be reachable!
  throw std::runtime_error("invalid format");
}

bool need_to_flip_v(pixel_format native_fmt, pixel_format out_fmt, bool native_flip_v)
{
  switch (out_fmt) {
    case pixel_format::bgr24:
    case pixel_format::rgb24:
    case pixel_format::bgra32:
    case pixel_format::rgba32: return native_fmt == pixel_format::grbg8;
    default: return false;
  }
}

void pseye_camera_filter::process_frame(bool active_previously, bool active, uint64_t filter_time)
{
  // handle simple state transitions here
  if (!active_previously && active) {
    if (!ensure_device_exists(true))
      return;
  } else if (active_previously && !active) {
    PSEYE_LOG_DEBUG("actually stopping device {}", !!device_);
    if (!device_)
      return;
    device_->stop();
  }

  if (!active)
    return;

  // check for parameter change
  if (width_ != GetCX() || height_ != GetCY() || interval_ != GetInterval()) {
    device_->stop();
    if (!ensure_device_exists(true))
      return;
  }

  uint8_t* ptr;
  if (!LockSampleData(&ptr))
    return;

  const auto out_fmt = convert_video_format(GetVideoFormat());
  const auto output = std::span(ptr, size_bytes(out_fmt, width_, height_));
  const bool need_flip = need_to_flip_v(device_->state().format, out_fmt, device_->state().flip_v);

  // try to dequeue a frame for |frame_wait_time| between checking if we're asked to stop
  do {
    const auto res = device_->frame_buffer().readable_frame_wait_for(frame_wait_time);
    if (!res.empty()) {
      convert_frame(device_->state().format, out_fmt, res, output, width_, height_, need_flip);
      device_->frame_buffer().finish_reading();
      break;
    }
  } while (is_active_);
  UnlockSampleData(filter_time, filter_time + interval_);
}

bool pseye_camera_filter::ensure_device_exists(bool want_initialized)
{
  if (!device_) {
    const auto device_handle = global_context.open_device(pseye_vendor_id, pseye_product_id);
    if (!device_handle) {
      PSEYE_LOG_WARNING("cannot find/create USB device");
      return false;
    }
    device_.reset(new simple_pseye_camera(device_handle, {}));
  }

  if (want_initialized && !device_->is_active()) {
    width_ = GetCX();
    height_ = GetCY();
    interval_ = GetInterval();

    try {
      device_->start(width_ != 320 ? size_mode::vga : size_mode::qvga, one_second_in100_ns / interval_,
                     pixel_format::grbg8);
    } catch (std::exception& e) {
      PSEYE_LOG_ERROR("failed to start camera with: w {} h {} fps {}: {}", width_, height_,
                      one_second_in100_ns / interval_, e.what());
      return false;
    }
    const auto& state = device_->state();
    PSEYE_LOG_DEBUG("started camera with: w {} h {} fps {}", state.width, state.height, state.rate);
  }
  return true;
}

uint64_t pseye_camera_filter::get_time()
{
  if (clock) {
    REFERENCE_TIME rt;
    HRESULT hr = clock->GetTime(&rt);
    if (SUCCEEDED(hr)) {
      return (uint64_t)rt;
    }
  }

  return std::chrono::duration_cast<hundred_ns>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

PSEYE_NS_END
