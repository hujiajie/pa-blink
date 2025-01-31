/*
 * Copyright (C) 2004, 2008, 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Collabora Ltd.
 * Copyright (C) 2011 Peter Varga (pvarga@webkit.org), University of Szeged
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "core/platform/text/RegularExpression.h"

// FIXME: These seem like a layering violation, but converting the strings manually
// without v8String is difficult, and calling into v8 without V8RecursionScope will
// assert. Perhaps v8 basic utilities shouldn't be in bindings, or we should put
// RegularExpression as some kind of abstract interface that's implemented in bindings.
#include "bindings/v8/V8Binding.h"
#include "bindings/v8/V8PerIsolateData.h"
#include "bindings/v8/V8RecursionScope.h"

namespace WebCore {

RegularExpression::RegularExpression(const String& pattern, TextCaseSensitivity caseSensitivity, MultilineMode multilineMode)
{
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    v8::Local<v8::Context> context = V8PerIsolateData::from(isolate)->ensureRegexContext();
    v8::Context::Scope scope(context);

    unsigned flags = v8::RegExp::kNone;
    if (caseSensitivity == TextCaseInsensitive)
        flags |= v8::RegExp::kIgnoreCase;
    if (multilineMode == MultilineEnabled)
        flags |= v8::RegExp::kMultiline;

    v8::TryCatch tryCatch;
    v8::Local<v8::RegExp> regex = v8::RegExp::New(v8String(pattern, context->GetIsolate()), static_cast<v8::RegExp::Flags>(flags));

    // If the regex failed to compile we'll get an empty handle.
    if (!regex.IsEmpty())
        m_regex.set(isolate, regex);
}

int RegularExpression::match(const String& string, int startFrom, int* matchLength) const
{
    if (matchLength)
        *matchLength = 0;

    if (m_regex.isEmpty() || string.isNull())
         return -1;

    // v8 strings are limited to int.
    if (string.length() > INT_MAX)
         return -1;

    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    v8::Local<v8::Context> context = V8PerIsolateData::current()->ensureRegexContext();
    v8::Context::Scope scope(context);
    v8::TryCatch tryCatch;

    V8RecursionScope::MicrotaskSuppression microtaskScope;

    v8::Local<v8::Function> exec = m_regex.newLocal(isolate)->Get(v8::String::NewSymbol("exec")).As<v8::Function>();

    v8::Handle<v8::Value> argv[] = { v8String(string.substringSharingImpl(startFrom), context->GetIsolate()) };
    v8::Local<v8::Value> returnValue = exec->Call(m_regex.get(), 1, argv);

    // RegExp#exec returns null if there's no match, otherwise it returns an
    // Array of strings with the first being the whole match string and others
    // being subgroups. The Array also has some random properties tacked on like
    // "index" which is the offset of the match.
    //
    // https://developer.mozilla.org/en-US/docs/JavaScript/Reference/Global_Objects/RegExp/exec

    if (!returnValue->IsArray())
         return -1;

    v8::Local<v8::Array> result = returnValue.As<v8::Array>();
    int matchOffset = result->Get(v8::String::NewSymbol("index"))->ToInt32()->Value();

    if (matchLength) {
        v8::Local<v8::String> match = result->Get(0).As<v8::String>();
        *matchLength = match->Length();
    }

    return matchOffset + startFrom;
}

} // namespace WebCore
