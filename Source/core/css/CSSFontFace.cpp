/*
 * Copyright (C) 2007, 2008, 2011 Apple Inc. All rights reserved.
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
#include "core/css/CSSFontFace.h"

#include "core/css/CSSFontFaceSource.h"
#include "core/css/CSSFontSelector.h"
#include "core/css/CSSSegmentedFontFace.h"
#include "core/css/FontLoader.h"
#include "core/dom/Document.h"
#include "RuntimeEnabledFeatures.h"
#include "core/platform/graphics/SimpleFontData.h"

namespace WebCore {

bool CSSFontFace::isLoaded() const
{
    size_t size = m_sources.size();
    for (size_t i = 0; i < size; i++) {
        if (!m_sources[i]->isLoaded())
            return false;
    }
    return true;
}

bool CSSFontFace::isValid() const
{
    size_t size = m_sources.size();
    for (size_t i = 0; i < size; i++) {
        if (m_sources[i]->isValid())
            return true;
    }
    return false;
}

void CSSFontFace::addedToSegmentedFontFace(CSSSegmentedFontFace* segmentedFontFace)
{
    m_segmentedFontFaces.add(segmentedFontFace);
}

void CSSFontFace::removedFromSegmentedFontFace(CSSSegmentedFontFace* segmentedFontFace)
{
    m_segmentedFontFaces.remove(segmentedFontFace);
}

void CSSFontFace::addSource(PassOwnPtr<CSSFontFaceSource> source)
{
    source->setFontFace(this);
    m_sources.append(source);
}

void CSSFontFace::fontLoaded(CSSFontFaceSource* source)
{
    if (source != m_activeSource)
        return;

    // FIXME: Can we assert that m_segmentedFontFaces is not empty? That may
    // require stopping in-progress font loading when the last
    // CSSSegmentedFontFace is removed.
    if (m_segmentedFontFaces.isEmpty())
        return;

    // Use one of the CSSSegmentedFontFaces' font selector. They all have
    // the same font selector, so it's wasteful to store it in the CSSFontFace.
    CSSFontSelector* fontSelector = (*m_segmentedFontFaces.begin())->fontSelector();
    fontSelector->fontLoaded();

    if (RuntimeEnabledFeatures::fontLoadEventsEnabled() && m_loadState == Loading) {
        if (source->ensureFontData())
            notifyFontLoader(Loaded);
        else if (!isValid())
            notifyFontLoader(Error);
    }

    HashSet<CSSSegmentedFontFace*>::iterator end = m_segmentedFontFaces.end();
    for (HashSet<CSSSegmentedFontFace*>::iterator it = m_segmentedFontFaces.begin(); it != end; ++it)
        (*it)->fontLoaded(this);
}

PassRefPtr<SimpleFontData> CSSFontFace::getFontData(const FontDescription& fontDescription, bool syntheticBold, bool syntheticItalic)
{
    m_activeSource = 0;
    if (!isValid())
        return 0;

    ASSERT(!m_segmentedFontFaces.isEmpty());
    CSSFontSelector* fontSelector = (*m_segmentedFontFaces.begin())->fontSelector();

    if (RuntimeEnabledFeatures::fontLoadEventsEnabled() && m_loadState == NotLoaded)
        notifyFontLoader(Loading);

    size_t size = m_sources.size();
    for (size_t i = 0; i < size; ++i) {
        if (RefPtr<SimpleFontData> result = m_sources[i]->getFontData(fontDescription, syntheticBold, syntheticItalic, fontSelector)) {
            m_activeSource = m_sources[i].get();
            if (RuntimeEnabledFeatures::fontLoadEventsEnabled() && m_loadState == Loading && m_sources[i]->isLoaded())
                notifyFontLoader(Loaded);
            return result.release();
        }
    }

    if (RuntimeEnabledFeatures::fontLoadEventsEnabled() && m_loadState == Loading)
        notifyFontLoader(Error);
    return 0;
}

void CSSFontFace::notifyFontLoader(LoadState newState)
{
    m_loadState = newState;

    Document* document = (*m_segmentedFontFaces.begin())->fontSelector()->document();
    if (!document)
        return;

    switch (newState) {
    case Loading:
        document->fontloader()->beginFontLoading(m_rule.get());
        break;
    case Loaded:
        document->fontloader()->fontLoaded(m_rule.get());
        break;
    case Error:
        document->fontloader()->loadError(m_rule.get(), m_activeSource);
        break;
    default:
        break;
    }
}

#if ENABLE(SVG_FONTS)
bool CSSFontFace::hasSVGFontFaceSource() const
{
    size_t size = m_sources.size();
    for (size_t i = 0; i < size; i++) {
        if (m_sources[i]->isSVGFontFaceSource())
            return true;
    }
    return false;
}
#endif

}
