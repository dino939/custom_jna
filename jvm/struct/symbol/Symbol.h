//
// Created by prone on 16.03.2026.
//

#ifndef CUSTOM_JNA_SYMBOL_H
#define CUSTOM_JNA_SYMBOL_H

#include <string>
#include "../../types.h"
#include "../../utils/assert/assert.hpp"


class Symbol {
    uint32_t _hash_and_refcount;
    u2 _length;
    u1 _body[2];

    enum {
        max_symbol_length = 0xffff
      };

public:
    u2 length() const;
    int utf8_length() const { return length(); }
    const u1* base() const;
    char  char_at(int idx) const;
    std::string as_string();
    u1 byte_at(int idx) const;
    char* as_C_string(char* buffer, int sz) const;
    const char* as_C_string() const;
    char* as_utf8() const;
    bool isValid() const;
};


#endif //CUSTOM_JNA_SYMBOL_H