#pragma once
/*
Copyright (c) 2026 Matthew H. Reilly (kb1vc)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * @file RTLSDRDev.hxx
 * @brief Shared RAII wrapper for an rtlsdr_dev_t handle.
 *
 * RTLSDRCtrl and RTLSDRRX both need the same device handle.
 * Holding a shared_ptr to this wrapper ensures rtlsdr_close() is
 * called exactly once when both threads finish.
 */

#include <rtl-sdr.h>
#include <memory>
#include <string>

namespace SoDa {

  struct RTLSDRDev {
    rtlsdr_dev_t * dev;
    std::string    name;

    explicit RTLSDRDev(uint32_t index) : dev(nullptr) {
      name = std::string(rtlsdr_get_device_name(index));
      rtlsdr_open(&dev, index);
    }

    ~RTLSDRDev() {
      if (dev) { rtlsdr_close(dev); dev = nullptr; }
    }

    RTLSDRDev(const RTLSDRDev &)            = delete;
    RTLSDRDev & operator=(const RTLSDRDev &) = delete;
  };

  using RTLSDRDevPtr = std::shared_ptr<RTLSDRDev>;

} // namespace SoDa
