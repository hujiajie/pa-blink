/*
 * Copyright (c) 2013, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "config.h"
#include "V8CData.h"

#include "core/rivertrail/CContext.h"
#include "core/rivertrail/CData.h"
#include "core/rivertrail/OCLdebug.h"
#include "bindings/v8/V8Binding.h"
#include "bindings/v8/V8DOMWrapper.h"
#include "V8CContext.h"
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

v8::Handle<v8::Value> V8CData::constructorCustom(const v8::Arguments& args)
{
    if (args.Length() != 1)
        return throwError(v8SyntaxError, "CData cannot be constructed because of wrong parameters.", args.GetIsolate());
    if (!V8CContext::HasInstance(args[0], args.GetIsolate(), worldType(args.GetIsolate())))
        return throwError(v8SyntaxError, "CData cannot be constructed because of wrong parameters.", args.GetIsolate());

    CContext* parent = V8CContext::toNative(v8::Handle<v8::Object>::Cast(args[0]));
    RefPtr<CData> cData = CData::create(parent);
    V8DOMWrapper::associateObjectWithWrapper(cData.release(), &info, args.Holder(), args.GetIsolate(), WrapperConfiguration::Dependent);
    return args.Holder();
}

v8::Handle<v8::Value> V8CData::getValueMethodCustom(const v8::Arguments& args)
{
   if (!openclFlag)
       return throwError(v8SyntaxError, "Cannot call getValue when OpenCL is not loaded.", args.GetIsolate());
    CData* imp = V8CData::toNative(args.Holder());
    switch (imp->getType()) {
    case ArrayBufferView::TypeInt8: {
        Int8Array* theArray = imp->getValue<Int8Array>();
        if (!theArray)
            return throwError(v8SyntaxError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray, v8::Handle<v8::Object>(), args.GetIsolate());
        break;
    }
    case ArrayBufferView::TypeUint8: {
        Uint8Array* theArray = imp->getValue<Uint8Array>();
        if (!theArray)
            return throwError(v8SyntaxError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray, v8::Handle<v8::Object>(), args.GetIsolate());
        break;
    }
    case ArrayBufferView::TypeInt16: {
        Int16Array* theArray = imp->getValue<Int16Array>();
        if (!theArray)
            return throwError(v8SyntaxError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray, v8::Handle<v8::Object>(), args.GetIsolate());
        break;
    }
    case ArrayBufferView::TypeUint16: {
        Uint16Array* theArray = imp->getValue<Uint16Array>();
        if (!theArray)
            return throwError(v8SyntaxError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray, v8::Handle<v8::Object>(), args.GetIsolate());
        break;
    }
    case ArrayBufferView::TypeInt32: {
        Int32Array* theArray = imp->getValue<Int32Array>();
        if (!theArray)
            return throwError(v8SyntaxError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray, v8::Handle<v8::Object>(), args.GetIsolate());
        break;
    }
    case ArrayBufferView::TypeUint32: {
        Uint32Array* theArray = imp->getValue<Uint32Array>();
        if (!theArray)
            return throwError(v8SyntaxError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray, v8::Handle<v8::Object>(), args.GetIsolate());
        break;
    }
    case ArrayBufferView::TypeFloat32: {
        Float32Array* theArray = imp->getValue<Float32Array>();
        if (!theArray)
            return throwError(v8SyntaxError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray, v8::Handle<v8::Object>(), args.GetIsolate());
        break;
    }
    case ArrayBufferView::TypeFloat64: {
        Float64Array* theArray = imp->getValue<Float64Array>();
        if (!theArray)
            return throwError(v8SyntaxError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray, v8::Handle<v8::Object>(), args.GetIsolate());
        break;
    }
    case ArrayBufferView::TypeUint8Clamped: {
        Uint8ClampedArray* theArray = imp->getValue<Uint8ClampedArray>();
        if (!theArray)
            return throwError(v8SyntaxError, "Cannot access typed array.", args.GetIsolate());
        return toV8(theArray, v8::Handle<v8::Object>(), args.GetIsolate());
        break;
    }
    default: {
        return throwError(v8SyntaxError, "Cannot access typed array.", args.GetIsolate());
        break;
    }
    }
}

v8::Handle<v8::Value> V8CData::writeToMethodCustom(const v8::Arguments& args)
{
    if (args.Length() != 1)
        return throwError(v8SyntaxError, "Cannot write because of invalid number of arguments.", args.GetIsolate());
    if (!openclFlag)
        return throwError(v8SyntaxError, "Cannot call writeTo when OpenCL is not loaded.", args.GetIsolate());
    CData* imp = V8CData::toNative(args.Holder());
    if (V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH(Uint8ClampedArray*, dest, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        imp->writeTo<Uint8ClampedArray>(dest);
        return v8Undefined();
    }
    return v8Undefined();
}

} // namespace WebCore
