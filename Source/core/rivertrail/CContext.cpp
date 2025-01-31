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

#include "core/rivertrail/CContext.h"

#include "core/html/ImageData.h"
#include "core/html/canvas/CanvasRenderingContext2D.h"
#include "core/platform/graphics/IntSize.h"
#include "core/rivertrail/CData.h"
#include "core/rivertrail/CKernel.h"
#include "core/rivertrail/CPlatform.h"
#include <stdlib.h>
#include <wtf/ArrayBuffer.h>
#include <wtf/Float32Array.h>
#include <wtf/Float64Array.h>
#include <wtf/Int16Array.h>
#include <wtf/Int32Array.h>
#include <wtf/Int8Array.h>
#include <wtf/Uint16Array.h>
#include <wtf/Uint32Array.h>
#include <wtf/Uint8Array.h>
#include <wtf/Uint8ClampedArray.h>

namespace WebCore {

CContext::CContext(CPlatform* aParent)
    : m_parent(aParent)
    , m_buildLog(0)
    , m_buildLogSize(0)
    , m_cmdQueue(0)
{
    DEBUG_LOG_CREATE("CContext", this);
#ifdef CLPROFILE
    m_clpExecStart = 0;
    m_clpExecEnd = 0;
#endif // CLPROFILE
#ifdef WINDOWS_ROUNDTRIP
    m_wrtExecStart.QuadPart = -1;
    m_wrtExecEnd.QuadPart = -1;
#endif // WINDOWS_ROUNDTRIP
}

CContext::~CContext()
{
    DEBUG_LOG_DESTROY("CContext", this);
#ifdef INCREMENTAL_MEM_RELEASE
    // Disable deferred free.
    m_deferMax = 0;
    // Free the pending queue.
    while (checkFree()) {};
    free(m_deferList);
    m_deferList = 0;
#endif // INCREMENTAL_MEM_RELEASE
    if (m_buildLog)
        free(m_buildLog);
}

#ifdef CLPROFILE
void CL_CALLBACK CContext::collectTimings(cl_event event, cl_int status, void* data)
{
    cl_int result;
    CContext* instance = (CContext*)data;

    DEBUG_LOG_STATUS("collectTimings", "enquiring for runtimes...");

    result = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &(instance->m_clpExecStart), 0);
    if (result != CL_SUCCESS) {
        DEBUG_LOG_ERROR("collectTimings", result);
        instance->m_clpExecStart = 0;
    }

    result = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &(instance->m_clpExecEnd), 0);
    if (result != CL_SUCCESS) {
        DEBUG_LOG_ERROR("collectTimings", result);
        instance->m_clpExecEnd = 0;
    }

    DEBUG_LOG_STATUS("collectTimings", "Collected start " << instance->m_clpExecStart << " and end " << instance->m_clpExecEnd);
}
#endif // CLPROFILE

unsigned CContext::initContext()
{
    cl_int err_code;
#ifdef INCREMENTAL_MEM_RELEASE
    m_deferList = (cl_mem*)malloc(DEFER_LIST_LENGTH * sizeof(cl_mem));
    m_deferPos = 0;
    m_deferMax = DEFER_LIST_LENGTH;
#endif // INCREMENTAL_MEM_RELEASE

    m_context = m_parent->openclUtil()->context();
    m_cmdQueue = m_parent->openclUtil()->queue();
    m_alignmentSize = m_parent->openclUtil()->alignmentSize();
    m_kernelFailureMem = createBuffer(CL_MEM_READ_WRITE, sizeof(int), 0, &err_code);
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("initContext", err_code);
        return RT_ERROR_NOT_AVAILABLE;
    }

    return RT_OK;
}

#ifdef INCREMENTAL_MEM_RELEASE
void CContext::deferFree(cl_mem obj)
{
    if (m_deferPos >= m_deferMax)
        clReleaseMemObject(obj);
    else
        m_deferList[m_deferPos++] = obj;
}

int CContext::checkFree()
{
    int freed = 0;
    while ((m_deferPos > 0) && (freed++ < DEFER_CHUNK_SIZE))
        clReleaseMemObject(m_deferList[--m_deferPos]);
    return freed;
}
#endif // INCREMENTAL_MEM_RELEASE

