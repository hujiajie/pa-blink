#include "config.h"
#include "CData.h"
#include "OCLdebug.h"

#include "CContext.h"

namespace WebCore {

CData::CData(CContext* aParent)
{
    DEBUG_LOG_CREATE("CData", this);
    m_parent = aParent;
    m_queue = 0;
    m_memObj = 0;
#ifdef PREALLOCATE_IN_JS_HEAP
    m_isMapped = false;
#endif // PREALLOCATE_IN_JS_HEAP
}

CData::~CData()
{
    DEBUG_LOG_DESTROY("CData", this);
    if (m_memObj) {
#ifdef INCREMENTAL_MEM_RELEASE
        deferFree(m_memObj);
#else // INCREMENTAL_MEM_RELEASE
        clReleaseMemObject(m_memObj);
#endif // INCREMENTAL_MEM_RELEASE
    }
    if (m_queue && m_isRetained) {
        DEBUG_LOG_STATUS("~CData", "releasing queue object");
        clReleaseCommandQueue(m_queue);
    }
    m_parent = 0;
}

#ifdef INCREMENTAL_MEM_RELEASE
inline int CData::checkFree()
{
    if (m_parent) {
        // Make sure we have not lost our parent due to CC.
        return m_parent->checkFree();
    }
}

inline void CData::deferFree(cl_mem obj)
{
    if (m_parent)
        m_parent->deferFree(obj);
    else {
        // Lost parent, maybe due to CC. Fall back.
        clReleaseMemObject(obj);
    }
}
#endif // INCREMENTAL_MEM_RELEASE

cl_int CData::enqueueReadBuffer(size_t size, void* ptr)
{
#ifdef INCREMENTAL_MEM_RELEASE
    cl_int err_code;
    int freed;

    do {
        freed = checkFree();
        err_code = clEnqueueReadBuffer(m_queue, m_memObj, CL_TRUE, 0, size, ptr, 0, 0, 0);
    } while (((err_code == CL_MEM_OBJECT_ALLOCATION_FAILURE) || (err_code == CL_OUT_OF_HOST_MEMORY)) && freed);

    return err_code;
#else // INCREMENTAL_MEM_RELEASE
    return clEnqueueReadBuffer(m_queue, m_memObj, CL_TRUE, 0, size, ptr, 0, 0, 0);
#endif // INCREMENTAL_MEM_RELEASE
}

unsigned CData::initCDataFloat32Array(cl_command_queue aQueue, cl_mem aMemObj, unsigned aType, unsigned aLength, unsigned aSize, PassRefPtr<Float32Array> anArray)
{
    cl_int err_code;

    m_type = aType;
    m_length = aLength;
    m_size = aSize;
    m_memObj = aMemObj;

    if (anArray.get())
        m_theFloat32Array = anArray;
    else
        m_theFloat32Array.clear();

    DEBUG_LOG_STATUS("initCData", "queue is " << aQueue << " buffer is " << aMemObj);

    err_code = clRetainCommandQueue(m_queue);
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("initCData", err_code);
        // We should really fail here but a bug in the whatif OpenCL
        // makes the above retain operation always fail.
        m_isRetained = false;
    } else
        m_isRetained = true;
    m_queue = aQueue;

    return RT_OK;
}

unsigned CData::initCDataFloat64Array(cl_command_queue aQueue, cl_mem aMemObj, unsigned aType, unsigned aLength, unsigned aSize, PassRefPtr<Float64Array> anArray)
{
    cl_int err_code;

    m_type = aType;
    m_length = aLength;
    m_size = aSize;
    m_memObj = aMemObj;

    if (anArray.get())
        m_theFloat64Array = anArray;
    else
        m_theFloat64Array.clear();

    DEBUG_LOG_STATUS("initCData", "queue is " << aQueue << " buffer is " << aMemObj);

    err_code = clRetainCommandQueue(m_queue);
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("initCData", err_code);
        // We should really fail here but a bug in the whatif OpenCL
        // makes the above retain operation always fail.
        m_isRetained = false;
    } else
        m_isRetained = true;
    m_queue = aQueue;

    return RT_OK;
}

