/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
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

#ifndef Node_h
#define Node_h

#include "bindings/v8/ScriptWrappable.h"
#include "core/dom/EventTarget.h"
#include "core/dom/MutationObserver.h"
#include "core/dom/SimulatedClickOptions.h"
#include "core/dom/TreeScope.h"
#include "core/editing/EditingBoundary.h"
#include "core/inspector/InspectorCounters.h"
#include "core/page/FocusDirection.h"
#include "core/platform/TreeShared.h"
#include "core/platform/graphics/LayoutRect.h"
#include "core/rendering/style/RenderStyleConstants.h"
#include "weborigin/KURLHash.h"
#include <wtf/Forward.h>
#include <wtf/ListHashSet.h>
#include <wtf/text/AtomicString.h>

// This needs to be here because Document.h also depends on it.
#define DUMP_NODE_STATISTICS 0

namespace WebCore {

class Attribute;
class ClassNodeList;
class ContainerNode;
class DOMSettableTokenList;
class Document;
class Element;
class Event;
class EventContext;
class EventDispatchMediator;
class EventListener;
class FloatPoint;
class Frame;
class HTMLInputElement;
class IntRect;
class KeyboardEvent;
class NSResolver;
class NamedNodeMap;
class NameNodeList;
class NodeList;
class NodeListsNodeData;
class NodeRareData;
class NodeRenderingContext;
class PlatformGestureEvent;
class PlatformKeyboardEvent;
class PlatformMouseEvent;
class PlatformWheelEvent;
class QualifiedName;
class RadioNodeList;
class RegisteredEventListener;
class RenderArena;
class RenderBox;
class RenderBoxModelObject;
class RenderObject;
class RenderStyle;
class ShadowRoot;
class TagNodeList;
class TouchEvent;

typedef int ExceptionCode;

const int nodeStyleChangeShift = 15;

// SyntheticStyleChange means that we need to go through the entire style change logic even though
// no style property has actually changed. It is used to restructure the tree when, for instance,
// RenderLayers are created or destroyed due to animation changes.
enum StyleChangeType {
    NoStyleChange = 0,
    InlineStyleChange = 1 << nodeStyleChangeShift,
    SyntheticStyleChange = 2 << nodeStyleChangeShift,
    FullStyleChange = 3 << nodeStyleChangeShift,
};

class NodeRareDataBase {
public:
    RenderObject* renderer() const { return m_renderer; }
    void setRenderer(RenderObject* renderer) { m_renderer = renderer; }

protected:
    NodeRareDataBase(RenderObject* renderer)
        : m_renderer(renderer)
    { }

private:
    RenderObject* m_renderer;
};

enum AttachBehavior {
    AttachNow,
    AttachLazily,
};

class Node : public EventTarget, public ScriptWrappable, public TreeShared<Node> {
    friend class Document;
    friend class TreeScope;
    friend class TreeScopeAdopter;

public:
    enum NodeType {
        ELEMENT_NODE = 1,
        ATTRIBUTE_NODE = 2,
        TEXT_NODE = 3,
        CDATA_SECTION_NODE = 4,
        ENTITY_NODE = 6,
        PROCESSING_INSTRUCTION_NODE = 7,
        COMMENT_NODE = 8,
        DOCUMENT_NODE = 9,
        DOCUMENT_TYPE_NODE = 10,
        DOCUMENT_FRAGMENT_NODE = 11,
        NOTATION_NODE = 12,
        XPATH_NAMESPACE_NODE = 13,
    };

    // EntityReference nodes are deprecated and impossible to create in WebKit.
    // We want Node.ENTITY_REFERNCE_NODE to exist in JS and this enum, makes the bindings
    // generation not complain about ENTITY_REFERENCE_NODE being missing from the implementation
    // while not requiring all switch(NodeType) blocks to include this deprecated constant.
    enum DeprecatedNodeType {
        ENTITY_REFERENCE_NODE = 5
    };

    enum DocumentPosition {
        DOCUMENT_POSITION_EQUIVALENT = 0x00,
        DOCUMENT_POSITION_DISCONNECTED = 0x01,
        DOCUMENT_POSITION_PRECEDING = 0x02,
        DOCUMENT_POSITION_FOLLOWING = 0x04,
        DOCUMENT_POSITION_CONTAINS = 0x08,
        DOCUMENT_POSITION_CONTAINED_BY = 0x10,
        DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC = 0x20,
    };

    static void init();
    static void shutdown();
#if ENABLE(PARTITION_ALLOC)
    // All Nodes are placed in their own heap partition for security.
    // See http://crbug.com/246860 for detail.
    void* operator new(size_t);
    void operator delete(void*);
#endif

    static bool isSupported(const String& feature, const String& version);

    static void startIgnoringLeaks();
    static void stopIgnoringLeaks();

    static void dumpStatistics();