PassRefPtr<CKernel> CContext::compileKernel(const String& source, const String& kernelName, const String& options)
{
    cl_program program;
    cl_kernel kernel;
    cl_int err_code, err_code2;
    cl_uint numDevices;
    cl_device_id* devices = 0;
    const char* sourceStr,* optionsStr,* kernelNameStr;
    RefPtr<CKernel> ret;
    unsigned result;

    CString sourceCString = source.utf8();
    sourceStr = sourceCString.data();
    DEBUG_LOG_STATUS("compileKernel", "Source: " << sourceStr);
    program = clCreateProgramWithSource(m_context, 1, (const char**)&sourceStr, 0, &err_code);
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("compileKernel", err_code);
        return ret.release();
    }

    CString optionsCString = options.utf8();
    optionsStr = optionsCString.data();
    err_code = clBuildProgram(program, 0, 0, optionsStr, 0, 0);
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("compileKernel", err_code);
    }

    err_code2 = clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &numDevices, 0);
    if (err_code2 != CL_SUCCESS) {
        DEBUG_LOG_ERROR("compileKernel", err_code2);
        goto FAIL;
    }

    devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
    err_code2 = clGetProgramInfo(program, CL_PROGRAM_DEVICES, numDevices * sizeof(cl_device_id), devices, 0);
    if (err_code2 != CL_SUCCESS) {
        DEBUG_LOG_ERROR("compileKernel", err_code);
        goto FAIL;
    }

    if (m_buildLogSize == 0) {
        if (m_buildLog)
            free(m_buildLog);
        m_buildLogSize = INITIAL_BUILDLOG_SIZE;
        m_buildLog = (char*)malloc(m_buildLogSize * sizeof(char));
    }

    err_code2 = clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, m_buildLogSize, 0, 0);
    while ((err_code2 == CL_INVALID_VALUE) && (m_buildLogSize < MAX_BUILDLOG_SIZE)) {
        if (m_buildLog) {
            free(m_buildLog);
            m_buildLogSize *= 2;
        }
        m_buildLog = (char*)malloc(m_buildLogSize * sizeof(char));
        err_code2 = clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, m_buildLogSize, 0, 0);
    }

    if (!m_buildLog) {
        DEBUG_LOG_STATUS("compileKernel", "Cannot allocate buildLog");
        m_buildLogSize = 0;
        goto DONE;
    }
    err_code2 = clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, m_buildLogSize, m_buildLog, 0);

    if (err_code2 != CL_SUCCESS) {
        DEBUG_LOG_ERROR("compileKernel", err_code);
        goto FAIL;
    }

    DEBUG_LOG_STATUS("compileKernel", "buildLog: " << m_buildLog);
    goto DONE;

FAIL:
    if (m_buildLog) {
        free(m_buildLog);
        m_buildLog = 0;
        m_buildLogSize = 0;
    }

DONE:
    if (devices)
        free(devices);

    CString kernelNameCString = kernelName.utf8();
    kernelNameStr = kernelNameCString.data();
    kernel = clCreateKernel(program, kernelNameStr, &err_code);
    clReleaseProgram(program);
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("compileKernel", err_code);
        return ret.release();
    }

    ret = CKernel::create(this);
    if (!ret) {
        clReleaseKernel(kernel);
        DEBUG_LOG_STATUS("compileKernel", "Cannot create new CKernel object");
        return ret.release();
    }

    // All kernels share the single buffer for the failure code.
    result = ret->initKernel(m_cmdQueue, kernel, m_kernelFailureMem);
    if (result != RT_OK) {
        clReleaseKernel(kernel);
        ret.clear();
        return ret.release();
    }

    return ret.release();
}

String CContext::buildLog()
{
    if (m_buildLog)
        return String(m_buildLog);
    else
        return String();
}

cl_mem CContext::createBuffer(cl_mem_flags flags, size_t size, void* ptr, cl_int* err)
{
#ifdef INCREMENTAL_MEM_RELEASE
    int freed;
    cl_mem result;
    do {
        freed = checkFree();
        result = clCreateBuffer(m_context, flags, size, ptr, err);
    } while (((*err == CL_OUT_OF_HOST_MEMORY) || (*err == CL_MEM_OBJECT_ALLOCATION_FAILURE)) && freed);
    return result;
#else // INCREMENTAL_MEM_RELEASE
    return clCreateBuffer(m_context, flags, size, ptr, err);
#endif // INCREMENTAL_MEM_RELEASE
}

template<class ArrayClass> inline unsigned char* CContext::getPointerFromTA(ArrayClass* ta)
{
    return (unsigned char*)ta->data();
}

template unsigned char* CContext::getPointerFromTA<Int8Array>(Int8Array* ta);
template unsigned char* CContext::getPointerFromTA<Uint8Array>(Uint8Array* ta);
template unsigned char* CContext::getPointerFromTA<Int16Array>(Int16Array* ta);
template unsigned char* CContext::getPointerFromTA<Uint16Array>(Uint16Array* ta);
template unsigned char* CContext::getPointerFromTA<Int32Array>(Int32Array* ta);
template unsigned char* CContext::getPointerFromTA<Uint32Array>(Uint32Array* ta);
template unsigned char* CContext::getPointerFromTA<Float32Array>(Float32Array* ta);
template unsigned char* CContext::getPointerFromTA<Float64Array>(Float64Array* ta);
template unsigned char* CContext::getPointerFromTA<Uint8ClampedArray>(Uint8ClampedArray* ta);

