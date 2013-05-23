#include "config.h"

#include "core/rivertrail/CKernel.h"

#include "core/rivertrail/CContext.h"
#include <limits>

#ifdef WINDOWS_ROUNDTRIP
#include "windows.h"
#endif /* WINDOWS_ROUNDTRIP */

namespace WebCore {

CKernel::CKernel(CContext* aParent)
    : m_kernel(0)
    , m_cmdQueue(0)
    , m_parent(aParent)
{
    DEBUG_LOG_CREATE("CKernel", this);
}

CKernel::~CKernel()
{
    DEBUG_LOG_DESTROY("CKernel", this);
    if (m_kernel)
        clReleaseKernel(m_kernel);
    m_parent = 0;
}

int CKernel::initKernel(cl_command_queue aCmdQueue, cl_kernel aKernel, cl_mem aFailureMem)
{
    cl_int err_code;

    m_kernel = aKernel;
    err_code = clRetainCommandQueue( aCmdQueue);
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("initCKernel", err_code);
        return err_code;
    }
    m_cmdQueue = aCmdQueue;

    m_failureMem = aFailureMem;

    err_code = clSetKernelArg(m_kernel, 0, sizeof(cl_mem), &m_failureMem);
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("initCKernel", err_code);
        return err_code;
    }

    return CL_SUCCESS;
}

unsigned long CKernel::numberOfArgs()
{
    cl_uint result;
    cl_int err_code;

    err_code = clGetKernelInfo(m_kernel, CL_KERNEL_NUM_ARGS, sizeof(cl_uint), &result, 0);
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("numberOfArgs", err_code);
        return std::numeric_limits<unsigned long>::max(); // We never get max value if eveything goes OK, since NUMBER_OF_ARTIFICIAL_ARGS > 0
    }

    /* skip internal arguments when counting */
    return result - NUMBER_OF_ARTIFICIAL_ARGS;
}

bool CKernel::setArgument( unsigned number, CData* argument)
{
    cl_int err_code;
    cl_mem buffer;

    /* skip internal arguments */
    number = number + NUMBER_OF_ARTIFICIAL_ARGS;

    buffer = argument->getContainedBuffer();
    DEBUG_LOG_STATUS("setArgument", "buffer is " << buffer);

    err_code = clSetKernelArg(m_kernel, number, sizeof(cl_mem), &buffer);
    
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("setArgument", err_code);
        return false;
    }

    return true;
}

template<class ArgClass> bool CKernel::setScalarArgument(unsigned number, const ArgClass value, const bool isIntegerB, const bool isHighPrecisionB)
{
    cl_int err_code;
    /* skip internal arguments */
    number = number + NUMBER_OF_ARTIFICIAL_ARGS;

    DEBUG_LOG_STATUS("setScalarArgument", "isIntegerB: " << isIntegerB  << " isHighPrecisionB " << isHighPrecisionB);

    if (isIntegerB) {
        DEBUG_LOG_STATUS("setScalarArgument", "setting integer argument " << number << " to value " << value);
        cl_int intVal = (cl_int) value;
        err_code = clSetKernelArg(m_kernel, number, sizeof(cl_int), &intVal);
    } else if (isHighPrecisionB) {
        DEBUG_LOG_STATUS("setScalarArgument", "setting double argument " << number << " to value " << value);
        cl_double doubleVal = (cl_double) value;
        err_code = clSetKernelArg(m_kernel, number, sizeof(cl_double), &doubleVal);
    } else {
        DEBUG_LOG_STATUS("setScalarArgument", "setting float argument " << number << " to value " << value);
        cl_float floatVal = (cl_float) value;
        err_code = clSetKernelArg(m_kernel, number, sizeof(cl_float), &floatVal);
    }

    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("setScalarArgument", err_code);
        return false;
    }
    return true;
}
template bool CKernel::setScalarArgument<int>(unsigned number, const int value, const bool isIntegerB, const bool isHighPrecisionB);
template bool CKernel::setScalarArgument<unsigned>(unsigned number, const unsigned value, const bool isIntegerB, const bool isHighPrecisionB);
template bool CKernel::setScalarArgument<double>(unsigned number, const double value, const bool isIntegerB, const bool isHighPrecisionB);

