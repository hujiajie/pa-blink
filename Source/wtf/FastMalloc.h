/*
 *  Copyright (C) 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef WTF_FastMalloc_h
#define WTF_FastMalloc_h

#include <new>
#include <stdlib.h>

#include "wtf/Platform.h"
#include "wtf/PossiblyNull.h"
#include "wtf/WTFExport.h"

namespace WTF {

    // These functions call CRASH() if an allocation fails.
    WTF_EXPORT void* fastMalloc(size_t);
    WTF_EXPORT void* fastZeroedMalloc(size_t);
    WTF_EXPORT void* fastCalloc(size_t numElements, size_t elementSize);
    WTF_EXPORT void* fastRealloc(void*, size_t);
    WTF_EXPORT char* fastStrDup(const char*);
    WTF_EXPORT size_t fastMallocGoodSize(size_t);

    struct TryMallocReturnValue {
        TryMallocReturnValue(void* data)
            : m_data(data)
        {
        }
        TryMallocReturnValue(const TryMallocReturnValue& source)
            : m_data(source.m_data)
        {
            source.m_data = 0;
        }
        ~TryMallocReturnValue() { ASSERT(!m_data); }
        template <typename T> bool getValue(T& data) WARN_UNUSED_RETURN;
        template <typename T> operator PossiblyNull<T>()
        { 
            T value; 
            getValue(value); 
            return PossiblyNull<T>(value);
        } 
    private:
        mutable void* m_data;
    };
    
    template <typename T> bool TryMallocReturnValue::getValue(T& data)
    {
        union u { void* data; T target; } res;
        res.data = m_data;
        data = res.target;
        bool returnValue = !!m_data;
        m_data = 0;
        return returnValue;
    }

    WTF_EXPORT TryMallocReturnValue tryFastMalloc(size_t n);
    WTF_EXPORT TryMallocReturnValue tryFastZeroedMalloc(size_t n);
    WTF_EXPORT TryMallocReturnValue tryFastCalloc(size_t n_elements, size_t element_size);
    WTF_EXPORT TryMallocReturnValue tryFastRealloc(void* p, size_t n);

    WTF_EXPORT void fastFree(void*);

#ifndef NDEBUG    
    WTF_EXPORT void fastMallocForbid();
    WTF_EXPORT void fastMallocAllow();
#endif

    WTF_EXPORT void releaseFastMallocFreeMemory();
    
    struct FastMallocStatistics {
        size_t reservedVMBytes;
        size_t committedVMBytes;
        size_t freeListBytes;
    };
    WTF_EXPORT FastMallocStatistics fastMallocStatistics();

    // This defines a type which holds an unsigned integer and is the same
    // size as the minimally aligned memory allocation.
    typedef unsigned long long AllocAlignmentInteger;

    namespace Internal {
        enum AllocType {                    // Start with an unusual number instead of zero, because zero is common.
            AllocTypeMalloc = 0x375d6750,   // Encompasses fastMalloc, fastZeroedMalloc, fastCalloc, fastRealloc.
            AllocTypeClassNew,              // Encompasses class operator new from FastAllocBase.
            AllocTypeClassNewArray,         // Encompasses class operator new[] from FastAllocBase.
            AllocTypeFastNew,               // Encompasses fastNew.
            AllocTypeFastNewArray,          // Encompasses fastNewArray.
            AllocTypeNew,                   // Encompasses global operator new.
            AllocTypeNewArray               // Encompasses global operator new[].
        };

        enum {
            ValidationPrefix = 0xf00df00d,
            ValidationSuffix = 0x0badf00d
        };

        typedef unsigned ValidationTag;

        struct ValidationHeader {
            AllocType m_type;
            unsigned m_size;
            ValidationTag m_prefix;
            unsigned m_alignment;
        };

        static const int ValidationBufferSize = sizeof(ValidationHeader) + sizeof(ValidationTag);
    }

#if ENABLE(WTF_MALLOC_VALIDATION)

    // Malloc validation is a scheme whereby a tag is attached to an
    // allocation which identifies how it was originally allocated.
    // This allows us to verify that the freeing operation matches the
    // allocation operation. If memory is allocated with operator new[]
    // but freed with free or delete, this system would detect that.
    // In the implementation here, the tag is an integer prepended to
    // the allocation memory which is assigned one of the AllocType
    // enumeration values. An alternative implementation of this
    // scheme could store the tag somewhere else or ignore it.
    // Users of FastMalloc don't need to know or care how this tagging
    // is implemented.

    namespace Internal {
    
        // Handle a detected alloc/free mismatch. By default this calls CRASH().
        void fastMallocMatchFailed(void* p);

        inline ValidationHeader* fastMallocValidationHeader(void* p)
        {
            return reinterpret_cast<ValidationHeader*>(static_cast<char*>(p) - sizeof(ValidationHeader));
        }

        inline ValidationTag* fastMallocValidationSuffix(void* p)
        {
            ValidationHeader* header = fastMallocValidationHeader(p);
            if (header->m_prefix != static_cast<unsigned>(ValidationPrefix))
                fastMallocMatchFailed(p);
            
            return reinterpret_cast<ValidationTag*>(static_cast<char*>(p) + header->m_size);
        }

        // Return the AllocType tag associated with the allocated block p.
        inline AllocType fastMallocMatchValidationType(void* p)
        {
            return fastMallocValidationHeader(p)->m_type;
        }

        // Set the AllocType tag to be associaged with the allocated block p.
        inline void setFastMallocMatchValidationType(void* p, AllocType allocType)
        {
            fastMallocValidationHeader(p)->m_type = allocType;
        }

    } // namespace Internal

    // This is a higher level function which is used by FastMalloc-using code.
    inline void fastMallocMatchValidateMalloc(void* p, Internal::AllocType allocType)
    {
        if (!p)
            return;

        Internal::setFastMallocMatchValidationType(p, allocType);
    }

    // This is a higher level function which is used by FastMalloc-using code.
    inline void fastMallocMatchValidateFree(void* p, Internal::AllocType)
    {
        if (!p)
            return;
    
        Internal::ValidationHeader* header = Internal::fastMallocValidationHeader(p);
        if (header->m_prefix != static_cast<unsigned>(Internal::ValidationPrefix))
            Internal::fastMallocMatchFailed(p);

        if (*Internal::fastMallocValidationSuffix(p) != Internal::ValidationSuffix)
            Internal::fastMallocMatchFailed(p);

        Internal::setFastMallocMatchValidationType(p, Internal::AllocTypeMalloc);  // Set it to this so that fastFree thinks it's OK.
    }

    inline void fastMallocValidate(void* p)
    {
        if (!p)
            return;
        
        Internal::ValidationHeader* header = Internal::fastMallocValidationHeader(p);
        if (header->m_prefix != static_cast<unsigned>(Internal::ValidationPrefix))
            Internal::fastMallocMatchFailed(p);
        
        if (*Internal::fastMallocValidationSuffix(p) != Internal::ValidationSuffix)
            Internal::fastMallocMatchFailed(p);
    }

#else

    inline void fastMallocMatchValidateMalloc(void*, Internal::AllocType)
    {
    }

    inline void fastMallocMatchValidateFree(void*, Internal::AllocType)
    {
    }

#endif

} // namespace WTF

using WTF::fastCalloc;
using WTF::fastFree;
using WTF::fastMalloc;
using WTF::fastMallocGoodSize;
using WTF::fastRealloc;
using WTF::fastStrDup;
using WTF::fastZeroedMalloc;
using WTF::tryFastCalloc;
using WTF::tryFastMalloc;
using WTF::tryFastRealloc;
using WTF::tryFastZeroedMalloc;

#ifndef NDEBUG    
using WTF::fastMallocForbid;
using WTF::fastMallocAllow;
#endif

#endif /* WTF_FastMalloc_h */
