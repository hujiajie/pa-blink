#pragma once
#include "config.h"
#include "CPlatform.h"

namespace WebCore {

cl_platform_id CPlatform::s_platforms = 0;
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
    cl_int err_code = clGetDeviceIDs(s_platforms, CL_DEVICE_TYPE_ALL, 0, 0, &devices);
    
    if (err_code != CL_SUCCESS)
        return -1;

    return devices;
}

int CPlatform::getPlatformPropertyHelper(cl_platform_info param, char*& out)
{
    char* rString = 0;
    size_t length;
    cl_int err;
    int result;

    err = clGetPlatformInfo(s_platforms, param, 0, 0, &length);

    if (err == CL_SUCCESS) {
        rString = new char[length+1];
        err = clGetPlatformInfo(s_platforms, param, length, rString, 0);
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
    char* aVersion;
    if (getPlatformPropertyHelper(CL_PLATFORM_VERSION, aVersion) != CL_SUCCESS)
        return String::fromUTF8("Cannot get version");
    String ret(aVersion);
    delete [] aVersion;
    return ret;
}
String CPlatform::name()
{
    char* aName;
    if (getPlatformPropertyHelper(CL_PLATFORM_NAME, aName) != CL_SUCCESS)
        return String::fromUTF8("Cannot get name");
    String ret(aName);
    delete [] aName;
    return ret;  
}
String CPlatform::vendor()
{
    char* aVendor;
    if (getPlatformPropertyHelper(CL_PLATFORM_VENDOR, aVendor) != CL_SUCCESS)
        return String::fromUTF8("Cannot get vendor");
    String ret(aVendor);
    delete [] aVendor;
    return ret;  
}
String CPlatform::profile()
{
    char* aProfile;
    if (getPlatformPropertyHelper(CL_PLATFORM_PROFILE, aProfile) != CL_SUCCESS)
        return String::fromUTF8("Cannot get profile");
    String ret(aProfile);
    delete [] aProfile;
    return ret; 
}
String CPlatform::extensions()
{
    char* aExtensions;
    if (getPlatformPropertyHelper(CL_PLATFORM_EXTENSIONS, aExtensions) != CL_SUCCESS)
        return String::fromUTF8("Cannot get extensions");
    String ret(aExtensions);
    delete [] aExtensions;
    return ret; 
}

} // namespace WebCore
