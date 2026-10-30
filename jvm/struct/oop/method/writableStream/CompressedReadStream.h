//
// Created by natty on 02.07.2026.
//

#ifndef XDANTIDUMPMEM_COMPRESSEDREADSTREAM_H
#define XDANTIDUMPMEM_COMPRESSEDREADSTREAM_H

#include "CompressedStream.h"
#include "../../utilities/UNSIGNED5.h"

class CompressedReadStream : public CompressedStream {
private:
    inline u_char read()                 { return _buffer[_position++]; }

public:
    CompressedReadStream(u_char* buffer, int position = 0)
    : CompressedStream(buffer, position) {}

    jboolean read_bool()                    { return (jboolean) read();      }
    jbyte    read_byte()                 { return (jbyte   ) read();      }
    jchar    read_char()                 { return (jchar   ) read_int();  }
    jshort   read_short()                { return (jshort  ) read_signed_int(); }
    jint     read_signed_int();
    jfloat   read_float();               // jfloat_cast(reverse_bits(read_int()))
    jdouble  read_double();              // jdouble_cast(2*reverse_bits(read_int))
    jlong    read_long();                // jlong_from(2*read_signed_int())

    jint     read_int() {
        return UNSIGNED5::read_uint(_buffer, _position, 0);
    }
};

#endif //XDANTIDUMPMEM_COMPRESSEDREADSTREAM_H