    enum StyleChange { NoChange, NoInherit, Inherit, Detach, Force };    
    static StyleChange diff(const RenderStyle*, const RenderStyle*, Document*);

    virtual ~Node();
    void willBeDeletedFrom(Document*);

    // DOM methods & attributes for Node

    bool hasTagName(const QualifiedName&) const;
    bool hasLocalName(const AtomicString&) const;
    virtual String nodeName() const = 0;
    virtual String nodeValue() const;
    virtual void setNodeValue(const String&, ExceptionCode&);
    virtual NodeType nodeType() const = 0;
    ContainerNode* parentNode() const;
    Element* parentElement() const;
    Node* previousSibling() const { return m_previous; }
    Node* nextSibling() const { return m_next; }
    PassRefPtr<NodeList> childNodes();
    Node* firstChild() const;
    Node* lastChild() const;
    bool hasAttributes() const;
    NamedNodeMap* attributes() const;

    // ChildNode interface API
    Element* previousElementSibling() const;
    Element* nextElementSibling() const;
    void remove(ExceptionCode&);

    Node* pseudoAwareNextSibling() const;
    Node* pseudoAwarePreviousSibling() const;
    Node* pseudoAwareFirstChild() const;
    Node* pseudoAwareLastChild() const;

    virtual KURL baseURI() const;
    
    void getSubresourceURLs(ListHashSet<KURL>&) const;

    // These should all actually return a node, but this is only important for language bindings,
    // which will already know and hold a ref on the right node to return. Returning bool allows
    // these methods to be more efficient since they don't need to return a ref
    bool insertBefore(PassRefPtr<Node> newChild, Node* refChild, ExceptionCode&, AttachBehavior = AttachNow);
    bool replaceChild(PassRefPtr<Node> newChild, Node* oldChild, ExceptionCode&, AttachBehavior = AttachNow);
    bool removeChild(Node* child, ExceptionCode&);
    bool appendChild(PassRefPtr<Node> newChild, ExceptionCode&, AttachBehavior = AttachNow);

    bool hasChildNodes() const { return firstChild(); }
    virtual PassRefPtr<Node> cloneNode(bool deep) = 0;
    virtual const AtomicString& localName() const;
    virtual const AtomicString& namespaceURI() const;
    virtual const AtomicString& prefix() const;
    virtual void setPrefix(const AtomicString&, ExceptionCode&);
    void normalize();

    bool isSameNode(Node* other) const { return this == other; }
    bool isEqualNode(Node*) const;
    bool isDefaultNamespace(const AtomicString& namespaceURI) const;
    String lookupPrefix(const AtomicString& namespaceURI) const;
    String lookupNamespaceURI(const String& prefix) const;
    String lookupNamespacePrefix(const AtomicString& namespaceURI, const Element* originalElement) const;
    
    String textContent(bool convertBRsToNewlines = false) const;
    void setTextContent(const String&, ExceptionCode&);
    
    Node* lastDescendant() const;
    Node* firstDescendant() const;

    // Other methods (not part of DOM)

    bool isElementNode() const { return getFlag(IsElementFlag); }
    bool isContainerNode() const { return getFlag(IsContainerFlag); }
    bool isTextNode() const { return getFlag(IsTextFlag); }
    bool isHTMLElement() const { return getFlag(IsHTMLFlag); }
    bool isSVGElement() const { return getFlag(IsSVGFlag); }

    bool isPseudoElement() const { return pseudoId() != NOPSEUDO; }
    bool isBeforePseudoElement() const { return pseudoId() == BEFORE; }
    bool isAfterPseudoElement() const { return pseudoId() == AFTER; }
    PseudoId pseudoId() const { return (isElementNode() && hasCustomStyleCallbacks()) ? customPseudoId() : NOPSEUDO; }

    bool isCustomElement() const { return getFlag(IsCustomElement); }
    void setIsCustomElement();

    virtual bool isMediaControlElement() const { return false; }
    virtual bool isMediaControls() const { return false; }
    virtual bool isWebVTTElement() const { return false; }
    bool isStyledElement() const { return getFlag(IsStyledElementFlag); }
    virtual bool isAttributeNode() const { return false; }
    virtual bool isCharacterDataNode() const { return false; }
    virtual bool isFrameOwnerElement() const { return false; }
    virtual bool isPluginElement() const { return false; }

    bool isDocumentNode() const;
    bool isTreeScope() const { return treeScope()->rootNode() == this; }
    bool isDocumentFragment() const { return getFlag(IsDocumentFragmentFlag); }
    bool isShadowRoot() const { return isDocumentFragment() && isTreeScope(); }
    bool isInsertionPoint() const { return getFlag(IsInsertionPointFlag); }

    bool inNamedFlow() const { return getFlag(InNamedFlowFlag); }
    bool hasCustomStyleCallbacks() const { return getFlag(HasCustomStyleCallbacksFlag); }

