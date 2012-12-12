#include "config.h"
#include "V8CData.h"

#include "CContext.h"
#include "CData.h"
#include "OCLdebug.h" // FIXME: It may be better to remove TYPE_FLOAT64 from this file
#include "V8Binding.h"
#include "V8CContext.h"
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

v8::Handle<v8::Value> V8CData::constructorCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CData.Constructor");

    if (!args.IsConstructCall())
        return throwTypeError("DOM object constructor cannot be called as a function.", args.GetIsolate());

    if (ConstructorMode::current() == ConstructorMode::WrapExistingObject)
        return args.Holder();

    if (args.Length() != 1)
        return throwError(GeneralError, "CData cannot be constructed because of wrong parameters.", args.GetIsolate());
    if (!V8CContext::HasInstance(args[0]))
        return throwError(GeneralError, "CData cannot be constructed because of wrong parameters.", args.GetIsolate());

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
    case TYPE_INT8:
		{
        Int8Array* theArray = imp->getValue<Int8Array>();
        if (!theArray)
            return throwError(GeneralError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray);
        break;
		}
    case TYPE_UINT8:
		{
        Uint8Array* theArray = imp->getValue<Uint8Array>();
        if (!theArray)
            return throwError(GeneralError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray);
        break;
		}
    case TYPE_INT16:
		{
        Int16Array* theArray = imp->getValue<Int16Array>();
        if (!theArray)
            return throwError(GeneralError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray);
        break;
		}
    case TYPE_UINT16:
		{
        Uint16Array* theArray = imp->getValue<Uint16Array>();
        if (!theArray)
            return throwError(GeneralError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray);
        break;
		}
    case TYPE_INT32:
		{
        Int32Array* theArray = imp->getValue<Int32Array>();
        if (!theArray)
            return throwError(GeneralError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray);
        break;
		}
    case TYPE_UINT32:
		{
        Uint32Array* theArray = imp->getValue<Uint32Array>();
        if (!theArray)
            return throwError(GeneralError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray);
        break;
		}
    case TYPE_FLOAT32:
        {
        Float32Array* theArray = imp->getValue<Float32Array>();
        if (!theArray)
            return throwError(GeneralError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray);
        break;
        }
    case TYPE_FLOAT64:
        {
        Float64Array* theArray = imp->getValue<Float64Array>();
        if (!theArray)
            return throwError(GeneralError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray);
        break;
        }
    case TYPE_UINT8_CLAMPED:
        {
        Uint8ClampedArray* theArray = imp->getValue<Uint8ClampedArray>();
        if (!theArray)
            return throwError(GeneralError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray);
        break;
        }
    default:
        {
        return throwError(GeneralError, "Cannot access typed array.", args.GetIsolate());
        break;
        }
    }
}

v8::Handle<v8::Value> V8CData::writeToCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CData.writeTo");
    if (args.Length() < 1)
        return throwNotEnoughArgumentsError(args.GetIsolate());
    CData* imp = V8CData::toNative(args.Holder());
    switch (imp->getType()) {
    case TYPE_INT8:
		{
        EXCEPTION_BLOCK(Int8Array*, dest, V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        imp->writeTo<Int8Array>(dest);
        return v8Undefined();
        break;
		}
    case TYPE_UINT8:
		{
        EXCEPTION_BLOCK(Uint8Array*, dest, V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        imp->writeTo<Uint8Array>(dest);
        return v8Undefined();
        break;
		}
    case TYPE_INT16:
		{
        EXCEPTION_BLOCK(Int16Array*, dest, V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        imp->writeTo<Int16Array>(dest);
        return v8Undefined();
        break;
		}
    case TYPE_UINT16:
		{
        EXCEPTION_BLOCK(Uint16Array*, dest, V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        imp->writeTo<Uint16Array>(dest);
        return v8Undefined();
        break;
		}
    case TYPE_INT32:
		{
        EXCEPTION_BLOCK(Int32Array*, dest, V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Int32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        imp->writeTo<Int32Array>(dest);
        return v8Undefined();
        break;
		}
    case TYPE_UINT32:
		{
        EXCEPTION_BLOCK(Uint32Array*, dest, V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        imp->writeTo<Uint32Array>(dest);
        return v8Undefined();
        break;
		}
    case TYPE_FLOAT32:
        {
        EXCEPTION_BLOCK(Float32Array*, dest, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        imp->writeTo<Float32Array>(dest);
        return v8Undefined();
        break;
        }
    case TYPE_FLOAT64:
        {
        EXCEPTION_BLOCK(Float64Array*, dest, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        imp->writeTo<Float64Array>(dest);
        return v8Undefined();
        break;
        }
    case TYPE_UINT8_CLAMPED:
        {
        EXCEPTION_BLOCK(Uint8ClampedArray*, dest, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        imp->writeTo<Uint8ClampedArray>(dest);
        return v8Undefined();
        break;
        }
    default:
        {
        return throwError(GeneralError, "Cannot access typed array.", args.GetIsolate());
        break;
        }
    }
}

} // namespace WebCore
