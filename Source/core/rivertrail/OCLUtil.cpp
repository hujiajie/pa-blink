#include "config.h"
#include "OCLUtil.h"
//#include "base/logging.h"

namespace WebCore {
/*
size_t shrRoundUp(size_t localWorkSize, size_t numItems) {
    size_t result = localWorkSize;
    while (result < numItems)
        result += localWorkSize;
    assert((result >= numItems) && ((result % localWorkSize) == 0));
    return result;
}
*/
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
    String version(temp);
	version_ =  version;
	delete [] temp;
  }
  // Name
  error = getPlatformPropertyHelper(CL_PLATFORM_NAME, temp);
  if (error != CL_SUCCESS) {
    DEBUG_LOG_ERROR("get platform name: ", error);
  } else {
    String name(temp);
    name_ = name;
	delete [] temp;
  }
  // Vendor
  error = getPlatformPropertyHelper(CL_PLATFORM_VENDOR, temp);
  if (error != CL_SUCCESS) {
    DEBUG_LOG_ERROR("get platform vendor: ", error);
  } else {
    String vendor(temp);
    vendor_ = vendor;
    delete [] temp;
  }
  // Profile
  error = getPlatformPropertyHelper(CL_PLATFORM_PROFILE, temp);
  if (error != CL_SUCCESS) {
    DEBUG_LOG_ERROR("get platform profile: ", error);
  } else {
    String profile(temp);
    profile_ = profile;
    delete [] temp;
  }
  // Extensions
  error = getPlatformPropertyHelper(CL_PLATFORM_EXTENSIONS, temp);
  if (error != CL_SUCCESS) {
    DEBUG_LOG_ERROR("get platform extensions: ", error);
  } else {
    String extensions(temp);
    extensions_ = extensions;
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
/*
void OCLUtil::post() {
  cl_int error = 0;   // Used to handle error codes
  // Allocate memory
  const int size = 100;
  float* src_a_h = new float[size];
  float* src_b_h = new float[size];
  float* res_h = new float[size];
  // Initialize both vectors
  for (int i = 0; i < size; i++) {
    *(src_a_h + i) = *(src_b_h + i) = (float) i;
  }
  const int mem_size = sizeof(float)*size;
  // Allocates a buffer of size mem_size and copies mem_size bytes from src_a_h
  cl_mem src_a_d = clCreateBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size, src_a_h, &error);
  cl_mem src_b_d = clCreateBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size, src_b_h, &error);
  cl_mem res_d = clCreateBuffer(context_, CL_MEM_WRITE_ONLY, mem_size, NULL, &error);
  if (error != CL_SUCCESS) {
    DVLOG(1) << "Error creating buffer: " << error;
  } else {
	DVLOG(1) << "Success creating buffer";
  }

  // Creates the program
  const char* source = "__kernel void add_vector( __global const float *vec1, __global const float *vec2, __global float *vec3, const int dims ) { const int idx = get_global_id( 0 ); if( idx < dims ) vec3[ idx ] = vec1[ idx ] + vec2[ idx ]; }";
  cl_program program = clCreateProgramWithSource(context_, 1, &source, 0, &error);
  if (error != CL_SUCCESS) {
    DVLOG(1) << "Error creating program: " << error;
  } else {
	DVLOG(1) << "Success creating program"; 
  }

  // Builds the program
  error = clBuildProgram(program, 1, &device_, NULL, NULL, NULL);
  if (error != CL_SUCCESS) {
    DVLOG(1) << "Error build program: " << error;
  } else {
	DVLOG(1) << "Success build program";
  }

  // Shows the log
  char* build_log;
  size_t log_size;
  // First call to know the proper size
  clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
  build_log = new char[log_size+1];
  // Second call to get the log
  clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
  build_log[log_size] = '\0';
  DVLOG(1) << "buildlog: " << build_log;
  delete[] build_log;

  // Extracting the kernel
  cl_kernel vector_add_kernel = clCreateKernel(program, "add_vector", &error);
  if (error != CL_SUCCESS) {
    DVLOG(1) << "Error creating kernel: " << error;
  } else {
	  DVLOG(1) << "Success creating kernel"; 
  }

  // Enqueuing parameters
  // Note that we inform the size of the cl_mem object, not the size of the memory pointed by it
  error = clSetKernelArg(vector_add_kernel, 0, sizeof(cl_mem), &src_a_d);
  error |= clSetKernelArg(vector_add_kernel, 1, sizeof(cl_mem), &src_b_d);
  error |= clSetKernelArg(vector_add_kernel, 2, sizeof(cl_mem), &res_d);
  error |= clSetKernelArg(vector_add_kernel, 3, sizeof(size_t), &size);
  if (error != CL_SUCCESS) {
    DVLOG(1) << "Error set kernel argument: " << error;
  } else {
	  DVLOG(1) << "Success set kernel argument"; 
  }

  // Launching kernel
  const size_t local_ws = 512;	// Number of work-items per work-group
  // shrRoundUp returns the smallest multiple of local_ws bigger than size
  const size_t global_ws = shrRoundUp(local_ws, size);	// Total number of work-items
  error = clEnqueueNDRangeKernel(queue_, vector_add_kernel, 1, NULL, &global_ws, &local_ws, 0, NULL, NULL);
  if (error != CL_SUCCESS) {
    DVLOG(1) << "Error run the kernel: " << error;
  } else {
	   DVLOG(1) << "Success run the kernel"; 
  }

  // Reading back
  float* check = new float[size];
  clEnqueueReadBuffer(queue_, res_d, CL_TRUE, 0, mem_size, check, 0, NULL, NULL);

  // Cleaning up
  delete[] src_a_h;
  delete[] src_b_h;
  delete[] res_h;
  delete[] check;
  clReleaseKernel(vector_add_kernel);
  clReleaseCommandQueue(queue_);
  clReleaseContext(context_);
  clReleaseMemObject(src_a_d);
  clReleaseMemObject(src_b_d);
  clReleaseMemObject(res_d);
}
*/
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
  } else
    result = err;
  return result;
}

int OCLUtil::numberOfDevices() {
  return numberOfDevices_;
}

cl_uint OCLUtil::alignmentSize() {
  return alignmentSize_;
}

String OCLUtil::version() {
  return version_;
}

String OCLUtil::name() {
  return name_;
}

String OCLUtil::vendor() {
  return vendor_;
}

String OCLUtil::profile() {
  return profile_;
}

String OCLUtil::extensions() {
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
