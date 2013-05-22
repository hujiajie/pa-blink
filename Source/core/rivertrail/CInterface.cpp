#include "config.h"
#include "CInterface.h"
#include "OCLdebug.h"

namespace WebCore {
CInterface::CInterface()
    :m_platforms(NULL)
{
    DEBUG_LOG_CREATE("CInterface", this);
}

CInterface::~CInterface()
{
    DEBUG_LOG_DESTROY("CInterface", this);
    if(m_platforms)
        m_platforms = NULL;
}

int CInterface::InitPlatformInfo()
{
    cl_int err_code;

    cl_uint nplatforms;
    
    err_code = clGetPlatformIDs( 0, NULL, &nplatforms);
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR( "InitPlatformInfo", err_code);
        return err_code;
    }

    m_platforms = new cl_platform_id[nplatforms];

    err_code = clGetPlatformIDs( nplatforms, m_platforms, &m_noOfPlatforms);
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR( "InitPlatformInfo", err_code);
        return err_code;
    }

    return err_code;

}

PassRefPtr<CPlatform> CInterface::getPlatform()
{
    int result = CL_SUCCESS;
    cl_int err_code;
    const cl_uint maxNameLength = 256;
    char name[maxNameLength];
    
    if (!m_platforms)
        result = InitPlatformInfo();

    if (result != CL_SUCCESS) 
        return 0;
        
    for (cl_uint i = 0; i < m_noOfPlatforms; i++) {
        err_code = clGetPlatformInfo(m_platforms[i], CL_PLATFORM_NAME, maxNameLength * sizeof(char), name, NULL);
        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR( "GetIntelPlatform", err_code);
            return 0;
        }
        if ((strcmp(name, "Intel(R) OpenCL") == 0) || (strcmp(name, "Apple") == 0)) {
            RefPtr<CPlatform> out = CPlatform::create(m_platforms[i]);
            if (!out)
                return 0;
            else
                return out.release();
        }
    }
    return 0;
}

unsigned long CInterface::version()
{
    return INTERFACE_VERSION;
}

} // namespace WebCore