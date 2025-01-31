/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2006, 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Rob Buis (rwlbuis@gmail.com)
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
 */

#include "config.h"
#include "core/html/HTMLLinkElement.h"

#include <wtf/StdLibExtras.h>
#include "HTMLNames.h"
#include "bindings/v8/ScriptEventListener.h"
#include "core/css/MediaList.h"
#include "core/css/MediaQueryEvaluator.h"
#include "core/css/StyleSheetContents.h"
#include "core/css/resolver/StyleResolver.h"
#include "core/dom/Attribute.h"
#include "core/dom/Document.h"
#include "core/dom/DocumentFragment.h"
#include "core/dom/DocumentStyleSheetCollection.h"
#include "core/dom/Event.h"
#include "core/dom/EventSender.h"
#include "core/html/HTMLImportsController.h"
#include "core/loader/FrameLoader.h"
#include "core/loader/cache/CachedCSSStyleSheet.h"
#include "core/loader/cache/CachedResourceLoader.h"
#include "core/loader/cache/CachedResourceRequest.h"
#include "core/page/Frame.h"
#include "core/page/FrameView.h"

namespace WebCore {

using namespace HTMLNames;

static LinkEventSender& linkLoadEventSender()
{
    DEFINE_STATIC_LOCAL(LinkEventSender, sharedLoadEventSender, (eventNames().loadEvent));
    return sharedLoadEventSender;
}

inline HTMLLinkElement::HTMLLinkElement(const QualifiedName& tagName, Document* document, bool createdByParser)
    : HTMLElement(tagName, document)
    , m_linkLoader(this)
    , m_sizes(DOMSettableTokenList::create())
    , m_createdByParser(createdByParser)
    , m_isInShadowTree(false)
    , m_beforeLoadRecurseCount(0)
{
    ASSERT(hasTagName(linkTag));
    ScriptWrappable::init(this);
}

PassRefPtr<HTMLLinkElement> HTMLLinkElement::create(const QualifiedName& tagName, Document* document, bool createdByParser)
{
    return adoptRef(new HTMLLinkElement(tagName, document, createdByParser));
}

HTMLLinkElement::~HTMLLinkElement()
{
    m_link.clear();

    if (inDocument())
        document()->styleSheetCollection()->removeStyleSheetCandidateNode(this);

    linkLoadEventSender().cancelEvent(this);
}

void HTMLLinkElement::parseAttribute(const QualifiedName& name, const AtomicString& value)
{
    if (name == relAttr) {
        m_relAttribute = LinkRelAttribute(value);
        process();
    } else if (name == hrefAttr) {
        process();
    } else if (name == typeAttr) {
        m_type = value;
        process();
    } else if (name == sizesAttr) {
        setSizes(value);
        process();
    } else if (name == mediaAttr) {
        m_media = value.string().lower();
        process();
    } else if (name == disabledAttr) {
        if (LinkStyle* link = linkStyle())
            link->setDisabledState(!value.isNull());
    } else if (name == onbeforeloadAttr)
        setAttributeEventListener(eventNames().beforeloadEvent, createAttributeEventListener(this, name, value));
    else {
        if (name == titleAttr) {
            if (LinkStyle* link = linkStyle())
                link->setSheetTitle(value);
        }

        HTMLElement::parseAttribute(name, value);
    }
}

bool HTMLLinkElement::shouldLoadLink()
{
    bool continueLoad = true;
    RefPtr<Document> originalDocument = document();
    int recursionRank = ++m_beforeLoadRecurseCount;
    if (!dispatchBeforeLoadEvent(getNonEmptyURLAttribute(hrefAttr)))
        continueLoad = false;

    // A beforeload handler might have removed us from the document or changed the document.
    if (continueLoad && (!inDocument() || document() != originalDocument))
        continueLoad = false;

    // If the beforeload handler recurses into the link element by mutating it, we should only
    // let the latest (innermost) mutation occur.
    if (recursionRank != m_beforeLoadRecurseCount)
        continueLoad = false;

    if (recursionRank == 1)
        m_beforeLoadRecurseCount = 0;

    return continueLoad;
}

LinkResource* HTMLLinkElement::linkResourceToProcess()
{
    bool visible = inDocument() && !m_isInShadowTree;
    if (!visible) {
        ASSERT(!linkStyle() || !linkStyle()->hasSheet());
        return 0;
    }

    if (!m_link) {
        if (m_relAttribute.isImport() && RuntimeEnabledFeatures::htmlImportsEnabled())
            m_link = LinkImport::create(this);
        else {
            RefPtr<LinkStyle> link = LinkStyle::create(this);
            if (fastHasAttribute(disabledAttr))
                link->setDisabledState(true);
            m_link = link.release();
        }
    }

    return m_link.get();
}

LinkStyle* HTMLLinkElement::linkStyle() const
{
    if (!m_link || m_link->type() != LinkResource::Style)
        return 0;
    return static_cast<LinkStyle*>(m_link.get());
}

LinkImport* HTMLLinkElement::linkImport() const
{
    if (!m_link || m_link->type() != LinkResource::Import)
        return 0;
    return static_cast<LinkImport*>(m_link.get());
}

DocumentFragment* HTMLLinkElement::import() const
{
    if (LinkImport* link = linkImport())
        return linkImport()->importedFragment();
    return 0;
}

void HTMLLinkElement::process()
{
    if (LinkResource* link = linkResourceToProcess())
        link->process();
}

Node::InsertionNotificationRequest HTMLLinkElement::insertedInto(ContainerNode* insertionPoint)
{
    HTMLElement::insertedInto(insertionPoint);
    if (!insertionPoint->inDocument())
        return InsertionDone;

    m_isInShadowTree = isInShadowTree();
    if (m_isInShadowTree)
        return InsertionDone;

    document()->styleSheetCollection()->addStyleSheetCandidateNode(this, m_createdByParser);

    process();
    return InsertionDone;
}

void HTMLLinkElement::removedFrom(ContainerNode* insertionPoint)
{
    HTMLElement::removedFrom(insertionPoint);
    if (!insertionPoint->inDocument())
        return;

    m_linkLoader.released();

    if (m_isInShadowTree) {
        ASSERT(!linkStyle() || !linkStyle()->hasSheet());
        return;
    }
    document()->styleSheetCollection()->removeStyleSheetCandidateNode(this);

    if (m_link)
        m_link->ownerRemoved();

    if (document()->renderer())
        document()->styleResolverChanged(DeferRecalcStyle);
}

void HTMLLinkElement::finishParsingChildren()
{
    m_createdByParser = false;
    HTMLElement::finishParsingChildren();
}

bool HTMLLinkElement::styleSheetIsLoading() const
{
    return linkStyle() && linkStyle()->styleSheetIsLoading();
}

void HTMLLinkElement::linkLoaded()
{
    dispatchEvent(Event::create(eventNames().loadEvent, false, false));
}

void HTMLLinkElement::linkLoadingErrored()
{
    dispatchEvent(Event::create(eventNames().errorEvent, false, false));
}

void HTMLLinkElement::didStartLinkPrerender()
{
    dispatchEvent(Event::create(eventNames().webkitprerenderstartEvent, false, false));
}

void HTMLLinkElement::didStopLinkPrerender()
{
    dispatchEvent(Event::create(eventNames().webkitprerenderstopEvent, false, false));
}

void HTMLLinkElement::didSendLoadForLinkPrerender()
{
    dispatchEvent(Event::create(eventNames().webkitprerenderloadEvent, false, false));
}

void HTMLLinkElement::didSendDOMContentLoadedForLinkPrerender()
{
    dispatchEvent(Event::create(eventNames().webkitprerenderdomcontentloadedEvent, false, false));
}

bool HTMLLinkElement::sheetLoaded()
{
    ASSERT(linkStyle());
    return linkStyle()->sheetLoaded();
}

void HTMLLinkElement::notifyLoadedSheetAndAllCriticalSubresources(bool errorOccurred)
{
    ASSERT(linkStyle());
    linkStyle()->notifyLoadedSheetAndAllCriticalSubresources(errorOccurred);
}

void HTMLLinkElement::dispatchPendingLoadEvents()
{
    linkLoadEventSender().dispatchPendingEvents();
}

void HTMLLinkElement::dispatchPendingEvent(LinkEventSender* eventSender)
{
    ASSERT_UNUSED(eventSender, eventSender == &linkLoadEventSender());
    ASSERT(linkStyle());
    if (linkStyle()->hasLoadedSheet())
        linkLoaded();
    else
        linkLoadingErrored();
}

void HTMLLinkElement::startLoadingDynamicSheet()
{
    ASSERT(linkStyle());
    linkStyle()->startLoadingDynamicSheet();
}

bool HTMLLinkElement::isURLAttribute(const Attribute& attribute) const
{
    return attribute.name().localName() == hrefAttr || HTMLElement::isURLAttribute(attribute);
}

KURL HTMLLinkElement::href() const
{
    return document()->completeURL(getAttribute(hrefAttr));
}

String HTMLLinkElement::rel() const
{
    return getAttribute(relAttr);
}

String HTMLLinkElement::target() const
{
    return getAttribute(targetAttr);
}

String HTMLLinkElement::type() const
{
    return getAttribute(typeAttr);
}

IconType HTMLLinkElement::iconType() const
{
    return m_relAttribute.iconType();
}

String HTMLLinkElement::iconSizes() const
{
    return m_sizes->toString();
}

void HTMLLinkElement::addSubresourceAttributeURLs(ListHashSet<KURL>& urls) const
{
    HTMLElement::addSubresourceAttributeURLs(urls);

    // Favicons are handled by a special case in LegacyWebArchive::create()
    if (m_relAttribute.iconType() != InvalidIcon)
        return;

    if (!m_relAttribute.isStyleSheet())
        return;
    
    // Append the URL of this link element.
    addSubresourceURL(urls, href());
    
    // Walk the URLs linked by the linked-to stylesheet.
    if (CSSStyleSheet* styleSheet = const_cast<HTMLLinkElement*>(this)->sheet())
        styleSheet->contents()->addSubresourceStyleURLs(urls);
}

DOMSettableTokenList* HTMLLinkElement::sizes() const
{
    return m_sizes.get();
}

void HTMLLinkElement::setSizes(const String& value)
{
    m_sizes->setValue(value);
}


PassRefPtr<LinkStyle> LinkStyle::create(HTMLLinkElement* owner)
{
    return adoptRef(new LinkStyle(owner));
}

LinkStyle::LinkStyle(HTMLLinkElement* owner)
    : LinkResource(owner)
    , m_disabledState(Unset)
    , m_pendingSheetType(None)
    , m_loading(false)
    , m_firedLoad(false)
    , m_loadedSheet(false)
{
}

LinkStyle::~LinkStyle()
{
    if (m_sheet)
        m_sheet->clearOwnerNode();

    if (m_cachedSheet)
        m_cachedSheet->removeClient(this);
}

Document* LinkStyle::document()
{
    return m_owner->document();
}

void LinkStyle::setCSSStyleSheet(const String& href, const KURL& baseURL, const String& charset, const CachedCSSStyleSheet* cachedStyleSheet)
{
    if (!m_owner->inDocument()) {
        ASSERT(!m_sheet);
        return;

    }
    // Completing the sheet load may cause scripts to execute.
    RefPtr<Node> protector(m_owner);

    CSSParserContext parserContext(m_owner->document(), baseURL, charset);

    if (RefPtr<StyleSheetContents> restoredSheet = const_cast<CachedCSSStyleSheet*>(cachedStyleSheet)->restoreParsedStyleSheet(parserContext)) {
        ASSERT(restoredSheet->isCacheable());
        ASSERT(!restoredSheet->isLoading());

        m_sheet = CSSStyleSheet::create(restoredSheet, m_owner);
        m_sheet->setMediaQueries(MediaQuerySet::create(m_owner->media()));
        m_sheet->setTitle(m_owner->title());

        m_loading = false;
        sheetLoaded();
        notifyLoadedSheetAndAllCriticalSubresources(false);
        return;
    }

    RefPtr<StyleSheetContents> styleSheet = StyleSheetContents::create(href, parserContext);

    m_sheet = CSSStyleSheet::create(styleSheet, m_owner);
    m_sheet->setMediaQueries(MediaQuerySet::create(m_owner->media()));
    m_sheet->setTitle(m_owner->title());

    styleSheet->parseAuthorStyleSheet(cachedStyleSheet, m_owner->document()->securityOrigin());

    m_loading = false;
    styleSheet->notifyLoadedSheet(cachedStyleSheet);
    styleSheet->checkLoaded();

    if (styleSheet->isCacheable())
        const_cast<CachedCSSStyleSheet*>(cachedStyleSheet)->saveParsedStyleSheet(styleSheet);
}

bool LinkStyle::sheetLoaded()
{
    if (!styleSheetIsLoading()) {
        removePendingSheet();
        return true;
    }
    return false;
}

void LinkStyle::notifyLoadedSheetAndAllCriticalSubresources(bool errorOccurred)
{
    if (m_firedLoad)
        return;
    m_loadedSheet = !errorOccurred;
    linkLoadEventSender().dispatchEventSoon(m_owner);
    m_firedLoad = true;
}

void LinkStyle::startLoadingDynamicSheet()
{
    ASSERT(m_pendingSheetType < Blocking);
    addPendingSheet(Blocking);
}

void LinkStyle::clearSheet()
{
    ASSERT(m_sheet);
    ASSERT(m_sheet->ownerNode() == m_owner);
    m_sheet->clearOwnerNode();
    m_sheet = 0;
}

bool LinkStyle::styleSheetIsLoading() const
{
    if (m_loading)
        return true;
    if (!m_sheet)
        return false;
    return m_sheet->contents()->isLoading();
}

void LinkStyle::addPendingSheet(PendingSheetType type)
{
    if (type <= m_pendingSheetType)
        return;
    m_pendingSheetType = type;

    if (m_pendingSheetType == NonBlocking)
        return;
    m_owner->document()->styleSheetCollection()->addPendingSheet();
}

void LinkStyle::removePendingSheet(RemovePendingSheetNotificationType notification)
{
    PendingSheetType type = m_pendingSheetType;
    m_pendingSheetType = None;

    if (type == None)
        return;
    if (type == NonBlocking) {
        // Document::removePendingSheet() triggers the style selector recalc for blocking sheets.
        m_owner->document()->styleResolverChanged(RecalcStyleImmediately);
        return;
    }

    m_owner->document()->styleSheetCollection()->removePendingSheet(
        notification == RemovePendingSheetNotifyImmediately
        ? DocumentStyleSheetCollection::RemovePendingSheetNotifyImmediately
        : DocumentStyleSheetCollection::RemovePendingSheetNotifyLater);
}

void LinkStyle::setDisabledState(bool disabled)
{
    LinkStyle::DisabledState oldDisabledState = m_disabledState;
    m_disabledState = disabled ? Disabled : EnabledViaScript;
    if (oldDisabledState != m_disabledState) {
        // If we change the disabled state while the sheet is still loading, then we have to
        // perform three checks:
        if (styleSheetIsLoading()) {
            // Check #1: The sheet becomes disabled while loading.
            if (m_disabledState == Disabled)
                removePendingSheet();

            // Check #2: An alternate sheet becomes enabled while it is still loading.
            if (m_owner->relAttribute().isAlternate() && m_disabledState == EnabledViaScript)
                addPendingSheet(Blocking);

            // Check #3: A main sheet becomes enabled while it was still loading and
            // after it was disabled via script. It takes really terrible code to make this
            // happen (a double toggle for no reason essentially). This happens on
            // virtualplastic.net, which manages to do about 12 enable/disables on only 3
            // sheets. :)
            if (!m_owner->relAttribute().isAlternate() && m_disabledState == EnabledViaScript && oldDisabledState == Disabled)
                addPendingSheet(Blocking);

            // If the sheet is already loading just bail.
            return;
        }

        // Load the sheet, since it's never been loaded before.
        if (!m_sheet && m_disabledState == EnabledViaScript) {
            if (m_owner->shouldProcessStyle())
                process();
        } else
            m_owner->document()->styleResolverChanged(DeferRecalcStyle); // Update the style selector.
    }
}

void LinkStyle::process()
{
    ASSERT(m_owner->shouldProcessStyle());
    String type = m_owner->typeValue().lower();
    LinkRequestBuilder builder(m_owner);

    if (m_owner->relAttribute().iconType() != InvalidIcon && builder.url().isValid() && !builder.url().isEmpty()) {
        if (!m_owner->shouldLoadLink())
            return;
        if (document()->frame())
            document()->frame()->loader()->didChangeIcons(m_owner->relAttribute().iconType());
    }

    if (!m_owner->loadLink(type, builder.url()))
        return;

    if ((m_disabledState != Disabled) && m_owner->relAttribute().isStyleSheet()
        && document()->frame() && builder.url().isValid()) {

        if (m_cachedSheet) {
            removePendingSheet();
            m_cachedSheet->removeClient(this);
            m_cachedSheet = 0;
        }

        if (!m_owner->shouldLoadLink())
            return;

        m_loading = true;

        bool mediaQueryMatches = true;
        if (!m_owner->media().isEmpty()) {
            RefPtr<RenderStyle> documentStyle = StyleResolver::styleForDocument(document());
            RefPtr<MediaQuerySet> media = MediaQuerySet::create(m_owner->media());
            MediaQueryEvaluator evaluator(document()->frame()->view()->mediaType(), document()->frame(), documentStyle.get());
            mediaQueryMatches = evaluator.eval(media.get());
        }

        // Don't hold up render tree construction and script execution on stylesheets
        // that are not needed for the rendering at the moment.
        bool blocking = mediaQueryMatches && !m_owner->isAlternate();
        addPendingSheet(blocking ? Blocking : NonBlocking);

        // Load stylesheets that are not needed for the rendering immediately with low priority.
        CachedResourceRequest request = builder.build(blocking);
        m_cachedSheet = document()->cachedResourceLoader()->requestCSSStyleSheet(request);

        if (m_cachedSheet)
            m_cachedSheet->addClient(this);
        else {
            // The request may have been denied if (for example) the stylesheet is local and the document is remote.
            m_loading = false;
            removePendingSheet();
        }
    } else if (m_sheet) {
        // we no longer contain a stylesheet, e.g. perhaps rel or type was changed
        clearSheet();
        document()->styleResolverChanged(DeferRecalcStyle);
    }
}

void LinkStyle::setSheetTitle(const String& title)
{
    if (m_sheet)
        m_sheet->setTitle(title);
}

void LinkStyle::ownerRemoved()
{
    if (m_sheet)
        clearSheet();

    if (styleSheetIsLoading())
        removePendingSheet(RemovePendingSheetNotifyLater);
}

} // namespace WebCore
