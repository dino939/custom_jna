//
// Created by natty on 02.07.2026.
//

#ifndef XDANTIDUMPMEM_EXCEPTIONTABLEELEMENT_H
#define XDANTIDUMPMEM_EXCEPTIONTABLEELEMENT_H

#include "../../../types.h"
class ExceptionTableElement {
public:
    u2 start_pc;
    u2 end_pc;
    u2 handler_pc;
    u2 catch_type_index;
};

#endif //XDANTIDUMPMEM_EXCEPTIONTABLEELEMENT_H
