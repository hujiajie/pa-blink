/*
 * Copyright (c) 2013, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef OCLUtil_h
#define OCLUtil_h

#include "OCLconfig.h"
#include "opencl_compat.h"
#include <string>

#if defined(WTF_OS_WINDOWS)
#include <windows.h>
#endif

namespace WebCore {

#if defined(WTF_OS_WINDOWS)
/* Declare variables of function entries */
extern bool openclFlag;

#define DECLARE_FUNCTION_ENTRY(name) extern name##Function __##name;
OPENCL_FUNCTION_LIST(DECLARE_FUNCTION_ENTRY)
#undef DECLARE_FUNCTION_ENTRY
#endif // WTF_OS_WINDOWS

static cl_platform_id platform_;
static cl_context context_;
static cl_command_queue queue_;
static std::string version_;
static std::string name_;
static std::string vendor_;
static std::string profile_;
static std::string platformExtensions_;
static cl_uint numberOfDevices_;
static cl_uint alignmentSize_;
static std::string deviceExtensions_;
static bool createContextSuccess;
static bool createCommandQueueSuccess;
#if defined(WTF_OS_WINDOWS)
static HMODULE openclModule;
#endif // WTF_OS_WINDOWS

class RivertrailExport OCLUtil {
 public:
  OCLUtil();
  ~OCLUtil();
  void Init();
  void Finalize();
  int getPlatformPropertyHelper(cl_platform_info param, char*& out);
  int numberOfDevices();
  cl_uint alignmentSize();
  std::string version();
  std::string name();
  std::string vendor();
  std::string profile();
  std::string platformExtensions();
  cl_platform_id platform();
  cl_context context();
  cl_command_queue queue();
  std::string deviceExtensions();
};

}  // namespace webcore

#endif  // OCLUtil_h
