#include "config.h"
#include "V8CKernel.h"

#include "CContext.h"
#include "CKernel.h"
#include "V8Binding.h"
#include "V8CContext.h"
#include "V8Proxy.h"
#include <limits>

namespace WebCore {

v8::Handle<v8::Value> V8CKernel::constructorCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CKernel.Constructor");

    if (!args.IsConstructCall())
        return V8Proxy::throwTypeError("DOM object constructor cannot be called as a function.", args.GetIsolate());

    if (ConstructorMode::current() == ConstructorMode::WrapExistingObject)
        return args.Holder();

    if (args.Length() != 1)
        return V8Proxy::throwError(V8Proxy::GeneralError, "CKernel cannot be constructed because of wrong parameters.", args.GetIsolate());
    if (!V8CContext::HasInstance(args[0]))
        return V8Proxy::throwError(V8Proxy::GeneralError, "CKernel cannot be constructed because of wrong parameters.", args.GetIsolate());

    CContext* parent = V8CContext::toNative(v8::Handle<v8::Object>::Cast(args[0]));
    RefPtr<CKernel> cKernel = CKernel::create(parent);
    V8DOMWrapper::setDOMWrapper(args.Holder(), &info, cKernel.get());
    V8DOMWrapper::setJSWrapperForDOMObject(cKernel.release(), v8::Persistent<v8::Object>::New(args.Holder()));
    return args.Holder();
}

v8::Handle<v8::Value> V8CKernel::setScalarArgumentCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CKernel.setScalarArgument");
    if (args.Length() < 4)
        return V8Proxy::throwNotEnoughArgumentsError(args.GetIsolate());
    CKernel* imp = V8CKernel::toNative(args.Holder());
    EXCEPTION_BLOCK(unsigned, number, toUInt32(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)));
    
    if (!args[2]->IsBoolean())
        return V8Proxy::throwTypeError(0, args.GetIsolate());
    bool isIntegerB = args[2]->BooleanValue();
    
    if (!args[3]->IsBoolean())
        return V8Proxy::throwTypeError(0, args.GetIsolate());
    bool isHighPrecisionB = args[3]->BooleanValue();
    
    if (args[1]->IsInt32()) {
        int value = args[1]->Int32Value();
        return v8Boolean(imp->setScalarArgument<int>(number, value, isIntegerB, isHighPrecisionB), args.GetIsolate());
    } else if (args[1]->IsNumber()) {
        double value = args[1]->NumberValue();
        return v8Boolean(imp->setScalarArgument<double>(number, value, isIntegerB, isHighPrecisionB), args.GetIsolate());
    } else 
        return V8Proxy::throwTypeError(0, args.GetIsolate());
}

v8::Handle<v8::Value> V8CKernel::runCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CKernel.run");
    if (args.Length() < 2)
        return V8Proxy::throwNotEnoughArgumentsError(args.GetIsolate());
    CKernel* imp = V8CKernel::toNative(args.Holder());
    EXCEPTION_BLOCK(unsigned, rank, toUInt32(MAYBE_MISSING_PARAMETER(args, 0, DefaultIsUndefined)));
    
    if (args[1].IsEmpty() || !args[1]->IsArray())
        return V8Proxy::throwTypeError(0, args.GetIsolate());
    v8::Handle<v8::Array> ashape = v8::Handle<v8::Array>::Cast(args[1]->ToObject());
    unsigned int* shape = new unsigned int[ashape->Length()];
    for(uint32_t i=0; i<ashape->Length(); i++)
        shape[i] = ashape->Get(i)->Int32Value();
    
    unsigned int* tile = NULL;
    if (!isUndefinedOrNull(args[2])){
        if (args[2].IsEmpty() || !args[2]->IsArray())
            return V8Proxy::throwTypeError(0, args.GetIsolate());
        v8::Handle<v8::Array> atile = v8::Handle<v8::Array>::Cast(args[2]->ToObject());   
        tile = new unsigned int[atile->Length()];
        for(uint32_t i=0; i<atile->Length(); i++)
            tile[i] = atile->Get(i)->Int32Value();
    }
        
    return v8UnsignedInteger(imp->run(rank, shape, tile), args.GetIsolate());
}

v8::Handle<v8::Value> V8CKernel::numberOfArgsAccessorGetter(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    INC_STATS("DOM.CKernel.numberOfArgs._get");
    CKernel* imp = V8CKernel::toNative(info.Holder());
    unsigned long number = imp->numberOfArgs();
    if (number == std::numeric_limits<unsigned long>::max())
        return V8Proxy::throwError(V8Proxy::GeneralError, "Cannot get numberOfArgs.", info.GetIsolate());
    return v8UnsignedInteger(number, info.GetIsolate());
}

} // namespace WebCore