    bool hasSyntheticAttrChildNodes() const { return getFlag(HasSyntheticAttrChildNodesFlag); }
    void setHasSyntheticAttrChildNodes(bool flag) { setFlag(flag, HasSyntheticAttrChildNodesFlag); }

    // If this node is in a shadow tree, returns its shadow host. Otherwise, returns 0.
    Element* shadowHost() const;
    // If this node is in a shadow tree, returns its shadow host. Otherwise, returns this.
    // Deprecated. Should use shadowHost() and check the return value.
    Node* deprecatedShadowAncestorNode() const;
    ShadowRoot* containingShadowRoot() const;
    ShadowRoot* youngestShadowRoot() const;

    // Returns 0, a child of ShadowRoot, or a legacy shadow root.
    Node* nonBoundaryShadowTreeRootNode();

    // Node's parent, shadow tree host.
    ContainerNode* parentOrShadowHostNode() const;
    Element* parentOrShadowHostElement() const;
    void setParentOrShadowHostNode(ContainerNode*);
    Node* highestAncestor() const;

    // Use when it's guaranteed to that shadowHost is 0.
    ContainerNode* parentNodeGuaranteedHostFree() const;
    // Returns the parent node, but 0 if the parent node is a ShadowRoot.
    ContainerNode* nonShadowBoundaryParentNode() const;

    bool selfOrAncestorHasDirAutoAttribute() const { return getFlag(SelfOrAncestorHasDirAutoFlag); }
    void setSelfOrAncestorHasDirAutoAttribute(bool flag) { setFlag(flag, SelfOrAncestorHasDirAutoFlag); }

    // Returns the enclosing event parent node (or self) that, when clicked, would trigger a navigation.
    Node* enclosingLinkEventParentOrSelf();

    bool isBlockFlowElement() const;

    // These low-level calls give the caller responsibility for maintaining the integrity of the tree.
    void setPreviousSibling(Node* previous) { m_previous = previous; }
    void setNextSibling(Node* next) { m_next = next; }

    virtual bool canContainRangeEndPoint() const { return false; }

    // FIXME: These two functions belong in editing -- "atomic node" is an editing concept.
    Node* previousNodeConsideringAtomicNodes() const;
    Node* nextNodeConsideringAtomicNodes() const;
    
    // Returns the next leaf node or 0 if there are no more.
    // Delivers leaf nodes as if the whole DOM tree were a linear chain of its leaf nodes.
    // Uses an editing-specific concept of what a leaf node is, and should probably be moved
    // out of the Node class into an editing-specific source file.
    Node* nextLeafNode() const;

    // Returns the previous leaf node or 0 if there are no more.
    // Delivers leaf nodes as if the whole DOM tree were a linear chain of its leaf nodes.
    // Uses an editing-specific concept of what a leaf node is, and should probably be moved
    // out of the Node class into an editing-specific source file.
    Node* previousLeafNode() const;

    // enclosingBlockFlowElement() is deprecated. Use enclosingBlock instead.
    Element* enclosingBlockFlowElement() const;

    bool isRootEditableElement() const;
    Element* rootEditableElement() const;
    Element* rootEditableElement(EditableType) const;

    bool inSameContainingBlockFlowElement(Node*);

    // Called by the parser when this element's close tag is reached,
    // signaling that all child tags have been parsed and added.
    // This is needed for <applet> and <object> elements, which can't lay themselves out
    // until they know all of their nested <param>s. [Radar 3603191, 4040848].
    // Also used for script elements and some SVG elements for similar purposes,
    // but making parsing a special case in this respect should be avoided if possible.
    virtual void finishParsingChildren() { }
    virtual void beginParsingChildren() { }

    // For <link> and <style> elements.
    virtual bool sheetLoaded() { return true; }
    virtual void notifyLoadedSheetAndAllCriticalSubresources(bool /* error loading subresource */) { }
    virtual void startLoadingDynamicSheet() { ASSERT_NOT_REACHED(); }

    bool hasName() const { return !isTextNode() && getFlag(HasNameOrIsEditingTextFlag); }
    bool hasID() const;
    bool hasClass() const;

    bool isUserActionElement() const { return getFlag(IsUserActionElement); }
    void setUserActionElement(bool flag) { setFlag(flag, IsUserActionElement); }

    bool active() const { return isUserActionElement() && isUserActionElementActive(); }
    bool inActiveChain() const { return isUserActionElement() && isUserActionElementInActiveChain(); }
    bool hovered() const { return isUserActionElement() && isUserActionElementHovered(); }
    bool focused() const { return isUserActionElement() && isUserActionElementFocused(); }

    bool attached() const { return getFlag(IsAttachedFlag); }
    void setAttached() { setFlag(IsAttachedFlag); }
    bool needsStyleRecalc() const { return styleChangeType() != NoStyleChange; }
    StyleChangeType styleChangeType() const { return static_cast<StyleChangeType>(m_nodeFlags & StyleChangeMask); }
    bool childNeedsStyleRecalc() const { return getFlag(ChildNeedsStyleRecalcFlag); }
    bool isLink() const { return getFlag(IsLinkFlag); }
    bool isEditingText() const { return isTextNode() && getFlag(HasNameOrIsEditingTextFlag); }

