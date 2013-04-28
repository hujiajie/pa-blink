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

#include "CL/opencl.h"

#if CL_VERSION_1_1
#else /* OpenCL 1.0 */
#define CL_CALLBACK
#endif

/* Wrap OpenCL functions */
/* Platform API */
typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clGetPlatformIDsFunction)(cl_uint          /* num_entries */,
                          cl_platform_id * /* platforms */,
                          cl_uint *        /* num_platforms */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL*  
clGetPlatformInfoFunction)(cl_platform_id   /* platform */, 
                           cl_platform_info /* param_name */,
                           size_t           /* param_value_size */, 
                           void *           /* param_value */,
                           size_t *         /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

/* Device APIs */
typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clGetDeviceIDsFunction)(cl_platform_id   /* platform */,
                        cl_device_type   /* device_type */, 
                        cl_uint          /* num_entries */, 
                        cl_device_id *   /* devices */, 
                        cl_uint *        /* num_devices */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clGetDeviceInfoFunction)(cl_device_id    /* device */,
                         cl_device_info  /* param_name */, 
                         size_t          /* param_value_size */, 
                         void *          /* param_value */,
                         size_t *        /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

/* Context APIs  */
typedef CL_API_ENTRY cl_context (CL_API_CALL* 
clCreateContextFromTypeFunction)(const cl_context_properties * /* properties */,
                                 cl_device_type                /* device_type */,
                                 void (CL_CALLBACK *     /* pfn_notify*/ )(const char *, const void *, size_t, void *),
                                 void *                        /* user_data */,
                                 cl_int *                      /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clGetContextInfoFunction)(cl_context         /* context */, 
                          cl_context_info    /* param_name */, 
                          size_t             /* param_value_size */, 
                          void *             /* param_value */, 
                          size_t *           /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

/* Command Queue APIs */
typedef CL_API_ENTRY cl_command_queue (CL_API_CALL* 
clCreateCommandQueueFunction)(cl_context                     /* context */, 
                              cl_device_id                   /* device */, 
                              cl_command_queue_properties    /* properties */,
                              cl_int *                       /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clRetainCommandQueueFunction)(cl_command_queue /* command_queue */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clReleaseCommandQueueFunction)(cl_command_queue /* command_queue */) CL_API_SUFFIX__VERSION_1_0;

/* Memory Object APIs */
typedef CL_API_ENTRY cl_mem (CL_API_CALL* 
clCreateBufferFunction)(cl_context   /* context */,
                        cl_mem_flags /* flags */,
                        size_t       /* size */,
                        void *       /* host_ptr */,
                        cl_int *     /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clReleaseMemObjectFunction)(cl_mem /* memobj */) CL_API_SUFFIX__VERSION_1_0; 
                            
/* Program Object APIs  */
typedef CL_API_ENTRY cl_program (CL_API_CALL* 
clCreateProgramWithSourceFunction)(cl_context        /* context */,
                                   cl_uint           /* count */,
                                   const char **     /* strings */,
                                   const size_t *    /* lengths */,
                                   cl_int *          /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clReleaseProgramFunction)(cl_program /* program */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clBuildProgramFunction)(cl_program           /* program */,
                        cl_uint              /* num_devices */,
                        const cl_device_id * /* device_list */,
                        const char *         /* options */, 
                        void (CL_CALLBACK *  /* pfn_notify */)(cl_program /* program */, void * /* user_data */),
                        void *               /* user_data */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clGetProgramInfoFunction)(cl_program         /* program */,
                          cl_program_info    /* param_name */,
                          size_t             /* param_value_size */,
                          void *             /* param_value */,
                          size_t *           /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clGetProgramBuildInfoFunction)(cl_program            /* program */,
                               cl_device_id          /* device */,
                               cl_program_build_info /* param_name */,
                               size_t                /* param_value_size */,
                               void *                /* param_value */,
                               size_t *              /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;
                                    
/* Kernel Object APIs */
typedef CL_API_ENTRY cl_kernel (CL_API_CALL* 
clCreateKernelFunction)(cl_program      /* program */,
                        const char *    /* kernel_name */,
                        cl_int *        /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clReleaseKernelFunction)(cl_kernel   /* kernel */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clSetKernelArgFunction)(cl_kernel    /* kernel */,
                        cl_uint      /* arg_index */,
                        size_t       /* arg_size */,
                        const void * /* arg_value */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clGetKernelInfoFunction)(cl_kernel       /* kernel */,
                         cl_kernel_info  /* param_name */,
                         size_t          /* param_value_size */,
                         void *          /* param_value */,
                         size_t *        /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

/* Event Object APIs  */
typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clWaitForEventsFunction)(cl_uint             /* num_events */,
                         const cl_event *    /* event_list */) CL_API_SUFFIX__VERSION_1_0;              

typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clReleaseEventFunction)(cl_event /* event */) CL_API_SUFFIX__VERSION_1_0;
                     
typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clSetEventCallbackFunction)( cl_event    /* event */,
                             cl_int      /* command_exec_callback_type */,
                             void (CL_CALLBACK * /* pfn_notify */)(cl_event, cl_int, void *),
                             void *      /* user_data */) CL_API_SUFFIX__VERSION_1_1;

/* Profiling APIs  */
typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clGetEventProfilingInfoFunction)(cl_event            /* event */,
                                 cl_profiling_info   /* param_name */,
                                 size_t              /* param_value_size */,
                                 void *              /* param_value */,
                                 size_t *            /* param_value_size_ret */) CL_API_SUFFIX__VERSION_1_0;

