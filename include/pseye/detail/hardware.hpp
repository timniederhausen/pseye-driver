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
#ifndef PSEYE_DETAIL_HARDWARE_HPP
#define PSEYE_DETAIL_HARDWARE_HPP

#include "pseye/detail/config.hpp"

#if PSEYE_HAS_PRAGMA_ONCE
# pragma once
#endif

#include <cstdint>

PSEYE_HARDWARE_NS_BEGIN

inline constexpr std::uint8_t bit_0 = 0x01;
inline constexpr std::uint8_t bit_1 = 0x02;
inline constexpr std::uint8_t bit_2 = 0x04;
inline constexpr std::uint8_t bit_3 = 0x08;
inline constexpr std::uint8_t bit_4 = 0x10;
inline constexpr std::uint8_t bit_5 = 0x20;
inline constexpr std::uint8_t bit_6 = 0x40;
inline constexpr std::uint8_t bit_7 = 0x80;

// TODO: packing for non-byte Register?
template <typename Register>
struct register_setting
{
  Register reg;
  std::uint8_t value;
};
static_assert(sizeof(register_setting<uint8_t>) == 2);

// template <typename Register, std::size_t N>
// using fixed_register_sequence = std::array<register_setting<Register>, N>;

PSEYE_HARDWARE_NS_END

#endif
