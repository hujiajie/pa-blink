/*
 * Copyright (C) 2011 Adobe Systems Incorporated. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#include "core/css/CSSShaderValue.h"

#include "core/css/CSSParser.h"
#include "core/dom/Document.h"
#include "core/dom/WebCoreMemoryInstrumentation.h"
#include "core/loader/cache/CachedResourceLoader.h"
#include "core/loader/cache/CachedResourceRequest.h"
#include "core/loader/cache/CachedResourceRequestInitiators.h"
#include "core/rendering/style/StyleCachedShader.h"
#include "core/rendering/style/StylePendingShader.h"

namespace WebCore {

CSSShaderValue::CSSShaderValue(const String& url)
    : CSSValue(CSSShaderClass)
    , m_url(url)
    , m_accessedShader(false)
{
}

CSSShaderValue::~CSSShaderValue()
{
}

KURL CSSShaderValue::completeURL(CachedResourceLoader* loader) const
{
    return loader->document()->completeURL(m_url);
}

StyleCachedShader* CSSShaderValue::cachedShader(CachedResourceLoader* loader)
{
    ASSERT(loader);

    if (!m_accessedShader) {
        m_accessedShader = true;

        CachedResourceRequest request(ResourceRequest(completeURL(loader)), cachedResourceRequestInitiators().css);
        if (CachedResourceHandle<CachedShader> cachedShader = loader->requestShader(request))
            m_shader = StyleCachedShader::create(cachedShader.get());
    }

    return (m_shader && m_shader->isCachedShader()) ? static_cast<StyleCachedShader*>(m_shader.get()) : 0;
}

StyleShader* CSSShaderValue::cachedOrPendingShader()
{
    if (!m_shader)
        m_shader = StylePendingShader::create(this);

    return m_shader.get();
}

String CSSShaderValue::customCssText() const
{
    StringBuilder result;
    result.appendLiteral("url(");
    result.append(quoteCSSURLIfNeeded(m_url));
    result.append(')');
    if (!m_format.isEmpty()) {
        result.appendLiteral(" format('");
        result.append(m_format);
        result.appendLiteral("')");
    }
    return result.toString();
}

bool CSSShaderValue::equals(const CSSShaderValue& other) const
{
    return m_url == other.m_url;
}

void CSSShaderValue::reportDescendantMemoryUsage(MemoryObjectInfo* memoryObjectInfo) const
{
    MemoryClassInfo info(memoryObjectInfo, this, WebCoreMemoryTypes::CSS);
    info.addMember(m_url, "url");
    info.addMember(m_format, "format");
}

} // namespace WebCore

