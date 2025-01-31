/*
 * Copyright (C) 2009, 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef V8ArrayBufferViewCustom_h
#define V8ArrayBufferViewCustom_h

#include "core/dom/ExceptionCode.h"
#include "wtf/ArrayBuffer.h"

#include "V8ArrayBuffer.h"
#include "bindings/v8/V8Binding.h"
#include "bindings/v8/V8ObjectConstructor.h"
#include "bindings/v8/custom/V8ArrayBufferCustom.h"

namespace WebCore {

const char tooLargeSize[] = "Size is too large (or is negative).";
const char outOfRangeLengthAndOffset[] = "Index is out of range.";

// Copy the elements from the source array to the typed destination array.
// Returns true if it succeeded, otherwise returns false.
bool copyElements(v8::Handle<v8::Object> destArray, v8::Handle<v8::Object> srcArray, uint32_t length, uint32_t offset, v8::Isolate*);

template<class ArrayClass>
void wrapArrayBufferView(const v8::FunctionCallbackInfo<v8::Value>& args, WrapperTypeInfo* type, ArrayClass array, v8::ExternalArrayType arrayType, bool hasIndexer)
{
    // Transform the holder into a wrapper object for the array.
    ASSERT(!hasIndexer || static_cast<int32_t>(array.get()->length()) >= 0);
    if (hasIndexer)
        args.Holder()->SetIndexedPropertiesToExternalArrayData(array.get()->baseAddress(), arrayType, array.get()->length());
    v8::Handle<v8::Object> wrapper = args.Holder();
    V8DOMWrapper::associateObjectWithWrapper(array.release(), type, wrapper, args.GetIsolate(), WrapperConfiguration::Independent);
    args.GetReturnValue().Set(wrapper);
}

// Template function used by the ArrayBufferView*Constructor callbacks.
template<class ArrayClass, class ElementType>
void constructWebGLArrayWithArrayBufferArgument(const v8::FunctionCallbackInfo<v8::Value>& args, WrapperTypeInfo* type, v8::ExternalArrayType arrayType, bool hasIndexer)
{
    ArrayBuffer* buf = V8ArrayBuffer::toNative(args[0]->ToObject());
    if (!buf) {
        throwTypeError("Could not convert argument 0 to a ArrayBuffer", args.GetIsolate());
        return;
    }
    bool ok;
    uint32_t offset = 0;
    int argLen = args.Length();
    if (argLen > 1) {
        offset = toUInt32(args[1], ok);
        if (!ok) {
            throwTypeError("Could not convert argument 1 to a number", args.GetIsolate());
            return;
        }
    }
    uint32_t length = 0;
    if (argLen > 2) {
        length = toUInt32(args[2], ok);
        if (!ok) {
            throwTypeError("Could not convert argument 2 to a number", args.GetIsolate());
            return;
        }
    } else {
        if ((buf->byteLength() - offset) % sizeof(ElementType)) {
            throwError(v8RangeError, "ArrayBuffer length minus the byteOffset is not a multiple of the element size.", args.GetIsolate());
            return;
        }
        length = (buf->byteLength() - offset) / sizeof(ElementType);
    }

    if (static_cast<int32_t>(length) < 0) {
        throwError(v8RangeError, tooLargeSize, args.GetIsolate());
        return;
    }

    RefPtr<ArrayClass> array = ArrayClass::create(buf, offset, length);
    if (!array) {
        throwError(v8RangeError, tooLargeSize, args.GetIsolate());
        return;
    }

    wrapArrayBufferView(args, type, array, arrayType, hasIndexer);
}

// Template function used by the ArrayBufferView*Constructor callbacks.
template<class ArrayClass, class JavaScriptWrapperArrayType, class ElementType>
void constructWebGLArray(const v8::FunctionCallbackInfo<v8::Value>& args, WrapperTypeInfo* type, v8::ExternalArrayType arrayType)
{
    if (!args.IsConstructCall()) {
        throwTypeError("DOM object constructor cannot be called as a function.", args.GetIsolate());
        return;
    }

    if (ConstructorMode::current() == ConstructorMode::WrapExistingObject) {
        args.GetReturnValue().Set(args.Holder());
        return;
    }

    int argLen = args.Length();
    if (!argLen) {
        // This happens when we return a previously constructed
        // ArrayBufferView, e.g. from the call to <Type>Array.subset().
        // The V8DOMWrapper will set the internal pointer in the
        // created object. Unfortunately it doesn't look like it's
        // possible to distinguish between this case and that where
        // the user calls "new <Type>Array()" from JavaScript. We must
        // construct an empty view to avoid crashes when fetching the
        // length.
        RefPtr<ArrayClass> array = ArrayClass::create(0);
        // Do not call SetIndexedPropertiesToExternalArrayData on this
        // object. Not only is there no point from a performance
        // perspective, but doing so causes errors in the subset() case.
        wrapArrayBufferView(args, type, array, arrayType, false);
        return;
    }

    // Supported constructors:
    // WebGL<T>Array(n) where n is an integer:
    //   -- create an empty array of n elements
    // WebGL<T>Array(arr) where arr is an array:
    //   -- create a WebGL<T>Array containing the contents of "arr"
    // WebGL<T>Array(buf, offset, length)
    //   -- create a WebGL<T>Array pointing to the ArrayBuffer
    //      "buf", starting at the specified offset, for the given
    //      length

    if (args[0]->IsNull()) {
        // Invalid first argument
        throwTypeError(0, args.GetIsolate());
        return;
    }

    // See whether the first argument is a ArrayBuffer.
    if (V8ArrayBuffer::HasInstance(args[0], args.GetIsolate(), worldType(args.GetIsolate()))) {
        constructWebGLArrayWithArrayBufferArgument<ArrayClass, ElementType>(args, type, arrayType, true);
        return;
    }

    // See whether the first argument is the same type as impl. In that case,
    // we can simply memcpy data from source to impl.
    if (JavaScriptWrapperArrayType::HasInstance(args[0], args.GetIsolate(), worldType(args.GetIsolate()))) {
        ArrayClass* source = JavaScriptWrapperArrayType::toNative(args[0]->ToObject());
        uint32_t length = source->length();

        if (static_cast<int32_t>(length) < 0) {
            throwError(v8RangeError, tooLargeSize, args.GetIsolate());
            return;
        }

        RefPtr<ArrayClass> array = ArrayClass::createUninitialized(length);
        if (!array.get()) {
            throwError(v8RangeError, tooLargeSize, args.GetIsolate());
            return;
        }

        array->buffer()->setDeallocationObserver(V8ArrayBufferDeallocationObserver::instance());
        v8::V8::AdjustAmountOfExternalAllocatedMemory(array->byteLength());

        memcpy(array->baseAddress(), source->baseAddress(), length * sizeof(ElementType));

        wrapArrayBufferView(args, type, array, arrayType, true);
        return;
    }

    uint32_t len = 0;
    v8::Handle<v8::Object> srcArray;
    bool doInstantiation = false;

    if (args[0]->IsObject()) {
        srcArray = args[0]->ToObject();
        if (srcArray.IsEmpty()) {
            throwTypeError("Could not convert argument 0 to an array", args.GetIsolate());
            return;
        }
        v8::Local<v8::Value> val = srcArray->Get(v8::String::NewSymbol("length"));
        if (val.IsEmpty()) {
            // Exception thrown during fetch of length property.
            return;
        }
        len = toUInt32(val);
        doInstantiation = true;
    } else {
        bool ok = false;
        int32_t tempLength = toInt32(args[0], ok); // NaN/+inf/-inf returns 0, this is intended by WebIDL
        if (ok && tempLength >= 0) {
            len = static_cast<uint32_t>(tempLength);
            doInstantiation = true;
        }
    }

    if (static_cast<int32_t>(len) < 0) {
        throwError(v8RangeError, tooLargeSize, args.GetIsolate());
        return;
    }

    RefPtr<ArrayClass> array;
    if (doInstantiation) {
        if (srcArray.IsEmpty())
            array = ArrayClass::create(len);
        else
            array = ArrayClass::createUninitialized(len);
    }

    if (!array.get()) {
        throwError(v8RangeError, tooLargeSize, args.GetIsolate());
        return;
    }

    if (doInstantiation) {
        array->buffer()->setDeallocationObserver(V8ArrayBufferDeallocationObserver::instance());
        v8::V8::AdjustAmountOfExternalAllocatedMemory(array->byteLength());
    }


    // Transform the holder into a wrapper object for the array.
    args.Holder()->SetIndexedPropertiesToExternalArrayData(array.get()->baseAddress(), arrayType, array.get()->length());

    if (!srcArray.IsEmpty()) {
        bool copied = copyElements(args.Holder(), srcArray, len, 0, args.GetIsolate());
        if (!copied) {
            for (unsigned i = 0; i < len; i++) {
                v8::Local<v8::Value> val = srcArray->Get(i);
                if (val.IsEmpty()) {
                    // Exception thrown during fetch.
                    return;
                }
                array->set(i, val->NumberValue());
            }
        }
    }

    v8::Handle<v8::Object> wrapper = args.Holder();
    V8DOMWrapper::associateObjectWithWrapper(array.release(), type, wrapper, args.GetIsolate(), WrapperConfiguration::Independent);
    args.GetReturnValue().Set(wrapper);
}

template <class CPlusPlusArrayType, class JavaScriptWrapperArrayType>
void setWebGLArrayHelper(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    if (args.Length() < 1) {
        throwNotEnoughArgumentsError(args.GetIsolate());
        return;
    }

    CPlusPlusArrayType* impl = JavaScriptWrapperArrayType::toNative(args.Holder());

    if (JavaScriptWrapperArrayType::HasInstance(args[0], args.GetIsolate(), worldType(args.GetIsolate()))) {
        // void set(in WebGL<T>Array array, [Optional] in unsigned long offset);
        CPlusPlusArrayType* src = JavaScriptWrapperArrayType::toNative(args[0]->ToObject());
        uint32_t offset = 0;
        if (args.Length() == 2)
            offset = toUInt32(args[1]);
        if (!impl->set(src, offset)) {
            throwError(v8RangeError, outOfRangeLengthAndOffset, args.GetIsolate());
            return;
        }
        return;
    }

    if (args[0]->IsObject()) {
        // void set(in sequence<long> array, [Optional] in unsigned long offset);
        v8::Local<v8::Object> array = args[0]->ToObject();
        uint32_t offset = 0;
        if (args.Length() == 2)
            offset = toUInt32(args[1]);
        uint32_t length = toUInt32(array->Get(v8::String::NewSymbol("length")));
        if (!impl->checkInboundData(offset, length)) {
            throwError(v8RangeError, outOfRangeLengthAndOffset, args.GetIsolate());
            return;
        }
        bool copied = copyElements(args.Holder(), array, length, offset, args.GetIsolate());
        if (!copied) {
            for (uint32_t i = 0; i < length; i++)
                impl->set(offset + i, array->Get(i)->NumberValue());
        }
        return;
    }

    throwTypeError("Invalid argument", args.GetIsolate());
}

}

#endif // V8ArrayBufferViewCustom_h
