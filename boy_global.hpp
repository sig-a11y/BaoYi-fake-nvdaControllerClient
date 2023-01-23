#pragma once
#ifndef __BOY_GLOBAL_H
#define __BOY_GLOBAL_H
#include "BoyCtrl.h"
#include <Windows.h>

namespace boy
{

#pragma region 常量定义
    /// 允许的长路径长度
    constexpr int PATH_NO_LIMIT_SIZE = 1024;
    /// 保益 DLL 版本
    constexpr LPCSTR BOY_DLL_VERSION = "v1.5.2";

    #ifdef _WIN64
    /// 保益 DLL 文件名
    constexpr LPCWSTR BOY_DLL_NAME = L"BoyCtrl-x64.dll";
    #else
    /// 保益 DLL 文件名
    constexpr LPCWSTR BOY_DLL_NAME = L"BoyCtrl.dll";
    #endif // def _WIN64

    #ifdef _DEBUG
    /// 输出日志名称
    constexpr LPCWSTR DLL_LOG_NAME = L"boyCtrl-debug.log";
    #else // release mode
    /// 置空，不输出日志
    constexpr LPCWSTR DLL_LOG_NAME = nullptr;
    #endif // def _DEBUG
#pragma region


#pragma region 全局变量定义
    /// DLL 所在文件夹路径
    extern TCHAR DLL_DIR_PATH[MAX_PATH];
    /// 保益 DLL 完整路径
    extern TCHAR BOY_DLL_FULLPATH[MAX_PATH];

    // -- 参数常量
    /// false=使用读屏通道，true=使用独立通道
    extern bool SPEAK_WITH_SLAVE;
    /// 是否排队朗读
    extern bool SPEAK_APPEND;
    /// 是否允许用户打断.使用读屏通道时该参数被忽略
    extern bool SPEAK_ALLOW_BREAK;
#pragma region


    void logWString(LPCSTR prefix, LPCSTR varName, LPCWSTR inWstring);

}
#endif // __BOY_GLOBAL_H