    void setHasName(bool f) { ASSERT(!isTextNode()); setFlag(f, HasNameOrIsEditingTextFlag); }
    void setChildNeedsStyleRecalc() { setFlag(ChildNeedsStyleRecalcFlag); }
    void clearChildNeedsStyleRecalc() { clearFlag(ChildNeedsStyleRecalcFlag); }

    void setNeedsStyleRecalc(StyleChangeType changeType = FullStyleChange);
    void clearNeedsStyleRecalc() { m_nodeFlags &= ~StyleChangeMask; }

    void setIsLink(bool f) { setFlag(f, IsLinkFlag); }
    void setIsLink() { setFlag(IsLinkFlag); }
    void clearIsLink() { clearFlag(IsLinkFlag); }

    void setInNamedFlow() { setFlag(InNamedFlowFlag); }
    void clearInNamedFlow() { clearFlag(InNamedFlowFlag); }

    bool hasScopedHTMLStyleChild() const { return getFlag(HasScopedHTMLStyleChildFlag); }
    void setHasScopedHTMLStyleChild(bool flag) { setFlag(flag, HasScopedHTMLStyleChildFlag); }

    bool hasEventTargetData() const { return getFlag(HasEventTargetDataFlag); }
    void setHasEventTargetData(bool flag) { setFlag(flag, HasEventTargetDataFlag); }

    bool isV8CollectableDuringMinorGC() const { return getFlag(V8CollectableDuringMinorGCFlag); }
    void setV8CollectableDuringMinorGC(bool flag) { setFlag(flag, V8CollectableDuringMinorGCFlag); }

    enum ShouldSetAttached {
        SetAttached,
        DoNotSetAttached
    };
    void lazyAttach(ShouldSetAttached = SetAttached);
    void lazyReattach(ShouldSetAttached = SetAttached);

    virtual void setFocus(bool flag);
    virtual void setActive(bool flag = true, bool pause = false);
    virtual void setHovered(bool flag = true);

    virtual short tabIndex() const;

    // Whether this kind of node can receive focus by default. Most nodes are
    // not focusable but some elements, such as form controls and links, are.
    virtual bool supportsFocus() const;
    // Whether the node can actually be focused.
    virtual bool isFocusable() const;
    virtual bool isKeyboardFocusable(KeyboardEvent*) const;
    virtual bool isMouseFocusable() const;
    virtual Node* focusDelegate();

    enum UserSelectAllTreatment {
        UserSelectAllDoesNotAffectEditability,
        UserSelectAllIsAlwaysNonEditable
    };
    bool isContentEditable(UserSelectAllTreatment = UserSelectAllDoesNotAffectEditability);
    bool isContentRichlyEditable();

    bool rendererIsEditable(EditableType editableType = ContentIsEditable, UserSelectAllTreatment treatment = UserSelectAllIsAlwaysNonEditable) const
    {
        switch (editableType) {
        case ContentIsEditable:
            return rendererIsEditable(Editable, treatment);
        case HasEditableAXRole:
            return isEditableToAccessibility(Editable);
        }
        ASSERT_NOT_REACHED();
        return false;
    }

    bool rendererIsRichlyEditable(EditableType editableType = ContentIsEditable) const
    {
        switch (editableType) {
        case ContentIsEditable:
            return rendererIsEditable(RichlyEditable, UserSelectAllIsAlwaysNonEditable);
        case HasEditableAXRole:
            return isEditableToAccessibility(RichlyEditable);
        }
        ASSERT_NOT_REACHED();
        return false;
    }

    virtual bool shouldUseInputMethod();
    virtual LayoutRect boundingBox() const;
    IntRect pixelSnappedBoundingBox() const { return pixelSnappedIntRect(boundingBox()); }
    LayoutRect renderRect(bool* isReplaced);
    IntRect pixelSnappedRenderRect(bool* isReplaced) { return pixelSnappedIntRect(renderRect(isReplaced)); }

    // Returns true if the node has a non-empty bounding box in layout.
    // This does not 100% guarantee the user can see it, but is pretty close.
    // Note: This method only works properly after layout has occurred.
    bool hasNonEmptyBoundingBox() const;

    unsigned nodeIndex() const;

    // Returns the DOM ownerDocument attribute. This method never returns NULL, except in the case 
    // of (1) a Document node or (2) a DocumentType node that is not used with any Document yet. 
    Document* ownerDocument() const;

    // Returns the document associated with this node. This method never returns NULL, except in the case 
    // of a DocumentType node that is not used with any Document yet. A Document node returns itself.
    Document* document() const
    {
        ASSERT(this);
        // FIXME: below ASSERT is useful, but prevents the use of document() in the constructor or destructor
        // due to the virtual function call to nodeType().
        ASSERT(documentInternal() || (nodeType() == DOCUMENT_TYPE_NODE && !inDocument()));
        return documentInternal();
    }

