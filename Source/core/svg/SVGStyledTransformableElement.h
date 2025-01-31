/*
 * Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
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

#ifndef SVGStyledTransformableElement_h
#define SVGStyledTransformableElement_h

#include "core/svg/SVGAnimatedTransformList.h"
#include "core/svg/SVGStyledLocatableElement.h"
#include "core/svg/SVGTransformable.h"

namespace WebCore {

class AffineTransform;
class Path;

class SVGStyledTransformableElement : public SVGStyledLocatableElement,
                                      public SVGTransformable {
public:
    virtual ~SVGStyledTransformableElement();

    virtual AffineTransform getCTM(StyleUpdateStrategy = AllowStyleUpdate);
    virtual AffineTransform getScreenCTM(StyleUpdateStrategy = AllowStyleUpdate);
    virtual SVGElement* nearestViewportElement() const;
    virtual SVGElement* farthestViewportElement() const;

    virtual AffineTransform localCoordinateSpaceTransform(SVGLocatable::CTMScope mode) const { return SVGTransformable::localCoordinateSpaceTransform(mode); }
    virtual AffineTransform animatedLocalTransform() const;
    virtual AffineTransform* supplementalTransform();

    virtual FloatRect getBBox(StyleUpdateStrategy = AllowStyleUpdate);

    // "base class" methods for all the elements which render as paths
    virtual void toClipPath(Path&);
    virtual RenderObject* createRenderer(RenderArena*, RenderStyle*);

protected:
    SVGStyledTransformableElement(const QualifiedName&, Document*, ConstructionType = CreateSVGElement);

    bool isSupportedAttribute(const QualifiedName&);
    virtual void parseAttribute(const QualifiedName&, const AtomicString&) OVERRIDE;
    virtual void svgAttributeChanged(const QualifiedName&);

    BEGIN_DECLARE_ANIMATED_PROPERTIES(SVGStyledTransformableElement)
        DECLARE_ANIMATED_TRANSFORM_LIST(Transform, transform)
    END_DECLARE_ANIMATED_PROPERTIES

private:
    virtual bool isStyledTransformable() const OVERRIDE { return true; }

    // Used by <animateMotion>
    OwnPtr<AffineTransform> m_supplementalTransform;
};

inline SVGStyledTransformableElement* toSVGStyledTransformableElement(Node* node)
{
    ASSERT_WITH_SECURITY_IMPLICATION(!node || node->isSVGElement());
    ASSERT_WITH_SECURITY_IMPLICATION(!node || toSVGElement(node)->isStyledTransformable());
    return static_cast<SVGStyledTransformableElement*>(node);
}

} // namespace WebCore

#endif // SVGStyledTransformableElement_h
