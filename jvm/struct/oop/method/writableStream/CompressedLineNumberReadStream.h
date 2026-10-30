//
// Created by natty on 02.07.2026.
//

#ifndef XDANTIDUMPMEM_COMPRESSEDLINENUMBERREADSTREAM_H
#define XDANTIDUMPMEM_COMPRESSEDLINENUMBERREADSTREAM_H
#include "CompressedReadStream.h"


class CompressedLineNumberReadStream: public CompressedReadStream {
private:
    int _bci;
    int _line;
public:
    // Constructor
    CompressedLineNumberReadStream(u_char* buffer);
    // Read (bci, line number) pair from stream. Returns false at end-of-stream.
    bool read_pair();
    // Accessing bci and line number (after calling read_pair)
    int bci() const                               { return _bci; }
    int line() const                              { return _line; }
};



#endif //XDANTIDUMPMEM_COMPRESSEDLINENUMBERREADSTREAM_H
