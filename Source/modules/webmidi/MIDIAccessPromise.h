/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
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

#ifndef MIDIAccessPromise_h
#define MIDIAccessPromise_h

#include "bindings/v8/ScriptWrappable.h"
#include "core/dom/ActiveDOMObject.h"
#include "modules/webmidi/MIDIOptions.h"
#include "wtf/OwnPtr.h"
#include "wtf/PassRefPtr.h"
#include "wtf/RefCounted.h"

namespace WebCore {

class DOMError;
class MIDIAccess;
class MIDIErrorCallback;
class MIDISuccessCallback;
class ScriptExecutionContext;

struct MIDIOptions;

class MIDIAccessPromise : public RefCounted<MIDIAccessPromise>, public ScriptWrappable, public ActiveDOMObject {
public:
    static PassRefPtr<MIDIAccessPromise> create(ScriptExecutionContext*, const Dictionary&);
    virtual ~MIDIAccessPromise();

    void then(PassRefPtr<MIDISuccessCallback>, PassRefPtr<MIDIErrorCallback>);

    // ActiveDOMObject
    virtual bool canSuspend() const OVERRIDE { return true; }

    void fulfill();
    void reject(DOMError*);

private:
    enum State {
        Pending,
        Accepted,
        Rejected,
        Invoked,
    };

    MIDIAccessPromise(ScriptExecutionContext*, const Dictionary&);

    State m_state;
    RefPtr<MIDISuccessCallback> m_successCallback;
    RefPtr<MIDIErrorCallback> m_errorCallback;
    OwnPtr<MIDIOptions> m_options;
    RefPtr<DOMError> m_error;
    RefPtr<MIDIAccess> m_access;
};

} // namespace WebCore

#endif // MIDIAccessPromise_h
