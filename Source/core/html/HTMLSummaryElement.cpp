/*
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "core/html/HTMLSummaryElement.h"

#include "HTMLNames.h"
#include "core/dom/KeyboardEvent.h"
#include "core/dom/NodeRenderingContext.h"
#include "core/dom/shadow/ShadowRoot.h"
#include "core/html/HTMLDetailsElement.h"
#include "core/html/shadow/DetailsMarkerControl.h"
#include "core/html/shadow/HTMLContentElement.h"
#include "core/rendering/RenderBlock.h"

namespace WebCore {

using namespace HTMLNames;

PassRefPtr<HTMLSummaryElement> HTMLSummaryElement::create(const QualifiedName& tagName, Document* document)
{
    RefPtr<HTMLSummaryElement> summary = adoptRef(new HTMLSummaryElement(tagName, document));
    summary->ensureUserAgentShadowRoot();
    return summary.release();
}

HTMLSummaryElement::HTMLSummaryElement(const QualifiedName& tagName, Document* document)
    : HTMLElement(tagName, document)
{
    ASSERT(hasTagName(summaryTag));
}

RenderObject* HTMLSummaryElement::createRenderer(RenderArena* arena, RenderStyle*)
{
    return new (arena) RenderBlock(this);
}

bool HTMLSummaryElement::childShouldCreateRenderer(const NodeRenderingContext& childContext) const
{
    if (childContext.node()->isPseudoElement())
        return true;
    return childContext.isOnEncapsulationBoundary() && HTMLElement::childShouldCreateRenderer(childContext);
}

void HTMLSummaryElement::didAddUserAgentShadowRoot(ShadowRoot* root)
{
    root->appendChild(DetailsMarkerControl::create(document()), ASSERT_NO_EXCEPTION, AttachLazily);
    root->appendChild(HTMLContentElement::create(document()), ASSERT_NO_EXCEPTION, AttachLazily);
}

HTMLDetailsElement* HTMLSummaryElement::detailsElement() const
{
    Node* mayDetails = const_cast<HTMLSummaryElement*>(this)->parentNodeForRenderingAndStyle();
    if (!mayDetails || !mayDetails->hasTagName(detailsTag))
        return 0;
    return static_cast<HTMLDetailsElement*>(mayDetails);
}

bool HTMLSummaryElement::isMainSummary() const
{
    if (HTMLDetailsElement* details = detailsElement())
        return details->findMainSummary() == this;

    return false;
}

static bool isClickableControl(Node* node)
{
    if (!node->isElementNode())
        return false;
    Element* element = toElement(node);
    if (element->isFormControlElement())
        return true;
    Element* host = element->shadowHost();
    return host && host->isFormControlElement();
}

bool HTMLSummaryElement::supportsFocus() const
{
    return isMainSummary();
}

void HTMLSummaryElement::defaultEventHandler(Event* event)
{
    if (isMainSummary() && renderer()) {
        if (event->type() == eventNames().DOMActivateEvent && !isClickableControl(event->target()->toNode())) {
            if (HTMLDetailsElement* details = detailsElement())
                details->toggleOpen();
            event->setDefaultHandled();
            return;
        }

        if (event->isKeyboardEvent()) {
            if (event->type() == eventNames().keydownEvent && static_cast<KeyboardEvent*>(event)->keyIdentifier() == "U+0020") {
                setActive(true, true);
                // No setDefaultHandled() - IE dispatches a keypress in this case.
                return;
            }
            if (event->type() == eventNames().keypressEvent) {
                switch (static_cast<KeyboardEvent*>(event)->charCode()) {
                case '\r':
                    dispatchSimulatedClick(event);
                    event->setDefaultHandled();
                    return;
                case ' ':
                    // Prevent scrolling down the page.
                    event->setDefaultHandled();
                    return;
                }
            }
            if (event->type() == eventNames().keyupEvent && static_cast<KeyboardEvent*>(event)->keyIdentifier() == "U+0020") {
                if (active())
                    dispatchSimulatedClick(event);
                event->setDefaultHandled();
                return;
            }
        }
    }

    HTMLElement::defaultEventHandler(event);
}

bool HTMLSummaryElement::willRespondToMouseClickEvents()
{
    if (isMainSummary() && renderer())
        return true;

    return HTMLElement::willRespondToMouseClickEvents();
}

}