unsigned CKernel::run(unsigned rank, unsigned* shape, unsigned* tile)
{
    cl_int err_code;
    cl_event runEvent, readEvent, writeEvent;
    size_t* global_work_size;
    size_t* local_work_size;
    const int zero = 0;
    unsigned* retval = new unsigned[1];

    DEBUG_LOG_STATUS("run", "preparing execution of kernel");

    if (sizeof(size_t) == sizeof(unsigned)) {
        global_work_size = (size_t*) shape;
    } else {
        global_work_size = new size_t[rank];
        if (!global_work_size) {
            DEBUG_LOG_STATUS("run", "allocation of global_work_size failed");
            return -1;
        }
        for (unsigned cnt = 0; cnt < rank; cnt++) {
            global_work_size[cnt] = shape[cnt];
        }
    }

#ifdef USE_LOCAL_WORKSIZE
    if (!tile) {
        local_work_size = 0;
    } else {
        if ((sizeof(size_t) == sizeof(unsigned))) {
            local_work_size = (size_t*) tile;
        } else {
            local_work_size = new size_t[rank];
            if (!local_work_size) {
                DEBUG_LOG_STATUS("run", "allocation of local_work_size failed");
                return -1;
            }
            for (int cnt = 0; cnt < rank; cnt++) {
                local_work_size[cnt] = (size_t) tile[cnt];
            }
        }
    }
#else /* USE_LOCAL_WORKSIZE */
    local_work_size = 0;
#endif /* USE_LOCAL_WORKSIZE */

    DEBUG_LOG_STATUS("run", "setting failure code to 0");

    err_code = clEnqueueWriteBuffer(m_cmdQueue, m_failureMem, CL_FALSE, 0, sizeof(int), &zero, 0, 0, &writeEvent);
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("run", err_code);
        return -1;
    }

    DEBUG_LOG_STATUS("run", "enqueing execution of kernel");

#ifdef WINDOWS_ROUNDTRIP
    CContext::recordBeginOfRoundTrip(m_parent);
#endif /* WINDOWS_ROUNDTRIP */

    err_code = clEnqueueNDRangeKernel(m_cmdQueue, m_kernel, rank, 0, global_work_size, 0, 1, &writeEvent, &runEvent);
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("run", err_code);
        return -1;
    }

    DEBUG_LOG_STATUS("run", "reading failure code");

    

    err_code = clEnqueueReadBuffer(m_cmdQueue, m_failureMem, CL_FALSE, 0, sizeof(int), retval, 1, &runEvent, &readEvent);
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("run", err_code);
        return -1;
    }

    DEBUG_LOG_STATUS("run", "waiting for execution to finish");
    
    // For now we always wait for the run to complete.
    // In the long run, we may want to interleave this with JS execution and only sync on result read.
    err_code = clWaitForEvents( 1, &readEvent);
    
    DEBUG_LOG_STATUS("run", "first event fired");        
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("run", err_code);
        return -1;
    }
#ifdef WINDOWS_ROUNDTRIP
    CContext::recordEndOfRoundTrip(m_parent);
#endif /* WINDOWS_ROUNDTRIP */
    
#ifdef CLPROFILE
#ifdef CLPROFILE_ASYNC
    err_code = clSetEventCallback( readEvent, CL_COMPLETE, &CContext::collectTimings, m_parent);
    
    DEBUG_LOG_STATUS("run", "second event fired");
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("run", err_code);
        return NS_ERROR_ABORT;
    }
#else /* CLPROFILE_ASYNC */
    CContext::collectTimings(runEvent, CL_COMPLETE, m_parent);
#endif /* CLPROFILE_ASYNC */
#endif /* CLPROFILE */
        
    DEBUG_LOG_STATUS("run", "execution completed successfully, start cleanup");
    
    if (global_work_size != (size_t*) shape) {
        delete global_work_size;
    }
#ifdef USE_LOCAL_WORKSIZE
    if (local_work_size != (size_t*) tile) {
        delete local_work_size;
    }
#endif /* USE_LOCAL_WORKSIZE */
    
    err_code = clReleaseEvent(readEvent);
    err_code = clReleaseEvent(runEvent);
    err_code = clReleaseEvent(writeEvent);

    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("run", err_code);
        return -1;
    }

    DEBUG_LOG_STATUS("run", "cleanup complete");

    return *retval;
}

} // namespace WebCore
