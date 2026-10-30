//
// Created by prone on 15.04.2026.
//

#ifndef CUSTOM_JNA_ARRAY_H
#define CUSTOM_JNA_ARRAY_H

#include "../../../types.h"


template <typename ArrayT>
class Array
{
protected:
    int _length;
    ArrayT _data[1];
    public:
     int length();
    [[nodiscard]] int length() const;

     ArrayT* data();
    const ArrayT* data() const;

     int index_of(const ArrayT& x);

    bool contains(const ArrayT& x);

    void set_length(int len) { _length = len; }

    ArrayT  at(int i);

    void at_put(int i, const ArrayT& x);

    ArrayT*   adr_at(int i);

    const ArrayT* adr_at2(int i) const;

    int  find(const ArrayT& x);
};


#endif //CUSTOM_JNA_ARRAY_H

/*  nonstatic_field(Array<Klass*>,               _length,                                       int)                                   \
nonstatic_field(Array<Klass*>,               _data[0],                                      Klass*)        */
