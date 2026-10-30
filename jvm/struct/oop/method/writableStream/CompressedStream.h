//
// Created by natty on 02.07.2026.
//

#ifndef XDANTIDUMPMEM_COMPRESSEDSTREAM_H
#define XDANTIDUMPMEM_COMPRESSEDSTREAM_H
#include "../../../../types.h"

class CompressedStream  {
protected:
    u_char* _buffer;
    int     _position;

public:
    CompressedStream(u_char* buffer, int position = 0) {
        _buffer   = buffer;
        _position = position;
    }

    void* buffer() const               { return _buffer; }

    // Positioning
    int position() const                 { return _position; }
    void set_position(int position)      { _position = position; }
};



#endif //XDANTIDUMPMEM_COMPRESSEDSTREAM_H
