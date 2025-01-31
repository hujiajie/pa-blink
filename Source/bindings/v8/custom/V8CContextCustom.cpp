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
#include "V8CContext.h"

#include "V8CanvasRenderingContext2D.h"
#include "V8CData.h"
#include "V8CKernel.h"
#include "V8CPlatform.h"
#include "V8Float32Array.h"
#include "V8Float64Array.h"
#include "V8Int16Array.h"
#include "V8Int32Array.h"
#include "V8Int8Array.h"
#include "V8Uint16Array.h"
#include "V8Uint32Array.h"
#include "V8Uint8Array.h"
#include "V8Uint8ClampedArray.h"
#include "bindings/v8/V8Binding.h"
#include "bindings/v8/V8DOMWrapper.h"
#include "core/html/canvas/CanvasRenderingContext2D.h"
#include "core/rivertrail/CContext.h"
#include "core/rivertrail/CData.h"
#include "core/rivertrail/CKernel.h"
#include "core/rivertrail/CPlatform.h"
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

void V8CContext::constructorCustom(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    if (args.Length() != 1) {
        throwError(v8SyntaxError, "CContext cannot be constructed because of wrong parameters.", args.GetIsolate());
        return;
    }
    if (!V8CPlatform::HasInstance(args[0], args.GetIsolate(), worldType(args.GetIsolate()))) {
        throwTypeError("CContext cannot be constructed because of wrong parameters.", args.GetIsolate());
        return;
    }

    CPlatform* parent = V8CPlatform::toNative(v8::Handle<v8::Object>::Cast(args[0]));
    RefPtr<CContext> cContext = CContext::create(parent);
    V8DOMWrapper::associateObjectWithWrapper(cContext.release(), &info, args.Holder(), args.GetIsolate(), WrapperConfiguration::Dependent);
    v8SetReturnValue(args, args.Holder());
}

void V8CContext::compileKernelMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    if (args.Length() < 2 || args.Length() > 3) {
        throwError(v8SyntaxError, "Cannot compile the kernel because of invalid number of arguments.", args.GetIsolate());
        return;
    }
#if defined(WTF_OS_WINDOWS)
    if (!openclFlag) {
        throwError(v8SyntaxError, "Cannot call compileKernel when OpenCL is not loaded.", args.GetIsolate());
        return;
    }
