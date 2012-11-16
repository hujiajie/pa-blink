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
#include "V8Proxy.h"
#include "V8Uint8ClampedArray.h"
#include <wtf/Float32Array.h>
#include <wtf/Float64Array.h>
#include <wtf/RefPtr.h>
#include <wtf/Uint8ClampedArray.h>

namespace WebCore {

v8::Handle<v8::Value> V8CContext::constructorCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CContext.Constructor");

    if (!args.IsConstructCall())
        return V8Proxy::throwTypeError("DOM object constructor cannot be called as a function.", args.GetIsolate());

    if (ConstructorMode::current() == ConstructorMode::WrapExistingObject)
        return args.Holder();

    if (args.Length() != 1)
        return V8Proxy::throwError(V8Proxy::GeneralError, "CContext cannot be constructed because of wrong parameters.", args.GetIsolate());
    if (!V8CPlatform::HasInstance(args[0]))
        return V8Proxy::throwError(V8Proxy::GeneralError, "CContext cannot be constructed because of wrong parameters.", args.GetIsolate());

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
        return V8Proxy::throwNotEnoughArgumentsError(args.GetIsolate());
    CContext* imp = V8CContext::toNative(args.Holder());
    STRING_TO_V8PARAMETER_EXCEPTION_BLOCK(V8Parameter<>, source, MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined));
    STRING_TO_V8PARAMETER_EXCEPTION_BLOCK(V8Parameter<>, kernelName, MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined));
    STRING_TO_V8PARAMETER_EXCEPTION_BLOCK(V8Parameter<>, options, MAYBE_MISSING_PARAMETER(args, 2, DefaultIsNullString));
    RefPtr<CKernel> cKernel = imp->compileKernel(source, kernelName, options);
    if (!cKernel)
        return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot create new CKernel object.", args.GetIsolate());
    return toV8(cKernel.release(), args.GetIsolate());
}

v8::Handle<v8::Value> V8CContext::mapDataCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CContext.mapData");
    if (args.Length() < 1)
        return V8Proxy::throwNotEnoughArgumentsError(args.GetIsolate());
    CContext* imp = V8CContext::toNative(args.Holder());
    if (V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Float32Array*, source, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Float32Array> float32Array = source;
        RefPtr<CData> cData = imp->mapData<Float32Array, TYPE_FLOAT32>(float32Array);
        if (!cData)
            return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.release(), args.GetIsolate());
    }
    if (V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Float64Array*, source, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Float64Array> float64Array = source;
        RefPtr<CData> cData = imp->mapData<Float64Array, TYPE_FLOAT64>(float64Array);
        if (!cData)
            return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.release(), args.GetIsolate());
    }
    if (V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint8ClampedArray*, source, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Uint8ClampedArray> uint8ClampedArray = source;
        RefPtr<CData> cData = imp->mapData<Uint8ClampedArray, TYPE_UINT8_CLAMPED>(uint8ClampedArray);
        if (!cData)
            return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.release(), args.GetIsolate());
    }
    return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot create new CData object.", args.GetIsolate());
}

v8::Handle<v8::Value> V8CContext::cloneDataCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CContext.cloneData");
    if (args.Length() < 1)
        return V8Proxy::throwNotEnoughArgumentsError(args.GetIsolate());
    CContext* imp = V8CContext::toNative(args.Holder());
    if (V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Float32Array*, source, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Float32Array>(source);
        if (!cData)
            return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.release(), args.GetIsolate());
    }
    if (V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Float64Array*, source, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Float64Array>(source);
        if (!cData)
            return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.release(), args.GetIsolate());
    }
    if (V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint8ClampedArray*, source, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Uint8ClampedArray>(source);
        if (!cData)
            return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.release(), args.GetIsolate());
    }
    return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot create new CData object.", args.GetIsolate());
}

v8::Handle<v8::Value> V8CContext::allocateDataCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CContext.allocateData");
    if (args.Length() < 1)
        return V8Proxy::throwNotEnoughArgumentsError(args.GetIsolate());
    CContext* imp = V8CContext::toNative(args.Holder());
    if (V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Float32Array*, templ, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Float32Array, float, TYPE_FLOAT32>(templ, length);
        if (!cData)
            return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.release(), args.GetIsolate());
    }
    if (V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Float64Array*, templ, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Float64Array, double, TYPE_FLOAT64>(templ, length);
        if (!cData)
            return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.release(), args.GetIsolate());
    }
    if (V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(Uint8ClampedArray*, templ, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Uint8ClampedArray, unsigned char, TYPE_UINT8_CLAMPED>(templ, length);
        if (!cData)
            return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot create new CData object.", args.GetIsolate());
        return toV8(cData.release(), args.GetIsolate());
    }
    return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot create new CData object.", args.GetIsolate());
}

v8::Handle<v8::Value> V8CContext::allocateData2Callback(const v8::Arguments& args)
{
    INC_STATS("DOM.CContext.allocateData2");
    if (args.Length() < 1)
        return V8Proxy::throwNotEnoughArgumentsError(args.GetIsolate());
    CContext* imp = V8CContext::toNative(args.Holder());
    EXCEPTION_BLOCK(CData*, templ, V8CData::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8CData::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
    EXCEPTION_BLOCK(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
    RefPtr<CData> cData = imp->allocateData2(templ, length);
    if (!cData)
        return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot create new CData object.", args.GetIsolate());
    return toV8(cData.release() , args.GetIsolate());
}

v8::Handle<v8::Value> V8CContext::canBeMappedCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CContext.canBeMapped");
    if (args.Length() < 1)
        return V8Proxy::throwNotEnoughArgumentsError(args.GetIsolate());
    CContext* imp = V8CContext::toNative(args.Holder());
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
        return V8Proxy::throwNotEnoughArgumentsError(args.GetIsolate());
    CContext* imp = V8CContext::toNative(args.Holder());
    if (V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(Float32Array*, source, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        EXCEPTION_BLOCK(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Float32Array>(ctx, source, width, height);
        return v8Undefined();
    }
    if (V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(Float64Array*, source, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        EXCEPTION_BLOCK(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Float64Array>(ctx, source, width, height);
        return v8Undefined();
    }
    if (V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) {
        EXCEPTION_BLOCK(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(Uint8ClampedArray*, source, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        EXCEPTION_BLOCK(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        EXCEPTION_BLOCK(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Uint8ClampedArray>(ctx, source, width, height);
        return v8Undefined();
    }
    return v8Undefined();
}

v8::Handle<v8::Value> V8CContext::getAlignmentOffsetCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CContext.getAlignmentOffset");
    if (args.Length() < 1)
        return V8Proxy::throwNotEnoughArgumentsError(args.GetIsolate());
    CContext* imp = V8CContext::toNative(args.Holder());
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
    return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot get alignment offset.", args.GetIsolate());
}

} // namespace WebCore
