#include "nvdll_impl.h" // API 导出
#include "dll.hpp" // 项目内导出
// -- [sys] win
#include <stdlib.h> // _wsplitpath_s, _wmakepath_s
#include <strsafe.h> // StringCchPrintf
#include <sstream> // stringstream
#include <string> // wstring
#include <vector> // vector
// -- [proj]
#include "log.hpp" // nvdll::log::; DLOG_F


namespace nvdll {

#pragma region 全局变量定义
    /// DLL 所在文件夹路径
    TCHAR DLL_DIR_PATH[MAX_PATH];
    HMODULE dllHandle;
#pragma region


    /**
     * @brief 获取并保存 DLL 所在文件夹路径.
     * @param hinstDLL DLL 实例句柄
     */
    void saveDllDirPath(HINSTANCE hinstDLL)
    {
        /// DLL 路径
        TCHAR DLL_PATH[MAX_PATH];
        // 获取 DLL 完整路径
        GetModuleFileName(hinstDLL, DLL_PATH, MAX_PATH);

        // -- 打印完整路径
        spdlog::info("BaoYi Dll API Version: {}", nvdll::boy::BOY_DLL_VERSION);
        spdlog::info(L"DLL_PATH={}", DLL_PATH);

        // -- 拆分路径
        std::wstring filename;
        /// 盘符
        std::vector<wchar_t> disk(8);
        /// 层级路径（不含盘符、最终文件名）
        std::vector<wchar_t> dirname(1024);
        filename = DLL_PATH;
        _wsplitpath_s(
            filename.c_str(),
            disk.data(), _MAX_DRIVE,
            dirname.data(), _MAX_DIR,
            nullptr, 0,
            nullptr, 0
        );

        // 拼接文件夹路径
        _wmakepath_s(DLL_DIR_PATH, disk.data(), dirname.data(), NULL, NULL);
        // 打印文件夹路径
        spdlog::info(L"DLL_DIR_PATH={}", DLL_DIR_PATH);

        // -- 拼接保益 DLL 完整路径
        StringCchPrintfW(boy::BOY_DLL_FULLPATH, MAX_PATH, L"%s\\%s", DLL_DIR_PATH, boy::BOY_DLL_NAME);
        spdlog::info(L"BOY_DLL_FULLPATH={}", boy::BOY_DLL_FULLPATH);
    }

    /// 加载函数指针
    FARPROC loadFunctionPtr(LPCSTR lpProcName)
    {
        if (nullptr == dllHandle)
        {
            spdlog::error("[loadFunctionPtr] nullptr == dllHandle, ret.");
            return nullptr;
        }

        SPDLOG_DEBUG("[loadFunctionPtr] GetProcAddress(dllHandle={}, lpProcName={})",
            (void*)dllHandle, lpProcName);
        auto funcHandle = GetProcAddress(dllHandle, lpProcName);
        if (!funcHandle)
        {
            spdlog::error("[loadFunctionPtr] Failed to get '{}'", lpProcName);
            freeDll();
            return nullptr;
        }

        SPDLOG_DEBUG("[loadFunctionPtr] load {} @ {}", lpProcName, (void*)funcHandle);
        return funcHandle;
    }

    /// 释放 DLL
    void freeDll()
    {
        SPDLOG_DEBUG("[freeDll] trying to free DLL: dllHandle={}", (void*)dllHandle);

        if (nullptr != dllHandle)
        {
            bool freeRet = FreeLibrary(dllHandle);
            // 成功，则返回值为非零值。
            if (!freeRet) {
                spdlog::error("[freeDll] FreeLibrary ret={}", (void*)dllHandle);
            }
            else 
            {
                SPDLOG_DEBUG("[freeDll] FreeLibrary ret={}", (void*)dllHandle);
            }
        }

        SPDLOG_DEBUG("[freeDll] After FreeLibrary: dllHandle={}", (void*)dllHandle);
    }

} // nvdll::
