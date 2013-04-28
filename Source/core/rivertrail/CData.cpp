#include "config.h"
#include "CData.h"

#include "CContext.h"

namespace WebCore {

CData::CData(CContext* aParent)
    : m_parent(aParent)
    , m_queue(0)
    , m_memObj(0)
{
    DEBUG_LOG_CREATE("CData", this);
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

template<> unsigned CData::initCData<Int8Array>(cl_command_queue aQueue, cl_mem aMemObj, unsigned aType, unsigned aLength, unsigned aSize, PassRefPtr<Int8Array> anArray)
{
    cl_int err_code;

    m_type = aType;
    m_length = aLength;
    m_size = aSize;
    m_memObj = aMemObj;

    if (anArray.get())
        m_theInt8Array = anArray;
    else
        m_theInt8Array.clear();

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

template<> unsigned CData::initCData<Uint8Array>(cl_command_queue aQueue, cl_mem aMemObj, unsigned aType, unsigned aLength, unsigned aSize, PassRefPtr<Uint8Array> anArray)
{
    cl_int err_code;

    m_type = aType;
    m_length = aLength;
    m_size = aSize;
    m_memObj = aMemObj;

    if (anArray.get())
        m_theUint8Array = anArray;
    else
        m_theUint8Array.clear();

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

template<> unsigned CData::initCData<Int16Array>(cl_command_queue aQueue, cl_mem aMemObj, unsigned aType, unsigned aLength, unsigned aSize, PassRefPtr<Int16Array> anArray)
{
    cl_int err_code;

    m_type = aType;
    m_length = aLength;
    m_size = aSize;
    m_memObj = aMemObj;

    if (anArray.get())
        m_theInt16Array = anArray;
    else
        m_theInt16Array.clear();

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

template<> unsigned CData::initCData<Uint16Array>(cl_command_queue aQueue, cl_mem aMemObj, unsigned aType, unsigned aLength, unsigned aSize, PassRefPtr<Uint16Array> anArray)
{
    cl_int err_code;

    m_type = aType;
    m_length = aLength;
    m_size = aSize;
    m_memObj = aMemObj;

    if (anArray.get())
        m_theUint16Array = anArray;
    else
        m_theUint16Array.clear();

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

template<> unsigned CData::initCData<Int32Array>(cl_command_queue aQueue, cl_mem aMemObj, unsigned aType, unsigned aLength, unsigned aSize, PassRefPtr<Int32Array> anArray)
{
    cl_int err_code;

    m_type = aType;
    m_length = aLength;
    m_size = aSize;
    m_memObj = aMemObj;

    if (anArray.get())
        m_theInt32Array = anArray;
    else
        m_theInt32Array.clear();

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

template<> unsigned CData::initCData<Uint32Array>(cl_command_queue aQueue, cl_mem aMemObj, unsigned aType, unsigned aLength, unsigned aSize, PassRefPtr<Uint32Array> anArray)
{
    cl_int err_code;

    m_type = aType;
    m_length = aLength;
    m_size = aSize;
    m_memObj = aMemObj;

    if (anArray.get())
        m_theUint32Array = anArray;
    else
        m_theUint32Array.clear();

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

template<> unsigned CData::initCData<Float32Array>(cl_command_queue aQueue, cl_mem aMemObj, unsigned aType, unsigned aLength, unsigned aSize, PassRefPtr<Float32Array> anArray)
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

template<> unsigned CData::initCData<Float64Array>(cl_command_queue aQueue, cl_mem aMemObj, unsigned aType, unsigned aLength, unsigned aSize, PassRefPtr<Float64Array> anArray)
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

template<> unsigned CData::initCData<Uint8ClampedArray>(cl_command_queue aQueue, cl_mem aMemObj, unsigned aType, unsigned aLength, unsigned aSize, PassRefPtr<Uint8ClampedArray> anArray)
{
    cl_int err_code;

    m_type = aType;
    m_length = aLength;
    m_size = aSize;
    m_memObj = aMemObj;

    if (anArray.get())
        m_theUint8ClampedArray = anArray;
    else
        m_theUint8ClampedArray.clear();

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

template<> Int8Array* CData::getValue<Int8Array>()
{
    cl_int err_code;
#ifdef PREALLOCATE_IN_JS_HEAP
    void* mem;
#endif // PREALLOCATE_IN_JS_HEAP

    if (m_theInt8Array.get()) {
#ifdef PREALLOCATE_IN_JS_HEAP
        if (false && !m_isMapped) {
            DEBUG_LOG_STATUS("getValue", "memory is " << m_theInt8Array.get());
            void* mem = clEnqueueMapBuffer(m_queue, m_memObj, CL_TRUE, CL_MAP_READ, 0, m_size, 0, 0, 0, &err_code);

            if (err_code != CL_SUCCESS) {
                DEBUG_LOG_ERROR("getValue", err_code);
                return 0;
            }
#ifndef DEBUG_OFF
            if (mem != m_theInt8Array->data())
                DEBUG_LOG_STATUS("getValue", "EnqueueMap returned wrong pointer");
#endif // DEBUG_OFF
            m_isMapped = true;
        }
#endif // PREALLOCATE_IN_JS_HEAP
        return m_theInt8Array.get();
    } else {
#ifdef INCREMENTAL_MEM_RELEASE
        checkFree();
#endif // INCREMENTAL_MEM_RELEASE

        if (m_parent->createAlignedTA<Int8Array, char>(m_type, m_length, m_theInt8Array) != RT_OK)
            return 0;

        if (!m_theInt8Array) {
            DEBUG_LOG_STATUS("getValue", "Cannot create typed array");
            return 0;
        }

        err_code = enqueueReadBuffer(m_size, m_theInt8Array->data());

        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("getValue", err_code);
            m_theInt8Array.clear();
            return 0;
        }

        DEBUG_LOG_STATUS("getValue", "materialized typed array");

        return m_theInt8Array.get();
    }
}

template<> Uint8Array* CData::getValue<Uint8Array>()
{
    cl_int err_code;
#ifdef PREALLOCATE_IN_JS_HEAP
    void* mem;
#endif // PREALLOCATE_IN_JS_HEAP

    if (m_theUint8Array.get()) {
#ifdef PREALLOCATE_IN_JS_HEAP
        if (false && !m_isMapped) {
            DEBUG_LOG_STATUS("getValue", "memory is " << m_theUint8Array.get());
            void* mem = clEnqueueMapBuffer(m_queue, m_memObj, CL_TRUE, CL_MAP_READ, 0, m_size, 0, 0, 0, &err_code);

            if (err_code != CL_SUCCESS) {
                DEBUG_LOG_ERROR("getValue", err_code);
                return 0;
            }
#ifndef DEBUG_OFF
            if (mem != m_theUint8Array->data())
                DEBUG_LOG_STATUS("getValue", "EnqueueMap returned wrong pointer");
#endif // DEBUG_OFF
            m_isMapped = true;
        }
#endif // PREALLOCATE_IN_JS_HEAP
        return m_theUint8Array.get();
    } else {
#ifdef INCREMENTAL_MEM_RELEASE
        checkFree();
#endif // INCREMENTAL_MEM_RELEASE

        if (m_parent->createAlignedTA<Uint8Array, unsigned char>(m_type, m_length, m_theUint8Array) != RT_OK)
            return 0;

        if (!m_theUint8Array) {
            DEBUG_LOG_STATUS("getValue", "Cannot create typed array");
            return 0;
        }

        err_code = enqueueReadBuffer(m_size, m_theUint8Array->data());

        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("getValue", err_code);
            m_theUint8Array.clear();
            return 0;
        }

        DEBUG_LOG_STATUS("getValue", "materialized typed array");

        return m_theUint8Array.get();
    }
}

template<> Int16Array* CData::getValue<Int16Array>()
{
    cl_int err_code;
#ifdef PREALLOCATE_IN_JS_HEAP
    void* mem;
#endif // PREALLOCATE_IN_JS_HEAP

    if (m_theInt16Array.get()) {
#ifdef PREALLOCATE_IN_JS_HEAP
        if (false && !m_isMapped) {
            DEBUG_LOG_STATUS("getValue", "memory is " << m_theInt16Array.get());
            void* mem = clEnqueueMapBuffer(m_queue, m_memObj, CL_TRUE, CL_MAP_READ, 0, m_size, 0, 0, 0, &err_code);

            if (err_code != CL_SUCCESS) {
                DEBUG_LOG_ERROR("getValue", err_code);
                return 0;
            }
#ifndef DEBUG_OFF
            if (mem != m_theInt16Array->data())
                DEBUG_LOG_STATUS("getValue", "EnqueueMap returned wrong pointer");
#endif // DEBUG_OFF
            m_isMapped = true;
        }
#endif // PREALLOCATE_IN_JS_HEAP
        return m_theInt16Array.get();
    } else {
#ifdef INCREMENTAL_MEM_RELEASE
        checkFree();
#endif // INCREMENTAL_MEM_RELEASE

        if (m_parent->createAlignedTA<Int16Array, short>(m_type, m_length, m_theInt16Array) != RT_OK)
            return 0;

        if (!m_theInt16Array) {
            DEBUG_LOG_STATUS("getValue", "Cannot create typed array");
            return 0;
        }

        err_code = enqueueReadBuffer(m_size, m_theInt16Array->data());

        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("getValue", err_code);
            m_theInt16Array.clear();
            return 0;
        }

        DEBUG_LOG_STATUS("getValue", "materialized typed array");

        return m_theInt16Array.get();
    }
}

template<> Uint16Array* CData::getValue<Uint16Array>()
{
    cl_int err_code;
#ifdef PREALLOCATE_IN_JS_HEAP
    void* mem;
#endif // PREALLOCATE_IN_JS_HEAP

    if (m_theUint16Array.get()) {
#ifdef PREALLOCATE_IN_JS_HEAP
        if (false && !m_isMapped) {
            DEBUG_LOG_STATUS("getValue", "memory is " << m_theUint16Array.get());
            void* mem = clEnqueueMapBuffer(m_queue, m_memObj, CL_TRUE, CL_MAP_READ, 0, m_size, 0, 0, 0, &err_code);

            if (err_code != CL_SUCCESS) {
                DEBUG_LOG_ERROR("getValue", err_code);
                return 0;
            }
#ifndef DEBUG_OFF
            if (mem != m_theUint16Array->data())
                DEBUG_LOG_STATUS("getValue", "EnqueueMap returned wrong pointer");
#endif // DEBUG_OFF
            m_isMapped = true;
        }
#endif // PREALLOCATE_IN_JS_HEAP
        return m_theUint16Array.get();
    } else {
#ifdef INCREMENTAL_MEM_RELEASE
        checkFree();
#endif // INCREMENTAL_MEM_RELEASE

        if (m_parent->createAlignedTA<Uint16Array, unsigned short>(m_type, m_length, m_theUint16Array) != RT_OK)
            return 0;

        if (!m_theUint16Array) {
            DEBUG_LOG_STATUS("getValue", "Cannot create typed array");
            return 0;
        }

        err_code = enqueueReadBuffer(m_size, m_theUint16Array->data());

        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("getValue", err_code);
            m_theUint16Array.clear();
            return 0;
        }

        DEBUG_LOG_STATUS("getValue", "materialized typed array");

        return m_theUint16Array.get();
    }
}

template<> Int32Array* CData::getValue<Int32Array>()
{
    cl_int err_code;
#ifdef PREALLOCATE_IN_JS_HEAP
    void* mem;
#endif // PREALLOCATE_IN_JS_HEAP

    if (m_theInt32Array.get()) {
#ifdef PREALLOCATE_IN_JS_HEAP
        if (false && !m_isMapped) {
            DEBUG_LOG_STATUS("getValue", "memory is " << m_theInt32Array.get());
            void* mem = clEnqueueMapBuffer(m_queue, m_memObj, CL_TRUE, CL_MAP_READ, 0, m_size, 0, 0, 0, &err_code);

            if (err_code != CL_SUCCESS) {
                DEBUG_LOG_ERROR("getValue", err_code);
                return 0;
            }
#ifndef DEBUG_OFF
            if (mem != m_theInt32Array->data())
                DEBUG_LOG_STATUS("getValue", "EnqueueMap returned wrong pointer");
#endif // DEBUG_OFF
            m_isMapped = true;
        }
#endif // PREALLOCATE_IN_JS_HEAP
        return m_theInt32Array.get();
    } else {
#ifdef INCREMENTAL_MEM_RELEASE
        checkFree();
#endif // INCREMENTAL_MEM_RELEASE

        if (m_parent->createAlignedTA<Int32Array, int>(m_type, m_length, m_theInt32Array) != RT_OK)
            return 0;

        if (!m_theInt32Array) {
            DEBUG_LOG_STATUS("getValue", "Cannot create typed array");
            return 0;
        }

        err_code = enqueueReadBuffer(m_size, m_theInt32Array->data());

        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("getValue", err_code);
            m_theInt32Array.clear();
            return 0;
        }

        DEBUG_LOG_STATUS("getValue", "materialized typed array");

        return m_theInt32Array.get();
    }
}

template<> Uint32Array* CData::getValue<Uint32Array>()
{
    cl_int err_code;
#ifdef PREALLOCATE_IN_JS_HEAP
    void* mem;
#endif // PREALLOCATE_IN_JS_HEAP

    if (m_theUint32Array.get()) {
#ifdef PREALLOCATE_IN_JS_HEAP
        if (false && !m_isMapped) {
            DEBUG_LOG_STATUS("getValue", "memory is " << m_theUint32Array.get());
            void* mem = clEnqueueMapBuffer(m_queue, m_memObj, CL_TRUE, CL_MAP_READ, 0, m_size, 0, 0, 0, &err_code);

            if (err_code != CL_SUCCESS) {
                DEBUG_LOG_ERROR("getValue", err_code);
                return 0;
            }
#ifndef DEBUG_OFF
            if (mem != m_theUint32Array->data())
                DEBUG_LOG_STATUS("getValue", "EnqueueMap returned wrong pointer");
#endif // DEBUG_OFF
            m_isMapped = true;
        }
#endif // PREALLOCATE_IN_JS_HEAP
        return m_theUint32Array.get();
    } else {
#ifdef INCREMENTAL_MEM_RELEASE
        checkFree();
#endif // INCREMENTAL_MEM_RELEASE

        if (m_parent->createAlignedTA<Uint32Array, unsigned>(m_type, m_length, m_theUint32Array) != RT_OK)
            return 0;

        if (!m_theUint32Array) {
            DEBUG_LOG_STATUS("getValue", "Cannot create typed array");
            return 0;
        }

        err_code = enqueueReadBuffer(m_size, m_theUint32Array->data());

        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("getValue", err_code);
            m_theUint32Array.clear();
            return 0;
        }

        DEBUG_LOG_STATUS("getValue", "materialized typed array");

        return m_theUint32Array.get();
    }
}

template<> Float32Array* CData::getValue<Float32Array>()
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

        if (m_parent->createAlignedTA<Float32Array, float>(m_type, m_length, m_theFloat32Array) != RT_OK)
            return 0;

        if (!m_theFloat32Array) {
            DEBUG_LOG_STATUS("getValue", "Cannot create typed array");
            return 0;
        }

        err_code = enqueueReadBuffer(m_size, m_theFloat32Array->data());

        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("getValue", err_code);
            m_theFloat32Array.clear();
            return 0;
        }

        DEBUG_LOG_STATUS("getValue", "materialized typed array");

        return m_theFloat32Array.get();
    }
}

