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
#ifndef PSEYE_DSHOW_CAMERAFILTER_HPP
#define PSEYE_DSHOW_CAMERAFILTER_HPP

#include "pseye/detail/config.hpp"

#if PSEYE_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <source/output-filter.hpp>

#include <atomic>
#include <thread>
#include <cstdint>

PSEYE_NS_BEGIN

class simple_pseye_camera;

class pseye_camera_filter : public DShow::OutputFilter
{
public:
  pseye_camera_filter();
  ~pseye_camera_filter() override;

  STDMETHODIMP Pause() override;
  STDMETHODIMP Run(REFERENCE_TIME tStart) override;
  STDMETHODIMP Stop() override;

protected:
  const wchar_t* FilterName() const override { return L"PlayStation Eye"; }

private:
  void run();
  void process_frame(bool active_previously, bool active, uint64_t filter_time);
  bool ensure_device_exists(bool want_initialized);

  uint64_t get_time();

  std::thread worker_thread_;

  std::atomic<bool> is_active_ = false;
  std::atomic<bool> is_stopped_ = false;
  uint32_t width_ = 0, height_ = 0;
  uint64_t interval_ = 0;

  std::unique_ptr<simple_pseye_camera> device_;
};

PSEYE_NS_END

#endif
