//
// Created by prone on 17.03.2026.
//

#ifndef CUSTOM_JNA_INSTANCEKLASS_H
#define CUSTOM_JNA_INSTANCEKLASS_H
#include "../../types.h"
#include "../symbol/Symbol.h"

class InstanceKlass
{
public:
    [[nodiscard]] Symbol* _name() const;
    [[nodiscard]] InstanceKlass* _next();
};



#endif //CUSTOM_JNA_INSTANCEKLASS_H