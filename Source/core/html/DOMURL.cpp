/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 * Copyright (C) 2012 Motorola Mobility Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "core/html/DOMURL.h"

#include "core/dom/ScriptExecutionContext.h"
#include "core/fileapi/Blob.h"
#include "core/fileapi/BlobURL.h"
#include "core/html/PublicURLManager.h"
#include "core/loader/cache/MemoryCache.h"
#include "modules/mediasource/MediaSourceBase.h"
#include "modules/mediastream/MediaStream.h"
#include "weborigin/KURL.h"
#include "wtf/MainThread.h"
#include "wtf/PassOwnPtr.h"

namespace WebCore {

String DOMURL::createObjectURL(ScriptExecutionContext* scriptExecutionContext, MediaSourceBase* source)
{
    // Since WebWorkers cannot obtain MediaSource objects, we should be on the main thread.
    ASSERT(isMainThread());

    if (!scriptExecutionContext || !source)
        return String();
    return createPublicURL(scriptExecutionContext, source);
}

String DOMURL::createObjectURL(ScriptExecutionContext* scriptExecutionContext, MediaStream* stream)
{
    // Since WebWorkers cannot obtain Stream objects, we should be on the main thread.
    ASSERT(isMainThread());

    if (!scriptExecutionContext || !stream)
        return String();
    return createPublicURL(scriptExecutionContext, stream);
}

String DOMURL::createObjectURL(ScriptExecutionContext* scriptExecutionContext, Blob* blob)
{
    if (!scriptExecutionContext || !blob)
        return String();
    return createPublicURL(scriptExecutionContext, blob);
}

String DOMURL::createPublicURL(ScriptExecutionContext* scriptExecutionContext, URLRegistrable* registrable)
{
    KURL publicURL = BlobURL::createPublicURL(scriptExecutionContext->securityOrigin());
    if (publicURL.isEmpty())
        return String();

    scriptExecutionContext->publicURLManager().registerURL(scriptExecutionContext->securityOrigin(), publicURL, registrable);

    return publicURL.string();
}

void DOMURL::revokeObjectURL(ScriptExecutionContext* scriptExecutionContext, const String& urlString)
{
    if (!scriptExecutionContext)
        return;

    KURL url(KURL(), urlString);
    MemoryCache::removeURLFromCache(scriptExecutionContext, url);
    scriptExecutionContext->publicURLManager().revoke(url);
}

} // namespace WebCore
