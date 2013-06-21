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

#ifndef CData_h
#define CData_h

#include "core/rivertrail/OCLUtil.h"
#include "core/rivertrail/OCLconfig.h"
#include "core/rivertrail/OCLdebug.h"
#include "core/rivertrail/opencl_compat.h"
#include <wtf/Float32Array.h>
#include <wtf/Float64Array.h>
#include <wtf/Int16Array.h>
#include <wtf/Int32Array.h>
#include <wtf/Int8Array.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/Uint16Array.h>
#include <wtf/Uint32Array.h>
#include <wtf/Uint8Array.h>
#include <wtf/Uint8ClampedArray.h>

namespace WebCore {

class CContext;

class CData : public RefCounted<CData> {
public:
    static PassRefPtr<CData> create(CContext* aParent) { return adoptRef(new CData(aParent)); }
    ~CData();

    template<class ArrayClass> unsigned initCData(cl_command_queue aQueue, cl_mem aMemObj, ArrayBufferView::ViewType aType, unsigned aLength, unsigned aSize, PassRefPtr<ArrayClass> anArray);
    cl_mem getContainedBuffer();
    ArrayBufferView::ViewType getType();
    unsigned getSize();
    unsigned getLength();
    template<class ArrayClass> ArrayClass* getValue();
    template<class ArrayClass> void writeTo(ArrayClass* dest);

private:
    CData(CContext* aParent);
    CContext* m_parent;
    cl_command_queue m_queue;
    cl_mem m_memObj;
    ArrayBufferView::ViewType m_type;
    unsigned m_length;
    unsigned m_size;
    RefPtr<Int8Array> m_theInt8Array;
    RefPtr<Uint8Array> m_theUint8Array;
    RefPtr<Int16Array> m_theInt16Array;
    RefPtr<Uint16Array> m_theUint16Array;
    RefPtr<Int32Array> m_theInt32Array;
    RefPtr<Uint32Array> m_theUint32Array;
    RefPtr<Float32Array> m_theFloat32Array;
    RefPtr<Float64Array> m_theFloat64Array;
    RefPtr<Uint8ClampedArray> m_theUint8ClampedArray;
    bool m_isRetained;
#ifdef PREALLOCATE_IN_JS_HEAP
    bool m_isMapped;
#endif // PREALLOCATE_IN_JS_HEAP
#ifdef INCREMENTAL_MEM_RELEASE
    int checkFree();
    void deferFree(cl_mem);
#endif // INCREMENTAL_MEM_RELEASE
    cl_int enqueueReadBuffer(size_t size, void* ptr);
};

} // namespace WebCore

#endif // CData_h