    TreeScope* treeScope() const { return m_treeScope; }

    // Returns true if this node is associated with a document and is in its associated document's
    // node tree, false otherwise.
    bool inDocument() const 
    { 
        ASSERT(documentInternal() || !getFlag(InDocumentFlag));
        return getFlag(InDocumentFlag);
    }
    bool isInShadowTree() const { return getFlag(IsInShadowTreeFlag); }
    bool isInTreeScope() const { return getFlag(static_cast<NodeFlags>(InDocumentFlag | IsInShadowTreeFlag)); }

    bool isDocumentTypeNode() const { return nodeType() == DOCUMENT_TYPE_NODE; }
    virtual bool childTypeAllowed(NodeType) const { return false; }
    unsigned childNodeCount() const;
    Node* childNode(unsigned index) const;

    void checkSetPrefix(const AtomicString& prefix, ExceptionCode&);
    bool isDescendantOf(const Node*) const;
    bool contains(const Node*) const;
    bool containsIncludingShadowDOM(const Node*) const;
    bool containsIncludingHostElements(const Node*) const;

    // Used to determine whether range offsets use characters or node indices.
    virtual bool offsetInCharacters() const;
    // Number of DOM 16-bit units contained in node. Note that rendered text length can be different - e.g. because of
    // css-transform:capitalize breaking up precomposed characters and ligatures.
    virtual int maxCharacterOffset() const;

    // Whether or not a selection can be started in this object
    virtual bool canStartSelection() const;

    // Getting points into and out of screen space
    FloatPoint convertToPage(const FloatPoint&) const;
    FloatPoint convertFromPage(const FloatPoint&) const;

    // -----------------------------------------------------------------------------
    // Integration with rendering tree

    // As renderer() includes a branch you should avoid calling it repeatedly in hot code paths.
    RenderObject* renderer() const { return hasRareData() ? m_data.m_rareData->renderer() : m_data.m_renderer; };
    void setRenderer(RenderObject* renderer)
    {
        if (hasRareData())
            m_data.m_rareData->setRenderer(renderer);
        else
            m_data.m_renderer = renderer;
    }

    // Use these two methods with caution.
    RenderBox* renderBox() const;
    RenderBoxModelObject* renderBoxModelObject() const;

    struct AttachContext {
        RenderStyle* resolvedStyle;
        bool performingReattach;

        AttachContext() : resolvedStyle(0), performingReattach(false) { }
    };

    // Attaches this node to the rendering tree. This calculates the style to be applied to the node and creates an
    // appropriate RenderObject which will be inserted into the tree (except when the style has display: none). This
    // makes the node visible in the FrameView.
    virtual void attach(const AttachContext& = AttachContext());

    // Detaches the node from the rendering tree, making it invisible in the rendered view. This method will remove
    // the node's rendering object from the rendering tree and delete it.
    virtual void detach(const AttachContext& = AttachContext());

#ifndef NDEBUG
    bool inDetach() const;
#endif

    void reattach(const AttachContext& = AttachContext());
    void lazyReattachIfAttached();
    ContainerNode* parentNodeForRenderingAndStyle();
    
    // Wrapper for nodes that don't have a renderer, but still cache the style (like HTMLOptionElement).
    RenderStyle* renderStyle() const;

    RenderStyle* computedStyle(PseudoId pseudoElementSpecifier = NOPSEUDO) { return virtualComputedStyle(pseudoElementSpecifier); }

    // -----------------------------------------------------------------------------
    // Notification of document structure changes (see ContainerNode.h for more notification methods)
    //
    // At first, WebKit notifies the node that it has been inserted into the document. This is called during document parsing, and also
    // when a node is added through the DOM methods insertBefore(), appendChild() or replaceChild(). The call happens _after_ the node has been added to the tree.
    // This is similar to the DOMNodeInsertedIntoDocument DOM event, but does not require the overhead of event
    // dispatching.
    //
    // WebKit notifies this callback regardless if the subtree of the node is a document tree or a floating subtree.
    // Implementation can determine the type of subtree by seeing insertionPoint->inDocument().
    // For a performance reason, notifications are delivered only to ContainerNode subclasses if the insertionPoint is out of document.
    //
    // There are another callback named didNotifyDescendantInsertions(), which is called after all the descendant is notified.
    // Only a few subclasses actually need this. To utilize this, the node should return InsertionShouldCallDidNotifyDescendantInsertions
    // from insrtedInto().
    //
    enum InsertionNotificationRequest {
        InsertionDone,
        InsertionShouldCallDidNotifySubtreeInsertions
    };

    virtual InsertionNotificationRequest insertedInto(ContainerNode* insertionPoint);
    virtual void didNotifySubtreeInsertions(ContainerNode*) { }

