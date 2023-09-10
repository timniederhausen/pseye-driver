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
#ifndef PSEYE_DSHOW_MODULEGUID_HPP
#define PSEYE_DSHOW_MODULEGUID_HPP

#include "pseye/detail/config.hpp"

#if PSEYE_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <windows.h>
#include <initguid.h>

PSEYE_NS_BEGIN

#ifdef DEFINE_GUID
// {9B2722A6-775A-4258-8C89-B7393CC29A32}
DEFINE_GUID(CLSID_PSEye_Video, 0x9b2722a6, 0x775a, 0x4258, 0x8c, 0x89, 0xb7, 0x39, 0x3c, 0xc2, 0x9a, 0x32);
#endif

PSEYE_NS_END

#endif
