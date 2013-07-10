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

#include "config.h"

#include "core/rivertrail/OCLUtil.h"

#include "core/rivertrail/OCLdebug.h"

namespace WebCore {

#if defined(WTF_OS_WINDOWS)
#define checkFunction(f)                                         \
    if (!(f)) {                                                  \
        FreeLibrary(openclModule);                               \
        openclModule = 0;                                        \
        DEBUG_LOG_STATUS("Init", "Get OpenCL function failed."); \
        return;                                                  \
    }

// Define OpenCL function entries
bool openclFlag = false;
#define DEFINE_FUNCTION_ENTRY(name) name##Function __##name = 0;
OPENCL_FUNCTION_LIST(DEFINE_FUNCTION_ENTRY)
#undef DEFINE_FUNCTION_ENTRY
#endif // WTF_OS_WINDOWS

void CL_CALLBACK reportCLError(const char* err_info, const void* private_info, size_t cb, void* user_data)
{
    DEBUG_LOG_CLERROR(err_info);
}

OCLUtil::OCLUtil() {
}

OCLUtil::~OCLUtil() {
}

void OCLUtil::Init() {
  cl_int error = 0;   // Used to handle error codes
  cl_uint numberOfPlatforms;
  cl_uint nplatforms;
  cl_device_id device;
  size_t length;

  createContextSuccess = false;
  createCommandQueueSuccess = false;

#if defined(WTF_OS_WINDOWS)
  openclModule = 0;
  // Load OpenCL library
  openclModule = LoadLibrary(TEXT("OpenCL.dll"));
  if (!openclModule) {
      DEBUG_LOG_STATUS("Init", "Load OpenCL.dll failed.");
      return;
  }
  // Initialize function entries
#define INITIALIZE_FUNCTION_ENTRY(name) checkFunction(__##name = (name##Function) GetProcAddress(openclModule, #name));
  OPENCL_FUNCTION_LIST(INITIALIZE_FUNCTION_ENTRY)
#undef INITIALIZE_FUNCTION_ENTRY
  openclFlag = true;
#endif // WTF_OS_WINDOWS

  // Platform
  error = clGetPlatformIDs( 0, 0, &nplatforms);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", error);
      return;
  }
  cl_platform_id* m_platforms = new cl_platform_id[nplatforms];
  error = clGetPlatformIDs(nplatforms, m_platforms, &numberOfPlatforms);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", error);
      delete [] m_platforms;
      return;
  }

  const cl_uint maxNameLength = 256;
  char name[maxNameLength];
  for (cl_uint i = 0; i < numberOfPlatforms; i++) {
      error = clGetPlatformInfo(m_platforms[i], CL_PLATFORM_NAME, maxNameLength * sizeof(char), name, 0);
      if (error != CL_SUCCESS) {
          DEBUG_LOG_ERROR("Init", error);
      } else {
          if (!strcmp(name, "Intel(R) OpenCL") || !strcmp(name, "AMD Accelerated Parallel Processing")) {
              platform_ = m_platforms[i];
              break;
          }
      }
  }
  delete [] m_platforms;
  if (!platform_) {
      DEBUG_LOG_STATUS("Init", "Find Intel or AMD platform failed.");
      return;
  }

  // Version
  char* temp;
  error = getPlatformPropertyHelper(CL_PLATFORM_VERSION, temp);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", error);
  } else {
      version_ =  std::string(temp);
      delete [] temp;
  }
  // Name
  error = getPlatformPropertyHelper(CL_PLATFORM_NAME, temp);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", error);
  } else {
      name_ = std::string(temp);
      delete [] temp;
  }
  // Vendor
  error = getPlatformPropertyHelper(CL_PLATFORM_VENDOR, temp);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", error);
  } else {
      vendor_ = std::string(temp);
      delete [] temp;
  }
  // Profile
  error = getPlatformPropertyHelper(CL_PLATFORM_PROFILE, temp);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", error);
  } else {
      profile_ = std::string(temp);
      delete [] temp;
  }
  // Platform Extensions
  error = getPlatformPropertyHelper(CL_PLATFORM_EXTENSIONS, temp);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", error);
  } else {
      platformExtensions_ = std::string(temp);
      delete [] temp;
  }

  // Number of Device
  cl_uint number;
  error = clGetDeviceIDs(platform_, CL_DEVICE_TYPE_ALL, 0, 0, &number);
  if (error != CL_SUCCESS) {
     DEBUG_LOG_ERROR("Init", error);
  } else {
     numberOfDevices_ = number;
  }

  // Context
  cl_context_properties context_properties[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform_, 0};
  context_ = clCreateContextFromType(context_properties, CL_DEVICE_TYPE_CPU, &reportCLError, this, &error);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", error);
      return;
  }
  createContextSuccess = true;

  // Device
  size_t cb;
  error = clGetContextInfo(context_, CL_CONTEXT_DEVICES, 0, 0, &cb);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", error);
      return;
  }
  cl_device_id* devices = (cl_device_id*)malloc(sizeof(cl_device_id) * cb);
  if (!devices) {
      DEBUG_LOG_STATUS("Init", "Cannot allocate device list");
      return;
  }
  error = clGetContextInfo(context_, CL_CONTEXT_DEVICES, cb, devices, 0);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", error);
      free(devices);
      return;
  }

  // Command Queue
  queue_ = clCreateCommandQueue(context_, devices[0],
#ifdef CLPROFILE
      CL_QUEUE_PROFILING_ENABLE |
#endif // CLPROFILE
#ifdef OUTOFORDERQUEUE
      CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
#endif // OUTOFORDERQUEUE
      0,
      &error);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", error);
      free(devices);
      return;
  }
  DEBUG_LOG_STATUS("Init", "queue is " << queue_);
  createCommandQueueSuccess = true;

  error = clGetDeviceInfo(devices[0], CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(alignmentSize_), &alignmentSize_, 0);
  if (error != CL_SUCCESS) {
      // We can tolerate this, simply do not align.
      alignmentSize_ = 8;
  }
  // We use byte, not bits.
  if (alignmentSize_ % 8) {
      // They align on sub-byte borders? Odd architecture this must be. Give up.
      alignmentSize_ = 1;
  } else {
      alignmentSize_ = alignmentSize_ / 8;
  }

  // Device Extensions
  error = clGetCommandQueueInfo(queue_, CL_QUEUE_DEVICE, sizeof(cl_device_id), &device, 0);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", error);
  } else {
      error = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, 0, 0, &length);
      if (error == CL_SUCCESS) {
          temp = new char[length+1];
          error = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, length, temp, 0);
          deviceExtensions_ = std::string(temp);
          delete [] temp;
      } else {
          DEBUG_LOG_ERROR("Init", error);
      }
  }
  temp = 0;

  free(devices);
}

