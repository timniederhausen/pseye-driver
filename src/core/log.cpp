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
#include "pseye/log.hpp"

PSEYE_NS_BEGIN

namespace {

static log_severity min_severity = log_severity::info;
static log_callback log_cb;

}

void set_min_severity(log_severity severity) noexcept
{ min_severity = severity; }

void set_log_callback(log_callback cb) noexcept
{ log_cb = cb; }

bool is_severity_enabled(log_severity severity) noexcept
{ return severity >= min_severity; }

void vlog(log_severity severity, const char* filename, int line, fmt::string_view format, fmt::format_args args)
{
  if (log_cb)
    log_cb(severity, filename, line, std::string_view(format.data(), format.size()), args);
}

PSEYE_NS_END
