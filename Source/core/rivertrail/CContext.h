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

#ifndef CContext_h
#define CContext_h

#include "core/rivertrail/OCLUtil.h"
#include "core/rivertrail/OCLConfig.h"
#include "core/rivertrail/OCLDebug.h"
#include "core/rivertrail/opencl_compat.h"
#include <wtf/ArrayBufferView.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/text/WTFString.h>

#ifdef WINDOWS_ROUNDTRIP
#include <windows.h>
#endif // WINDOWS_ROUNDTRIP

namespace WebCore {

class CanvasRenderingContext2D;
class CData;
class CKernel;
class CPlatform;

class CContext : public RefCounted<CContext> {
public:
    static PassRefPtr<CContext> create(CPlatform* aParent) { return adoptRef(new CContext(aParent)); }
    ~CContext();

    unsigned initContext();

#ifdef CLPROFILE
    static void CL_CALLBACK collectTimings(cl_event event, cl_int status, void* data);
#endif // CLPROFILE
    template<class ArrayClass> static unsigned char* getPointerFromTA(ArrayClass* ta);
    template<class ArrayClass, class ElementType> unsigned createAlignedTA(ArrayBufferView::ViewType type, size_t length, RefPtr<ArrayClass>& retval);
#ifdef WINDOWS_ROUNDTRIP
    static void recordBeginOfRoundTrip(CContext* parent);
    static void recordEndOfRoundTrip(CContext* parent);
#endif // WINDOWS_ROUNDTRIP

#ifdef INCREMENTAL_MEM_RELEASE
    int checkFree();
    void deferFree(cl_mem);
#endif // INCREMENTAL_MEM_RELEASE

    PassRefPtr<CKernel> compileKernel(const String& source, const String& kernelName, const String& options);
    String buildLog();
    template<class ArrayClass, ArrayBufferView::ViewType type> PassRefPtr<CData> mapData(PassRefPtr<ArrayClass> source);
    template<class ArrayClass> PassRefPtr<CData> cloneData(ArrayClass* source);
    template<class ArrayClass, class ElementType, ArrayBufferView::ViewType type> PassRefPtr<CData> allocateData(ArrayClass* templ, unsigned length);
    PassRefPtr<CData> allocateData2(CData* templ, unsigned length);
    template<class ArrayClass> bool canBeMapped(ArrayClass* source);
    unsigned long long lastExecutionTime();
    unsigned long long lastRoundTripTime();
    String extensions();
    template<class ArrayClass, class ElementType> void writeToContext2D(CanvasRenderingContext2D* ctx, ArrayClass* source, int width, int height);
    unsigned alignmentSize();
    template<class ArrayClass> unsigned getAlignmentOffset(ArrayClass* source);

private:
    CContext(CPlatform* parent);
    CPlatform* m_parent;
    cl_context m_context; // the corresponding OpenCL context object
    cl_command_queue m_cmdQueue; // command queue shared by all child objects (e.g. kernels)
    char* m_buildLog; // shared string used to store the build log in compileKernel
    size_t m_buildLogSize; // current size of buildLog, 0 if not yet allocated
    cl_mem m_kernelFailureMem; // memory buffer used to communicate abortion of kernels; shared among all kernels
    cl_uint m_alignmentSize; // stores the alignment size for the used devices

    cl_mem createBuffer(cl_mem_flags flags, size_t size, void* ptr, cl_int* err);

#ifdef CLPROFILE
    cl_ulong m_clpExecStart;
    cl_ulong m_clpExecEnd;
#endif // CLPROFILE
#ifdef WINDOWS_ROUNDTRIP
    LARGE_INTEGER m_wrtExecStart;
    LARGE_INTEGER m_wrtExecEnd;
#endif // WINDOWS_ROUNDTRIP
#ifdef INCREMENTAL_MEM_RELEASE
    cl_mem* m_deferList;
    unsigned m_deferPos;
    unsigned m_deferMax;
#endif // INCREMENTAL_MEM_RELEASE
};

} // namespace WebCore

#endif // CContext_h
