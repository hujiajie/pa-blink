#pragma once
#include "config.h"
#include "CPlatform.h"

namespace WebCore {

CPlatform::CPlatform()
{
    DEBUG_LOG_CREATE("CPlatform", this);  
    opencl_util = new OCLUtil();
    opencl_util->Init();
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

OCLUtil* CPlatform::openclUtil()
{
    return opencl_util;
}

} // namespace WebCore
