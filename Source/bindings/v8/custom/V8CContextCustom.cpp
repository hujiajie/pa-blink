#include "config.h"
#include "V8CContext.h"

#include "CanvasRenderingContext2D.h"
#include "CContext.h"
#include "CData.h"
#include "CKernel.h"
#include "CPlatform.h"
#include "V8Binding.h"
#include "V8CanvasRenderingContext2D.h"
#include "V8CData.h"
#include "V8CKernel.h"
#include "V8CPlatform.h"
#include "V8DOMWrapper.h"
#include "V8Float32Array.h"
#include "V8Float64Array.h"
#include "V8Int16Array.h"
#include "V8Int32Array.h"
#include "V8Int8Array.h"
#include "V8Uint16Array.h"
#include "V8Uint32Array.h"
#include "V8Uint8Array.h"
#include "V8Uint8ClampedArray.h"
#include <wtf/Float32Array.h>
#include <wtf/Float64Array.h>
#include <wtf/Int16Array.h>
#include <wtf/Int32Array.h>
#include <wtf/Int8Array.h>
#include <wtf/RefPtr.h>
#include <wtf/Uint16Array.h>
#include <wtf/Uint32Array.h>
#include <wtf/Uint8Array.h>
#include <wtf/Uint8ClampedArray.h>

namespace WebCore {

v8::Handle<v8::Value> V8CContext::constructorCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CContext.Constructor");

    if (!args.IsConstructCall())
        return throwTypeError("DOM object constructor cannot be called as a function.", args.GetIsolate());

    if (ConstructorMode::current() == ConstructorMode::WrapExistingObject)
        return args.Holder();

    if (args.Length() != 1)
        return throwError(GeneralError, "CContext cannot be constructed because of wrong parameters.", args.GetIsolate());
    if (!V8CPlatform::HasInstance(args[0]))
        return throwError(GeneralError, "CContext cannot be constructed because of wrong parameters.", args.GetIsolate());

    CPlatform* parent = V8CPlatform::toNative(v8::Handle<v8::Object>::Cast(args[0]));
    RefPtr<CContext> cContext = CContext::create(parent);
    V8DOMWrapper::setDOMWrapper(args.Holder(), &info, cContext.get());
    V8DOMWrapper::setJSWrapperForDOMObject(cContext.release(), v8::Persistent<v8::Object>::New(args.Holder()));
    return args.Holder();
}

v8::Handle<v8::Value> V8CContext::compileKernelCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CContext.compileKernel");
    if (args.Length() < 2)
        return throwNotEnoughArgumentsError(args.GetIsolate());
    CContext* imp = V8CContext::toNative(args.Holder());
    STRING_TO_V8PARAMETER_EXCEPTION_BLOCK(V8Parameter<>, source, MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined));
    STRING_TO_V8PARAMETER_EXCEPTION_BLOCK(V8Parameter<>, kernelName, MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined));
    STRING_TO_V8PARAMETER_EXCEPTION_BLOCK(V8Parameter<>, options, MAYBE_MISSING_PARAMETER(args, 2, DefaultIsNullString));
    RefPtr<CKernel> cKernel = imp->compileKernel(source, kernelName, options);
    if (!cKernel)
        return throwError(GeneralError, "Cannot create new CKernel object.", args.GetIsolate());
    return toV8(cKernel.get());
}

v8::Handle<v8::Value> V8CContext::mapDataCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CContext.mapData");
    if (args.Length() < 1)
        return throwNotEnoughArgumentsError(args.GetIsolate());
    CContext* imp = V8CContext::toNative(args.Holder());
    if (V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Int8Array*, source, V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Int8Array> int8Array = source;
        RefPtr<CData> cData = imp->mapData<Int8Array, TYPE_INT8>(int8Array);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint8Array*, source, V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Uint8Array> uint8Array = source;
        RefPtr<CData> cData = imp->mapData<Uint8Array, TYPE_UINT8>(uint8Array);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Int16Array*, source, V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Int16Array> int16Array = source;
        RefPtr<CData> cData = imp->mapData<Int16Array, TYPE_INT16>(int16Array);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint16Array*, source, V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Uint16Array> uint16Array = source;
        RefPtr<CData> cData = imp->mapData<Uint16Array, TYPE_UINT16>(uint16Array);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Int32Array*, source, V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Int32Array> int32Array = source;
        RefPtr<CData> cData = imp->mapData<Int32Array, TYPE_INT32>(int32Array);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint32Array*, source, V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Uint32Array> uint32Array = source;
        RefPtr<CData> cData = imp->mapData<Uint32Array, TYPE_UINT32>(uint32Array);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Float32Array*, source, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Float32Array> float32Array = source;
        RefPtr<CData> cData = imp->mapData<Float32Array, TYPE_FLOAT32>(float32Array);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Float64Array*, source, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Float64Array> float64Array = source;
        RefPtr<CData> cData = imp->mapData<Float64Array, TYPE_FLOAT64>(float64Array);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint8ClampedArray*, source, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Uint8ClampedArray> uint8ClampedArray = source;
        RefPtr<CData> cData = imp->mapData<Uint8ClampedArray, TYPE_UINT8_CLAMPED>(uint8ClampedArray);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
}