template<> Float64Array* CData::getValue<Float64Array>()
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

        if (m_parent->createAlignedTA<Float64Array, double>(m_type, m_length, m_theFloat64Array) != RT_OK)
            return 0;

        if (!m_theFloat64Array) {
            DEBUG_LOG_STATUS("getValue", "Cannot create typed array");
            return 0;
        }

        err_code = enqueueReadBuffer(m_size, m_theFloat64Array->data());

        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("getValue", err_code);
            m_theFloat64Array.clear();
            return 0;
        }

        DEBUG_LOG_STATUS("getValue", "materialized typed array");

        return m_theFloat64Array.get();
    }
}

template<> Uint8ClampedArray* CData::getValue<Uint8ClampedArray>()
{
    cl_int err_code;
#ifdef PREALLOCATE_IN_JS_HEAP
    void* mem;
#endif // PREALLOCATE_IN_JS_HEAP

    if (m_theUint8ClampedArray.get()) {
#ifdef PREALLOCATE_IN_JS_HEAP
        if (false && !m_isMapped) {
            DEBUG_LOG_STATUS("getValue", "memory is " << m_theUint8ClampedArray.get());
            void* mem = clEnqueueMapBuffer(m_queue, m_memObj, CL_TRUE, CL_MAP_READ, 0, m_size, 0, 0, 0, &err_code);

            if (err_code != CL_SUCCESS) {
                DEBUG_LOG_ERROR("getValue", err_code);
                return 0;
            }
#ifndef DEBUG_OFF
            if (mem != m_theUint8ClampedArray->data())
                DEBUG_LOG_STATUS("getValue", "EnqueueMap returned wrong pointer");
#endif // DEBUG_OFF
            m_isMapped = true;
        }
#endif // PREALLOCATE_IN_JS_HEAP
        return m_theUint8ClampedArray.get();
    } else {
#ifdef INCREMENTAL_MEM_RELEASE
        checkFree();
#endif // INCREMENTAL_MEM_RELEASE

        if (m_parent->createAlignedTA<Uint8ClampedArray, unsigned char>(m_type, m_length, m_theUint8ClampedArray) != RT_OK)
            return 0;

        if (!m_theUint8ClampedArray) {
            DEBUG_LOG_STATUS("getValue", "Cannot create typed array");
            return 0;
        }

        err_code = enqueueReadBuffer(m_size, m_theUint8ClampedArray->data());

        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("getValue", err_code);
            m_theUint8ClampedArray.clear();
            return 0;
        }

        DEBUG_LOG_STATUS("getValue", "materialized typed array");

        return m_theUint8ClampedArray.get();
    }
}

template<class ArrayClass> void CData::writeTo(ArrayClass* dest)
{
}

template void CData::writeTo<Int8Array>(Int8Array* dest);
template void CData::writeTo<Uint8Array>(Uint8Array* dest);
template void CData::writeTo<Int16Array>(Int16Array* dest);
template void CData::writeTo<Uint16Array>(Uint16Array* dest);
template void CData::writeTo<Int32Array>(Int32Array* dest);
template void CData::writeTo<Uint32Array>(Uint32Array* dest);
template void CData::writeTo<Float32Array>(Float32Array* dest);
template void CData::writeTo<Float64Array>(Float64Array* dest);
template void CData::writeTo<Uint8ClampedArray>(Uint8ClampedArray* dest);

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