    // Notifies the node that it is no longer part of the tree.
    //
    // This is a dual of insertedInto(), and is similar to the DOMNodeRemovedFromDocument DOM event, but does not require the overhead of event
    // dispatching, and is called _after_ the node is removed from the tree.
    //
    virtual void removedFrom(ContainerNode* insertionPoint);

#ifndef NDEBUG
    String debugName() const;

    virtual void formatForDebugger(char* buffer, unsigned length) const;

    void showNode(const char* prefix = "") const;
    void showTreeForThis() const;
    void showNodePathForThis() const;
    void showTreeAndMark(const Node* markedNode1, const char* markedLabel1, const Node* markedNode2 = 0, const char* markedLabel2 = 0) const;
    void showTreeForThisAcrossFrame() const;
#endif

    void invalidateNodeListCachesInAncestors(const QualifiedName* attrName = 0, Element* attributeOwnerElement = 0);
    NodeListsNodeData* nodeLists();
    void clearNodeLists();

    PassRefPtr<NodeList> getElementsByTagName(const AtomicString&);
    PassRefPtr<NodeList> getElementsByTagNameNS(const AtomicString& namespaceURI, const AtomicString& localName);
    PassRefPtr<NodeList> getElementsByName(const String& elementName);
    PassRefPtr<NodeList> getElementsByClassName(const String& classNames);
    PassRefPtr<RadioNodeList> radioNodeList(const AtomicString&);

    virtual bool willRespondToMouseMoveEvents();
    virtual bool willRespondToMouseClickEvents();
    virtual bool willRespondToTouchEvents();

    PassRefPtr<Element> querySelector(const AtomicString& selectors, ExceptionCode&);
    PassRefPtr<NodeList> querySelectorAll(const AtomicString& selectors, ExceptionCode&);

    unsigned short compareDocumentPosition(const Node*) const;

    enum ShadowTreesTreatment {
        TreatShadowTreesAsDisconnected,
        TreatShadowTreesAsComposed
    };

    unsigned short compareDocumentPositionInternal(const Node*, ShadowTreesTreatment) const;

    virtual Node* toNode();

    virtual const AtomicString& interfaceName() const;
    virtual ScriptExecutionContext* scriptExecutionContext() const;

    virtual bool addEventListener(const AtomicString& eventType, PassRefPtr<EventListener>, bool useCapture);
    virtual bool removeEventListener(const AtomicString& eventType, EventListener*, bool useCapture);

    // Handlers to do/undo actions on the target node before an event is dispatched to it and after the event
    // has been dispatched.  The data pointer is handed back by the preDispatch and passed to postDispatch.
    virtual void* preDispatchEventHandler(Event*) { return 0; }
    virtual void postDispatchEventHandler(Event*, void* /*dataFromPreDispatch*/) { }

    using EventTarget::dispatchEvent;
    virtual bool dispatchEvent(PassRefPtr<Event>) OVERRIDE;

    void dispatchScopedEvent(PassRefPtr<Event>);
    void dispatchScopedEventDispatchMediator(PassRefPtr<EventDispatchMediator>);

    virtual void handleLocalEvents(Event*);

    void dispatchSubtreeModifiedEvent();
    bool dispatchDOMActivateEvent(int detail, PassRefPtr<Event> underlyingEvent);
    void dispatchFocusInEvent(const AtomicString& eventType, PassRefPtr<Node> oldFocusedNode);
    void dispatchFocusOutEvent(const AtomicString& eventType, PassRefPtr<Node> newFocusedNode);

    bool dispatchKeyEvent(const PlatformKeyboardEvent&);
    bool dispatchWheelEvent(const PlatformWheelEvent&);
    bool dispatchMouseEvent(const PlatformMouseEvent&, const AtomicString& eventType, int clickCount = 0, Node* relatedTarget = 0);
    bool dispatchGestureEvent(const PlatformGestureEvent&);
    bool dispatchTouchEvent(PassRefPtr<TouchEvent>);

    void dispatchSimulatedClick(Event* underlyingEvent, SimulatedClickMouseEventOptions = SendNoEvents, SimulatedClickVisualOptions = ShowPressedLook);

    virtual bool dispatchBeforeLoadEvent(const String& sourceURL);
    virtual void dispatchFocusEvent(PassRefPtr<Node> oldFocusedNode, FocusDirection);
    virtual void dispatchBlurEvent(PassRefPtr<Node> newFocusedNode);
    virtual void dispatchChangeEvent();
    virtual void dispatchInputEvent();

    // Perform the default action for an event.
    virtual void defaultEventHandler(Event*);

    using TreeShared<Node>::ref;
    using TreeShared<Node>::deref;

    virtual EventTargetData* eventTargetData();
    virtual EventTargetData* ensureEventTargetData();