void OCLUtil::Finalize() {
    cl_int error;
#if defined(WTF_OS_WINDOWS)
    if (!openclFlag)
        return;
#endif // WTF_OS_WINDOWS
    if (createCommandQueueSuccess) {
        error = clReleaseCommandQueue(queue_);
        if (error != CL_SUCCESS) {
            DEBUG_LOG_ERROR("Finalize", error);
        } else {
            createCommandQueueSuccess = false;
        }
    }
    if (createContextSuccess) {
        error = clReleaseContext(context_);
        if (error != CL_SUCCESS) {
            DEBUG_LOG_ERROR("Finalize", error);
        } else {
            createContextSuccess = false;
        }
    }
#if defined(WTF_OS_WINDOWS)
    FreeLibrary(openclModule);
    openclModule = 0;
    openclFlag = false;
#endif // WTF_OS_WINDOWS
}

int OCLUtil::getPlatformPropertyHelper(cl_platform_info param, char*& out) {
  char* rString = 0;
  size_t length;
  cl_int err;
  int result;

  err = clGetPlatformInfo(platform_, param, 0, 0, &length);

  if (err == CL_SUCCESS) {
      rString = new char[length+1];
      err = clGetPlatformInfo(platform_, param, length, rString, 0);
      out = rString;
      result = CL_SUCCESS;
  } else {
      result = err;
  }
  return result;
}

int OCLUtil::numberOfDevices() {
    return numberOfDevices_;
}

cl_uint OCLUtil::alignmentSize() {
    return alignmentSize_;
}

std::string OCLUtil::version() {
    return version_;
}

std::string OCLUtil::name() {
    return name_;
}

std::string OCLUtil::vendor() {
    return vendor_;
}

std::string OCLUtil::profile() {
    return profile_;
}

std::string OCLUtil::platformExtensions() {
    return platformExtensions_;
}

cl_platform_id OCLUtil::platform() {
    return platform_;
}

cl_context OCLUtil::context() {
    return context_;
}

cl_command_queue OCLUtil::queue() {
    return queue_;
}

std::string OCLUtil::deviceExtensions() {
    return deviceExtensions_;
}
}  // namespace content
