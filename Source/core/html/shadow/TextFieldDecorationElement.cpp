/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
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
#include "core/html/shadow/TextFieldDecorationElement.h"

#include "CSSPropertyNames.h"
#include "CSSValueKeywords.h"
#include "core/dom/Event.h"
#include "core/dom/NodeRenderStyle.h"
#include "core/dom/shadow/ElementShadow.h"
#include "core/dom/shadow/ShadowRoot.h"
#include "core/html/HTMLInputElement.h"
#include "core/html/shadow/HTMLShadowElement.h"
#include "core/rendering/RenderImage.h"

namespace WebCore {

using namespace HTMLNames;

// TextFieldDecorator ----------------------------------------------------------------

TextFieldDecorator::~TextFieldDecorator()
{
}

// TextFieldDecorationElement ----------------------------------------------------------------

// FIXME: This class is only used in Chromium, and has no layout tests!!

TextFieldDecorationElement::TextFieldDecorationElement(Document* document, TextFieldDecorator* decorator)
    : HTMLDivElement(HTMLNames::divTag, document)
    , m_textFieldDecorator(decorator)
    , m_isInHoverState(false)
{
    ASSERT(decorator);
    setHasCustomStyleCallbacks();
}

PassRefPtr<TextFieldDecorationElement> TextFieldDecorationElement::create(Document* document, TextFieldDecorator* decorator)
{
    return adoptRef(new TextFieldDecorationElement(document, decorator));
}

TextFieldDecorationElement* TextFieldDecorationElement::fromShadowRoot(ShadowRoot* shadowRoot)
{
    if (!shadowRoot->firstChild()
        || !shadowRoot->firstChild()->lastChild()
        || !shadowRoot->firstChild()->lastChild()->isElementNode()
        || !toElement(shadowRoot->firstChild()->lastChild())->isTextFieldDecoration())
        return 0;
    return toTextFieldDecorationElement(shadowRoot->firstChild()->lastChild());
}

static inline void getDecorationRootAndDecoratedRoot(HTMLInputElement* input, ShadowRoot*& decorationRoot, ShadowRoot*& decoratedRoot)
{
    ShadowRoot* existingRoot = input->youngestShadowRoot();
    ShadowRoot* newRoot = 0;
    while (existingRoot->childNodeCount() == 1 && existingRoot->firstChild()->hasTagName(shadowTag)) {
        newRoot = existingRoot;
        existingRoot = existingRoot->olderShadowRoot();
        ASSERT(existingRoot);
    }
    if (newRoot)
        newRoot->removeChild(newRoot->firstChild());
    else {
        // FIXME: This interacts really badly with author shadow roots because now
        // we can interleave user agent and author shadow roots on the element meaning
        // input.shadowRoot may be inaccessible if the browser has decided to decorate
        // the input.
        newRoot = input->ensureShadow()->addShadowRoot(input, ShadowRoot::UserAgentShadowRoot);
    }
    decorationRoot = newRoot;
    decoratedRoot = existingRoot;
}

void TextFieldDecorationElement::decorate(HTMLInputElement* input, bool visible)
{
    ASSERT(input);
    ShadowRoot* existingRoot;
    ShadowRoot* decorationRoot;
    getDecorationRootAndDecoratedRoot(input, decorationRoot, existingRoot);
    ASSERT(decorationRoot);
    ASSERT(existingRoot);
    RefPtr<HTMLDivElement> box = HTMLDivElement::create(input->document());
    decorationRoot->appendChild(box);
    box->setInlineStyleProperty(CSSPropertyDisplay, CSSValueWebkitFlex);
    box->setInlineStyleProperty(CSSPropertyWebkitAlignItems, CSSValueCenter);
    ASSERT(existingRoot->childNodeCount() == 1);
    toHTMLElement(existingRoot->firstChild())->setInlineStyleProperty(CSSPropertyWebkitFlexGrow, 1.0, CSSPrimitiveValue::CSS_NUMBER);
    box->appendChild(HTMLShadowElement::create(HTMLNames::shadowTag, input->document()));
    setInlineStyleProperty(CSSPropertyDisplay, visible ? CSSValueBlock : CSSValueNone);
    box->appendChild(this);
}

inline HTMLInputElement* TextFieldDecorationElement::hostInput()
{
    // TextFieldDecorationElement is created only by C++ code, and it is always
    // in <input> shadow.
    return toHTMLInputElement(shadowHost());
}

bool TextFieldDecorationElement::isTextFieldDecoration() const
{
    return true;
}

void TextFieldDecorationElement::updateImage()
{
    if (!renderer() || !renderer()->isImage())
        return;
    RenderImageResource* resource = toRenderImage(renderer())->imageResource();
    CachedImage* image;
    if (hostInput()->isDisabledFormControl())
        image = m_textFieldDecorator->imageForDisabledState();
    else if (hostInput()->isReadOnly())
        image = m_textFieldDecorator->imageForReadonlyState();
    else if (m_isInHoverState)
        image = m_textFieldDecorator->imageForHoverState();
    else
        image = m_textFieldDecorator->imageForNormalState();
    ASSERT(image);
    resource->setCachedImage(image);
}

PassRefPtr<RenderStyle> TextFieldDecorationElement::customStyleForRenderer()
{
    RefPtr<RenderStyle> originalStyle = originalStyleForRenderer();
    RefPtr<RenderStyle> style = RenderStyle::clone(originalStyle.get());
    RenderStyle* inputStyle = hostInput()->renderStyle();
    ASSERT(inputStyle);
    style->setWidth(Length(inputStyle->fontSize(), Fixed));
    style->setHeight(Length(inputStyle->fontSize(), Fixed));
    updateImage();
    return style.release();
}

RenderObject* TextFieldDecorationElement::createRenderer(RenderArena* arena, RenderStyle*)
{
    RenderImage* image = new (arena) RenderImage(this);
    image->setImageResource(RenderImageResource::create());
    return image;
}

void TextFieldDecorationElement::attach(const AttachContext& context)
{
    HTMLDivElement::attach(context);
    updateImage();
}

void TextFieldDecorationElement::detach(const AttachContext& context)
{
    m_textFieldDecorator->willDetach(hostInput());
    HTMLDivElement::detach(context);
}

bool TextFieldDecorationElement::isMouseFocusable() const
{
    return false;
}

void TextFieldDecorationElement::defaultEventHandler(Event* event)
{
    RefPtr<HTMLInputElement> input(hostInput());
    if (!input || input->isDisabledOrReadOnly() || !event->isMouseEvent()) {
        if (!event->defaultHandled())
            HTMLDivElement::defaultEventHandler(event);
        return;
    }

    RefPtr<TextFieldDecorationElement> protector(this);
    if (event->type() == eventNames().clickEvent) {
        m_textFieldDecorator->handleClick(input.get());
        event->setDefaultHandled();
    }

    if (event->type() == eventNames().mouseoverEvent) {
        m_isInHoverState = true;
        updateImage();
    }

    if (event->type() == eventNames().mouseoutEvent) {
        m_isInHoverState = false;
        updateImage();
    }

    if (!event->defaultHandled())
        HTMLDivElement::defaultEventHandler(event);
}

bool TextFieldDecorationElement::willRespondToMouseMoveEvents()
{
    const HTMLInputElement* input = hostInput();
    if (!input->isDisabledOrReadOnly())
        return true;

    return HTMLDivElement::willRespondToMouseMoveEvents();
}

bool TextFieldDecorationElement::willRespondToMouseClickEvents()
{
    const HTMLInputElement* input = hostInput();
    if (!input->isDisabledOrReadOnly())
        return true;

    return HTMLDivElement::willRespondToMouseClickEvents();
}

} // namespace WebCore
