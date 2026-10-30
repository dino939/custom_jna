//
// Created by prone on 17.03.2026.
//

#ifndef CUSTOM_JNA_JNABOOTSTRAP_H
#define CUSTOM_JNA_JNABOOTSTRAP_H
#include "../jna.h"

class JNABootStrap
{
    static JNABootStrap* instance;
    const environment env;
    public:
        JNABootStrap();
    ~JNABootStrap();

    [[nodiscard]] environment getEnv() const;

    static void destroy()
    {
        delete instance;
    }
    static JNABootStrap* getInstance()
    {
        if (!instance)
        {
            instance = new JNABootStrap();
        }
        return instance;
    }
};


#endif //CUSTOM_JNA_JNABOOTSTRAP_H