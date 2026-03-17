//
// Created by prone on 16.03.2026.
//



#define min(a,b)            (((a) < (b)) ? (a) : (b))

#include "Symbol.h"

u2 Symbol::length() const
{
  return  _length;
}

const u1* Symbol::base() const
{
  return &_body[0];
}

char Symbol::char_at(int index) const {
  assert(index >=0 && index < length(), "symbol index overflow");
  return (char)base()[index];
}

std::string Symbol::as_string()
{
  return  std::string(base(), utf8_length());
}

u1 Symbol::byte_at(int idx) const {
  return base()[idx];
}

char* Symbol::as_C_string(char* buffer, int sz) const {
  if (sz > 0) {
    int len =  min(sz - 1, (int)_length);
    for (int i = 0; i < len; i++)
      buffer[i] = byte_at(i);
    buffer[len] = '\0';
  }
  return buffer;
}

const char* Symbol::as_C_string() const {
  int len = _length;
  char* str = new char[len + 1];
  return as_C_string(str, len + 1);
}

char* Symbol::as_utf8() const {
  int len = _length;
  char* str = new char[len + 1];
  return as_C_string(str, len + 1);
}
bool Symbol::isValid() const { return _length > 0; }