template<class ArrayClass, class ElementType> unsigned CContext::createAlignedTA(ArrayBufferView::ViewType type, size_t length, RefPtr<ArrayClass>& res)
{
    RefPtr<ArrayBuffer> buffer;
    uintptr_t offset;
    buffer = ArrayBuffer::create(sizeof(ElementType) * length + m_alignmentSize, 1);
    if (!buffer)
        return RT_ERROR_OUT_OF_MEMORY;
    offset = (uintptr_t)buffer->data();
    offset = (offset + m_alignmentSize) / m_alignmentSize * m_alignmentSize - offset;
    res = ArrayClass::create(buffer, offset, length);

    return RT_OK;
}

template<> unsigned CContext::createAlignedTA<Int8Array, char>(ArrayBufferView::ViewType type, size_t length, RefPtr<Int8Array>& res)
{
    // We only return float or double arrays, so fail in all other cases.
    return RT_ERROR_NOT_IMPLEMENTED;
}

template<> unsigned CContext::createAlignedTA<Uint8Array, unsigned char>(ArrayBufferView::ViewType type, size_t length, RefPtr<Uint8Array>& res)
{
    // We only return float or double arrays, so fail in all other cases.
    return RT_ERROR_NOT_IMPLEMENTED;
}

template<> unsigned CContext::createAlignedTA<Int16Array, short>(ArrayBufferView::ViewType type, size_t length, RefPtr<Int16Array>& res)
{
    // We only return float or double arrays, so fail in all other cases.
    return RT_ERROR_NOT_IMPLEMENTED;
}

template<> unsigned CContext::createAlignedTA<Uint16Array, unsigned short>(ArrayBufferView::ViewType type, size_t length, RefPtr<Uint16Array>& res)
{
    // We only return float or double arrays, so fail in all other cases.
    return RT_ERROR_NOT_IMPLEMENTED;
}

template<> unsigned CContext::createAlignedTA<Int32Array, int>(ArrayBufferView::ViewType type, size_t length, RefPtr<Int32Array>& res)
{
    // We only return float or double arrays, so fail in all other cases.
    return RT_ERROR_NOT_IMPLEMENTED;
}

template<> unsigned CContext::createAlignedTA<Uint32Array, unsigned>(ArrayBufferView::ViewType type, size_t length, RefPtr<Uint32Array>& res)
{
    // We only return float or double arrays, so fail in all other cases.
    return RT_ERROR_NOT_IMPLEMENTED;
}

template unsigned CContext::createAlignedTA<Float32Array, float>(ArrayBufferView::ViewType type, size_t length, RefPtr<Float32Array>& res);
template unsigned CContext::createAlignedTA<Float64Array, double>(ArrayBufferView::ViewType type, size_t length, RefPtr<Float64Array>& res);

template<> unsigned CContext::createAlignedTA<Uint8ClampedArray, unsigned char>(ArrayBufferView::ViewType type, size_t length, RefPtr<Uint8ClampedArray>& res)
{
    // We only return float or double arrays, so fail in all other cases.
    return RT_ERROR_NOT_IMPLEMENTED;
}

template<class ArrayClass, ArrayBufferView::ViewType type> PassRefPtr<CData> CContext::mapData(PassRefPtr<ArrayClass> source)
{
    cl_int err_code;
    unsigned result;
    RefPtr<ArrayClass> tArray;
    RefPtr<CData> data;

    tArray = source;
    data = CData::create(this);
    if (!data) {
        DEBUG_LOG_STATUS("mapData", "Cannot create new CData object");
        return data.release();
    }

    // USE_HOST_PTR is save as the CData object will keep the associated typed array alive as long as the
    // memory buffer lives.
    cl_mem_flags flags = CL_MEM_READ_ONLY;
    void* tArrayBuffer = 0;
    size_t arrayByteLength = tArray->byteLength();
    if (!arrayByteLength)
        arrayByteLength = 1;
    else {
        tArrayBuffer = getPointerFromTA<ArrayClass>(tArray.get());
        flags |= CL_MEM_USE_HOST_PTR;
    }

    cl_mem memObj = createBuffer(flags, arrayByteLength, tArrayBuffer, &err_code);
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("mapData", err_code);
        data.clear();
        return data.release();
    }

    result = data->initCData<ArrayClass>(m_cmdQueue, memObj, type, tArray->length(), tArray->byteLength(), tArray);
    if (result == RT_OK)
        return data.release();

    data.clear();
    return data.release();
}

template PassRefPtr<CData> CContext::mapData<Int8Array, ArrayBufferView::TypeInt8>(PassRefPtr<Int8Array> source);
template PassRefPtr<CData> CContext::mapData<Uint8Array, ArrayBufferView::TypeUint8>(PassRefPtr<Uint8Array> source);
template PassRefPtr<CData> CContext::mapData<Int16Array, ArrayBufferView::TypeInt16>(PassRefPtr<Int16Array> source);
template PassRefPtr<CData> CContext::mapData<Uint16Array, ArrayBufferView::TypeUint16>(PassRefPtr<Uint16Array> source);
template PassRefPtr<CData> CContext::mapData<Int32Array, ArrayBufferView::TypeInt32>(PassRefPtr<Int32Array> source);
template PassRefPtr<CData> CContext::mapData<Uint32Array, ArrayBufferView::TypeUint32>(PassRefPtr<Uint32Array> source);
template PassRefPtr<CData> CContext::mapData<Float32Array, ArrayBufferView::TypeFloat32>(PassRefPtr<Float32Array> source);
template PassRefPtr<CData> CContext::mapData<Float64Array, ArrayBufferView::TypeFloat64>(PassRefPtr<Float64Array> source);
template PassRefPtr<CData> CContext::mapData<Uint8ClampedArray, ArrayBufferView::TypeUint8Clamped>(PassRefPtr<Uint8ClampedArray> source);