    void getRegisteredMutationObserversOfType(HashMap<MutationObserver*, MutationRecordDeliveryOptions>&, MutationObserver::MutationType, const QualifiedName* attributeName);
    void registerMutationObserver(MutationObserver*, MutationObserverOptions, const HashSet<AtomicString>& attributeFilter);
    void unregisterMutationObserver(MutationObserverRegistration*);
    void registerTransientMutationObserver(MutationObserverRegistration*);
    void unregisterTransientMutationObserver(MutationObserverRegistration*);
    void notifyMutationObserversNodeWillDetach();

    virtual void registerScopedHTMLStyleChild();
    virtual void unregisterScopedHTMLStyleChild();
    size_t numberOfScopedHTMLStyleChildren() const;

    virtual void reportMemoryUsage(MemoryObjectInfo*) const;

    void textRects(Vector<IntRect>&) const;

    unsigned connectedSubframeCount() const;
    void incrementConnectedSubframeCount(unsigned amount = 1);
    void decrementConnectedSubframeCount(unsigned amount = 1);
    void updateAncestorConnectedSubframeCountForRemoval() const;
    void updateAncestorConnectedSubframeCountForInsertion() const;

private:
    enum NodeFlags {
        IsTextFlag = 1,
        IsContainerFlag = 1 << 1,
        IsElementFlag = 1 << 2,
        IsStyledElementFlag = 1 << 3,
        IsHTMLFlag = 1 << 4,
        IsSVGFlag = 1 << 5,
        IsAttachedFlag = 1 << 6,
        ChildNeedsStyleRecalcFlag = 1 << 7,
        InDocumentFlag = 1 << 8,
        IsLinkFlag = 1 << 9,
        IsUserActionElement = 1 << 10,
        HasRareDataFlag = 1 << 11,
        IsDocumentFragmentFlag = 1 << 12,

        // These bits are used by derived classes, pulled up here so they can
        // be stored in the same memory word as the Node bits above.
        IsParsingChildrenFinishedFlag = 1 << 13, // Element
        HasSVGRareDataFlag = 1 << 14, // SVGElement

        StyleChangeMask = 1 << nodeStyleChangeShift | 1 << (nodeStyleChangeShift + 1),

        SelfOrAncestorHasDirAutoFlag = 1 << 17,

        HasNameOrIsEditingTextFlag = 1 << 18,

        InNamedFlowFlag = 1 << 19,
        HasSyntheticAttrChildNodesFlag = 1 << 20,
        HasCustomStyleCallbacksFlag = 1 << 21,
        HasScopedHTMLStyleChildFlag = 1 << 22,
        HasEventTargetDataFlag = 1 << 23,
        V8CollectableDuringMinorGCFlag = 1 << 24,
        IsInsertionPointFlag = 1 << 25,
        IsInShadowTreeFlag = 1 << 26,
        IsCustomElement = 1 << 27,

        DefaultNodeFlags = IsParsingChildrenFinishedFlag
    };

    // 4 bits remaining

    bool getFlag(NodeFlags mask) const { return m_nodeFlags & mask; }
    void setFlag(bool f, NodeFlags mask) const { m_nodeFlags = (m_nodeFlags & ~mask) | (-(int32_t)f & mask); } 
    void setFlag(NodeFlags mask) const { m_nodeFlags |= mask; } 
    void clearFlag(NodeFlags mask) const { m_nodeFlags &= ~mask; } 

protected:
    enum ConstructionType { 
        CreateOther = DefaultNodeFlags,
        CreateText = DefaultNodeFlags | IsTextFlag,
        CreateContainer = DefaultNodeFlags | IsContainerFlag, 
        CreateElement = CreateContainer | IsElementFlag, 
        CreatePseudoElement =  CreateElement | InDocumentFlag,
        CreateShadowRoot = CreateContainer | IsDocumentFragmentFlag | IsInShadowTreeFlag,
        CreateDocumentFragment = CreateContainer | IsDocumentFragmentFlag,
        CreateStyledElement = CreateElement | IsStyledElementFlag, 
        CreateHTMLElement = CreateStyledElement | IsHTMLFlag,
        CreateSVGElement = CreateStyledElement | IsSVGFlag,
        CreateDocument = CreateContainer | InDocumentFlag,
        CreateInsertionPoint = CreateHTMLElement | IsInsertionPointFlag,
        CreateEditingText = CreateText | HasNameOrIsEditingTextFlag,
    };

    Node(TreeScope* treeScope, ConstructionType type)
        : m_nodeFlags(type)
        , m_parentOrShadowHostNode(0)
        , m_treeScope(treeScope)
        , m_previous(0)
        , m_next(0)
    {
        ScriptWrappable::init(this);
        if (!m_treeScope)
            m_treeScope = TreeScope::noDocumentInstance();
        m_treeScope->guardRef();

#if !defined(NDEBUG) || (defined(DUMP_NODE_STATISTICS) && DUMP_NODE_STATISTICS)
        trackForDebugging();
#endif
        InspectorCounters::incrementCounter(InspectorCounters::NodeCounter);
    }

