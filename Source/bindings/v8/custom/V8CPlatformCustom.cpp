#include "config.h"
#include "V8CPlatform.h"

#include "core/rivertrail/CPlatform.h"
#include "bindings/v8/V8Binding.h"
#include "V8CContext.h"

namespace WebCore {

v8::Handle<v8::Value> V8CPlatform::createContextMethodCustom(const v8::Arguments& args)
{
    CPlatform* imp = V8CPlatform::toNative(args.Holder());
    RefPtr<CContext> cContext = imp->createContext();
    if (!cContext)
        return throwError(v8GeneralError, "Cannot create new CContext object.", args.GetIsolate());
	return toV8(cContext.get(), v8::Handle<v8::Object>(), args.GetIsolate());
}

v8::Handle<v8::Value> V8CPlatform::numberOfDevicesAttrGetterCustom(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    CPlatform* imp = V8CPlatform::toNative(info.Holder());
    int number = imp->openclUtil()->numberOfDevices();
    if (number == -1)
        return throwError(v8GeneralError, "Cannot get numberOfDevices.", info.GetIsolate());
    return v8UnsignedInteger(number, info.GetIsolate());
}

} // namespace WebCore