template<class ArrayClass> PassRefPtr<CData> CContext::cloneData(ArrayClass* source)
{
    RefPtr<CData> data;

    return data.release();
}

template PassRefPtr<CData> CContext::cloneData<Int8Array>(Int8Array* source);
template PassRefPtr<CData> CContext::cloneData<Uint8Array>(Uint8Array* source);
template PassRefPtr<CData> CContext::cloneData<Int16Array>(Int16Array* source);
template PassRefPtr<CData> CContext::cloneData<Uint16Array>(Uint16Array* source);
template PassRefPtr<CData> CContext::cloneData<Int32Array>(Int32Array* source);
template PassRefPtr<CData> CContext::cloneData<Uint32Array>(Uint32Array* source);
template PassRefPtr<CData> CContext::cloneData<Float32Array>(Float32Array* source);
template PassRefPtr<CData> CContext::cloneData<Float64Array>(Float64Array* source);
template PassRefPtr<CData> CContext::cloneData<Uint8ClampedArray>(Uint8ClampedArray* source);

template<class ArrayClass, class ElementType, ArrayBufferView::ViewType type> PassRefPtr<CData> CContext::allocateData(ArrayClass* templ, unsigned length)
{
    cl_int err_code;
    unsigned result;
    ArrayClass* tArray;
    size_t bytePerElements;
    RefPtr<CData> data;

    tArray = templ;

    data = CData::create(this);
    if (!data) {
        DEBUG_LOG_STATUS("allocateData", "Cannot create new CData object");
        return data.release();
    }

    if (!length) {
        DEBUG_LOG_STATUS("allocateData", "size not provided, assuming template's size");
        length = tArray->length();
    }

    bytePerElements = tArray->byteLength() / tArray->length();

    DEBUG_LOG_STATUS("allocateData", "length " << length << " bytePerElements " << bytePerElements);

#ifdef PREALLOCATE_IN_JS_HEAP
    RefPtr<ArrayClass> jsArray;
    if (createAlignedTA<ArrayClass, ElementType>(type, length, jsArray) != RT_OK) {
        data.clear();
        return data.release();
    }
    if (!jsArray) {
        DEBUG_LOG_STATUS("allocateData", "Cannot create typed array");
        data.clear();
        return data.release();
    }

    cl_mem memObj = createBuffer(CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, jsArray->byteLength(), getPointerFromTA<ArrayClass>(jsArray.get()), &err_code);
#else // PREALLOCATE_IN_JS_HEAP
    RefPtr<ArrayClass> jsArray;
    cl_mem memObj = createBuffer(CL_MEM_READ_WRITE, length * bytePerElements, 0, &err_code);
#endif // PREALLOCATE_IN_JS_HEAP
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR("allocateData", err_code);
        data.clear();
        return data.release();
    }

    result = data->initCData<ArrayClass>(m_cmdQueue, memObj, type, length, length * bytePerElements, jsArray);

    if (result == RT_OK)
        return data.release();

    data.clear();
    return data.release();
}

template PassRefPtr<CData> CContext::allocateData<Int8Array, char, ArrayBufferView::TypeInt8>(Int8Array* templ, unsigned length);
template PassRefPtr<CData> CContext::allocateData<Uint8Array, unsigned char, ArrayBufferView::TypeUint8>(Uint8Array* templ, unsigned length);
template PassRefPtr<CData> CContext::allocateData<Int16Array, short, ArrayBufferView::TypeInt16>(Int16Array* templ, unsigned length);
template PassRefPtr<CData> CContext::allocateData<Uint16Array, unsigned short, ArrayBufferView::TypeUint16>(Uint16Array* templ, unsigned length);
template PassRefPtr<CData> CContext::allocateData<Int32Array, int, ArrayBufferView::TypeInt32>(Int32Array* templ, unsigned length);
template PassRefPtr<CData> CContext::allocateData<Uint32Array, unsigned, ArrayBufferView::TypeUint32>(Uint32Array* templ, unsigned length);
template PassRefPtr<CData> CContext::allocateData<Float32Array, float, ArrayBufferView::TypeFloat32>(Float32Array* templ, unsigned length);
template PassRefPtr<CData> CContext::allocateData<Float64Array, double, ArrayBufferView::TypeFloat64>(Float64Array* templ, unsigned length);
template PassRefPtr<CData> CContext::allocateData<Uint8ClampedArray, unsigned char, ArrayBufferView::TypeUint8Clamped>(Uint8ClampedArray* templ, unsigned length);

