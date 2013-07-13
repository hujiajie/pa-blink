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

#ifndef CKernel_h
#define CKernel_h

#include "core/rivertrail/CData.h"
#include "core/rivertrail/OCLUtil.h"
#include "core/rivertrail/OCLConfig.h"
#include "core/rivertrail/OCLDebug.h"
#include "core/rivertrail/opencl_compat.h"
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

class CContext;
class CKernel: public RefCounted<CKernel> {
public:
    static PassRefPtr<CKernel> create(CContext* aParent)
        {
            return adoptRef(new CKernel(aParent));
        }
    ~CKernel();

    unsigned long numberOfArgs();
    bool setArgument( unsigned number, CData* argument);
    template<class ArgClass> bool setScalarArgument(unsigned number, const ArgClass value, const bool isIntegerB, const bool isHighPrecisionB);
    unsigned run(unsigned rank, unsigned* shape, unsigned* tile);
    
    int initKernel(cl_command_queue, cl_kernel, cl_mem failureMem);

private:
    CKernel(CContext* aParent);
    cl_kernel m_kernel;
    cl_command_queue m_cmdQueue;
    cl_mem m_failureMem;
    CContext* m_parent;
    
};

} // namespace WebCore
#endif // CKernel_h
