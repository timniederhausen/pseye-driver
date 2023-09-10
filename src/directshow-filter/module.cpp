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
#include "module.hpp"
#include "camera-filter.hpp"
#include "module-guid.hpp"
#include "registry.hpp"

#include "pseye/driver/usb_context.hpp"
#include "pseye/log.hpp"

#include <codecvt>

PSEYE_NS_BEGIN

std::atomic<ULONG> locks = 0;
usb_context global_context;
HINSTANCE dll_instance = nullptr;

inline constexpr wchar_t camera_filter_display_name[] = L"PlayStation Eye";

static HRESULT RegServers(bool reg)
{
  wchar_t file[MAX_PATH];
  if (!GetModuleFileNameW(dll_instance, file, MAX_PATH)) {
    return HRESULT_FROM_WIN32(GetLastError());
  }

  if (reg) {
    return RegServer(CLSID_PSEye_Video, camera_filter_display_name, file);
  } else {
    return UnregServer(CLSID_PSEye_Video);
  }
}

static HRESULT RegFilters(bool reg)
{
  static const REGPINTYPES AMSMediaTypesV = {&MEDIATYPE_Video, &MEDIASUBTYPE_RGB32};

  static const REGFILTERPINS AMSPinVideo = {nullptr,     false,   true, false,          false,
                                            &CLSID_NULL, nullptr, 1,    &AMSMediaTypesV};

  REGFILTER2 rf2;
  rf2.dwVersion = 1;
  rf2.dwMerit = MERIT_DO_NOT_USE;
  rf2.cPins = 1;
  rf2.rgPins = &AMSPinVideo;
  return EditVideoDeviceFilter(CLSID_PSEye_Video, camera_filter_display_name, rf2, reg);
}

STDAPI DllRegisterServer()
{
  auto hr = RegServers(true);
  if (FAILED(hr)) {
    RegServers(false);
    return hr;
  }

  CoInitialize(0);

  hr = RegFilters(true);
  if (FAILED(hr)) {
    RegFilters(false);
    RegServers(false);
  }

  CoUninitialize();
  return hr;
}

STDAPI DllUnregisterServer()
{
  CoInitialize(0);
  RegFilters(false);
  RegServers(false);
  CoUninitialize();
  return S_OK;
}

STDAPI DllInstall(BOOL install, LPCWSTR)
{
  return install ? DllRegisterServer() : DllUnregisterServer();
}

STDAPI DllCanUnloadNow()
{
  return locks ? S_FALSE : S_OK;
}

STDAPI DllGetClassObject(REFCLSID cls, REFIID riid, void** p_ptr)
{
  if (!p_ptr)
    return E_POINTER;

  *p_ptr = nullptr;

  if (riid != IID_IClassFactory && riid != IID_IUnknown) {
    return E_NOINTERFACE;
  }

  if (IsEqualCLSID(cls, CLSID_PSEye_Video)) {
    *p_ptr = new SimpleClassFactory<pseye_camera_filter>();
    return S_OK;
  }

  return E_INVALIDARG;
}

void dshow_log_callback(DShow::LogType level, const wchar_t* msg, void*)
{
  const auto utf8_msg = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(msg);
  switch (level) {
    case DShow::LogType::Error: PSEYE_LOG_ERROR("DShow: {}", utf8_msg); break;
    case DShow::LogType::Warning: PSEYE_LOG_WARNING("DShow: {}", utf8_msg); break;
    case DShow::LogType::Info: PSEYE_LOG_INFO("DShow: {}", utf8_msg); break;
    case DShow::LogType::Debug: PSEYE_LOG_DEBUG("DShow: {}", utf8_msg); break;
  }
}

void simple_pseye_log(log_severity severity,
                      const char* filename,
                      int line,
                      std::string_view format,
                      fmt::format_args args)
{
#ifdef _DEBUG
  static FILE* fp = nullptr;
  static bool tried = false;
  if (!fp && !tried) {
    const char* log_filename = std::getenv("PSEYE_LOG_FILENAME");
    if (!log_filename)
      log_filename = "D:\\pseye\\pseye-dshow.log";
    fp = std::fopen(log_filename, "wb+");
    if (!fp)
      fp = std::fopen("D:\\pseye\\pseye-dshow-1.log", "wb+");
    tried = true;
  }

  if (!fp)
    return;

  fmt::print(fp, "{}:{}: {}\n", filename ? filename : "none", line, fmt::vformat(format, args));
  std::fflush(fp);
#endif
}

extern "C" BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID)
{
  if (reason == DLL_PROCESS_ATTACH) {
    dll_instance = inst;

    set_min_severity(log_severity::debug);
    set_log_callback(&simple_pseye_log);

    DShow::SetLogCallback(dshow_log_callback, nullptr);
  }
  return true;
}

PSEYE_NS_END