/* Enqueued Commands APIs */
typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clEnqueueReadBufferFunction)(cl_command_queue    /* command_queue */,
                             cl_mem              /* buffer */,
                             cl_bool             /* blocking_read */,
                             size_t              /* offset */,
                             size_t              /* cb */, 
                             void *              /* ptr */,
                             cl_uint             /* num_events_in_wait_list */,
                             const cl_event *    /* event_wait_list */,
                             cl_event *          /* event */) CL_API_SUFFIX__VERSION_1_0;
                               
typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clEnqueueWriteBufferFunction)(cl_command_queue   /* command_queue */, 
                              cl_mem             /* buffer */, 
                              cl_bool            /* blocking_write */, 
                              size_t             /* offset */, 
                              size_t             /* cb */, 
                              const void *       /* ptr */, 
                              cl_uint            /* num_events_in_wait_list */, 
                              const cl_event *   /* event_wait_list */, 
                              cl_event *         /* event */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY void * (CL_API_CALL* 
clEnqueueMapBufferFunction)(cl_command_queue /* command_queue */,
                            cl_mem           /* buffer */,
                            cl_bool          /* blocking_map */, 
                            cl_map_flags     /* map_flags */,
                            size_t           /* offset */,
                            size_t           /* cb */,
                            cl_uint          /* num_events_in_wait_list */,
                            const cl_event * /* event_wait_list */,
                            cl_event *       /* event */,
                            cl_int *         /* errcode_ret */) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL* 
clEnqueueNDRangeKernelFunction)(cl_command_queue /* command_queue */,
                                cl_kernel        /* kernel */,
                                cl_uint          /* work_dim */,
                                const size_t *   /* global_work_offset */,
                                const size_t *   /* global_work_size */,
                                const size_t *   /* local_work_size */,
                                cl_uint          /* num_events_in_wait_list */,
                                const cl_event * /* event_wait_list */,
                                cl_event *       /* event */) CL_API_SUFFIX__VERSION_1_0;

#define OPENCL_FUNCTION_LIST(macro) \
    macro(clGetPlatformIDs) \
    macro(clGetPlatformInfo) \
    macro(clGetDeviceIDs) \
    macro(clGetDeviceInfo) \
    macro(clCreateContextFromType) \
    macro(clGetContextInfo) \
    macro(clCreateCommandQueue) \
    macro(clRetainCommandQueue) \
    macro(clReleaseCommandQueue) \
    macro(clCreateBuffer) \
    macro(clReleaseMemObject) \
    macro(clCreateProgramWithSource) \
    macro(clReleaseProgram) \
    macro(clBuildProgram) \
    macro(clGetProgramInfo) \
    macro(clGetProgramBuildInfo) \
    macro(clCreateKernel) \
    macro(clReleaseKernel) \
    macro(clSetKernelArg) \
    macro(clGetKernelInfo) \
    macro(clWaitForEvents) \
    macro(clReleaseEvent) \
    macro(clSetEventCallback) \
    macro(clGetEventProfilingInfo) \
    macro(clEnqueueReadBuffer) \
    macro(clEnqueueWriteBuffer) \
    macro(clEnqueueMapBuffer) \
    macro(clEnqueueNDRangeKernel) \

/* Set alias */
// TODO: Use OPENCL_FUNCTION_LIST to avoid the long list.
#define clGetPlatformIDs __clGetPlatformIDs
#define clGetPlatformInfo __clGetPlatformInfo
#define clGetDeviceIDs __clGetDeviceIDs
#define clGetDeviceInfo __clGetDeviceInfo
#define clCreateContextFromType __clCreateContextFromType
#define clGetContextInfo __clGetContextInfo
#define clCreateCommandQueue __clCreateCommandQueue
#define clRetainCommandQueue __clRetainCommandQueue
#define clReleaseCommandQueue __clReleaseCommandQueue
#define clCreateBuffer __clCreateBuffer
#define clReleaseMemObject __clReleaseMemObject
#define clCreateProgramWithSource __clCreateProgramWithSource
#define clReleaseProgram __clReleaseProgram
#define clBuildProgram __clBuildProgram
#define clGetProgramInfo __clGetProgramInfo
#define clGetProgramBuildInfo __clGetProgramBuildInfo
#define clCreateKernel __clCreateKernel
#define clReleaseKernel __clReleaseKernel
#define clSetKernelArg __clSetKernelArg
#define clGetKernelInfo __clGetKernelInfo
#define clWaitForEvents __clWaitForEvents
#define clReleaseEvent __clReleaseEvent
#define clSetEventCallback __clSetEventCallback
#define clGetEventProfilingInfo __clGetEventProfilingInfo
#define clEnqueueReadBuffer __clEnqueueReadBuffer
#define clEnqueueWriteBuffer __clEnqueueWriteBuffer
#define clEnqueueMapBuffer __clEnqueueMapBuffer
#define clEnqueueNDRangeKernel __clEnqueueNDRangeKernel
