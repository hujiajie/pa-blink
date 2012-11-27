#ifndef CPlatform_h
#define CPlatform_h
    
#include "CContext.h"
#include "OCLconfig.h"
#include "OCLdebug.h"
#include "opencl_compat.h"
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/text/WTFString.h>

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
    CPlatform(cl_platform_id);
    int getPlatformPropertyHelper(cl_platform_info param, char*& out);
    
    static cl_platform_id s_platforms;
    
};

} // namespace WebCore
#endif // CPlatform_h
