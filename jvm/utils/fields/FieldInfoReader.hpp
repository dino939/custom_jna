#pragma once
#include <cstdint>


/*
 * @brief
 * this code for 21 jvm
 *
 */

#include "FieldInfo.hpp"


class FieldInfoReader {
private:


    static const int L = 191;
    static const int lg_H = 6;
    static const int X = 1;
    static const int MAX_LENGTH = 5;


    const uint8_t* _data;
    int _pos;
    int _end;
    int _next_index;

    uint32_t read_uint() {
        if (_pos >= _end) {
            return 0;
        }
        uint8_t b0 = _data[_pos];
        uint32_t summ = b0 - X;

        if (summ < L) {
            _pos++;
            return summ;
        }
        int lg_H_i = lg_H;
        int i = 1;

        while (true) {
            if (_pos + i >= _end) {
                return 0;
            }
            uint8_t bi = _data[_pos + i];
            summ += (bi - X) << lg_H_i;

            if (bi < X + L || i == 4) {
                _pos += i + 1;
                return summ;
            }
            lg_H_i += lg_H;
            i++;
        }
    }

public:

    // FieldInfoReader(InstanceKlass* klass) :
    //     _data(klass->_fieldinfo_stream()->adr_at(0)),
    //     _pos(0),
    //     _end(klass->_fieldinfo_stream()->length()),
    //     _next_index(0) {};
    //
    //

    FieldInfoReader(const Array<u1>* array) :
        _data(array->adr_at2(0)),
        _pos(0),
        _end(array->length()),
        _next_index(0) {}
    uint32_t next_unit() {
        return read_uint();
    }

    void read_field_info(FieldInfo_20 &field_info) {
        u2 name_index = static_cast<u2>(read_uint());
        u2 signature_index = static_cast<u2>(read_uint());

        u4 offset = read_uint();
        u4 access = read_uint();
        u4 flags = read_uint();

        field_info._name_index = name_index;
        field_info._signature_index = signature_index;
        field_info._offset = offset;
        field_info._access_flags = AccessFlags(access);
        field_info._field_flags = FieldFlags(flags);

        if (field_info._field_flags.is_initialized()) {
            field_info._initializer_index = static_cast<u2>(read_uint());
        }
        else {
            field_info._initializer_index = 0;
        }
        if (field_info._field_flags.is_generic()) {
            field_info._generic_signature_index = static_cast<u2>(read_uint());
        }
        else {
            field_info._generic_signature_index = 0;
        }

        if (field_info._field_flags.is_contended()) {
            field_info._contention_group =  static_cast<u2>(read_uint());
        }
        else {
            field_info._contention_group = 0;
        }
    }

    void skip_field_info() {
        read_uint(); // name_index
        read_uint(); // signature_index
        read_uint(); // offset

        read_uint(); // access_flags
        u4 field_flags = read_uint();

        FieldFlags fflags(field_flags);

        if (fflags.is_initialized()) {
            read_uint();
        }
        if (fflags.is_generic()) {
            read_uint();
        }
        if (fflags.is_contended()) {
            read_uint();
        }
        _next_index++;
    }

    int position() {
        return _pos;
    }
};