PassRefPtr<CData> CContext::allocateData2(CData* templ, unsigned length)
{
    CData* cData = templ;
    cl_int err_code;
    unsigned result;
    size_t bytePerElements;
    RefPtr<CData> data;

    data = CData::create(this);
    if (!data) {
        DEBUG_LOG_STATUS("allocateData2", "Cannot create new CData object");
        return data.release();
    }

    if (!length) {
        DEBUG_LOG_STATUS("allocateData2", "length not provided, assuming template's size");
        length = cData->getLength();
    }

    bytePerElements = cData->getSize() / cData->getLength();

    DEBUG_LOG_STATUS("allocateData2", "length " << length << " bytePerElements " << bytePerElements);

    switch (cData->getType()) {
    case ArrayBufferView::TypeInt8: {
#ifdef PREALLOCATE_IN_JS_HEAP
        RefPtr<Int8Array> jsArray;
        if (createAlignedTA<Int8Array, char>(cData->getType(), length, jsArray) != RT_OK) {
            data.clear();
            return data.release();
        }
        if (!jsArray) {
            DEBUG_LOG_STATUS("allocateData2", "Cannot create typed array");
            data.clear();
            return data.release();
        }

        cl_mem memObj = createBuffer(CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, jsArray->byteLength(), jsArray->data(), &err_code);
#else // PREALLOCATE_IN_JS_HEAP
        RefPtr<Int8Array> jsArray;
        cl_mem memObj = createBuffer(CL_MEM_READ_WRITE, length * bytePerElements, 0, &err_code);
#endif // PREALLOCATE_IN_JS_HEAP
        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("allocateData2", err_code);
            data.clear();
            return data.release();
        }

        result = data->initCData<Int8Array>(m_cmdQueue, memObj, cData->getType(), length, length * bytePerElements, jsArray);

        if (result == RT_OK)
            return data.release();

        data.clear();
        return data.release();

        break;
    }
    case ArrayBufferView::TypeUint8: {
#ifdef PREALLOCATE_IN_JS_HEAP
        RefPtr<Uint8Array> jsArray;
        if (createAlignedTA<Uint8Array, unsigned char>(cData->getType(), length, jsArray) != RT_OK) {
            data.clear();
            return data.release();
        }
        if (!jsArray) {
            DEBUG_LOG_STATUS("allocateData2", "Cannot create typed array");
            data.clear();
            return data.release();
        }

        cl_mem memObj = createBuffer(CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, jsArray->byteLength(), jsArray->data(), &err_code);
#else // PREALLOCATE_IN_JS_HEAP
        RefPtr<Uint8Array> jsArray;
        cl_mem memObj = createBuffer(CL_MEM_READ_WRITE, length * bytePerElements, 0, &err_code);
#endif // PREALLOCATE_IN_JS_HEAP
        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("allocateData2", err_code);
            data.clear();
            return data.release();
        }

        result = data->initCData<Uint8Array>(m_cmdQueue, memObj, cData->getType(), length, length * bytePerElements, jsArray);

        if (result == RT_OK)
            return data.release();

        data.clear();
        return data.release();

        break;
    }
    case ArrayBufferView::TypeInt16: {
#ifdef PREALLOCATE_IN_JS_HEAP
        RefPtr<Int16Array> jsArray;
        if (createAlignedTA<Int16Array, short>(cData->getType(), length, jsArray) != RT_OK) {
            data.clear();
            return data.release();
        }
        if (!jsArray) {
            DEBUG_LOG_STATUS("allocateData2", "Cannot create typed array");
            data.clear();
            return data.release();
        }

        cl_mem memObj = createBuffer(CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, jsArray->byteLength(), jsArray->data(), &err_code);
#else // PREALLOCATE_IN_JS_HEAP
        RefPtr<Int16Array> jsArray;
        cl_mem memObj = createBuffer(CL_MEM_READ_WRITE, length * bytePerElements, 0, &err_code);
#endif // PREALLOCATE_IN_JS_HEAP
        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("allocateData2", err_code);
            data.clear();
            return data.release();
        }

        result = data->initCData<Int16Array>(m_cmdQueue, memObj, cData->getType(), length, length * bytePerElements, jsArray);

        if (result == RT_OK)
            return data.release();

        data.clear();
        return data.release();

        break;
    }
    case ArrayBufferView::TypeUint16: {
#ifdef PREALLOCATE_IN_JS_HEAP
        RefPtr<Uint16Array> jsArray;
        if (createAlignedTA<Uint16Array, unsigned short>(cData->getType(), length, jsArray) != RT_OK) {
            data.clear();
            return data.release();
        }
        if (!jsArray) {
            DEBUG_LOG_STATUS("allocateData2", "Cannot create typed array");
            data.clear();
            return data.release();
        }

        cl_mem memObj = createBuffer(CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, jsArray->byteLength(), jsArray->data(), &err_code);
#else // PREALLOCATE_IN_JS_HEAP
        RefPtr<Uint16Array> jsArray;
        cl_mem memObj = createBuffer(CL_MEM_READ_WRITE, length * bytePerElements, 0, &err_code);
#endif // PREALLOCATE_IN_JS_HEAP
        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("allocateData2", err_code);
            data.clear();
            return data.release();
        }

        result = data->initCData<Uint16Array>(m_cmdQueue, memObj, cData->getType(), length, length * bytePerElements, jsArray);

        if (result == RT_OK)
            return data.release();

        data.clear();
        return data.release();

        break;
    }
    case ArrayBufferView::TypeInt32: {
#ifdef PREALLOCATE_IN_JS_HEAP
        RefPtr<Int32Array> jsArray;
        if (createAlignedTA<Int32Array, int>(cData->getType(), length, jsArray) != RT_OK) {
            data.clear();
            return data.release();
        }
        if (!jsArray) {
            DEBUG_LOG_STATUS("allocateData2", "Cannot create typed array");
            data.clear();
            return data.release();
        }

        cl_mem memObj = createBuffer(CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, jsArray->byteLength(), jsArray->data(), &err_code);
#else // PREALLOCATE_IN_JS_HEAP
        RefPtr<Int32Array> jsArray;
        cl_mem memObj = createBuffer(CL_MEM_READ_WRITE, length * bytePerElements, 0, &err_code);
#endif // PREALLOCATE_IN_JS_HEAP
        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("allocateData2", err_code);
            data.clear();
            return data.release();
        }

        result = data->initCData<Int32Array>(m_cmdQueue, memObj, cData->getType(), length, length * bytePerElements, jsArray);

        if (result == RT_OK)
            return data.release();

        data.clear();
        return data.release();

        break;
    }
    case ArrayBufferView::TypeUint32: {
#ifdef PREALLOCATE_IN_JS_HEAP
        RefPtr<Uint32Array> jsArray;
        if (createAlignedTA<Uint32Array, unsigned>(cData->getType(), length, jsArray) != RT_OK) {
            data.clear();
            return data.release();
        }
        if (!jsArray) {
            DEBUG_LOG_STATUS("allocateData2", "Cannot create typed array");
            data.clear();
            return data.release();
        }

        cl_mem memObj = createBuffer(CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, jsArray->byteLength(), jsArray->data(), &err_code);
#else // PREALLOCATE_IN_JS_HEAP
        RefPtr<Uint32Array> jsArray;
        cl_mem memObj = createBuffer(CL_MEM_READ_WRITE, length * bytePerElements, 0, &err_code);
#endif // PREALLOCATE_IN_JS_HEAP
        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("allocateData2", err_code);
            data.clear();
            return data.release();
        }

        result = data->initCData<Uint32Array>(m_cmdQueue, memObj, cData->getType(), length, length * bytePerElements, jsArray);

        if (result == RT_OK)
            return data.release();

        data.clear();
        return data.release();

        break;
    }
    case ArrayBufferView::TypeFloat32: {
#ifdef PREALLOCATE_IN_JS_HEAP
        RefPtr<Float32Array> jsArray;
        if (createAlignedTA<Float32Array, float>(cData->getType(), length, jsArray) != RT_OK) {
            data.clear();
            return data.release();
        }
        if (!jsArray) {
            DEBUG_LOG_STATUS("allocateData2", "Cannot create typed array");
            data.clear();
            return data.release();
        }

        cl_mem memObj = createBuffer(CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, jsArray->byteLength(), jsArray->data(), &err_code);
#else // PREALLOCATE_IN_JS_HEAP
        RefPtr<Float32Array> jsArray;
        cl_mem memObj = createBuffer(CL_MEM_READ_WRITE, length * bytePerElements, 0, &err_code);
#endif // PREALLOCATE_IN_JS_HEAP
        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("allocateData2", err_code);
            data.clear();
            return data.release();
        }

        result = data->initCData<Float32Array>(m_cmdQueue, memObj, cData->getType(), length, length * bytePerElements, jsArray);

        if (result == RT_OK)
            return data.release();

        data.clear();
        return data.release();

        break;
    }
    case ArrayBufferView::TypeFloat64: {
#ifdef PREALLOCATE_IN_JS_HEAP
        RefPtr<Float64Array> jsArray;
        if (createAlignedTA<Float64Array, double>(cData->getType(), length, jsArray) != RT_OK) {
            data.clear();
            return data.release();
        }
        if (!jsArray) {
            DEBUG_LOG_STATUS("allocateData2", "Cannot create typed array");
            data.clear();
            return data.release();
        }

        cl_mem memObj = createBuffer(CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, jsArray->byteLength(), jsArray->data(), &err_code);
#else // PREALLOCATE_IN_JS_HEAP
        RefPtr<Float64Array> jsArray;
        cl_mem memObj = createBuffer(CL_MEM_READ_WRITE, length * bytePerElements, 0, &err_code);
#endif // PREALLOCATE_IN_JS_HEAP
        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("allocateData2", err_code);
            data.clear();
            return data.release();
        }

        result = data->initCData<Float64Array>(m_cmdQueue, memObj, cData->getType(), length, length * bytePerElements, jsArray);

        if (result == RT_OK)
            return data.release();

        data.clear();
        return data.release();

        break;
    }
    case ArrayBufferView::TypeUint8Clamped: {
#ifdef PREALLOCATE_IN_JS_HEAP
        RefPtr<Uint8ClampedArray> jsArray;
        if (createAlignedTA<Uint8ClampedArray, unsigned char>(cData->getType(), length, jsArray) != RT_OK) {
            data.clear();
            return data.release();
        }
        if (!jsArray) {
            DEBUG_LOG_STATUS("allocateData2", "Cannot create typed array");
            data.clear();
            return data.release();
        }

        cl_mem memObj = createBuffer(CL_MEM_USE_HOST_PTR | CL_MEM_READ_WRITE, jsArray->byteLength(), jsArray->data(), &err_code);
#else // PREALLOCATE_IN_JS_HEAP
        RefPtr<Uint8ClampedArray> jsArray;
        cl_mem memObj = createBuffer(CL_MEM_READ_WRITE, length * bytePerElements, 0, &err_code);
#endif // PREALLOCATE_IN_JS_HEAP
        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR("allocateData2", err_code);
            data.clear();
            return data.release();
        }

        result = data->initCData<Uint8ClampedArray>(m_cmdQueue, memObj, cData->getType(), length, length * bytePerElements, jsArray);

        if (result == RT_OK)
            return data.release();

        data.clear();
        return data.release();

        break;
    }
    default: {
        data.clear();
        return data.release();

        break;
    }
    }
}

