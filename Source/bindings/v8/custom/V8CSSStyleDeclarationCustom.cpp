/*
 * Copyright (C) 2007-2011 Google Inc. All rights reserved.
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

#include "config.h"
#include "V8CSSStyleDeclaration.h"

#include "CSSPropertyNames.h"
#include "core/css/CSSParser.h"
#include "core/css/CSSPrimitiveValue.h"
#include "core/css/CSSStyleDeclaration.h"
#include "core/css/CSSValue.h"
#include "core/dom/EventTarget.h"
#include "core/page/RuntimeCSSEnabled.h"

#include "bindings/v8/V8Binding.h"

#include "wtf/ASCIICType.h"
#include "wtf/PassRefPtr.h"
#include "wtf/RefPtr.h"
#include "wtf/StdLibExtras.h"
#include "wtf/Vector.h"
#include "wtf/text/StringBuilder.h"
#include "wtf/text/StringConcatenate.h"

using namespace WTF;
using namespace std;

namespace WebCore {

// FIXME: Next two functions look lifted verbatim from JSCSSStyleDeclarationCustom. Please remove duplication.

// Check for a CSS prefix.
// Passed prefix is all lowercase.
// First character of the prefix within the property name may be upper or lowercase.
// Other characters in the prefix within the property name must be lowercase.
// The prefix within the property name must be followed by a capital letter.
static bool hasCSSPropertyNamePrefix(const String& propertyName, const char* prefix)
{
#ifndef NDEBUG
    ASSERT(*prefix);
    for (const char* p = prefix; *p; ++p)
        ASSERT(isASCIILower(*p));
    ASSERT(propertyName.length());
#endif

    if (toASCIILower(propertyName[0]) != prefix[0])
        return false;

    unsigned length = propertyName.length();
    for (unsigned i = 1; i < length; ++i) {
        if (!prefix[i])
            return isASCIIUpper(propertyName[i]);
        if (propertyName[i] != prefix[i])
            return false;
    }
    return false;
}

class CSSPropertyInfo {
public:
    CSSPropertyID propID;
    bool hadPixelOrPosPrefix;
};

// When getting properties on CSSStyleDeclarations, the name used from
// Javascript and the actual name of the property are not the same, so
// we have to do the following translation. The translation turns upper
// case characters into lower case characters and inserts dashes to
// separate words.
//
// Example: 'backgroundPositionY' -> 'background-position-y'
//
// Also, certain prefixes such as 'pos', 'css-' and 'pixel-' are stripped
// and the hadPixelOrPosPrefix out parameter is used to indicate whether or
// not the property name was prefixed with 'pos-' or 'pixel-'.
static CSSPropertyInfo* cssPropertyInfo(v8::Handle<v8::String> v8PropertyName)
{
    String propertyName = toWebCoreString(v8PropertyName);
    typedef HashMap<String, CSSPropertyInfo*> CSSPropertyInfoMap;
    DEFINE_STATIC_LOCAL(CSSPropertyInfoMap, map, ());
    CSSPropertyInfo* propInfo = map.get(propertyName);
    if (!propInfo) {
        unsigned length = propertyName.length();
        bool hadPixelOrPosPrefix = false;
        if (!length)
            return 0;

        StringBuilder builder;
        builder.reserveCapacity(length);

        unsigned i = 0;

        if (hasCSSPropertyNamePrefix(propertyName, "css"))
            i += 3;
        else if (hasCSSPropertyNamePrefix(propertyName, "pixel")) {
            i += 5;
            hadPixelOrPosPrefix = true;
        } else if (hasCSSPropertyNamePrefix(propertyName, "pos")) {
            i += 3;
            hadPixelOrPosPrefix = true;
        } else if (hasCSSPropertyNamePrefix(propertyName, "webkit"))
            builder.append('-');
        else if (isASCIIUpper(propertyName[0]))
            return 0;

        builder.append(toASCIILower(propertyName[i++]));

        for (; i < length; ++i) {
            UChar c = propertyName[i];
            if (!isASCIIUpper(c))
                builder.append(c);
            else {
                builder.append('-');
                builder.append(toASCIILower(c));
            }
        }

        String propName = builder.toString();
        CSSPropertyID propertyID = cssPropertyID(propName);
        if (propertyID && RuntimeCSSEnabled::isCSSPropertyEnabled(propertyID)) {
            propInfo = new CSSPropertyInfo();
            propInfo->hadPixelOrPosPrefix = hadPixelOrPosPrefix;
            propInfo->propID = propertyID;
            map.add(propertyName, propInfo);
        }
    }
    return propInfo;
}

void V8CSSStyleDeclaration::namedPropertyEnumerator(const v8::PropertyCallbackInfo<v8::Array>& info)
{
    typedef Vector<String, numCSSProperties - 1> PreAllocatedPropertyVector;
    DEFINE_STATIC_LOCAL(PreAllocatedPropertyVector, propertyNames, ());
    static unsigned propertyNamesLength = 0;

    if (propertyNames.isEmpty()) {
        for (int id = firstCSSProperty; id <= lastCSSProperty; ++id) {
            CSSPropertyID propertyId = static_cast<CSSPropertyID>(id);
            if (RuntimeCSSEnabled::isCSSPropertyEnabled(propertyId))
                propertyNames.append(getJSPropertyName(propertyId));
        }
        sort(propertyNames.begin(), propertyNames.end(), codePointCompareLessThan);
        propertyNamesLength = propertyNames.size();
    }

    v8::Handle<v8::Array> properties = v8::Array::New(propertyNamesLength);
    for (unsigned i = 0; i < propertyNamesLength; ++i) {
        String key = propertyNames.at(i);
        ASSERT(!key.isNull());
        properties->Set(v8Integer(i, info.GetIsolate()), v8String(key, info.GetIsolate()));
    }

    v8SetReturnValue(info, properties);
}

void V8CSSStyleDeclaration::namedPropertyQuery(v8::Local<v8::String> v8Name, const v8::PropertyCallbackInfo<v8::Integer>& info)
{
    // NOTE: cssPropertyInfo lookups incur several mallocs.
    // Successful lookups have the same cost the first time, but are cached.
    if (cssPropertyInfo(v8Name)) {
        v8SetReturnValueInt(info, 0);
        return;
    }
}

void V8CSSStyleDeclaration::namedPropertyGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    // First look for API defined attributes on the style declaration object.
    if (info.Holder()->HasRealNamedCallbackProperty(name))
        return;

    // Search the style declaration.
    CSSPropertyInfo* propInfo = cssPropertyInfo(name);

    // Do not handle non-property names.
    if (!propInfo)
        return;

    CSSStyleDeclaration* imp = V8CSSStyleDeclaration::toNative(info.Holder());
    RefPtr<CSSValue> cssValue = imp->getPropertyCSSValueInternal(static_cast<CSSPropertyID>(propInfo->propID));
    if (cssValue) {
        if (propInfo->hadPixelOrPosPrefix
            && cssValue->isPrimitiveValue()) {
            v8SetReturnValue(info, static_cast<CSSPrimitiveValue*>(
                cssValue.get())->getFloatValue(CSSPrimitiveValue::CSS_PX));
            return;
        }
        v8SetReturnValue(info, v8StringOrNull(cssValue->cssText(), info.GetIsolate()));
        return;
    }

    String result = imp->getPropertyValueInternal(static_cast<CSSPropertyID>(propInfo->propID));
    if (result.isNull())
        result = ""; // convert null to empty string.

    v8SetReturnValue(info, v8String(result, info.GetIsolate()));
}

void V8CSSStyleDeclaration::namedPropertySetter(v8::Local<v8::String> name, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    CSSStyleDeclaration* imp = V8CSSStyleDeclaration::toNative(info.Holder());
    CSSPropertyInfo* propInfo = cssPropertyInfo(name);
    if (!propInfo)
        return;

    String propertyValue = toWebCoreStringWithNullCheck(value);
    if (propInfo->hadPixelOrPosPrefix)
        propertyValue.append("px");

    ExceptionCode ec = 0;
    imp->setPropertyInternal(static_cast<CSSPropertyID>(propInfo->propID), propertyValue, false, ec);

    if (ec) {
        setDOMException(ec, info.GetIsolate());
        return;
    }

    v8SetReturnValue(info, value);
}

} // namespace WebCore
