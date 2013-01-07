#ifndef CPlatform_h
#define CPlatform_h
    
#include "CContext.h"
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
    ~CPlatform();

    PassRefPtr<CContext> createContext();
    String version();
    String name();
    String vendor();
    String profile();
    String extensions();
    OCLUtil* openclUtil();

private:
    CPlatform();
    OCLUtil* opencl_util;
};

} // namespace WebCore
#endif // CPlatform_h
