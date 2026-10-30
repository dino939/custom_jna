//
// Created by natty on 02.07.2026.
//

#include "CompressedReadStream.h"

jint CompressedReadStream::read_signed_int() {
    return UNSIGNED5::decode_sign(read_int());
}
