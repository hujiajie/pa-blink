#include "config.h"
#include "CInterface.h"

namespace WebCore {

CInterface::CInterface()
    : m_platforms(0)
{
    DEBUG_LOG_CREATE("CInterface", this);
}

CInterface::~CInterface()
{
    DEBUG_LOG_DESTROY("CInterface", this);
    if(m_platforms)
        m_platforms = 0;
}

int CInterface::initPlatformInfo()
{
    cl_int err_code;

    cl_uint nplatforms;
    
    err_code = clGetPlatformIDs( 0, 0, &nplatforms);
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR( "initPlatformInfo", err_code);
        return err_code;
    }

    m_platforms = new cl_platform_id[nplatforms];

    err_code = clGetPlatformIDs( nplatforms, m_platforms, &m_noOfPlatforms);
    if (err_code != CL_SUCCESS) {
        DEBUG_LOG_ERROR( "initPlatformInfo", err_code);
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
        result = initPlatformInfo();

    if (result != CL_SUCCESS) 
        return 0;
        
    for (cl_uint i = 0; i < m_noOfPlatforms; i++) {
        err_code = clGetPlatformInfo(m_platforms[i], CL_PLATFORM_NAME, maxNameLength * sizeof(char), name, 0);
        if (err_code != CL_SUCCESS) {
            DEBUG_LOG_ERROR( "GetIntelPlatform", err_code);
            return 0;
        }
        if (!strcmp(name, "Intel(R) OpenCL") || !strcmp(name, "Apple")) {
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