v8::Handle<v8::Value> V8CContext::cloneDataCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CContext.cloneData");
    if (args.Length() < 1)
        return throwNotEnoughArgumentsError(args.GetIsolate());
    CContext* imp = V8CContext::toNative(args.Holder());
    if (V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Int8Array*, source, V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Int8Array>(source);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint8Array*, source, V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Uint8Array>(source);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Int16Array*, source, V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Int16Array>(source);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint16Array*, source, V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Uint16Array>(source);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Int32Array*, source, V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Int32Array>(source);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint32Array*, source, V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Uint32Array>(source);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Float32Array*, source, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Float32Array>(source);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Float64Array*, source, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Float64Array>(source);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint8ClampedArray*, source, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Uint8ClampedArray>(source);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
}

v8::Handle<v8::Value> V8CContext::allocateDataCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CContext.allocateData");
    if (args.Length() < 1)
        return throwNotEnoughArgumentsError(args.GetIsolate());
    CContext* imp = V8CContext::toNative(args.Holder());
    if (V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Int8Array*, templ, V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Int8Array, char, TYPE_INT8>(templ, length);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint8Array*, templ, V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Uint8Array, unsigned char, TYPE_UINT8>(templ, length);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Int16Array*, templ, V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Int16Array, short, TYPE_INT16>(templ, length);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint16Array*, templ, V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Uint16Array, unsigned short, TYPE_UINT16>(templ, length);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Int32Array*, templ, V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Int32Array, int, TYPE_INT32>(templ, length);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint32Array*, templ, V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Uint32Array, unsigned, TYPE_UINT32>(templ, length);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Float32Array*, templ, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Float32Array, float, TYPE_FLOAT32>(templ, length);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Float64Array*, templ, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Float64Array, double, TYPE_FLOAT64>(templ, length);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    if (V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint8ClampedArray*, templ, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Uint8ClampedArray, unsigned char, TYPE_UINT8_CLAMPED>(templ, length);
        if (!cData)
            return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.get());
    }
    return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
}

