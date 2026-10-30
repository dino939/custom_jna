//
// Created by prone on 15.04.2026.
//

#include "Array.h"
#include "../constantPool/ResolvedIndyEntry.h"
#include "../klass/RecordComponent.h"
#include "../../../offset/OffsetExtractor.h"
#include "../../../utils/assert/assert.hpp"
#include <format>

class ConstMethod;
class Method;
template class Array<u1>;           // для _tags()
template class Array<u2>;           // для _operands()
template class Array<u4>;           // если используется


// Основные типы, используемые в вашем коде
template class Array<InstanceKlass*>;      // для _resolved_klasses()
template class Array<int>;      // для _resolved_klasses()
template class Array<Method*>;             // для _methods
template class Array<Array<u1>*>;    // для _fields_annotations и _fields_type_annotations
template class Array<uint16_t>;            // для _nest_members, _permitted_subclasses
template class Array<RecordComponent*>;     // для _record_components
template class Array<ConstMethod*>;        // если используется
template class Array<unsigned char>;
template class Array<ResolvedIndyEntry>;   // для _resolved_indy_entries


template <typename ArrayT>
int Array<ArrayT>::length()
{
    return _length;
}

template <typename ArrayT>
int Array<ArrayT>::length() const
{
    return _length;
}

template <typename ArrayT>
ArrayT* Array<ArrayT>::data()
{
    return &_data[0];
}

template <typename ArrayT>
const ArrayT* Array<ArrayT>::data() const
{
    return &_data[0];
}

template <typename ArrayT>
int Array<ArrayT>::index_of(const ArrayT& x) {
    int i = length();
    while (i-- > 0 && data()[i] != x);

    return i;
}

// sort the array.
template <typename ArrayT>
bool Array<ArrayT>::contains(const ArrayT& x)      { return index_of(x) >= 0; }

template <typename ArrayT>
ArrayT    Array<ArrayT>::at(int i)                 { assert(i >= 0 && i< this->length(),std::format("oob: 0 <= {} < {}", i, length()) ); return data()[i]; }

template <typename ArrayT>
void Array<ArrayT>::at_put(const int i, const ArrayT& x) { assert(i >= 0 && i< this->length(),std::format("oob: 0 <= {} < {}", i, length()) ); data()[i] = x; }

template <typename ArrayT>
ArrayT*   Array<ArrayT>::adr_at(int i)             { assert(i >= 0 && i< this->length(),std::format("oob: 0 <= {} < {}", i, length()) ); return &data()[i]; }

template <typename ArrayT>
const ArrayT*   Array<ArrayT>::adr_at2(int i)     const        { assert(i >= 0 && i< this->length(),std::format("oob: 0 <= {} < {}", i, length()) ); return &data()[i]; }

template <typename ArrayT>
int  Array<ArrayT>::find(const ArrayT& x)                { return index_of(x); }


