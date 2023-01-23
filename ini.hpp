#pragma once
#ifndef __BOY_INI_H
#define __BOY_INI_H
#include <wtypes.h>

namespace ini {
    extern bool SPEAK_WITH_SLAVE;
    extern bool SPEAK_APPEND;
    extern bool SPEAK_ALLOW_BREAK;

    void loadIni(PCWSTR pszBaseDirIn);

} // ini::

#endif // __BOY_INI_H