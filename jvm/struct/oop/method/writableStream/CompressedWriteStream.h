//
// Created by natty on 02.07.2026.
//

#ifndef XDANTIDUMPMEM_COMPRESSEDWRITESTREAM_H
#define XDANTIDUMPMEM_COMPRESSEDWRITESTREAM_H
#include "CompressedStream.h"
#include "../../utilities/UNSIGNED5.h"


class CompressedWriteStream : public CompressedStream {
private:
    bool full() {
        return _position >= _size;
    }
    void store(u_char b) {
        _buffer[_position++] = b;
    }
    void write(u_char b) {
        if (full()) grow();
        store(b);
    }
    void grow();

protected:
    int _size;

public:
    CompressedWriteStream(int initial_size);
    CompressedWriteStream(u_char* buffer, int initial_size, int position = 0)
    : CompressedStream(buffer, position) { _size = initial_size; }

    void write_bool(jboolean value)      { write(value);      }
    void write_byte(jbyte value)         { write(value);      }
    void write_char(jchar value)         { write_int(value); }
    void write_short(jshort value)       { write_signed_int(value);  }
    void write_signed_int(jint value)    { write_int(UNSIGNED5::encode_sign(value)); }
    void write_float(jfloat value);      // write_int(reverse_bits(jint_cast(v)))
    void write_double(jdouble value);    // write_int(reverse_bits(<low,high>))
    void write_long(jlong value);        // write_signed_int(<low,high>)

    void write_int(juint value) {
        UNSIGNED5::write_uint_grow(value, _buffer, _position, _size,
                                   [&](int){ grow(); });
    }

};


#endif //XDANTIDUMPMEM_COMPRESSEDWRITESTREAM_H
