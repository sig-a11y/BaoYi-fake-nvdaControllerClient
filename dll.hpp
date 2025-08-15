#pragma once
#ifndef __BOY_DLL_H
#define __BOY_DLL_H
// -- [sys] win
#include <wtypes.h> // LPCSTR; LPCWSTR; TCHAR
#include <minwindef.h> // HINSTANCE

namespace nvdll {

    namespace boy
    {
        /// 保益 DLL 完整路径
        extern TCHAR BOY_DLL_FULLPATH[MAX_PATH];

        bool loadDLL();
        //void freeDll();

        error_status_t __stdcall testIfRunning_impl();
        error_status_t __stdcall speakText_impl(const wchar_t* text);
        error_status_t __stdcall cancelSpeech_impl();
    } // nvdll::boy::

    /// <summary>
    /// 争渡读屏 ZDSR 的接口定义
    /// </summary>
    namespace zdsr
    {
        /// DLL 完整路径
        extern TCHAR DLL_FULLPATH[MAX_PATH];
        //extern HMODULE dllHandle;

        bool loadDLL();
        //void freeDll();

        error_status_t __stdcall testIfRunning_impl();
        error_status_t __stdcall speakText_impl(const wchar_t* text);
        error_status_t __stdcall cancelSpeech_impl();
    }

    /// NVDA 接口版本
    constexpr LPCSTR NVDA_API_VERSION = "v2.0";

#pragma region 全局变量定义
    /// DLL 所在文件夹路径
    extern TCHAR DLL_DIR_PATH[MAX_PATH];
    extern HMODULE dllHandle;
#pragma region

#pragma region 导出函数
    void printDllInfo();
    void saveDllDirPath(HINSTANCE hinstDLL);
    FARPROC loadFunctionPtr(LPCSTR lpProcName);

    void loadDLL();
    void freeDll();

    error_status_t StopSpeaking();
#pragma region
} // nvdll::

#endif // __BOY_DLL_H