/*
 * Copyright (C) 2007 Nikolas Zimmermann <zimmermann@kde.org>
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

#ifndef SVGElementInstanceList_h
#define SVGElementInstanceList_h

#include "bindings/v8/ScriptWrappable.h"
#include "core/svg/SVGElementInstance.h"
#include "wtf/RefCounted.h"

namespace WebCore {

class SVGElementInstanceList : public RefCounted<SVGElementInstanceList>, public ScriptWrappable {
public:
    static PassRefPtr<SVGElementInstanceList> create(PassRefPtr<SVGElementInstance> root) { return adoptRef(new SVGElementInstanceList(root)); }
    virtual ~SVGElementInstanceList();

    unsigned length() const;
    SVGElementInstance* item(unsigned index);

private:
    SVGElementInstanceList(PassRefPtr<SVGElementInstance>);

    RefPtr<SVGElementInstance> m_rootInstance;
};

} // namespace WebCore

#endif
