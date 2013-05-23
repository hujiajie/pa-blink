#include "config.h"
#include "V8CInterface.h"

#include "core/rivertrail/CInterface.h"
#include "bindings/v8/V8Binding.h"
#include "V8CPlatform.h"

namespace WebCore {

v8::Handle<v8::Value> V8CInterface::constructorCustom(const v8::Arguments& args)
{
    RefPtr<CInterface> cInterface = CInterface::create();
    V8DOMWrapper::associateObjectWithWrapper(cInterface.release(), &info, args.Holder(), args.GetIsolate(), WrapperConfiguration::Dependent);
    return args.Holder();;
}

v8::Handle<v8::Value> V8CInterface::getPlatformMethodCustom(const v8::Arguments& args)
{
    CInterface* imp = V8CInterface::toNative(args.Holder());
    RefPtr<CPlatform> cPlatform = imp->getPlatform();
    if (!cPlatform)
        return throwError(v8GeneralError, "Cannot create new CPlatform object.", args.GetIsolate());
    return toV8(cPlatform.get(), v8::Handle<v8::Object>(), args.GetIsolate());
}

} // namespace WebCore
