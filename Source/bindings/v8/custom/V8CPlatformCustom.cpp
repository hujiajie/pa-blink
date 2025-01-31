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
#include "V8CPlatform.h"

#include "V8CContext.h"
#include "bindings/v8/V8Binding.h"
#include "core/rivertrail/CPlatform.h"

namespace WebCore {

void V8CPlatform::createContextMethodCustom(const v8::FunctionCallbackInfo<v8::Value>& args)
{
#if defined(WTF_OS_WINDOWS)
    if (!openclFlag) {
        throwError(v8SyntaxError, "Cannot call createContext when OpenCL is not loaded.", args.GetIsolate());
        return;
    }
#endif // WTF_OS_WINDOWS
    CPlatform* imp = V8CPlatform::toNative(args.Holder());
    RefPtr<CContext> cContext = imp->createContext();
    if (!cContext) {
        throwError(v8SyntaxError, "Cannot create new CContext object.", args.GetIsolate());
        return;
    }
    v8SetReturnValue(args, toV8(cContext.get(), v8::Handle<v8::Object>(), args.GetIsolate()));
}

void V8CPlatform::numberOfDevicesAttrGetterCustom(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    CPlatform* imp = V8CPlatform::toNative(info.Holder());
    int number = imp->openclUtil()->numberOfDevices();
    if (number == -1) {
        throwError(v8SyntaxError, "Cannot get numberOfDevices.", info.GetIsolate());
        return;
    }
    v8SetReturnValue(info, v8UnsignedInteger(number, info.GetIsolate()));
}

} // namespace WebCore