    virtual void didMoveToNewDocument(Document* oldDocument);
    
    virtual void addSubresourceAttributeURLs(ListHashSet<KURL>&) const { }

    bool hasRareData() const { return getFlag(HasRareDataFlag); }

    NodeRareData* rareData() const;
    NodeRareData* ensureRareData();
    void clearRareData();

    void clearEventTargetData();

    void setHasCustomStyleCallbacks() { setFlag(true, HasCustomStyleCallbacksFlag); }

    Document* documentInternal() const { return treeScope()->documentScope(); }
    void setTreeScope(TreeScope* scope) { m_treeScope = scope; }

private:
    friend class TreeShared<Node>;

    virtual PseudoId customPseudoId() const
    {
        ASSERT(hasCustomStyleCallbacks());
        return NOPSEUDO;
    }

    void removedLastRef();
    bool hasTreeSharedParent() const { return !!parentOrShadowHostNode(); }

    enum EditableLevel { Editable, RichlyEditable };
    bool rendererIsEditable(EditableLevel, UserSelectAllTreatment = UserSelectAllIsAlwaysNonEditable) const;
    bool isEditableToAccessibility(EditableLevel) const;

    bool isUserActionElementActive() const;
    bool isUserActionElementInActiveChain() const;
    bool isUserActionElementHovered() const;
    bool isUserActionElementFocused() const;

    void setStyleChange(StyleChangeType);

    // Used to share code between lazyAttach and setNeedsStyleRecalc.
    void markAncestorsWithChildNeedsStyleRecalc();

    virtual void refEventTarget();
    virtual void derefEventTarget();

    virtual RenderStyle* nonRendererStyle() const { return 0; }

    virtual RenderStyle* virtualComputedStyle(PseudoId = NOPSEUDO);

    Element* ancestorElement() const;

    void trackForDebugging();

    Vector<OwnPtr<MutationObserverRegistration> >* mutationObserverRegistry();
    HashSet<MutationObserverRegistration*>* transientMutationObserverRegistry();

    mutable uint32_t m_nodeFlags;
    ContainerNode* m_parentOrShadowHostNode;
    TreeScope* m_treeScope;
    Node* m_previous;
    Node* m_next;
    // When a node has rare data we move the renderer into the rare data.
    union DataUnion {
        DataUnion() : m_renderer(0) { }
        RenderObject* m_renderer;
        NodeRareDataBase* m_rareData;
    } m_data;

protected:
    bool isParsingChildrenFinished() const { return getFlag(IsParsingChildrenFinishedFlag); }
    void setIsParsingChildrenFinished() { setFlag(IsParsingChildrenFinishedFlag); }
    void clearIsParsingChildrenFinished() { clearFlag(IsParsingChildrenFinishedFlag); }

    bool hasSVGRareData() const { return getFlag(HasSVGRareDataFlag); }
    void setHasSVGRareData() { setFlag(HasSVGRareDataFlag); }
    void clearHasSVGRareData() { clearFlag(HasSVGRareDataFlag); }
};

// Used in Node::addSubresourceAttributeURLs() and in addSubresourceStyleURLs()
inline void addSubresourceURL(ListHashSet<KURL>& urls, const KURL& url)
{
    if (!url.isNull())
        urls.add(url);
}

inline void Node::setParentOrShadowHostNode(ContainerNode* parent)
{
    ASSERT(isMainThread());
    m_parentOrShadowHostNode = parent;
}

inline ContainerNode* Node::parentOrShadowHostNode() const
{
    ASSERT(isMainThread());
    return m_parentOrShadowHostNode;
}

inline ContainerNode* Node::parentNode() const
{
    return isShadowRoot() ? 0 : parentOrShadowHostNode();
}

inline ContainerNode* Node::parentNodeGuaranteedHostFree() const
{
    ASSERT(!isShadowRoot());
    return parentOrShadowHostNode();
}

inline void Node::reattach(const AttachContext& context)
{
    AttachContext reattachContext(context);
    reattachContext.performingReattach = true;

    if (attached())
        detach(reattachContext);
    attach(reattachContext);
}

inline void Node::lazyReattachIfAttached()
{
    if (attached())
        lazyReattach();
}

inline void Node::lazyReattach(ShouldSetAttached shouldSetAttached)
{
    if (attached())
        detach();
    lazyAttach(shouldSetAttached);
}

// Need a template since ElementShadow is not a Node, but has the style recalc methods.
template<class T>
inline bool shouldRecalcStyle(Node::StyleChange change, const T* node)
{
    return change >= Node::Inherit || node->childNeedsStyleRecalc() || node->needsStyleRecalc();
}

} //namespace

#ifndef NDEBUG
// Outside the WebCore namespace for ease of invocation from gdb.
void showTree(const WebCore::Node*);
void showNodePath(const WebCore::Node*);
#endif

#endif