template<class ArrayClass> bool CContext::canBeMapped(ArrayClass* source)
{
#ifdef SUPPORT_MAPPING_ARRAYS
    return true;
#else
    return false;
#endif
}

template<> bool CContext::canBeMapped<Int8Array>(Int8Array* source)
{
    return false;
}

template<> bool CContext::canBeMapped<Uint8Array>(Uint8Array* source)
{
    return false;
}

template<> bool CContext::canBeMapped<Int16Array>(Int16Array* source)
{
    return false;
}

template<> bool CContext::canBeMapped<Uint16Array>(Uint16Array* source)
{
    return false;
}

template<> bool CContext::canBeMapped<Int32Array>(Int32Array* source)
{
    return false;
}

template<> bool CContext::canBeMapped<Uint32Array>(Uint32Array* source)
{
    return false;
}

template bool CContext::canBeMapped<Float32Array>(Float32Array* source);
template bool CContext::canBeMapped<Float64Array>(Float64Array* source);

template<> bool CContext::canBeMapped<Uint8ClampedArray>(Uint8ClampedArray* source)
{
    return false;
}

unsigned long long CContext::lastExecutionTime()
{
#ifdef CLPROFILE
    if (!m_clpExecEnd || !m_clpExecStart)
        return 0;
    else
        return m_clpExecEnd - m_clpExecStart;
#else // CLPROFILE
    return 0;
#endif // CLPROFILE
}

