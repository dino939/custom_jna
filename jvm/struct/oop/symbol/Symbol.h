//
// Created by prone on 16.03.2026.
//

#ifndef CUSTOM_JNA_SYMBOL_H
#define CUSTOM_JNA_SYMBOL_H

#include <string>
#include "../../../types.h"
#include "../../../utils/assert/assert.hpp"


class Symbol {
    volatile uint32_t _hash_and_refcount;
    u2 _length;
    u1 _body[2];

    enum {
        max_symbol_length = 0xffff
      };

public:
    Symbol(const char* str, size_t len) {
        _length = (u2)len;
        _hash_and_refcount = 0;
        memcpy(_body, str, len);
        _body[len] = '\0';
    }
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

    int refcount() const { return _hash_and_refcount & 0xffff; }
    void increment_refcount() {
        _hash_and_refcount = (_hash_and_refcount & ~0xffffu) | ((_hash_and_refcount & 0xffffu) + 1);
    }
};


#endif //CUSTOM_JNA_SYMBOL_H