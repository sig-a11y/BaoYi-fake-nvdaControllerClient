#pragma once
#ifndef __BOY_DLL_H
#define __BOY_DLL_H
// -- [sys] win
#include <wtypes.h> // LPCSTR; LPCWSTR; TCHAR
#include <minwindef.h> // HINSTANCE


namespace dll {
#pragma region 常量定义
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
#pragma region

#pragma region 导出函数
    void saveDllDirPath(HINSTANCE hinstDLL);
    void freeDll();
#pragma region
} // dll::

#endif // __BOY_DLL_H