#ifndef CKernel_h
#define CKernel_h

#include "CData.h"
#include "OCLconfig.h"
#include "OCLdebug.h"
#include "opencl_compat.h"
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

class CContext;
class CKernel: public RefCounted<CKernel> {
public:
    static PassRefPtr<CKernel> create(CContext* aParent)
        {
            return adoptRef(new CKernel(aParent));
        }
    ~CKernel();

    unsigned long numberOfArgs();
    bool setArgument( unsigned number, CData* argument);
    template<class ArgClass> bool setScalarArgument(unsigned number, const ArgClass value, const bool isIntegerB, const bool isHighPrecisionB);
    unsigned run(unsigned rank, unsigned* shape, unsigned* tile);
    
    int initKernel(cl_command_queue, cl_kernel, cl_mem failureMem);

private:
    CKernel(CContext* aParent);
    cl_kernel m_kernel;
    cl_command_queue m_cmdQueue;
    cl_mem m_failureMem;
    CContext* m_parent;
    
};

} // namespace WebCore
#endif // CKernel_h
