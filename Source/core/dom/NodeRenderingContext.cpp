/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (C) 2011 Google Inc. All rights reserved.
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
#include "core/dom/NodeRenderingContext.h"

#include "HTMLNames.h"
#include "SVGNames.h"
#include "core/css/resolver/StyleResolver.h"
#include "core/dom/ContainerNode.h"
#include "core/dom/FullscreenController.h"
#include "core/dom/Node.h"
#include "core/dom/PseudoElement.h"
#include "core/dom/Text.h"
#include "core/dom/shadow/ContentDistributor.h"
#include "core/dom/shadow/ElementShadow.h"
#include "core/dom/shadow/ShadowRoot.h"
#include "core/html/HTMLInputElement.h"
#include "core/html/shadow/HTMLContentElement.h"
#include "core/html/shadow/HTMLShadowElement.h"
#include "core/rendering/FlowThreadController.h"
#include "core/rendering/RenderFullScreen.h"
#include "core/rendering/RenderNamedFlowThread.h"
#include "core/rendering/RenderObject.h"
#include "core/rendering/RenderText.h"
#include "core/rendering/RenderView.h"
#include "core/rendering/style/StyleInheritedData.h"

namespace WebCore {

using namespace HTMLNames;

NodeRenderingContext::NodeRenderingContext(Node* node)
    : m_node(node)
    , m_parentFlowRenderer(0)
{
    m_renderingParent = NodeRenderingTraversal::parent(node, &m_parentDetails);
}

NodeRenderingContext::NodeRenderingContext(Node* node, RenderStyle* style)
    : m_node(node)
    , m_renderingParent(0)
    , m_style(style)
    , m_parentFlowRenderer(0)
{
}

NodeRenderingContext::NodeRenderingContext(Node* node, const Node::AttachContext& context)
: m_node(node)
, m_style(context.resolvedStyle)
, m_parentFlowRenderer(0)
{
    m_renderingParent = NodeRenderingTraversal::parent(node, &m_parentDetails);
}

NodeRenderingContext::~NodeRenderingContext()
{
}

static bool isRendererReparented(const RenderObject* renderer)
{
    if (!renderer->node()->isElementNode())
        return false;
    if (renderer->style() && !renderer->style()->flowThread().isEmpty())
        return true;
    if (toElement(renderer->node())->shouldBeReparentedUnderRenderView(renderer->style()))
        return true;
    return false;
}

RenderObject* NodeRenderingContext::nextRenderer() const
{
    if (RenderObject* renderer = m_node->renderer())
        return renderer->nextSibling();

    Element* element = m_node->isElementNode() ? toElement(m_node) : 0;
    if (element && element->shouldBeReparentedUnderRenderView(m_style.get())) {
        // FIXME: Reparented renderers not in the top layer should probably be
        // ordered in DOM tree order. We don't have a good way to do that yet,
        // since NodeRenderingTraversal isn't aware of reparenting. It's safe to
        // just append for now; it doesn't disrupt the top layer rendering as
        // the layer collection in RenderLayer only requires that top layer
        // renderers are orderered correctly relative to each other.
        if (!element->isInTopLayer())
            return 0;

        const Vector<RefPtr<Element> >& topLayerElements = element->document()->topLayerElements();
        size_t position = topLayerElements.find(element);
        ASSERT(position != notFound);
        for (size_t i = position + 1; i < topLayerElements.size(); ++i) {
            if (RenderObject* renderer = topLayerElements[i]->renderer())
                return renderer;
        }
        return 0;
    }

    if (m_parentFlowRenderer)
        return m_parentFlowRenderer->nextRendererForNode(m_node);

    // Avoid an O(N^2) problem with this function by not checking for
    // nextRenderer() when the parent element hasn't attached yet.
    if (m_renderingParent && !m_renderingParent->attached())
        return 0;

    for (Node* sibling = NodeRenderingTraversal::nextSibling(m_node); sibling; sibling = NodeRenderingTraversal::nextSibling(sibling)) {
        RenderObject* renderer = sibling->renderer();
        if (renderer && !isRendererReparented(renderer))
            return renderer;
    }

    return 0;
}

RenderObject* NodeRenderingContext::previousRenderer() const
{
    if (RenderObject* renderer = m_node->renderer())
        return renderer->previousSibling();

    // FIXME: This doesn't work correctly for reparented elements that are
    // display: none. We'd need to duplicate the logic in nextRenderer, but since
    // nothing needs that yet just assert.
    ASSERT(!m_node->isElementNode() || !toElement(m_node)->shouldBeReparentedUnderRenderView(m_style.get()));

    if (m_parentFlowRenderer)
        return m_parentFlowRenderer->previousRendererForNode(m_node);

    // FIXME: We should have the same O(N^2) avoidance as nextRenderer does
    // however, when I tried adding it, several tests failed.
    for (Node* sibling = NodeRenderingTraversal::previousSibling(m_node); sibling; sibling = NodeRenderingTraversal::previousSibling(sibling)) {
        RenderObject* renderer = sibling->renderer();
        if (renderer && !isRendererReparented(renderer))
            return renderer;
    }

    return 0;
}

RenderObject* NodeRenderingContext::parentRenderer() const
{
    if (RenderObject* renderer = m_node->renderer())
        return renderer->parent();

    if (m_node->isElementNode() && toElement(m_node)->shouldBeReparentedUnderRenderView(m_style.get())) {
        // The parent renderer of reparented elements is the RenderView, but only
        // if the normal parent would have had a renderer.
        // FIXME: This behavior isn't quite right as the spec for top layer
        // only talks about display: none ancestors so putting a <dialog> inside
        // an <optgroup> seems like it should still work even though this check
        // will prevent it.
        if (!m_renderingParent || !m_renderingParent->renderer())
            return 0;
        return m_node->document()->renderView();
    }

    if (m_parentFlowRenderer)
        return m_parentFlowRenderer;

    return m_renderingParent ? m_renderingParent->renderer() : 0;
}

bool NodeRenderingContext::shouldCreateRenderer() const
{
    if (!m_renderingParent)
        return false;
    RenderObject* parentRenderer = this->parentRenderer();
    if (!parentRenderer)
        return false;
    if (!parentRenderer->canHaveChildren())
        return false;
    if (!m_renderingParent->childShouldCreateRenderer(*this))
        return false;
    return true;
}

void NodeRenderingContext::moveToFlowThreadIfNeeded()
{
    ASSERT(m_node->isElementNode());
    ASSERT(m_style);
    if (!RuntimeEnabledFeatures::cssRegionsEnabled())
        return;

    if (m_style->flowThread().isEmpty())
        return;

    // As per http://dev.w3.org/csswg/css3-regions/#flow-into, pseudo-elements such as ::first-line, ::first-letter, ::before or ::after
    // cannot be directly collected into a named flow.
    if (m_node->isPseudoElement())
        return;

    // FIXME: Do not collect elements if they are in shadow tree.
    if (m_node->isInShadowTree())
        return;

    if (m_node->isElementNode() && FullscreenController::isActiveFullScreenElement(toElement(m_node)))
        return;

    // Allow only svg root elements to be directly collected by a render flow thread.
    if (m_node->isSVGElement()
        && (!(m_node->hasTagName(SVGNames::svgTag) && m_node->parentNode() && !m_node->parentNode()->isSVGElement())))
        return;

    m_flowThread = m_style->flowThread();
    ASSERT(m_node->document()->renderView());
    FlowThreadController* flowThreadController = m_node->document()->renderView()->flowThreadController();
    m_parentFlowRenderer = flowThreadController->ensureRenderFlowThreadWithName(m_flowThread);
    flowThreadController->registerNamedFlowContentNode(m_node, m_parentFlowRenderer);
}

bool NodeRenderingContext::isOnEncapsulationBoundary() const
{
    return isOnUpperEncapsulationBoundary() || isLowerEncapsulationBoundary(m_parentDetails.insertionPoint()) || isLowerEncapsulationBoundary(m_node->parentNode());
}

bool NodeRenderingContext::isOnUpperEncapsulationBoundary() const
{
    return m_node->parentNode() && m_node->parentNode()->isShadowRoot();
}

void NodeRenderingContext::createRendererForElementIfNeeded()
{
    ASSERT(!m_node->renderer());

    Element* element = toElement(m_node);

    if (!shouldCreateRenderer())
        return;
    m_style = element->styleForRenderer();
    ASSERT(m_style);

    moveToFlowThreadIfNeeded();

    if (!element->rendererIsNeeded(*this))
        return;

    RenderObject* parentRenderer = this->parentRenderer();
    RenderObject* nextRenderer = this->nextRenderer();

    Document* document = element->document();
    RenderObject* newRenderer = element->createRenderer(document->renderArena(), m_style.get());
    if (!newRenderer)
        return;

    if (!parentRenderer->isChildAllowed(newRenderer, m_style.get())) {
        newRenderer->destroy();
        return;
    }

    // Make sure the RenderObject already knows it is going to be added to a RenderFlowThread before we set the style
    // for the first time. Otherwise code using inRenderFlowThread() in the styleWillChange and styleDidChange will fail.
    newRenderer->setFlowThreadState(parentRenderer->flowThreadState());

    element->setRenderer(newRenderer);
    newRenderer->setAnimatableStyle(m_style.release()); // setAnimatableStyle() can depend on renderer() already being set.

    if (FullscreenController::isActiveFullScreenElement(element)) {
        newRenderer = RenderFullScreen::wrapRenderer(newRenderer, parentRenderer, document);
        if (!newRenderer)
            return;
    }

    // Note: Adding newRenderer instead of renderer(). renderer() may be a child of newRenderer.
    parentRenderer->addChild(newRenderer, nextRenderer);
}

void NodeRenderingContext::createRendererForTextIfNeeded()
{
    ASSERT(!m_node->renderer());

    Text* textNode = toText(m_node);

    if (!shouldCreateRenderer())
        return;

    RenderObject* parentRenderer = this->parentRenderer();
    ASSERT(parentRenderer);
    Document* document = textNode->document();

    if (resetStyleInheritance())
        m_style = document->styleResolver()->defaultStyleForElement();
    else
        m_style = parentRenderer->style();

    if (!textNode->textRendererIsNeeded(*this))
        return;
    RenderText* newRenderer = textNode->createTextRenderer(document->renderArena(), m_style.get());
    if (!newRenderer)
        return;
    if (!parentRenderer->isChildAllowed(newRenderer, m_style.get())) {
        newRenderer->destroy();
        return;
    }

    // Make sure the RenderObject already knows it is going to be added to a RenderFlowThread before we set the style
    // for the first time. Otherwise code using inRenderFlowThread() in the styleWillChange and styleDidChange will fail.
    newRenderer->setFlowThreadState(parentRenderer->flowThreadState());

    RenderObject* nextRenderer = this->nextRenderer();
    textNode->setRenderer(newRenderer);
    // Parent takes care of the animations, no need to call setAnimatableStyle.
    newRenderer->setStyle(m_style.release());
    parentRenderer->addChild(newRenderer, nextRenderer);
}

}
