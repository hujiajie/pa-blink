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

#ifndef InspectorInstrumentationCustom_inl_h
#define InspectorInstrumentationCustom_inl_h

namespace WebCore {

namespace InspectorInstrumentation {

bool profilerEnabledImpl(InstrumentingAgents*);
bool isDebuggerPausedImpl(InstrumentingAgents*);
InspectorInstrumentationCookie willProcessRuleImpl(InstrumentingAgents*, StyleRule*, StyleResolver*);
bool collectingHTMLParseErrorsImpl(InstrumentingAgents*);

bool canvasAgentEnabled(ScriptExecutionContext*);
bool consoleAgentEnabled(ScriptExecutionContext*);
bool timelineAgentEnabled(ScriptExecutionContext*);

inline bool profilerEnabled(Page* page)
{
    if (InstrumentingAgents* instrumentingAgents = instrumentingAgentsForPage(page))
        return profilerEnabledImpl(instrumentingAgents);
    return false;
}

inline bool isDebuggerPaused(Frame* frame)
{
    FAST_RETURN_IF_NO_FRONTENDS(false);
    if (InstrumentingAgents* instrumentingAgents = instrumentingAgentsForFrame(frame))
        return isDebuggerPausedImpl(instrumentingAgents);
    return false;
}

inline InspectorInstrumentationCookie willProcessRule(Document* document, StyleRule* rule, StyleResolver* styleResolver)
{
    FAST_RETURN_IF_NO_FRONTENDS(InspectorInstrumentationCookie());
    if (!rule)
        return InspectorInstrumentationCookie();
    if (InstrumentingAgents* instrumentingAgents = instrumentingAgentsForDocument(document))
        return willProcessRuleImpl(instrumentingAgents, rule, styleResolver);
    return InspectorInstrumentationCookie();
}

inline bool collectingHTMLParseErrors(Page* page)
{
    FAST_RETURN_IF_NO_FRONTENDS(false);
    if (InstrumentingAgents* instrumentingAgents = instrumentingAgentsForPage(page))
        return collectingHTMLParseErrorsImpl(instrumentingAgents);
    return false;
}

} // namespace InspectorInstrumentation

} // namespace WebCore

#endif // !defined(InspectorInstrumentationCustom_inl_h)
