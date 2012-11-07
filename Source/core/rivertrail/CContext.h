#ifndef CContext_h
#define CContext_h

#include "CL/opencl.h"
#include "PlatformString.h"
#include <wtf/Float32Array.h>
#include <wtf/Float64Array.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/Uint8ClampedArray.h>

#include "OCLconfig.h"
#include "opencl_compat.h"

#ifdef WINDOWS_ROUNDTRIP
#include <windows.h>
#endif // WINDOWS_ROUNDTRIP

namespace WebCore {

class CanvasRenderingContext2D;
class CData;
class CKernel;
class CPlatform;

class CContext : public RefCounted<CContext> {
public:
    static PassRefPtr<CContext> create(CPlatform* aParent) { return adoptRef(new CContext(aParent)); }
    ~CContext();

    unsigned initContext(cl_platform_id platform);

#ifdef CLPROFILE
    static void CL_CALLBACK collectTimings(cl_event event, cl_int status, void* data);
#endif // CLPROFILE
    static void CL_CALLBACK reportCLError(const char* err_info, const void* private_info, size_t cb, void* user_data);
    static unsigned char* getPointerFromTAFloat32Array(Float32Array* ta);
    static unsigned char* getPointerFromTAFloat64Array(Float64Array* ta);
    static unsigned char* getPointerFromTAUint8ClampedArray(Uint8ClampedArray* ta);
    unsigned createAlignedTAFloat32Array(unsigned type, size_t length, RefPtr<Float32Array>& retval);
    unsigned createAlignedTAFloat64Array(unsigned type, size_t length, RefPtr<Float64Array>& retval);
    unsigned createAlignedTAUint8ClampedArray(unsigned type, size_t length, RefPtr<Uint8ClampedArray>& retval);
#ifdef WINDOWS_ROUNDTRIP
    static void recordBeginOfRoundTrip(CContext* parent);
    static void recordEndOfRoundTrip(CContext* parent);
#endif // WINDOWS_ROUNDTRIP

#ifdef INCREMENTAL_MEM_RELEASE
    int checkFree();
    void deferFree(cl_mem);
#endif // INCREMENTAL_MEM_RELEASE

    PassRefPtr<CKernel> compileKernel(const String& source, const String& kernelName, const String& options);
    String buildLog();
    PassRefPtr<CData> mapDataFloat32Array(PassRefPtr<Float32Array> source);
    PassRefPtr<CData> mapDataFloat64Array(PassRefPtr<Float64Array> source);
    PassRefPtr<CData> mapDataUint8ClampedArray(PassRefPtr<Uint8ClampedArray> source);
    PassRefPtr<CData> cloneDataFloat32Array(Float32Array* source);
    PassRefPtr<CData> cloneDataFloat64Array(Float64Array* source);
    PassRefPtr<CData> cloneDataUint8ClampedArray(Uint8ClampedArray* source);
    PassRefPtr<CData> allocateDataFloat32Array(Float32Array* templ, unsigned length);
    PassRefPtr<CData> allocateDataFloat64Array(Float64Array* templ, unsigned length);
    PassRefPtr<CData> allocateDataUint8ClampedArray(Uint8ClampedArray* templ, unsigned length);
    PassRefPtr<CData> allocateData2(CData* templ, unsigned length);
    bool canBeMappedFloat32Array(Float32Array* source);
    bool canBeMappedFloat64Array(Float64Array* source);
    bool canBeMappedUint8ClampedArray(Uint8ClampedArray* source);
    unsigned long long lastExecutionTime();
    unsigned long long lastRoundTripTime();
    void writeToContext2DFloat32Array(CanvasRenderingContext2D* ctx, Float32Array* source, int width, int height);
    void writeToContext2DFloat64Array(CanvasRenderingContext2D* ctx, Float64Array* source, int width, int height);
    void writeToContext2DUint8ClampedArray(CanvasRenderingContext2D* ctx, Uint8ClampedArray* source, int width, int height);
    unsigned alignmentSize();
    unsigned getAlignmentOffsetFloat32Array(Float32Array* source);
    unsigned getAlignmentOffsetFloat64Array(Float64Array* source);
    unsigned getAlignmentOffsetUint8ClampedArray(Uint8ClampedArray* source);

private:
    CContext(CPlatform* parent);
    CPlatform* m_parent;
    cl_context m_context; // the corresponding OpenCL context object
    cl_command_queue m_cmdQueue; // command queue shared by all child objects (e.g. kernels)
    char* m_buildLog; // shared string used to store the build log in compileKernel
    size_t m_buildLogSize; // current size of buildLog, 0 if not yet allocated
    cl_mem m_kernelFailureMem; // memory buffer used to communicate abortion of kernels; shared among all kernels
    cl_uint m_alignmentSize; // stores the alignment size for the used devices

    cl_mem createBuffer(cl_mem_flags flags, size_t size, void* ptr, cl_int* err);

#ifdef CLPROFILE
    cl_ulong m_clpExecStart;
    cl_ulong m_clpExecEnd;
#endif // CLPROFILE
#ifdef WINDOWS_ROUNDTRIP
    LARGE_INTEGER m_wrtExecStart;
    LARGE_INTEGER m_wrtExecEnd;
#endif // WINDOWS_ROUNDTRIP
#ifdef INCREMENTAL_MEM_RELEASE
    cl_mem* m_deferList;
    unsigned m_deferPos;
    unsigned m_deferMax;
#endif // INCREMENTAL_MEM_RELEASE
};

} // namespace WebCore

#endif // CContext_h