unsigned long long CContext::lastRoundTripTime()
{
#ifdef WINDOWS_ROUNDTRIP
    if ((m_wrtExecStart.QuadPart == -1) || (m_wrtExecEnd.QuadPart == -1))
        return 0;
    else {
        LARGE_INTEGER freq;
        if (!QueryPerformanceFrequency(&freq)) {
            DEBUG_LOG_STATUS("lastRoundTrupTime", "cannot read performance counter frequency.");
            return 0;
        }
        double diff = (double)(m_wrtExecEnd.QuadPart - m_wrtExecStart.QuadPart);
        double time = diff / (double)freq.QuadPart * 1000000000;
        return (unsigned long long)time;
    }
#else // WINDOWS_ROUNDTRIP
    return 0;
#endif // WINDOWS_ROUNDTRIP
}

#ifdef WINDOWS_ROUNDTRIP
void CContext::recordBeginOfRoundTrip(CContext* parent)
{
    CContext* self = parent;
    if (!QueryPerformanceCounter(&(self->m_wrtExecStart))) {
        DEBUG_LOG_STATUS("recordBeginOfRoundTrip", "querying performance counter failed");
        self->m_wrtExecStart.QuadPart = -1;
    }
}

void CContext::recordEndOfRoundTrip(CContext* parent)
{
    CContext* self = parent;
    if (self->m_wrtExecStart.QuadPart == -1) {
        DEBUG_LOG_STATUS("recordEndOfRoundTrip", "no previous start data");
        return;
    }
    if (!QueryPerformanceCounter(&(self->m_wrtExecEnd))) {
        DEBUG_LOG_STATUS("recordEndOfRoundTrip", "querying performance counter failed");
        self->m_wrtExecStart.QuadPart = -1;
        self->m_wrtExecEnd.QuadPart = -1;
    }
}
#endif // WINDOWS_ROUNDTRIP