#endif // WTF_OS_WINDOWS
    CContext* imp = V8CContext::toNative(args.Holder());
    V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID(V8StringResource<>, source, MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined));
    V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID(V8StringResource<>, kernelName, MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined));
    V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID(V8StringResource<>, options, MAYBE_MISSING_PARAMETER(args, 2, DefaultIsNullString));
    RefPtr<CKernel> cKernel = imp->compileKernel(source, kernelName, options);
    if (!cKernel) {
        throwError(v8SyntaxError, "Cannot create new CKernel object.", args.GetIsolate());
        return;
    }
    v8SetReturnValue(args, toV8(cKernel.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
}

void V8CContext::mapDataMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    if (args.Length() != 1) {
        throwError(v8SyntaxError, "Cannot map data because of invalid number of arguments.", args.GetIsolate());
        return;
    }
#if defined(WTF_OS_WINDOWS)
    if (!openclFlag) {
        throwError(v8SyntaxError, "Cannot call mapData when OpenCL is not loaded.", args.GetIsolate());
        return;
    }
#endif // WTF_OS_WINDOWS
    CContext* imp = V8CContext::toNative(args.Holder());
    if (V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Int8Array*, source, V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Int8Array> int8Array = source;
        RefPtr<CData> cData = imp->mapData<Int8Array, ArrayBufferView::TypeInt8>(int8Array);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint8Array*, source, V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Uint8Array> uint8Array = source;
        RefPtr<CData> cData = imp->mapData<Uint8Array, ArrayBufferView::TypeUint8>(uint8Array);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Int16Array*, source, V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Int16Array> int16Array = source;
        RefPtr<CData> cData = imp->mapData<Int16Array, ArrayBufferView::TypeInt16>(int16Array);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint16Array*, source, V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Uint16Array> uint16Array = source;
        RefPtr<CData> cData = imp->mapData<Uint16Array, ArrayBufferView::TypeUint16>(uint16Array);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Int32Array*, source, V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Int32Array> int32Array = source;
        RefPtr<CData> cData = imp->mapData<Int32Array, ArrayBufferView::TypeInt32>(int32Array);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint32Array*, source, V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Uint32Array> uint32Array = source;
        RefPtr<CData> cData = imp->mapData<Uint32Array, ArrayBufferView::TypeUint32>(uint32Array);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Float32Array*, source, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Float32Array> float32Array = source;
        RefPtr<CData> cData = imp->mapData<Float32Array, ArrayBufferView::TypeFloat32>(float32Array);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Float64Array*, source, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Float64Array> float64Array = source;
        RefPtr<CData> cData = imp->mapData<Float64Array, ArrayBufferView::TypeFloat64>(float64Array);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint8ClampedArray*, source, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<Uint8ClampedArray> uint8ClampedArray = source;
        RefPtr<CData> cData = imp->mapData<Uint8ClampedArray, ArrayBufferView::TypeUint8Clamped>(uint8ClampedArray);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
}

void V8CContext::cloneDataMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    if (args.Length() != 1) {
        throwError(v8SyntaxError, "Cannot clone data because of invalid number of arguments.", args.GetIsolate());
        return;
    }
#if defined(WTF_OS_WINDOWS)
    if (!openclFlag) {
        throwError(v8SyntaxError, "Cannot call cloneData when OpenCL is not loaded.", args.GetIsolate());
        return;
    }
#endif // WIF_OS_WIN
    CContext* imp = V8CContext::toNative(args.Holder());
    if (V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Int8Array*, source, V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Int8Array>(source);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint8Array*, source, V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Uint8Array>(source);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Int16Array*, source, V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Int16Array>(source);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint16Array*, source, V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Uint16Array>(source);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Int32Array*, source, V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Int32Array>(source);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint32Array*, source, V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Uint32Array>(source);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Float32Array*, source, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Float32Array>(source);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Float64Array*, source, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Float64Array>(source);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint8ClampedArray*, source, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        RefPtr<CData> cData = imp->cloneData<Uint8ClampedArray>(source);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
}

void V8CContext::allocateDataMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    if (args.Length() < 1 || args.Length() > 2) {
        throwError(v8SyntaxError, "Cannot allocate data because of invalid number of arguments.", args.GetIsolate());
        return;
    }
#if defined(WTF_OS_WINDOWS)
    if (!openclFlag) {
        throwError(v8SyntaxError, "Cannot call allocateData when OpenCL is not loaded.", args.GetIsolate());
        return;
    }
#endif // WTF_OS_WINDOWS
    CContext* imp = V8CContext::toNative(args.Holder());
    if (V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Int8Array*, templ, V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Int8Array, char, ArrayBufferView::TypeInt8>(templ, length);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint8Array*, templ, V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Uint8Array, unsigned char, ArrayBufferView::TypeUint8>(templ, length);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Int16Array*, templ, V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Int16Array, short, ArrayBufferView::TypeInt16>(templ, length);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint16Array*, templ, V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Uint16Array, unsigned short, ArrayBufferView::TypeUint16>(templ, length);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Int32Array*, templ, V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Int32Array, int, ArrayBufferView::TypeInt32>(templ, length);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint32Array*, templ, V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Uint32Array, unsigned, ArrayBufferView::TypeUint32>(templ, length);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Float32Array*, templ, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Float32Array, float, ArrayBufferView::TypeFloat32>(templ, length);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Float64Array*, templ, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Float64Array, double, ArrayBufferView::TypeFloat64>(templ, length);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    if (V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint8ClampedArray*, templ, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
        RefPtr<CData> cData = imp->allocateData<Uint8ClampedArray, unsigned char, ArrayBufferView::TypeUint8Clamped>(templ, length);
        if (!cData) {
            throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
            return;
        }
        v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
        return;
    }
    throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
}

void V8CContext::allocateData2MethodCustom(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    if (args.Length() < 1 || args.Length() > 2) {
        throwError(v8SyntaxError, "Cannot allocate data because of invalid number of arguments.", args.GetIsolate());
        return;
    }
#if defined(WTF_OS_WINDOWS)
    if (!openclFlag) {
        throwError(v8SyntaxError, "Cannot call allocateData2 when OpenCL is not loaded.", args.GetIsolate());
        return;
    }
#endif // WTF_OS_WINDOWS
    CContext* imp = V8CContext::toNative(args.Holder());
    V8TRYCATCH_VOID(CData*, templ, V8CData::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8CData::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
    V8TRYCATCH_VOID(unsigned, length, toUInt32(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined)));
    RefPtr<CData> cData = imp->allocateData2(templ, length);
    if (!cData) {
        throwError(v8SyntaxError, "Cannot create new CData object.", args.GetIsolate());
        return;
    }
    v8SetReturnValue(args, toV8(cData.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
}

void V8CContext::canBeMappedMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    if (args.Length() != 1) {
        throwError(v8SyntaxError, "Cannot be mapped becaused of invalid number of arguments.", args.GetIsolate());
        return;
    }
#if defined(WTF_OS_WINDOWS)
    if (!openclFlag) {
        throwError(v8SyntaxError, "Cannot call canBeMapped when OpenCL is not loaded.", args.GetIsolate());
        return;
    }
#endif // WTF_OS_WINDOWS
    CContext* imp = V8CContext::toNative(args.Holder());
    if (V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Int8Array*, source, V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args, v8Boolean(imp->canBeMapped<Int8Array>(source), args.GetIsolate()));
        return;
    }
    if (V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint8Array*, source, V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args,  v8Boolean(imp->canBeMapped<Uint8Array>(source), args.GetIsolate()));
        return;
    }
    if (V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Int16Array*, source, V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args, v8Boolean(imp->canBeMapped<Int16Array>(source), args.GetIsolate()));
        return;
    }
    if (V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint16Array*, source, V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args, v8Boolean(imp->canBeMapped<Uint16Array>(source), args.GetIsolate()));
        return;
    }
    if (V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Int32Array*, source, V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args, v8Boolean(imp->canBeMapped<Int32Array>(source), args.GetIsolate()));
        return;
    }
    if (V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint32Array*, source, V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args, v8Boolean(imp->canBeMapped<Uint32Array>(source), args.GetIsolate()));
        return;
    }
    if (V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Float32Array*, source, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args, v8Boolean(imp->canBeMapped<Float32Array>(source), args.GetIsolate()));
        return;
    }
    if (V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Float64Array*, source, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args, v8Boolean(imp->canBeMapped<Float64Array>(source), args.GetIsolate()));
        return;
    }
    if (V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint8ClampedArray*, source, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args, v8Boolean(imp->canBeMapped<Uint8ClampedArray>(source), args.GetIsolate()));
        return;
    }
    v8SetReturnValue(args, v8Boolean(false, args.GetIsolate()));
}

