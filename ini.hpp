#pragma once
#ifndef __BOY_INI_H
#define __BOY_INI_H

namespace ini {
#pragma region 全局变量定义
    /// false=使用读屏通道，true=使用独立通道
    extern bool SPEAK_WITH_SLAVE;
    /// 是否排队朗读
    extern bool SPEAK_APPEND;
    /// 是否允许用户打断.使用读屏通道时该参数被忽略
    extern bool SPEAK_ALLOW_BREAK;
#pragma region


    void loadIni();

} // ini::

#endif // __BOY_INI_H