//
// Created by natty on 02.07.2026.
//

#ifndef XDANTIDUMPMEM_UNSIGNED5_H
#define XDANTIDUMPMEM_UNSIGNED5_H
#include <cassert>
#include <cstdint>

#include "../../../types.h"


class UNSIGNED5 {
 private:
  // Math constants for the modified UNSIGNED5 coding of Pack200
  static const int lg_H  = 6;        // log-base-2 of H (lg 64 == 6)
  static const int H     = 1<<lg_H;  // number of "high" bytes (64)
  static const int X     = 1  ;      // there is one excluded byte ('\0')
  static const int MAX_b = (1<<BitsPerByte)-1;  // largest byte value
  static const int L     = (MAX_b+1)-X-H;       // number of "low" bytes (191)

 public:
  static const int MAX_LENGTH = 5;   // lengths are in [1..5]
  static const uint32_t MAX_VALUE = (uint32_t)-1;  // 2^^32-1

  // The default method for reading and writing bytes is simply
  // b=a[i] and a[i]=b, as defined by this helpful functor.
  template<typename ARR, typename OFF>
  struct ArrayGetSet {
    uint8_t operator()(ARR a, OFF i) const { return a[i]; };
    void operator()(ARR a, OFF i, uint8_t b) const { a[i] = b; };
    // So, an expression ArrayGetSet() acts like these lambdas:
    //auto get = [&](ARR a, OFF i){ return a[i]; };
    //auto set = [&](ARR a, OFF i, uint8_t x){ a[i] = x; };
  };

  // decode a single unsigned 32-bit int from an array-like base address
  // returns the decoded value, updates offset_rw
  // that is, offset_rw is both read and written
  // warning:  caller must ensure there is at least one byte available
  // the limit is either zero meaning no limit check, or an exclusive offset
  // in PRODUCT builds, limit is ignored
  template<typename ARR, typename OFF, typename GET = ArrayGetSet<ARR,OFF>>
  static uint32_t read_uint(ARR array, OFF& offset_rw, OFF limit, GET get = GET()) {
    const OFF pos = offset_rw;
    const uint32_t b_0 = (uint8_t) get(array, pos);  //b_0 = a[0]
    uint32_t sum = b_0 - X;
    if (sum < L) {  // common case
      offset_rw = pos + 1;
      return sum;
    }
    // must collect more bytes:  b[1]...b[4]
    int lg_H_i = lg_H;  // lg(H)*i == lg(H^^i)
    for (int i = 1; ; i++) {  // for i in [1..4]
      const uint32_t b_i = (uint8_t) get(array, pos + i);  //b_i = a[i]
      sum += (b_i - X) << lg_H_i;  // sum += (b[i]-X)*(64^^i)
      if (b_i < X+L || i == MAX_LENGTH-1) {
        offset_rw = pos + i + 1;
        return sum;
      }
      lg_H_i += lg_H;
    }
  }

  // encode a single unsigned 32-bit int into an array-like span
  // offset_rw is both read and written
  // the limit is either zero meaning no limit check, or an exclusive offset
  // warning:  caller must ensure there is available space
  template<typename ARR, typename OFF, typename SET = ArrayGetSet<ARR,OFF>>
  static void write_uint(uint32_t value, ARR array, OFF& offset_rw, OFF limit, SET set = SET()) {
    const OFF pos = offset_rw;
    if (value < L) {
      const uint32_t b_0 = X + value;
      set(array, pos, (uint8_t)b_0);  //a[0] = b_0
      offset_rw = pos + 1;
      return;
    }
    uint32_t sum = value;
    for (int i = 0; ; i++) {  // for i in [0..4]
      if (sum < L || i == MAX_LENGTH-1) {
        // remainder is either a "low code" or the 5th byte
        uint32_t b_i = X + sum;
        set(array, pos + i, (uint8_t)b_i);  //a[i] = b_i
        offset_rw = pos + i + 1;
        return;
      }
      sum -= L;
      uint32_t b_i = X + L + (sum % H);  // this is a "high code"
      set(array, pos + i, (uint8_t)b_i);  //a[i] = b_i
      sum >>= lg_H;                 // extracted 6 bits
    }
  }

