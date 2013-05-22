#ifndef CINTERFACE_H
#define CINTERFACE_H

#include "CPlatform.h"

#include <wtf/Forward.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/text/WTFString.h>

#include "OCLconfig.h"
#include "opencl_compat.h"

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
    //static CInterface *getInstance();

private:
    CInterface();
    //static  CInterface* singleton;
    cl_platform_id* m_platforms;
    cl_uint m_noOfPlatforms;
    int InitPlatformInfo();
};

} // namespace WebCore
 
#endif