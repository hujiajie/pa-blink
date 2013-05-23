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

#pragma once
#include "config.h"

#include "core/rivertrail/CPlatform.h"

namespace WebCore {

CPlatform::CPlatform()
{
    DEBUG_LOG_CREATE("CPlatform", this);  
    opencl_util = new OCLUtil();
}

CPlatform::~CPlatform()
{
    DEBUG_LOG_DESTROY("CPlatform", this);
}

PassRefPtr<CContext> CPlatform::createContext()
{
    RefPtr<CContext> out = CContext::create(this);
    int result = out->initContext();
    if (result == CL_SUCCESS)
        return out.release();
    return 0;
}

String CPlatform::version()
{
    WTF::String version(opencl_util->version().c_str(), opencl_util->version().length());
    return version;
}

String CPlatform::name()
{
    WTF::String name(opencl_util->name().c_str(), opencl_util->name().length());
    return name;
}

String CPlatform::vendor()
{
    WTF::String vendor(opencl_util->vendor().c_str(), opencl_util->vendor().length());
    return vendor;
}

String CPlatform::profile()
{
    WTF::String profile(opencl_util->profile().c_str(), opencl_util->profile().length());
    return profile;
}

String CPlatform::extensions()
{
    WTF::String extensions(opencl_util->platformExtensions().c_str(), opencl_util->platformExtensions().length());
    return extensions;
}

OCLUtil* CPlatform::openclUtil()
{
    return opencl_util;
}

} // namespace WebCore