template<class ArrayClass, class ElementType> void CContext::writeToContext2D(CanvasRenderingContext2D* ctx, ArrayClass* source, int width, int height)
{
#ifdef DIRECT_WRITE
    ArrayClass* srcArray;
    RefPtr<Uint8ClampedArray> uint8ClampedArray;
    IntSize intSize(width, height);
    RefPtr<ImageData> imageData;
    ExceptionCode exceptionCode;

    srcArray = source;

    unsigned size = srcArray->length();

    if (size != width * height * 4)
        return;

    unsigned char* data = (unsigned char*)malloc(size);
    ElementType* src = srcArray->data();
    for (unsigned i = 0; i < size; i++) {
        ElementType val = src[i];
        data[i] = val > 0 ? (val < 255 ? ((int)val) : 255) : 0;
    }
    uint8ClampedArray = Uint8ClampedArray::create(data, size);
    if (!uint8ClampedArray) {
        free(data);
        return;
    }
    imageData = ImageData::create(intSize, uint8ClampedArray);
    if (!imageData) {
        free(data);
        return;
    }
    ctx->putImageData(imageData.get(), width, height, exceptionCode);
    free(data);
#endif // DIRECT_WRITE
}

template<> void CContext::writeToContext2D<Int8Array, char>(CanvasRenderingContext2D* ctx, Int8Array* source, int width, int height)
{
}

template<> void CContext::writeToContext2D<Uint8Array, unsigned char>(CanvasRenderingContext2D* ctx, Uint8Array* source, int width, int height)
{
}

template<> void CContext::writeToContext2D<Int16Array, short>(CanvasRenderingContext2D* ctx, Int16Array* source, int width, int height)
{
}

template<> void CContext::writeToContext2D<Uint16Array, unsigned short>(CanvasRenderingContext2D* ctx, Uint16Array* source, int width, int height)
{
}

template<> void CContext::writeToContext2D<Int32Array, int>(CanvasRenderingContext2D* ctx, Int32Array* source, int width, int height)
{
}

template<> void CContext::writeToContext2D<Uint32Array, unsigned>(CanvasRenderingContext2D* ctx, Uint32Array* source, int width, int height)
{
}

template void CContext::writeToContext2D<Float32Array, float>(CanvasRenderingContext2D* ctx, Float32Array* source, int width, int height);
template void CContext::writeToContext2D<Float64Array, double>(CanvasRenderingContext2D* ctx, Float64Array* source, int width, int height);

template<> void CContext::writeToContext2D<Uint8ClampedArray, unsigned char>(CanvasRenderingContext2D* ctx, Uint8ClampedArray* source, int width, int height)
{
#ifdef DIRECT_WRITE
    Uint8ClampedArray* srcArray;
    RefPtr<Uint8ClampedArray> uint8ClampedArray;
    IntSize intSize(width, height);
    RefPtr<ImageData> imageData;
    ExceptionCode exceptionCode;

    srcArray = source;

    unsigned size = srcArray->length();

    if (size != width * height * 4)
        return;

    unsigned char* src = srcArray->data();
    uint8ClampedArray = Uint8ClampedArray::create(src, size);
    if (!uint8ClampedArray)
        return;
    imageData = ImageData::create(intSize, uint8ClampedArray);
    if (!imageData)
        return;
    ctx->putImageData(imageData.get(), width, height, exceptionCode);
#endif // DIRECT_WRITE
}

unsigned CContext::alignmentSize()
{
    return m_alignmentSize;
}

template<class ArrayClass> unsigned CContext::getAlignmentOffset(ArrayClass* source)
{
    ArrayClass* object;
    unsigned char* data;

    object = source;
    data = getPointerFromTA<ArrayClass>(object);

    return (((uintptr_t)data) + m_alignmentSize) / m_alignmentSize * m_alignmentSize - ((uintptr_t)data);
}

template unsigned CContext::getAlignmentOffset<Int8Array>(Int8Array* source);
template unsigned CContext::getAlignmentOffset<Uint8Array>(Uint8Array* source);
template unsigned CContext::getAlignmentOffset<Int16Array>(Int16Array* source);
template unsigned CContext::getAlignmentOffset<Uint16Array>(Uint16Array* source);
template unsigned CContext::getAlignmentOffset<Int32Array>(Int32Array* source);
template unsigned CContext::getAlignmentOffset<Uint32Array>(Uint32Array* source);
template unsigned CContext::getAlignmentOffset<Float32Array>(Float32Array* source);
template unsigned CContext::getAlignmentOffset<Float64Array>(Float64Array* source);
template unsigned CContext::getAlignmentOffset<Uint8ClampedArray>(Uint8ClampedArray* source);

String CContext::extensions()
{
    WTF::String extensions(m_parent->openclUtil()->deviceExtensions().c_str(), m_parent->openclUtil()->deviceExtensions().length());
    return extensions;
}

} // namespace WebCore
