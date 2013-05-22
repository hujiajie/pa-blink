#pragma once
#include "config.h"
#include "CPlatform.h"
#include "OCLdebug.h"

namespace WebCore {

cl_platform_id CPlatform::s_platforms = NULL;
CPlatform::CPlatform(cl_platform_id aPlatform)
{
    DEBUG_LOG_CREATE("CPlatform", this);  
    s_platforms = aPlatform;
}

CPlatform::~CPlatform()
{
    DEBUG_LOG_DESTROY("CPlatform", this);
}

int CPlatform::numberOfDevices()
{
    cl_uint devices;
    cl_int err_code = clGetDeviceIDs(s_platforms, CL_DEVICE_TYPE_ALL, 0, NULL, &devices);
    
    if (err_code != CL_SUCCESS)
        return -1;

    return devices;
}

int CPlatform::getPlatformPropertyHelper(cl_platform_info param, char* & out)
{
    char* rString = NULL;
    size_t length;
    cl_int err;
    int result;

    err = clGetPlatformInfo(s_platforms, param, 0, NULL, &length);

    if (err == CL_SUCCESS) {
        rString = new char[length+1];
        err = clGetPlatformInfo(s_platforms, param, length, rString, NULL);
        out = rString;  
        result = CL_SUCCESS;
    } else
        result = err;
    return result;
}

PassRefPtr<CContext> CPlatform::createContext()
{
    RefPtr<CContext> out = CContext::create(this);
    int result = out->initContext(s_platforms);
    if (result == CL_SUCCESS)
        return out.release();
    return 0;
}

String CPlatform::version()
{
    char* aVersion="";
    getPlatformPropertyHelper(CL_PLATFORM_VERSION, aVersion);
    return String(aVersion);
}
String CPlatform::name()
{
    char* aName="";
    getPlatformPropertyHelper(CL_PLATFORM_NAME, aName);
    return String(aName);
    
}
String CPlatform::vendor()
{
    char* aVendor="";
    getPlatformPropertyHelper(CL_PLATFORM_VENDOR, aVendor);
    return String(aVendor);
}       
String CPlatform::profile()
{
    char* aProfile="";
    getPlatformPropertyHelper(CL_PLATFORM_PROFILE, aProfile);
    return String(aProfile);
}
String CPlatform::extensions()
{
    char* aExtensions="";
    getPlatformPropertyHelper(CL_PLATFORM_EXTENSIONS, aExtensions);
    return String(aExtensions);
}
} // namespace WebCore