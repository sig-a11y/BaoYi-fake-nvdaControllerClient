#pragma once
#ifndef __BOY_INI_H
#define __BOY_INI_H
// -- [sys] win
#include <wtypes.h> // PCWSTR

namespace nvdll {
namespace ini {
    extern bool SPEAK_WITH_SLAVE;
    extern bool SPEAK_APPEND;
    extern bool SPEAK_ALLOW_BREAK;
    extern bool SPEAK_ALL_KEY_BREAK;

    void loadIni(PCWSTR pszBaseDirIn);

} // nvdll::ini::
} // nvdll::
#endif // __BOY_INI_H