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
#ifndef PSEYE_DSHOW_MODULE_HPP
#define PSEYE_DSHOW_MODULE_HPP

#include "pseye/detail/config.hpp"

#if PSEYE_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "pseye/driver/usb_context.hpp"

#include <windows.h>

PSEYE_NS_BEGIN

extern std::atomic<ULONG> locks;
extern usb_context global_context;
extern HINSTANCE dll_instance;

template <typename T>
class SimpleClassFactory : public IClassFactory
{
public:
  virtual ~SimpleClassFactory() = default;

  // IUnknown
  STDMETHODIMP QueryInterface(REFIID riid, void** p_ptr) override
  {
    if (!p_ptr)
      return E_POINTER;
    if ((riid == IID_IUnknown) || (riid == IID_IClassFactory)) {
      AddRef();
      *p_ptr = this;
      return S_OK;
    }
    *p_ptr = nullptr;
    return E_NOINTERFACE;
  }
  STDMETHODIMP_(ULONG) AddRef() override { return ++refs; }
  STDMETHODIMP_(ULONG) Release() override
  {
    const auto new_refs = --refs;
    if (new_refs == 0) {
      delete this;
      return 0;
    }
    return new_refs;
  }

  // IClassFactory
  STDMETHODIMP CreateInstance(LPUNKNOWN parent, REFIID riid, void** p_ptr) override
  {
    if (!p_ptr)
      return E_POINTER;

    *p_ptr = nullptr;

    /* don't bother supporting the "parent" functionality */
    if (parent)
      return E_NOINTERFACE;

    *p_ptr = new T();
    return S_OK;
  }

  STDMETHODIMP LockServer(BOOL lock) override
  {
    if (lock)
      ++locks;
    else
      --locks;
    return S_OK;
  }

private:
  std::atomic<ULONG> refs = 1; // start out at 1 ref!
};

PSEYE_NS_END

#endif
