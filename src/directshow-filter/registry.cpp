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
#include "registry.hpp"

#include <wil/registry.h>
#include <wil/result.h>
#include <wil/com.h>

PSEYE_NS_BEGIN

HRESULT RegServer(const CLSID& cls, const wchar_t* desc, const wchar_t* file, const wchar_t* model, const wchar_t* type)
{
  wchar_t cls_str[CHARS_IN_GUID];
  wchar_t temp[MAX_PATH];

  ::StringFromGUID2(cls, cls_str, CHARS_IN_GUID);
  ::StringCbPrintfW(temp, sizeof(temp), L"CLSID\\%s", cls_str);

  return wil::ResultFromException(WI_DIAGNOSTICS_INFO, [&]() {
    const auto key = wil::reg::create_unique_key(HKEY_CLASSES_ROOT, temp, wil::reg::key_access::readwrite);
    const auto subkey = wil::reg::create_unique_key(key.get(), type, wil::reg::key_access::readwrite);
    wil::reg::set_value(key.get(), nullptr, desc);
    wil::reg::set_value(subkey.get(), nullptr, file);
    wil::reg::set_value(subkey.get(), L"ThreadingModel", model);
  });
}

HRESULT UnregServer(const CLSID& cls)
{
  wchar_t cls_str[CHARS_IN_GUID];
  wchar_t temp[MAX_PATH];

  ::StringFromGUID2(cls, cls_str, CHARS_IN_GUID);
  ::StringCbPrintfW(temp, sizeof(temp), L"CLSID\\%s", cls_str);

  return HRESULT_FROM_WIN32(::RegDeleteTreeW(HKEY_CLASSES_ROOT, temp));
}

HRESULT EditVideoDeviceFilter(const CLSID& cls, const wchar_t* name, const REGFILTER2& rf2, bool is_registration)
{
  wil::com_ptr<IFilterMapper2> fm;
  RETURN_IF_FAILED(
      CoCreateInstance(CLSID_FilterMapper2, nullptr, CLSCTX_INPROC_SERVER, IID_IFilterMapper2, fm.put_void()));

  if (is_registration) {
    wil::com_ptr<IMoniker> moniker;
    RETURN_IF_FAILED(fm->RegisterFilter(cls, name, moniker.put(), &CLSID_VideoInputDeviceCategory, nullptr, &rf2));
  } else {
    RETURN_IF_FAILED(fm->UnregisterFilter(&CLSID_VideoInputDeviceCategory, 0, cls));
  }

  return S_OK;
}

PSEYE_NS_END
