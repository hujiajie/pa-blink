#include "config.h"
#include "CInterface.h"

namespace WebCore {

CInterface::CInterface()
{
    DEBUG_LOG_CREATE("CInterface", this);
}

CInterface::~CInterface()
{
    DEBUG_LOG_DESTROY("CInterface", this);
}

PassRefPtr<CPlatform> CInterface::getPlatform()
{
    RefPtr<CPlatform> out = CPlatform::create();
    if (!out)
        return 0;
    else
        return out.release();
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
