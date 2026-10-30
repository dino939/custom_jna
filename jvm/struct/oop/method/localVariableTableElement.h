//
// Created by natty on 02.07.2026.
//

#ifndef XDANTIDUMPMEM_LOCALVARIABLETABLEELEMENT_H
#define XDANTIDUMPMEM_LOCALVARIABLETABLEELEMENT_H
#include "../../../types.h"

class LocalVariableTableElement {
public:
    u2 start_bci;
    u2 length;
    u2 name_cp_index;
    u2 descriptor_cp_index;
    u2 signature_cp_index;
    u2 slot;
};

#endif //XDANTIDUMPMEM_LOCALVARIABLETABLEELEMENT_H
