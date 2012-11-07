#include "config.h"
#include "V8CData.h"

#include "CContext.h"
#include "CData.h"
#include "OCLdebug.h"//FIX ME: may better to remove TYPE_FLOAT64 from this file
#include "V8Binding.h"
#include "V8CContext.h"
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

v8::Handle<v8::Value> V8CData::constructorCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CData.Constructor");

    if (!args.IsConstructCall())
        return V8Proxy::throwTypeError("DOM object constructor cannot be called as a function.", args.GetIsolate());

    if (ConstructorMode::current() == ConstructorMode::WrapExistingObject)
        return args.Holder();

    if (args.Length() != 1)
        return V8Proxy::throwError(V8Proxy::GeneralError, "CData cannot be constructed because of wrong parameters.", args.GetIsolate());
    if (!V8CContext::HasInstance(args[0]))
        return V8Proxy::throwError(V8Proxy::GeneralError, "CData cannot be constructed because of wrong parameters.", args.GetIsolate());

    CContext* parent = V8CContext::toNative(v8::Handle<v8::Object>::Cast(args[0]));
    RefPtr<CData> cData = CData::create(parent);
    V8DOMWrapper::setDOMWrapper(args.Holder(), &info, cData.get());
    V8DOMWrapper::setJSWrapperForDOMObject(cData.release(), v8::Persistent<v8::Object>::New(args.Holder()));
    return args.Holder();
}

v8::Handle<v8::Value> V8CData::getValueCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CData.getValue");
    CData* imp = V8CData::toNative(args.Holder());
    switch (imp->getType()) {
    case TYPE_FLOAT32:
		{
        Float32Array* theArray = imp->getValueFloat32Array();
        if (!theArray)
            return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray, args.GetIsolate());
        break;
		}
    case TYPE_FLOAT64:
		{
        Float64Array* theArray = imp->getValueFloat64Array();
        if (!theArray)
            return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray, args.GetIsolate());
        break;
		}
    case TYPE_UINT8_CLAMPED:
		{
        Uint8ClampedArray* theArray = imp->getValueUint8ClampedArray();
        if (!theArray)
            return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray, args.GetIsolate());
        break;
		}
    default:
		{
        return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot access typed array.", args.GetIsolate());
        break;
		}
    }
}

v8::Handle<v8::Value> V8CData::writeToCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CData.writeTo");
    if (args.Length() < 1)
        return V8Proxy::throwNotEnoughArgumentsError(args.GetIsolate());
    CData* imp = V8CData::toNative(args.Holder());
    switch (imp->getType()) {
    case TYPE_FLOAT32:
		{
        EXCEPTION_BLOCK(Float32Array*, dest, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        imp->writeToFloat32Array(dest);
        return v8Undefined();
        break;
		}
    case TYPE_FLOAT64:
		{
        EXCEPTION_BLOCK(Float64Array*, dest, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        imp->writeToFloat64Array(dest);
        return v8Undefined();
        break;
		}
    case TYPE_UINT8_CLAMPED:
		{
        EXCEPTION_BLOCK(Uint8ClampedArray*, dest, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        imp->writeToUint8ClampedArray(dest);
        return v8Undefined();
        break;
		}
    default:
		{
        return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot access typed array.", args.GetIsolate());
        break;
		}
    }
}

} // namespace WebCore
