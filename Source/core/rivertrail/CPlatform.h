#ifndef CPLATFORM_H
#define CPLATFORM_H
    
#include "CContext.h"

#include <wtf/Forward.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/text/WTFString.h>

#include "OCLconfig.h"
#include "opencl_compat.h"

namespace WebCore {

class CPlatform: public RefCounted<CPlatform> {
public:
    static PassRefPtr<CPlatform> create()
        {
            return adoptRef(new CPlatform());
        }
    static PassRefPtr<CPlatform> create(cl_platform_id platform)
        {
            return adoptRef(new CPlatform(platform));
        }
    ~CPlatform();
        
    PassRefPtr<CContext> createContext();
    int numberOfDevices();
    String version(); 
    String name(); 
    String vendor(); 
    String profile(); 
    String extensions(); 
    
private:
    CPlatform(){};
    CPlatform(cl_platform_id platform);
    int getPlatformPropertyHelper(cl_platform_info param, char* & out);
    
    static cl_platform_id s_platforms;
    
};
} // namespace WebCore
#endif