#pragma once
#ifndef __NVDLL_INPUT_H
/// 鼠标/键盘输入监听
#define __NVDLL_INPUT_H

namespace nvdll
{
    namespace input 
    {

        extern void runInputListener();
        extern void killListenerThread();

        extern bool setInputHook();
        extern void removeInputHook();

    } // nvdll::input::
} // nvdll::
#endif // __NVDLL_INPUT_H