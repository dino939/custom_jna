//
// Created by prone on 16.03.2026.
//



#define min(a,b)            (((a) < (b)) ? (a) : (b))

#include "Symbol.h"
#include "../../../offset/OffsetExtractor.h"
u2 Symbol::length() const
{
  if (this == nullptr) return 0;
  static offset bodyOffset = OffsetExtractor::getInstance()->findVMStructEntry("Symbol", "_length")->offset;
  return *reinterpret_cast<u2*>(reinterpret_cast<uintptr_t>(this) + bodyOffset);
}

const u1* Symbol::base() const
{
    static offset bodyOffset = OffsetExtractor::getInstance()->findVMStructEntry("Symbol", "_body")->offset;
    return reinterpret_cast<const u1*>(reinterpret_cast<uintptr_t>(this) + bodyOffset);
}

char Symbol::char_at(int index) const {
  assert(index >=0 && index < length(), "symbol index overflow");
  return (char)base()[index];
}

std::string Symbol::as_string()
{
  return  std::string(reinterpret_cast<const char*>(base()), utf8_length());
}

u1 Symbol::byte_at(int idx) const {
  return base()[idx];
}

char* Symbol::as_C_string(char* buffer, int sz) const {
  if (sz > 0) {
    int len =  min(sz - 1, (int)length());
    for (int i = 0; i < len; i++)
      buffer[i] = byte_at(i);
    buffer[len] = '\0';
  }
  return buffer;
}

const char* Symbol::as_C_string() const {
  int len = length();
  char* str = new char[len + 1];
  return as_C_string(str, len + 1);
}

char* Symbol::as_utf8() const {
  int len = length();
  char* str = new char[len + 1];
  return as_C_string(str, len + 1);
}
bool Symbol::isValid() const { return length() > 0; }

