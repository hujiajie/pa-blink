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
#include "V8CKernel.h"

#include "core/rivertrail/CContext.h"
#include "core/rivertrail/CData.h"
#include "core/rivertrail/CKernel.h"
#include "bindings/v8/V8Binding.h"
#include "V8CContext.h"
#include "V8CData.h"
#include <limits>

namespace WebCore {

v8::Handle<v8::Value> V8CKernel::constructorCustom(const v8::Arguments& args)
{
    if (args.Length() != 1)
        return throwError(v8SyntaxError, "CKernel cannot be constructed because of wrong parameters.", args.GetIsolate());
    if (!V8CContext::HasInstance(args[0], args.GetIsolate(), worldType(args.GetIsolate())))
        return throwError(v8SyntaxError, "CKernel cannot be constructed because of wrong parameters.", args.GetIsolate());

    CContext* parent = V8CContext::toNative(v8::Handle<v8::Object>::Cast(args[0]));
    RefPtr<CKernel> cKernel = CKernel::create(parent);
    V8DOMWrapper::associateObjectWithWrapper(cKernel.release(), &info, args.Holder(), args.GetIsolate(), WrapperConfiguration::Dependent);
    return args.Holder();
}

v8::Handle<v8::Value> V8CKernel::setArgumentMethodCustom(const v8::Arguments& args)
{
    if (args.Length() != 2)
        return throwNotEnoughArgumentsError(args.GetIsolate());
    if (!openclFlag)
        return throwError(v8SyntaxError, "Cannot call setArgument when OpenCL is not loaded.", args.GetIsolate());
    CKernel* imp = V8CKernel::toNative(args.Holder());
    V8TRYCATCH(unsigned, number, toUInt32(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)));
    V8TRYCATCH(CData*, argument, V8CData::HasInstance(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined), args.GetIsolate(), worldType(args.GetIsolate())) ? V8CData::toNative(v8::Handle<v8::Object>::Cast(MAYBE_MISSING_PARAMETER(args, 1, DefaultIsUndefined))) : 0);
    return v8Boolean(imp->setArgument(number, argument), args.GetIsolate());
}

v8::Handle<v8::Value> V8CKernel::setScalarArgumentMethodCustom(const v8::Arguments& args)
{
    if (args.Length() != 4)
        return throwError(v8SyntaxError, "Cannot set scalar because of invalid number of arguments", args.GetIsolate());
    if (!openclFlag)
        return throwError(v8SyntaxError, "Cannot call setScalarArgument when OpenCL is not loaded.", args.GetIsolate());
    CKernel* imp = V8CKernel::toNative(args.Holder());
    V8TRYCATCH(unsigned, number, toUInt32(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)));
    
    if (!args[2]->IsBoolean())
        return throwTypeError(0, args.GetIsolate());
    bool isIntegerB = args[2]->BooleanValue();
    
    if (!args[3]->IsBoolean())
        return throwTypeError(0, args.GetIsolate());
    bool isHighPrecisionB = args[3]->BooleanValue();
    
    if (args[1]->IsInt32()) {
        int value = args[1]->Int32Value();
        return v8Boolean(imp->setScalarArgument<int>(number, value, isIntegerB, isHighPrecisionB), args.GetIsolate());
    }
    if (args[1]->IsUint32()) {
        unsigned value = args[1]->Uint32Value();
        return v8Boolean(imp->setScalarArgument<unsigned>(number, value, isIntegerB, isHighPrecisionB), args.GetIsolate());
    }
    if (args[1]->IsNumber()) {
        double value = args[1]->NumberValue();
        return v8Boolean(imp->setScalarArgument<double>(number, value, isIntegerB, isHighPrecisionB), args.GetIsolate());
    } 
    return throwTypeError(0, args.GetIsolate());
}

v8::Handle<v8::Value> V8CKernel::runMethodCustom(const v8::Arguments& args)
{
    if (args.Length() != 3)
        return throwError(v8SyntaxError, "Cannot run because of invalid number of arguments.", args.GetIsolate());
    if (!openclFlag)
        return throwError(v8SyntaxError, "Cannot call run when OpenCL is not loaded.", args.GetIsolate());
    CKernel* imp = V8CKernel::toNative(args.Holder());
    V8TRYCATCH(unsigned, rank, toUInt32(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)));
    
    if (args[1].IsEmpty() || !args[1]->IsArray())
        return throwTypeError(0, args.GetIsolate());
    v8::Handle<v8::Array> ashape = v8::Handle<v8::Array>::Cast(args[1]->ToObject());
    unsigned* shape = new unsigned[ashape->Length()];
    for(uint32_t i = 0; i < ashape->Length(); i++)
        shape[i] = ashape->Get(i)->Int32Value();
    
    unsigned* tile = 0;
    if (!isUndefinedOrNull(args[2])){
        if (args[2].IsEmpty() || !args[2]->IsArray())
            return throwTypeError(0, args.GetIsolate());
        v8::Handle<v8::Array> atile = v8::Handle<v8::Array>::Cast(args[2]->ToObject());   
        tile = new unsigned[atile->Length()];
        for(uint32_t i = 0; i < atile->Length(); i++)
            tile[i] = atile->Get(i)->Int32Value();
    }
        
    return v8UnsignedInteger(imp->run(rank, shape, tile), args.GetIsolate());
}

v8::Handle<v8::Value> V8CKernel::numberOfArgsAttrGetterCustom(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    if (!openclFlag)
        return throwError(v8SyntaxError, "Cannot get numberOfArgs when OpenCL is not loaded.", info.GetIsolate());
    CKernel* imp = V8CKernel::toNative(info.Holder());
    unsigned long number = imp->numberOfArgs();
    if (number == std::numeric_limits<unsigned long>::max())
        return throwError(v8SyntaxError, "Cannot get numberOfArgs.", info.GetIsolate());
    return v8UnsignedInteger(number, info.GetIsolate());
}

} // namespace WebCore
