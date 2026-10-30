//
// Created by prone on 26.06.2026.
//

#ifndef CUSTOM_JNA_OOPHANDLE_H
#define CUSTOM_JNA_OOPHANDLE_H
#include "../../../types.h"



class OopHandle
{
private:
    object* _obj;

public:
    OopHandle() : _obj(nullptr)
    {
    }

    explicit OopHandle(object* w) : _obj(w)
    {
    }

    bool is_empty() const;
    object* get_ptr() const;
    object get_oop() const;
};

#endif //CUSTOM_JNA_OOPHANDLE_H
