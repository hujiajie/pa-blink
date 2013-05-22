#ifndef CKERNEL_H
#define CKERNEL_H

#include "CData.h"

#include <wtf/Forward.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/text/WTFString.h>

#include "OCLconfig.h"
#include "opencl_compat.h"

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
    bool setArgument( unsigned int number, CData* argument);
    bool setScalarArgumentInt(unsigned int number, const int value, const bool isIntegerB, const bool isHighPrecisionB);
    bool setScalarArgumentDouble(unsigned int number, const double value, const bool isIntegerB, const bool isHighPrecisionB);
    unsigned int run(unsigned int rank, unsigned int* shape, unsigned int* tile);
    
    int initKernel(cl_command_queue aCmdQueue, cl_kernel aKernel, cl_mem failureMem);

private:
    CKernel(CContext* aParent);
    cl_kernel m_kernel;
    cl_command_queue m_cmdQueue;
    cl_mem m_failureMem;
    CContext* m_parent;
    
};
} // namespace WebCore
#endif