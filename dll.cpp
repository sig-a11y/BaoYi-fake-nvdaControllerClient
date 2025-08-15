#include "dll.hpp" // 项目内导出
#include "nvdll_impl.h" // API 导出
// -- [sys] win
#include <stdlib.h> // _wsplitpath_s, _wmakepath_s
#include <strsafe.h> // StringCchPrintf
#include <sstream> // stringstream
#include <string> // wstring
#include <vector> // vector
// -- 3rd
#include "boysr.hpp"
#include "zdsr.hpp"
// -- [proj]
#include "log.hpp" // nvdll::log::; DLOG_F


namespace nvdll {

#pragma region 全局变量定义
    /// DLL 所在文件夹路径
    TCHAR DLL_DIR_PATH[MAX_PATH];
    HMODULE dllHandle;
#pragma region

    /**
     * 打印 DLL 信息
     */
    void printDllInfo()
    {
        spdlog::info("BaoYi-fake-nvdaControllerClient");
        spdlog::info("  Github URL:  https://github.com/sig-a11y/BaoYi-fake-nvdaControllerClient");
        spdlog::info("  Author: inkydragon @ github");

        // DLL 接口信息
        spdlog::info("DLL Compiled at: {} {}", __DATE__, __TIME__);
        spdlog::info("  NVDA Client API Version: {}", nvdll::NVDA_API_VERSION);
#   ifdef FAKE_NVDA_ZDSR
        spdlog::info(L"  ZDSR Dll API Version: {}", zdsr_api::DLL_VERSION);
#   else
        spdlog::info(L"  BaoYi Dll API Version: {}", boysr_api::DLL_VERSION);
#   endif
    }

    /**
     * @brief 获取并保存 DLL 所在文件夹路径.
     * @param hinstDLL DLL 实例句柄
     */
    void saveDllDirPath(HINSTANCE hinstDLL)
    {
        spdlog::info(L"Get DLL Path...");

        /// DLL 路径
        TCHAR DLL_PATH[MAX_PATH];
        // 获取 DLL 完整路径
        GetModuleFileName(hinstDLL, DLL_PATH, MAX_PATH);

        // -- 打印完整路径
        spdlog::debug(L"  NVDA DLL Path:\t{}", DLL_PATH);

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
        spdlog::debug(L"  DLL DIR PATH:\t{}", DLL_DIR_PATH);

        // -- 拼接 DLL 完整路径
#   ifdef FAKE_NVDA_ZDSR
        StringCchPrintfW(zdsr::DLL_FULLPATH, MAX_PATH, L"%s\\%s", DLL_DIR_PATH, zdsr_api::DLL_NAME);
        spdlog::info(L"  SR DLL path:\t{}", zdsr::DLL_FULLPATH);
#   else
        StringCchPrintfW(boy::BOY_DLL_FULLPATH, MAX_PATH, L"%s\\%s", DLL_DIR_PATH, boysr_api::DLL_NAME);
        spdlog::info(L"  SR DLL path:\t{}", boy::BOY_DLL_FULLPATH);
#   endif
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

    void loadDLL()
    {
#ifdef FAKE_NVDA_ZDSR
        nvdll::zdsr::loadDLL();
#else
        nvdll::boy::loadDLL();
#endif
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


#pragma region DLL 导出函数实现

error_status_t __stdcall testIfRunning_impl()
{
#ifdef FAKE_NVDA_ZDSR
    return nvdll::zdsr::testIfRunning_impl();
#else
    return nvdll::boy::testIfRunning_impl();
#endif
}

error_status_t __stdcall speakText_impl(const wchar_t* text)
{
#ifdef FAKE_NVDA_ZDSR
    return nvdll::zdsr::speakText_impl(text);
#else
    return nvdll::boy::speakText_impl(text);
#endif
}

error_status_t __stdcall cancelSpeech_impl()
{
#ifdef FAKE_NVDA_ZDSR
    return nvdll::zdsr::cancelSpeech_impl();
#else
    return nvdll::boy::cancelSpeech_impl();
#endif
}

error_status_t __stdcall brailleMessage_impl(const wchar_t* message)
{
    return RPC_S_CANNOT_SUPPORT;
}


/* V2.0 functions */

error_status_t __stdcall getProcessId_impl(unsigned long* pid)
{
    if (nullptr != pid)
    {
        spdlog::debug("[getProcessId_impl] *pid={}", *pid);  // 发布版本输出
    }
    spdlog::debug("[getProcessId_impl] pid={}", (void*)pid);  // 发布版本输出

    return RPC_S_CANNOT_SUPPORT;
}

error_status_t __stdcall speakSsml_impl(
    const wchar_t* ssml,
    const enum SYMBOL_LEVEL symbolLevel = SYMBOL_LEVEL_UNCHANGED,
    const enum SPEECH_PRIORITY priority = SPEECH_PRIORITY_NORMAL,
    const bool asynchronous = true)
{
    spdlog::debug(L"[speakSsml_impl] ssml={}, symbolLevel={}, priority={}, asynchronous={}",
        ssml, (int)symbolLevel, (int)priority, asynchronous);

    return RPC_S_CANNOT_SUPPORT;
}

error_status_t __stdcall setOnSsmlMarkReachedCallback_impl(onSsmlMarkReachedFuncType callback)
{
    spdlog::debug("[setOnSsmlMarkReachedCallback_impl] callback={}", (void*)callback);
    return RPC_S_CANNOT_SUPPORT;
}

#pragma endregion
