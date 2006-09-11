/*
    Copyright (C) 2004, 2005 Nikolas Zimmermann <wildfox@kde.org>
                  2004, 2005 Rob Buis <buis@kde.org>

    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KSVG_SVGList_H
#define KSVG_SVGList_H

#ifdef SVG_SUPPORT

#include <wtf/Vector.h>

#include "Shared.h"
#include "FloatPoint.h"
#include "PlatformString.h"

namespace WebCore {

    // TODO: We will probably need to pass ExceptionCodes around here...

    template<class Item>
    class SVGListBase : public Shared<SVGListBase<Item> >
    {
    public:
        SVGListBase() { }
        virtual ~SVGListBase() { }

        virtual Item nullItem() const = 0;

        unsigned int numberOfItems() const { return m_vector.size(); }
        void clear() { m_vector.clear(); }

        Item initialize(Item newItem)
        {
            clear();
            return appendItem(newItem);
        }

        Item getFirst() const
        {
            if (m_vector.isEmpty())
                return nullItem();

            return m_vector.first();
        }

        Item getLast() const
        {
            if (m_vector.isEmpty())
                return nullItem();

            return m_vector.last();
        }

        Item getItem(unsigned int index)
        {
            if (m_vector.size() < index)
                return nullItem();

            return m_vector.at(index);
        }

        const Item getItem(unsigned int index) const
        {
            if (m_vector.size() < index)
                return nullItem();

            return m_vector.at(index);
        }

        Item insertItemBefore(Item newItem, unsigned int index)
        {
            m_vector.insert(index, newItem);
            return newItem;
        }

        Item replaceItem(Item newItem, unsigned int index)
        {
            if (m_vector.size() < index)
                return nullItem();

            m_vector.at(index) = newItem;
            return newItem;
        }

        Item removeItem(unsigned int index)
        {
            if (m_vector.size() < index)
                return nullItem();

            Item item = m_vector.at(index);
            remove(index);
            return item;
        }

        void removeItem(const Item item)
        {
            m_vector.remove(item);
        }

        Item appendItem(Item newItem)
        {
            m_vector.append(newItem);
            return newItem;
        }

    private:
        Vector<Item> m_vector;
    };

    template<class Item>
    class SVGList : public SVGListBase<Item>
    {
    public:
        virtual Item nullItem() const { return 0; }
    };

    // Specialization for String...
    template<>
    class SVGList<String> : public SVGListBase<String>
    {
    public:
        virtual String nullItem() const { return String(); }
    };

    // Specialization for FloatPoint...
    template<>
    class SVGList<FloatPoint> : public SVGListBase<FloatPoint>
    {
    public:
        virtual FloatPoint nullItem() const { return FloatPoint(); }
    };

} // namespace WebCore

#endif // SVG_SUPPORT
#endif // KSVG_SVGList_H

// vim:ts=4:noet
