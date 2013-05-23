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

#include "core/rivertrail/CInterface.h"

namespace WebCore {

CInterface::CInterface()
{
    DEBUG_LOG_CREATE("CInterface", this);
}

CInterface::~CInterface()
{
    DEBUG_LOG_DESTROY("CInterface", this);
}

PassRefPtr<CPlatform> CInterface::getPlatform()
{
    RefPtr<CPlatform> out = CPlatform::create();
    if (!out)
        return 0;
    else
        return out.release();
}

unsigned long CInterface::version()
{
    return INTERFACE_VERSION;
}

// Comment getInstance() for future
// CInterface* CInterface::s_singleton = 0;

/*PassRefPtr<CInterface> CInterface::getInstance(bool& hasSingleton)
{
    if (s_singleton)
        return PassRefPtr<CInterface>(s_singleton);
        
    hasSingleton = false;
    RefPtr<CInterface> result = CInterface::create();
    s_singleton = result.get();
    return result.release();
}*/

} // namespace WebCore
