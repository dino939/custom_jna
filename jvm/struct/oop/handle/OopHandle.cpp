//
// Created by prone on 26.06.2026.
//

#include "OopHandle.h"

bool OopHandle::is_empty() const
{
    return _obj == nullptr;
}

object* OopHandle::get_ptr() const
{
    return _obj;
}

object OopHandle::get_oop() const
{
    return *_obj;
}
