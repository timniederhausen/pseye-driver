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
#ifndef PSEYE_DRIVER_PSEYEDEVICECONTROLLEROPS_HPP
#define PSEYE_DRIVER_PSEYEDEVICECONTROLLEROPS_HPP

#include "pseye/detail/hardware.hpp"

#if PSEYE_HAS_PRAGMA_ONCE
# pragma once
#endif

#include "pseye/hw/ov534.hpp"
#include "pseye/hw/ov7725.hpp"

#include <cstddef>
#include <cstdint>

PSEYE_NS_BEGIN

class pseye_device_controller;

// Camera Bridge Processor
std::uint8_t read_register(pseye_device_controller& controller, ov534::reg reg);
void write_register(pseye_device_controller& controller, ov534::reg reg, uint8_t val);

template <std::size_t N>
void write_register(pseye_device_controller& controller, const register_setting<ov534::reg> (&sequence)[N])
{
  for (std::size_t i = 0; i != N; ++i)
    write_register(controller, sequence[i].reg, sequence[i].value);
}

// Camera Bridge Processor -> Serial Camera Control Bus (SCCB) Master -> Camera sensor
uint8_t read_sccb_register(pseye_device_controller& controller, ov7725::reg reg);
void write_sccb_register(pseye_device_controller& controller, ov7725::reg reg, uint8_t val);
bool check_sccb_status(pseye_device_controller& controller);

template <std::size_t N>
void write_sccb_register(pseye_device_controller& controller,
                         const register_setting<ov7725::reg> (&sequence)[N])
{
  for (std::size_t i = 0; i != N; ++i)
    write_sccb_register(controller, sequence[i].reg, sequence[i].value);
}

// Write to indirect video data registers (own address space)
void write_video_data(pseye_device_controller& controller, const void* data, int len, std::uint8_t offset = 0);

template <typename T>
void write_video_data(pseye_device_controller& controller, const T& data, std::uint8_t offset = 0)
{
  write_video_data(controller, &data, sizeof(data), offset);
}

PSEYE_NS_END

#endif
