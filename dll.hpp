#pragma once
#ifndef __BOY_DLL_H
#define __BOY_DLL_H
// -- [sys] win
#include <wtypes.h> // LPCSTR; LPCWSTR; TCHAR
#include <minwindef.h> // HINSTANCE


namespace nvdll {
#pragma region 常量定义
    namespace boy
    {
        /// 保益 DLL 完整路径
        extern TCHAR BOY_DLL_FULLPATH[MAX_PATH];

        /// 保益 DLL 版本
        constexpr LPCSTR BOY_DLL_VERSION = "v1.7";

#ifdef _WIN64
        /// 保益 DLL 文件名
        constexpr LPCWSTR BOY_DLL_NAME = L"BoyCtrl-x64.dll";
#else
        /// 保益 DLL 文件名
        constexpr LPCWSTR BOY_DLL_NAME = L"BoyCtrl.dll";
#endif // def _WIN64
    } // nvdll::boy::

    /// 保益 DLL 输出日志名称。日志放在当前工作目录
    constexpr LPCWSTR DLL_LOG_NAME = L"boyCtrl-debug.log";
#pragma region

#pragma region 全局变量定义
    /// DLL 所在文件夹路径
    extern TCHAR DLL_DIR_PATH[MAX_PATH];
    extern HMODULE dllHandle;
#pragma region

#pragma region 导出函数
    void saveDllDirPath(HINSTANCE hinstDLL);
    FARPROC loadFunctionPtr(LPCSTR lpProcName);
    void freeDll();

    bool IsScreenReaderRunning();
    error_status_t StopSpeaking();
#pragma region
} // nvdll::

#endif // __BOY_DLL_H