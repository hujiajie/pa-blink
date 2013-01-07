#include "config.h"
#include "OCLUtil.h"

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

  // Platform
  error = clGetPlatformIDs( 0, 0, &nplatforms);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR( "get platform number", error);
  }
  cl_platform_id* m_platforms = new cl_platform_id[nplatforms];
  error = clGetPlatformIDs(nplatforms, m_platforms, &numberOfPlatforms);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("get platform id: ", error);
  }

  const cl_uint maxNameLength = 256;
  char name[maxNameLength];
  for (cl_uint i = 0; i < numberOfPlatforms; i++) {
      error = clGetPlatformInfo(m_platforms[i], CL_PLATFORM_NAME, maxNameLength * sizeof(char), name, 0);
      if (error != CL_SUCCESS) {
          DEBUG_LOG_ERROR( "GetIntelPlatform Failed", error);
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
      DEBUG_LOG_ERROR("get platform version: ", error);
  } else {
      version_ =  std::string(temp);
      delete [] temp;
  }
  // Name
  error = getPlatformPropertyHelper(CL_PLATFORM_NAME, temp);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("get platform name: ", error);
  } else {
      name_ = std::string(temp);
      delete [] temp;
  }
  // Vendor
  error = getPlatformPropertyHelper(CL_PLATFORM_VENDOR, temp);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("get platform vendor: ", error);
  } else {
      vendor_ = std::string(temp);
      delete [] temp;
  }
  // Profile
  error = getPlatformPropertyHelper(CL_PLATFORM_PROFILE, temp);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("get platform profile: ", error);
  } else {
      profile_ = std::string(temp);
      delete [] temp;
  }
  // Extensions
  error = getPlatformPropertyHelper(CL_PLATFORM_EXTENSIONS, temp);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("get platform extensions: ", error);
  } else {
      extensions_ = std::string(temp);
      delete [] temp;
  }
  temp = NULL;

  // Number of Device
  cl_uint number;
  error = clGetDeviceIDs(platform_, CL_DEVICE_TYPE_ALL, 0, 0, &number);
  if (error != CL_SUCCESS) {
     DEBUG_LOG_ERROR("get the number of devices: ", error);
  } else {
     numberOfDevices_ = number;
  }

  // Context
  cl_context_properties context_properties[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform_, 0};
  context_ = clCreateContextFromType(context_properties, CL_DEVICE_TYPE_CPU, &reportCLError, this, &error);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("init context", error);
  }

  // Device
  size_t cb;
  error = clGetContextInfo(context_, CL_CONTEXT_DEVICES, 0, 0, &cb);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("get devices from context: ", error);
  }
  cl_device_id* devices = (cl_device_id*)malloc(sizeof(cl_device_id) * cb);
  if (!devices) {
      DEBUG_LOG_STATUS("get devices: ", "Cannot allocate device list");
  }
  error = clGetContextInfo(context_, CL_CONTEXT_DEVICES, cb, devices, 0);
  if (error != CL_SUCCESS) {
      DEBUG_LOG_ERROR("get device info: ", error);
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
      DEBUG_LOG_ERROR("get command queue", error);
      free(devices);
  }

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
  free(devices);
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

std::string OCLUtil::extensions() {
    return extensions_;
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
}  // namespace content
