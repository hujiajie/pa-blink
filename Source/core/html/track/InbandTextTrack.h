/*
 * Copyright (C) 2012, 2013 Apple Inc. All rights reserved.
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

#ifndef InbandTextTrack_h
#define InbandTextTrack_h

#include "core/html/track/TextTrack.h"
#include "core/platform/graphics/InbandTextTrackPrivate.h"
#include "core/platform/graphics/InbandTextTrackPrivateClient.h"
#include <wtf/RefPtr.h>

namespace WebCore {

class Document;
class InbandTextTrackPrivate;
class MediaPlayer;
class TextTrackCue;

class InbandTextTrack : public TextTrack, public InbandTextTrackPrivateClient {
public:
    static PassRefPtr<InbandTextTrack> create(ScriptExecutionContext*, TextTrackClient*, PassRefPtr<InbandTextTrackPrivate>);
    virtual ~InbandTextTrack();

    virtual bool isClosedCaptions() const OVERRIDE;
    virtual bool containsOnlyForcedSubtitles() const OVERRIDE;
    virtual bool isMainProgramContent() const OVERRIDE;
    virtual bool isEasyToRead() const OVERRIDE;
    virtual void setMode(const AtomicString&) OVERRIDE;
    size_t inbandTrackIndex();

private:
    InbandTextTrack(ScriptExecutionContext*, TextTrackClient*, PassRefPtr<InbandTextTrackPrivate>);

    virtual void addGenericCue(InbandTextTrackPrivate*, GenericCueData*) OVERRIDE;
    virtual void addWebVTTCue(InbandTextTrackPrivate*, double, double, const String&, const String&, const String&) OVERRIDE;

    RefPtr<InbandTextTrackPrivate> m_private;
};

} // namespace WebCore

#endif
