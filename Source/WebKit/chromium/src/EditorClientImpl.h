/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
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

#ifndef EditorClientImpl_h
#define EditorClientImpl_h

#include "core/page/EditorClient.h"
#include "core/platform/Timer.h"
#include "core/platform/text/TextCheckerClient.h"
#include <wtf/Deque.h>
#include <wtf/HashSet.h>

namespace WebCore {
class Frame;
class HTMLInputElement;
}

namespace WebKit {
class WebViewImpl;
class WebTextCheckingCompletionImpl;

class EditorClientImpl : public WebCore::EditorClient, public WebCore::TextCheckerClient {
public:
    EditorClientImpl(WebViewImpl* webView);

    virtual ~EditorClientImpl();

    virtual bool smartInsertDeleteEnabled() OVERRIDE;
    virtual bool isSelectTrailingWhitespaceEnabled() OVERRIDE;
    virtual bool isContinuousSpellCheckingEnabled() OVERRIDE;
    virtual void toggleContinuousSpellChecking() OVERRIDE;
    virtual bool isGrammarCheckingEnabled() OVERRIDE;
    virtual bool shouldBeginEditing(WebCore::Range*) OVERRIDE;
    virtual bool shouldEndEditing(WebCore::Range*) OVERRIDE;
    virtual bool shouldInsertNode(WebCore::Node*, WebCore::Range*, WebCore::EditorInsertAction) OVERRIDE;
    virtual bool shouldInsertText(const WTF::String&, WebCore::Range*, WebCore::EditorInsertAction) OVERRIDE;
    virtual bool shouldDeleteRange(WebCore::Range*) OVERRIDE;
    virtual bool shouldChangeSelectedRange(WebCore::Range* fromRange, WebCore::Range* toRange,
        WebCore::EAffinity, bool stillSelecting) OVERRIDE;
    virtual bool shouldApplyStyle(WebCore::StylePropertySet*, WebCore::Range*) OVERRIDE;
    virtual void didBeginEditing() OVERRIDE;
    virtual void respondToChangedContents() OVERRIDE;
    virtual void respondToChangedSelection(WebCore::Frame*) OVERRIDE;
    virtual void didEndEditing() OVERRIDE;
    virtual void didCancelCompositionOnSelectionChange() OVERRIDE;
    virtual void registerUndoStep(PassRefPtr<WebCore::UndoStep>) OVERRIDE;
    virtual void registerRedoStep(PassRefPtr<WebCore::UndoStep>) OVERRIDE;
    virtual void clearUndoRedoOperations() OVERRIDE;
    virtual bool canCopyCut(WebCore::Frame*, bool defaultValue) const OVERRIDE;
    virtual bool canPaste(WebCore::Frame*, bool defaultValue) const OVERRIDE;
    virtual bool canUndo() const OVERRIDE;
    virtual bool canRedo() const OVERRIDE;
    virtual void undo() OVERRIDE;
    virtual void redo() OVERRIDE;
    virtual void handleKeyboardEvent(WebCore::KeyboardEvent*) OVERRIDE;
    virtual void textFieldDidEndEditing(WebCore::Element*) OVERRIDE;
    virtual void textDidChangeInTextField(WebCore::Element*) OVERRIDE;
    virtual bool doTextFieldCommandFromEvent(WebCore::Element*, WebCore::KeyboardEvent*) OVERRIDE;
    virtual bool shouldEraseMarkersAfterChangeSelection(WebCore::TextCheckingType) const OVERRIDE;
    virtual void checkSpellingOfString(const UChar*, int length, int* misspellingLocation, int* misspellingLength) OVERRIDE;
    virtual void checkGrammarOfString(const UChar*, int length, WTF::Vector<WebCore::GrammarDetail>&,
        int* badGrammarLocation, int* badGrammarLength) OVERRIDE;
    virtual WTF::String getAutoCorrectSuggestionForMisspelledWord(const WTF::String&) OVERRIDE;
    virtual void updateSpellingUIWithMisspelledWord(const WTF::String&) OVERRIDE;
    virtual void showSpellingUI(bool show) OVERRIDE;
    virtual bool spellingUIIsShowing() OVERRIDE;
    virtual void willSetInputMethodState() OVERRIDE;
    virtual void requestCheckingOfString(WTF::PassRefPtr<WebCore::TextCheckingRequest>) OVERRIDE;
    virtual bool supportsGlobalSelection() OVERRIDE;

    virtual WebCore::TextCheckerClient* textChecker() { return this; }

    const char* interpretKeyEvent(const WebCore::KeyboardEvent*);

private:
    bool handleEditingKeyboardEvent(WebCore::KeyboardEvent*);
    void modifySelection(WebCore::Frame*, WebCore::KeyboardEvent*);

    // Returns whether or not the focused control needs spell-checking.
    // Currently, this function just retrieves the focused node and determines
    // whether or not it is a <textarea> element or an element whose
    // contenteditable attribute is true.
    // FIXME: Bug 740540: This code just implements the default behavior
    // proposed in this issue. We should also retrieve "spellcheck" attributes
    // for text fields and create a flag to over-write the default behavior.
    bool shouldSpellcheckByDefault();

    WebViewImpl* m_webView;
    bool m_inRedo;

    typedef Deque<RefPtr<WebCore::UndoStep> > UndoManagerStack;
    UndoManagerStack m_undoStack;
    UndoManagerStack m_redoStack;

    // This flag is set to false if spell check for this editor is manually
    // turned off. The default setting is SpellCheckAutomatic.
    enum {
        SpellCheckAutomatic,
        SpellCheckForcedOn,
        SpellCheckForcedOff
    };
    int m_spellCheckThisFieldStatus;
};

} // namespace WebKit

#endif
