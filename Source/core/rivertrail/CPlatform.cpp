//#pragma once
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