v8::Handle<v8::Value> V8CContext::allocateData2Callback(const v8::Arguments& args)
{
    INC_STATS("DOM.CContext.allocateData2");
    if (args.Length() < 1)
        return throwNotEnoughArgumentsError(args.GetIsolate());
    CContext* imp = V8CContext::toNative(args.Holder());
    EXCEPTION_BLOCK(CData*, templ, V8CData::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8CData::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
    EXCEPTION_BLOCK(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
    RefPtr<CData> cData = imp->allocateData2(templ, length);
    if (!cData)
        return throwError(GeneralError, "Cannot create new CData object.", args.GetIsolate());
    return toV8(cData.get());
}

v8::Handle<v8::Value> V8CContext::canBeMappedCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CContext.canBeMapped");
    if (args.Length() < 1)
        return throwNotEnoughArgumentsError(args.GetIsolate());
    CContext* imp = V8CContext::toNative(args.Holder());
    if (V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Int8Array*, source, V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8Boolean(imp->canBeMapped<Int8Array>(source), args.GetIsolate());
    }
    if (V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint8Array*, source, V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8Boolean(imp->canBeMapped<Uint8Array>(source), args.GetIsolate());
    }
    if (V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Int16Array*, source, V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8Boolean(imp->canBeMapped<Int16Array>(source), args.GetIsolate());
    }
    if (V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint16Array*, source, V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8Boolean(imp->canBeMapped<Uint16Array>(source), args.GetIsolate());
    }
    if (V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Int32Array*, source, V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8Boolean(imp->canBeMapped<Int32Array>(source), args.GetIsolate());
    }
    if (V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint32Array*, source, V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8Boolean(imp->canBeMapped<Uint32Array>(source), args.GetIsolate());
    }
    if (V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Float32Array*, source, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8Boolean(imp->canBeMapped<Float32Array>(source), args.GetIsolate());
    }
    if (V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Float64Array*, source, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8Boolean(imp->canBeMapped<Float64Array>(source), args.GetIsolate());
    }
    if (V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint8ClampedArray*, source, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8Boolean(imp->canBeMapped<Uint8ClampedArray>(source), args.GetIsolate());
    }
    return v8Boolean(false, args.GetIsolate());
}

v8::Handle<v8::Value> V8CContext::writeToContext2DCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CContext.writeToContext2D");
    if (args.Length() < 4)
        return throwNotEnoughArgumentsError(args.GetIsolate());
    CContext* imp = V8CContext::toNative(args.Holder());
    if (V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(Int8Array*, source, V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)) ? V8Int8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        EXCEPTION_BLOCK(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Int8Array, char>(ctx, source, width, height);
        return v8Undefined();
    }
    if (V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(Uint8Array*, source, V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)) ? V8Uint8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        EXCEPTION_BLOCK(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Uint8Array, unsigned char>(ctx, source, width, height);
        return v8Undefined();
    }
    if (V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(Int16Array*, source, V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)) ? V8Int16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        EXCEPTION_BLOCK(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Int16Array, short>(ctx, source, width, height);
        return v8Undefined();
    }
    if (V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(Uint16Array*, source, V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)) ? V8Uint16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        EXCEPTION_BLOCK(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Uint16Array, unsigned short>(ctx, source, width, height);
        return v8Undefined();
    }
    if (V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(Int32Array*, source, V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)) ? V8Int32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        EXCEPTION_BLOCK(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Int32Array, int>(ctx, source, width, height);
        return v8Undefined();
    }
    if (V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(Uint32Array*, source, V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)) ? V8Uint32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        EXCEPTION_BLOCK(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Uint32Array, unsigned>(ctx, source, width, height);
        return v8Undefined();
    }
    if (V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(Float32Array*, source, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        EXCEPTION_BLOCK(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Float32Array, float>(ctx, source, width, height);
        return v8Undefined();
    }
    if (V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(Float64Array*, source, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        EXCEPTION_BLOCK(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Float64Array, double>(ctx, source, width, height);
        return v8Undefined();
    }
    if (V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(Uint8ClampedArray*, source, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        EXCEPTION_BLOCK(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Uint8ClampedArray, unsigned char>(ctx, source, width, height);
        return v8Undefined();
    }
    return v8Undefined();
}

v8::Handle<v8::Value> V8CContext::getAlignmentOffsetCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CContext.getAlignmentOffset");
    if (args.Length() < 1)
        return throwNotEnoughArgumentsError(args.GetIsolate());
    CContext* imp = V8CContext::toNative(args.Holder());
    if (V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Int8Array*, source, V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8UnsignedInteger(imp->getAlignmentOffset<Int8Array>(source), args.GetIsolate());
    }
    if (V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint8Array*, source, V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8UnsignedInteger(imp->getAlignmentOffset<Uint8Array>(source), args.GetIsolate());
    }
    if (V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Int16Array*, source, V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8UnsignedInteger(imp->getAlignmentOffset<Int16Array>(source), args.GetIsolate());
    }
    if (V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint16Array*, source, V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8UnsignedInteger(imp->getAlignmentOffset<Uint16Array>(source), args.GetIsolate());
    }
    if (V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Int32Array*, source, V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8UnsignedInteger(imp->getAlignmentOffset<Int32Array>(source), args.GetIsolate());
    }
    if (V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint32Array*, source, V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8UnsignedInteger(imp->getAlignmentOffset<Uint32Array>(source), args.GetIsolate());
    }
    if (V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Float32Array*, source, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8UnsignedInteger(imp->getAlignmentOffset<Float32Array>(source), args.GetIsolate());
    }
    if (V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Float64Array*, source, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8UnsignedInteger(imp->getAlignmentOffset<Float64Array>(source), args.GetIsolate());
    }
    if (V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint8ClampedArray*, source, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        return v8UnsignedInteger(imp->getAlignmentOffset<Uint8ClampedArray>(source), args.GetIsolate());
    }
    return throwError(GeneralError, "Cannot get alignment offset.", args.GetIsolate());
}

} // namespace WebCore
