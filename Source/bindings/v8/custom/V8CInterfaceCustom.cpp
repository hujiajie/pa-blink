#include "config.h"
#include "CInterface.h"
#include "V8CInterface.h"
#include "V8Binding.h"
#include "V8Proxy.h"

namespace WebCore {

v8::Handle<v8::Value> V8CInterface::constructorCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CInterface.Constructor");

    if (!args.IsConstructCall())
        return V8Proxy::throwTypeError("DOM object constructor cannot be called as a function.", args.GetIsolate());

    if (ConstructorMode::current() == ConstructorMode::WrapExistingObject)
        return args.Holder();

    RefPtr<CInterface> cInterface = CInterface::create();
    V8DOMWrapper::setDOMWrapper(args.Holder(), &info, cInterface.get());
    V8DOMWrapper::setJSWrapperForDOMObject(cInterface.release(), v8::Persistent<v8::Object>::New(args.Holder()));
    return args.Holder();;
}

} // namespace WebCore
