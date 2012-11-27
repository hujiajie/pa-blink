#ifndef CInterface_h
#define CInterface_h

#include "CPlatform.h"
#include "OCLconfig.h"
#include "OCLdebug.h"
#include "opencl_compat.h"
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/text/WTFString.h>

namespace WebCore {
   
class CInterface: public RefCounted<CInterface> {
public:
    static PassRefPtr<CInterface> create()
    {
        return adoptRef(new CInterface());
    }
    ~CInterface();
    
    PassRefPtr<CPlatform> getPlatform();
    unsigned long version();
    // static PassRefPtr<CInterface> getInstance(bool& hasSingleton);

private:
    CInterface();
    // static  CInterface* s_singleton;
    cl_platform_id* m_platforms;
    cl_uint m_noOfPlatforms;
    int initPlatformInfo();
};

} // namespace WebCore
 
#endif // CInterface_h
