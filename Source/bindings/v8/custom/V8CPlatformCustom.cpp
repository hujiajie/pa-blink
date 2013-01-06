#include "config.h"
#include "V8CPlatform.h"

#include "CPlatform.h"
#include "V8Binding.h"
#include "V8CContext.h"

namespace WebCore {

v8::Handle<v8::Value> V8CPlatform::createContextCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.CPlatform.createContext");
    CPlatform* imp = V8CPlatform::toNative(args.Holder());
    RefPtr<CContext> cContext = imp->createContext();
    if (!cContext)
        return throwError(GeneralError, "Cannot create new CContext object.", args.GetIsolate());
	return toV8(cContext.get());
}

v8::Handle<v8::Value> V8CPlatform::numberOfDevicesAccessorGetter(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    INC_STATS("DOM.CPlatform.numberOfDevices._get");
    CPlatform* imp = V8CPlatform::toNative(info.Holder());
    int number = imp->openclUtil()->numberOfDevices();
    if (number == -1)
        return throwError(GeneralError, "Cannot get numberOfDevices.", info.GetIsolate());
    return v8UnsignedInteger(number, info.GetIsolate());
}

} // namespace WebCore
