//
// Created by natty on 02.07.2026.
//

#ifndef XDANTIDUMPMEM_EXCEPTIONTABLE_H
#define XDANTIDUMPMEM_EXCEPTIONTABLE_H
#include <cassert>

#include "exceptionTableElement.h"
#include "method.h"
#include "../../../types.h"

// Utility class for access exception handlers
class ExceptionTable  {
private:
    ExceptionTableElement* _table;
    u2  _length;

public:
    ExceptionTable(const Method* m) {
        if (m->constMethod()->has_exception_handler()) {
            _table = m->constMethod()->exception_table_start();
            _length = m->constMethod()->exception_table_length();
        } else {
            _table = nullptr;
            _length = 0;
        }
    }

    int length() const {
        return _length;
    }

    u2 start_pc(int idx) const {
        //assert(idx < _length, "out of bounds");
        return _table[idx].start_pc;
    }

    void set_start_pc(int idx, u2 value) {
        //assert(idx < _length, "out of bounds");
        _table[idx].start_pc = value;
    }

    u2 end_pc(int idx) const {
        //assert(idx < _length, "out of bounds");
        return _table[idx].end_pc;
    }

    void set_end_pc(int idx, u2 value) {
        //assert(idx < _length, "out of bounds");
        _table[idx].end_pc = value;
    }

    u2 handler_pc(int idx) const {
        //assert(idx < _length, "out of bounds");
        return _table[idx].handler_pc;
    }

    void set_handler_pc(int idx, u2 value) {
       // assert(idx < _length, "out of bounds");
        _table[idx].handler_pc = value;
    }

    u2 catch_type_index(int idx) const {
       // assert(idx < _length, "out of bounds");
        return _table[idx].catch_type_index;
    }

    void set_catch_type_index(int idx, u2 value) {
        //assert(idx < _length, "out of bounds");
        _table[idx].catch_type_index = value;
    }
};

#endif //XDANTIDUMPMEM_EXCEPTIONTABLE_H