  // returns the encoded byte length of an unsigned 32-bit int
  static constexpr int encoded_length(uint32_t value) {
    // model the reading of [0..5] high-bytes, followed possibly by a low-byte
    // Be careful:  the constexpr magic evaporates if undefined behavior
    // results from any of these expressions.  Beware of signed overflow!
    uint32_t sum = 0;
    uint32_t lg_H_i = 0;
    for (uint32_t i = 0; ; i++) {  // for i in [1..4]
      if (value <= sum + ((L-1) << lg_H_i) || i == MAX_LENGTH-1) {
        return i + 1;  // stopping at byte i implies length is i+1
      }
      sum += (MAX_b - X) << lg_H_i;
      lg_H_i += lg_H;
    }
  }

  // reports the largest uint32_t value that can be encoded using len bytes
  // len must be in the range [1..5]
  static constexpr uint32_t max_encoded_in_length(uint32_t len) {
    if (len >= MAX_LENGTH)  return MAX_VALUE;  // largest non-overflow value
    // Be careful:  the constexpr magic evaporates if undefined behavior
    // results from any of these expressions.  Beware of signed overflow!
    uint32_t all_combinations = 0;
    uint32_t combinations_i = L;  // L * H^i
    for (uint32_t i = 0; i < len; i++) {
      // count combinations of <H*L> that end at byte i
      all_combinations += combinations_i;
      combinations_i <<= lg_H;
    }
    return all_combinations - 1;
  }

  // tells if a value, when encoded, would fit between the offset and limit
  template<typename OFF>
  static constexpr bool fits_in_limit(uint32_t value, OFF offset, OFF limit) {
    return (offset + MAX_LENGTH <= limit ||
            offset + encoded_length(value) <= limit);
  }

  // parses one encoded value for correctness and returns the size,
  // or else returns zero if there is a problem (bad limit or excluded byte)
  // the limit is either zero meaning no limit check, or an exclusive offset
  template<typename ARR, typename OFF, typename GET = ArrayGetSet<ARR,OFF>>
  static int check_length(ARR array, OFF offset, OFF limit = 0,
                          GET get = GET()) {
    const OFF pos = offset;
    const uint32_t b_0 = (uint8_t) get(array, pos);  //b_0 = a[0]
    if (b_0 < X+L) {
      return (b_0 < X) ? 0 : 1;
    }
    // parse more bytes:  b[1]...b[4]
    for (int i = 1; ; i++) {  // for i in [1..4]
      if (limit != 0 && pos + i >= limit)  return 0;  // limit failure
      const uint32_t b_i = (uint8_t) get(array, pos + i);  //b_i = a[i]
      if (b_i < X)  return 0;  // excluded byte found
      if (b_i < X+L || i == MAX_LENGTH-1) {
        return i + 1;
      }
    }
  }

  template<typename ARR, typename OFF, typename GFN,
           typename SET = ArrayGetSet<ARR,OFF>>
  static void write_uint_grow(uint32_t value,
                              ARR& array, OFF& offset, OFF& limit,
                              GFN grow, SET set = SET()) {
    const OFF pos = offset;
    if (!fits_in_limit(value, pos, limit)) {
      grow(MAX_LENGTH);  // caller must ensure it somehow fixes array/limit span
    }
    write_uint(value, array, offset, limit, set);
  }

  /// Handy state machines for that will help you with reading,
  /// sizing, and writing (with optional growth).

  // Reader example use:
  //  struct MyReaderHelper {
  //    char operator()(char* a, int i) const { return a[i]; }
  //  };
  //  using MyReader = UNSIGNED5::Reader<char*, int, MyReaderHelper>;
  //  MyReader r(array); while (r.has_next())  print(r.next_uint());
  static uint32_t encode_sign(int32_t value) { return ((uint32_t)value << 1) ^ (value >> 31); }
  static int32_t decode_sign(uint32_t value) { return (value >> 1) ^ -(int32_t)(value & 1); }

};


#endif //XDANTIDUMPMEM_UNSIGNED5_H