Float32Array* CData::getValueFloat32Array()
{
    cl_int err_code;
#ifdef PREALLOCATE_IN_JS_HEAP
    void* mem;
#endif // PREALLOCATE_IN_JS_HEAP

    if (m_theFloat32Array.get()) {
#ifdef PREALLOCATE_IN_JS_HEAP
        if (false && !m_isMapped) {
            DEBUG_LOG_STATUS("getValue", "memory is " << m_theFloat32Array.get());
            void* mem = clEnqueueMapBuffer(m_queue, m_memObj, CL_TRUE, CL_MAP_READ, 0, m_size, 0, 0, 0, &err_code);

            if (err_code != CL_SUCCESS) {
                DEBUG_LOG_ERROR("getValue", err_code);
                return 0;
            }
#ifndef DEBUG_OFF
            if (mem != m_theFloat32Array->data())
                DEBUG_LOG_STATUS("getValue", "EnqueueMap returned wrong pointer");
#endif // DEBUG_OFF
            m_isMapped = true;
        }
#endif // PREALLOCATE_IN_JS_HEAP
        return m_theFloat32Array.get();
    } else {
#ifdef INCREMENTAL_MEM_RELEASE
        checkFree();
#endif // INCREMENTAL_MEM_RELEASE

        if (m_parent->createAlignedTAFloat32Array(m_type, m_length, m_theFloat32Array) != RT_OK)
            return 0;

        if (!m_theFloat32Array) {
            DEBUG_LOG_STATUS("getValue", "Cannot create typed array");
            return 0;
        }

        err_code = enqueueReadBuffer(m_size, m_theFloat32Array->data());

        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("getValue", err_code);
            return 0;
        }

        DEBUG_LOG_STATUS("getValue", "materialized typed array");

        return m_theFloat32Array.get();
    }
}

Float64Array* CData::getValueFloat64Array()
{
    cl_int err_code;
#ifdef PREALLOCATE_IN_JS_HEAP
    void* mem;
#endif // PREALLOCATE_IN_JS_HEAP

    if (m_theFloat64Array.get()) {
#ifdef PREALLOCATE_IN_JS_HEAP
        if (false && !m_isMapped) {
            DEBUG_LOG_STATUS("getValue", "memory is " << m_theFloat64Array.get());
            void* mem = clEnqueueMapBuffer(m_queue, m_memObj, CL_TRUE, CL_MAP_READ, 0, m_size, 0, 0, 0, &err_code);

            if (err_code != CL_SUCCESS) {
                DEBUG_LOG_ERROR("getValue", err_code);
                return 0;
            }
#ifndef DEBUG_OFF
            if (mem != m_theFloat64Array->data())
                DEBUG_LOG_STATUS("getValue", "EnqueueMap returned wrong pointer");
#endif // DEBUG_OFF
            m_isMapped = true;
        }
#endif // PREALLOCATE_IN_JS_HEAP
        return m_theFloat64Array.get();
    } else {
#ifdef INCREMENTAL_MEM_RELEASE
        checkFree();
#endif // INCREMENTAL_MEM_RELEASE

        if (m_parent->createAlignedTAFloat64Array(m_type, m_length, m_theFloat64Array) != RT_OK)
            return 0;

        if (!m_theFloat64Array) {
            DEBUG_LOG_STATUS("getValue", "Cannot create typed array");
            return 0;
        }

        err_code = enqueueReadBuffer(m_size, m_theFloat64Array->data());

        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("getValue", err_code);
            return 0;
        }

        DEBUG_LOG_STATUS("getValue", "materialized typed array");

        return m_theFloat64Array.get();
    }
}

void CData::writeToFloat32Array(Float32Array* dest)
{
}

void CData::writeToFloat64Array(Float64Array* dest)
{
}

cl_mem CData::getContainedBuffer()
{
    return m_memObj;
}

unsigned CData::getSize()
{
    return m_size;
}

unsigned CData::getType()
{
    return m_type;
}

unsigned CData::getLength()
{
    return m_length;
}

} // namespace WebCore
