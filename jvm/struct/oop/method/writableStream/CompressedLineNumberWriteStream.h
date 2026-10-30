//
// Created by natty on 02.07.2026.
//

#ifndef XDANTIDUMPMEM_COMPRESSEDLINENUMBERWRITESTREAM_H
#define XDANTIDUMPMEM_COMPRESSEDLINENUMBERWRITESTREAM_H
#include "CompressedWriteStream.h"


class CompressedLineNumberWriteStream: public CompressedWriteStream {
private:
    int _bci;
    int _line;
public:
    // Constructor
    CompressedLineNumberWriteStream(int initial_size) : CompressedWriteStream(initial_size), _bci(0), _line(0) {}
    CompressedLineNumberWriteStream(u_char* buffer, int initial_size) : CompressedWriteStream(buffer, initial_size), _bci(0), _line(0) {}

    // Write (bci, line number) pair to stream
    void write_pair_regular(int bci_delta, int line_delta);

    // If (bci delta, line delta) fits in (5-bit unsigned, 3-bit unsigned)
    // we save it as one byte, otherwise we write a 0xFF escape character
    // and use regular compression. 0x0 is used as end-of-stream terminator.
    void write_pair_inline(int bci, int line);

    void write_pair(int bci, int line);

    // Write end-of-stream marker
    void write_terminator()                        { write_byte(0); }
};


#endif //XDANTIDUMPMEM_COMPRESSEDLINENUMBERWRITESTREAM_H
