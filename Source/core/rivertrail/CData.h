#ifndef CData_h
#define CData_h

#include "OCLconfig.h"
#include "OCLdebug.h"
#include "opencl_compat.h"
#include <wtf/Float32Array.h>
#include <wtf/Float64Array.h>
#include <wtf/Int16Array.h>
#include <wtf/Int32Array.h>
#include <wtf/Int8Array.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/Uint16Array.h>
#include <wtf/Uint32Array.h>
#include <wtf/Uint8Array.h>
#include <wtf/Uint8ClampedArray.h>

namespace WebCore {

class CContext;

class CData : public RefCounted<CData> {
public:
    static PassRefPtr<CData> create(CContext* aParent) { return adoptRef(new CData(aParent)); }
    ~CData();

    template<class ArrayClass> unsigned initCData(cl_command_queue aQueue, cl_mem aMemObj, ArrayBufferView::ViewType aType, unsigned aLength, unsigned aSize, PassRefPtr<ArrayClass> anArray);
    cl_mem getContainedBuffer();
    ArrayBufferView::ViewType getType();
    unsigned getSize();
    unsigned getLength();
    template<class ArrayClass> ArrayClass* getValue();
    template<class ArrayClass> void writeTo(ArrayClass* dest);

private:
    CData(CContext* aParent);
    CContext* m_parent;
    cl_command_queue m_queue;
    cl_mem m_memObj;
    ArrayBufferView::ViewType m_type;
    unsigned m_length;
    unsigned m_size;
    RefPtr<Int8Array> m_theInt8Array;
    RefPtr<Uint8Array> m_theUint8Array;
    RefPtr<Int16Array> m_theInt16Array;
    RefPtr<Uint16Array> m_theUint16Array;
    RefPtr<Int32Array> m_theInt32Array;
    RefPtr<Uint32Array> m_theUint32Array;
    RefPtr<Float32Array> m_theFloat32Array;
    RefPtr<Float64Array> m_theFloat64Array;
    RefPtr<Uint8ClampedArray> m_theUint8ClampedArray;
    bool m_isRetained;
#ifdef PREALLOCATE_IN_JS_HEAP
    bool m_isMapped;
#endif // PREALLOCATE_IN_JS_HEAP
#ifdef INCREMENTAL_MEM_RELEASE
    int checkFree();
    void deferFree(cl_mem);
#endif // INCREMENTAL_MEM_RELEASE
    cl_int enqueueReadBuffer(size_t size, void* ptr);
};

} // namespace WebCore

#endif // CData_h