void V8CContext::writeToContext2DMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    if (args.Length() != 4) {
        throwError(v8SyntaxError, "Cannot write because of invalid number of arguments.", args.GetIsolate());
        return;
    }
#if defined(WTF_OS_WINDOWS)
    if (!openclFlag) {
        throwError(v8SyntaxError, "Cannot call writeToContext2D when OpenCL is not loaded.", args.GetIsolate());
        return;
    }
#endif // WTF_OS_WINDOWS
    CContext* imp = V8CContext::toNative(args.Holder());
    if (V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(Int8Array*, source, V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        V8TRYCATCH_VOID(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Int8Array, char>(ctx, source, width, height);
        v8SetReturnValue(args, v8Undefined());
        return;
    }
    if (V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(Uint8Array*, source, V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        V8TRYCATCH_VOID(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Uint8Array, unsigned char>(ctx, source, width, height);
        v8SetReturnValue(args, v8Undefined());
        return;
    }
    if (V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(Int16Array*, source, V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        V8TRYCATCH_VOID(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Int16Array, short>(ctx, source, width, height);
        v8SetReturnValue(args, v8Undefined());
        return;
    }
    if (V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(Uint16Array*, source, V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        V8TRYCATCH_VOID(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Uint16Array, unsigned short>(ctx, source, width, height);
        v8SetReturnValue(args, v8Undefined());
        return;
    }
    if (V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(Int32Array*, source, V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        V8TRYCATCH_VOID(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Int32Array, int>(ctx, source, width, height);
        v8SetReturnValue(args, v8Undefined());
        return;
    }
    if (V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(Uint32Array*, source, V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        V8TRYCATCH_VOID(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Uint32Array, unsigned>(ctx, source, width, height);
        v8SetReturnValue(args, v8Undefined());
        return;
    }
    if (V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(Float32Array*, source, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        V8TRYCATCH_VOID(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Float32Array, float>(ctx, source, width, height);
        v8SetReturnValue(args, v8Undefined());
        return;
    }
    if (V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(Float64Array*, source, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        V8TRYCATCH_VOID(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Float64Array, double>(ctx, source, width, height);
        v8SetReturnValue(args, v8Undefined());
        return;
    }
    if (V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(CanvasRenderingContext2D*, ctx, V8CanvasRenderingContext2D::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8CanvasRenderingContext2D::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(Uint8ClampedArray*, source, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
        V8TRYCATCH_VOID(int, width, toInt32(MAYBE_MISSING_PARAMETER(args, 2, DefaultIsUndefined)));
        V8TRYCATCH_VOID(int, height, toInt32(MAYBE_MISSING_PARAMETER(args, 3, DefaultIsUndefined)));
        imp->writeToContext2D<Uint8ClampedArray, unsigned char>(ctx, source, width, height);
        v8SetReturnValue(args, v8Undefined());
        return;
    }
    v8SetReturnValue(args, v8Undefined());
}

void V8CContext::getAlignmentOffsetMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    if (args.Length() != 1) {
        throwError(v8SyntaxError, "Cannot get alignment offset because of invalid number of arguments.", args.GetIsolate());
        return;
    }
#if defined(WTF_OS_WINDOWS)
    if (!openclFlag) {
        throwError(v8SyntaxError, "Cannot call getAlignmentOffset when OpenCL is not loaded.", args.GetIsolate());
        return;
    }
#endif // WTF_OS_WINDOWS
    CContext* imp = V8CContext::toNative(args.Holder());
    if (V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Int8Array*, source, V8Int8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args, v8UnsignedInteger(imp->getAlignmentOffset<Int8Array>(source), args.GetIsolate()));
        return;
    }
    if (V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint8Array*, source, V8Uint8Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint8Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args, v8UnsignedInteger(imp->getAlignmentOffset<Uint8Array>(source), args.GetIsolate()));
        return;
    }
    if (V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Int16Array*, source, V8Int16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args, v8UnsignedInteger(imp->getAlignmentOffset<Int16Array>(source), args.GetIsolate()));
        return;
    }
    if (V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint16Array*, source, V8Uint16Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint16Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args, v8UnsignedInteger(imp->getAlignmentOffset<Uint16Array>(source), args.GetIsolate()));
        return;
    }
    if (V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Int32Array*, source, V8Int32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Int32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args, v8UnsignedInteger(imp->getAlignmentOffset<Int32Array>(source), args.GetIsolate()));
        return;
    }
    if (V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint32Array*, source, V8Uint32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args, v8UnsignedInteger(imp->getAlignmentOffset<Uint32Array>(source), args.GetIsolate()));
        return;
    }
    if (V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Float32Array*, source, V8Float32Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Float32Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args, v8UnsignedInteger(imp->getAlignmentOffset<Float32Array>(source), args.GetIsolate()));
        return;
    }
    if (V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Float64Array*, source, V8Float64Array::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Float64Array::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args, v8UnsignedInteger(imp->getAlignmentOffset<Float64Array>(source), args.GetIsolate()));
        return;
    }
    if (V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate()))) {
        V8TRYCATCH_VOID(Uint8ClampedArray*, source, V8Uint8ClampedArray::HasInstance(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8Uint8ClampedArray::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined))) : 0);
        v8SetReturnValue(args, v8UnsignedInteger(imp->getAlignmentOffset<Uint8ClampedArray>(source), args.GetIsolate()));
        return;
    }
    throwError(v8SyntaxError, "Cannot get alignment offset.", args.GetIsolate());
    return;
}

} // namespace WebCore
