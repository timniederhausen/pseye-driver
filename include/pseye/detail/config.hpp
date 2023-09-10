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
#ifndef PSEYE_DETAIL_CONFIG_HPP
#define PSEYE_DETAIL_CONFIG_HPP

// Choose either/or
#define PSEYE_NS_BEGIN namespace pseye {
#define PSEYE_NS_END }
#define PSEYE_HARDWARE_NS_BEGIN namespace pseye { inline namespace hw {
#define PSEYE_HARDWARE_NS_END } }

#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__CODEGEARC__)
# if defined(PSEYE_DYN_LINK)
#  if defined(PSEYE_SOURCE)
#   define PSEYE_DECL __declspec(dllexport)
#  else
#   define PSEYE_DECL __declspec(dllimport)
#  endif
# endif
#endif

#if !defined(PSEYE_DECL)
# define PSEYE_DECL
#endif

// TODO: every supported compiler has that?
#define PSEYE_HAS_PRAGMA_ONCE 1

#endif
