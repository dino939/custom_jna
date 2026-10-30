//
// Created by natty on 02.07.2026.
//

#include "CompressedLineNumberReadStream.h"

CompressedLineNumberReadStream::CompressedLineNumberReadStream(u_char* buffer) : CompressedReadStream(buffer) {
    _bci = 0;
    _line = 0;
};

bool CompressedLineNumberReadStream::read_pair() {
    jubyte next = read_byte();
    // Check for terminator
    if (next == 0) return false;
    if (next == 0xFF) {
        // Escape character, regular compression used
        _bci  += read_signed_int();
        _line += read_signed_int();
    } else {
        // Single byte compression used
        _bci  += next >> 3;
        _line += next & 0x7;
    }
    return true;
}