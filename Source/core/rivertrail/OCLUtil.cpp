#include "config.h"

#include "core/rivertrail/OCLUtil.h"

namespace WebCore {

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

  // Platform
  error = clGetPlatformIDs( 0, 0, &nplatforms);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", "Get platform number error: " << error);
  }
  cl_platform_id* m_platforms = new cl_platform_id[nplatforms];
  error = clGetPlatformIDs(nplatforms, m_platforms, &numberOfPlatforms);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", "Get platform id error: " << error);
  }

  const cl_uint maxNameLength = 256;
  char name[maxNameLength];
  for (cl_uint i = 0; i < numberOfPlatforms; i++) {
      error = clGetPlatformInfo(m_platforms[i], CL_PLATFORM_NAME, maxNameLength * sizeof(char), name, 0);
      if (error != CL_SUCCESS) {
          DEBUG_LOG_ERROR("Init", "Get platform name error: " << error);
      } else {
          if (!strcmp(name, "Intel(R) OpenCL") || !strcmp(name, "Apple"))
              platform_ = m_platforms[i];
	  }
  }
  delete [] m_platforms;

  // Version
  char* temp;
  error = getPlatformPropertyHelper(CL_PLATFORM_VERSION, temp);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", "Get platform version error: " << error);
  } else {
      version_ =  std::string(temp);
      delete [] temp;
  }
  // Name
  error = getPlatformPropertyHelper(CL_PLATFORM_NAME, temp);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", "Get platform name error: " << error);
  } else {
      name_ = std::string(temp);
      delete [] temp;
  }
  // Vendor
  error = getPlatformPropertyHelper(CL_PLATFORM_VENDOR, temp);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", "Get platform vendor error: " << error);
  } else {
      vendor_ = std::string(temp);
      delete [] temp;
  }
  // Profile
  error = getPlatformPropertyHelper(CL_PLATFORM_PROFILE, temp);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", "Get platform profile error: " << error);
  } else {
      profile_ = std::string(temp);
      delete [] temp;
  }
  // Platform Extensions
  error = getPlatformPropertyHelper(CL_PLATFORM_EXTENSIONS, temp);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", "Get platform extension error: " << error);
  } else {
      platformExtensions_ = std::string(temp);
      delete [] temp;
  }

  // Number of Device
  cl_uint number;
  error = clGetDeviceIDs(platform_, CL_DEVICE_TYPE_ALL, 0, 0, &number);
  if (error != CL_SUCCESS) {
     DEBUG_LOG_ERROR("Init", "Get device number error: " << error);
  } else {
     numberOfDevices_ = number;
  }

  // Context
  cl_context_properties context_properties[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform_, 0};
  context_ = clCreateContextFromType(context_properties, CL_DEVICE_TYPE_CPU, &reportCLError, this, &error);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", "Create context error: " << error);
  }
  createContextSuccess = true;

  // Device
  size_t cb;
  error = clGetContextInfo(context_, CL_CONTEXT_DEVICES, 0, 0, &cb);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", "Get context device number error: " << error);
  }
  cl_device_id* devices = (cl_device_id*)malloc(sizeof(cl_device_id) * cb);
  if (!devices) {
      DEBUG_LOG_STATUS("Init", "Cannot allocate device list");
  }
  error = clGetContextInfo(context_, CL_CONTEXT_DEVICES, cb, devices, 0);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("Init", "Get context device info error: " << error);
      free(devices);
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
      DEBUG_LOG_ERROR("Init", "Create command queue error: " << error);
      free(devices);
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
      DEBUG_LOG_ERROR("Init", "Get command queue device error: " << error);
  } else {
      error = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, 0, 0, &length);
      if (error == CL_SUCCESS) {
          temp = new char[length+1];
          error = clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, length, temp, 0);
          deviceExtensions_ = std::string(temp);
          delete [] temp;
      } else {
          DEBUG_LOG_ERROR("Init", "Get device extension error: " << error);
      }
  }
  temp = 0;

  free(devices);
}

void OCLUtil::Finalize() {
    cl_int error;

    if (createCommandQueueSuccess) {
        error = clReleaseCommandQueue(queue_);
        if (error != CL_SUCCESS) {
            DEBUG_LOG_ERROR("Finalize", "Release command queue error: " << error);
        } else {
            createCommandQueueSuccess = false;
        }
    }
    if (createContextSuccess) {
        error = clReleaseContext(context_);
        if (error != CL_SUCCESS) {
            DEBUG_LOG_ERROR("Finalize", "Release context error: " << error);
        } else {
            createContextSuccess = false;
        }
    }
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
