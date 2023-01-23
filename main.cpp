#include "boy_impl.h"
#include "ini.hpp"
#include <iostream>
#include <sstream>
#include "loguru.hpp"
#include <cassert>
#include <stdio.h>

// ---- 私有头文件
#include "boy_global.hpp"
#include "dll.hpp"
using namespace boy;

namespace boy
{
    /// DLL 所在文件夹路径
    TCHAR DLL_DIR_PATH[MAX_PATH];
    /// 保益 DLL 完整路径
    TCHAR BOY_DLL_FULLPATH[MAX_PATH];

    /// 通过日志输出 WString 类型的变量
    void logWString(LPCSTR prefix, LPCSTR varName, LPCWSTR inWstring)
    {
        /// 输出字符串的长度，包括结尾的 null
        size_t outStringLen;
        /// 转化后的字符串，待输出
        char outString[PATH_NO_LIMIT_SIZE];
        wcstombs_s(&outStringLen, outString, (size_t)PATH_NO_LIMIT_SIZE, inWstring, (size_t)MAX_PATH - 1);
        DLOG_F(INFO, "[%s] %s[]: %s", prefix, varName, outString);
    }
}

#pragma region 全局变量定义
/// DLL 句柄
static HMODULE dllHandle;
static BoyCtrlInitializeFunc boyCtrlInitialize;
static BoyCtrlUninitializeFunc boyCtrlUninitialize;
static BoyCtrlSpeakFunc boyCtrlSpeak;
static BoyCtrlStopSpeakingFunc boyCtrlStopSpeaking;
static BoyCtrlPauseScreenReaderFunc boyCtrlPauseScreenReader;
#pragma region

#pragma region 加载保益 DLL
/// 释放 DLL
void freeDll()
{
    DLOG_F(INFO, "[freeDll] trying to free DLL: dllHandle=%x", dllHandle);

    if (nullptr != dllHandle)
    {
        bool freeRet = FreeLibrary(dllHandle);
        DLOG_F(INFO, "[freeDll] FreeLibrary ret=%d", freeRet);
    }

    DLOG_F(INFO, "[freeDll] ret: dllHandle=%x", dllHandle);
}

/// 加载函数指针
FARPROC loadFunctionPtr(_In_ LPCSTR lpProcName)
{
    std::stringstream eout;

    if (nullptr == dllHandle)
    {
        eout.clear();
        eout << "[loadFunctionPtr] "
            << "DLL dllHandle==nullptr. "
            << "DLL 句柄为空。"
            << std::endl;
        DLOG_F(INFO, eout.str().c_str());
        return nullptr;
    }

    DLOG_F(INFO, 
        "[loadFunctionPtr] GetProcAddress(dllHandle=%x, lpProcName=%s)", 
        dllHandle, lpProcName);
    auto funcHandle = GetProcAddress(dllHandle, lpProcName);
    if (!funcHandle)
    {
        eout.clear();
        eout << "[loadFunctionPtr] "
            << "Failed to get '" << lpProcName << "'. " 
            << "获取函数指针失败：'" << lpProcName << "'" 
            << std::endl;
        DLOG_F(INFO, eout.str().c_str());
        freeDll();
        return nullptr;
    }

    DLOG_F(INFO, "[loadFunctionPtr] ret: funcHandle=%x", funcHandle);
    return funcHandle;
}


/// 加载 DLL 及导入函数
bool loadBaoYiDll()
{
    std::stringstream eout;

    // -- 加载 DLL
    // 使用完整路径加载：保益 DLL 和 nvda 放在一起
    if (nullptr == dllHandle)
    {
        dllHandle = LoadLibrary(BOY_DLL_FULLPATH);
    }
    // 仅使用 DLL 名加载：保益 DLL 和主程序 exe 放在一起
    if (nullptr == dllHandle)
    {
        dllHandle = LoadLibrary(BOY_DLL_NAME);
    }
    // 检查 DLL 是否成功加载
    if (!dllHandle)
    {
        // TODO: 打印错误详细信息。打印实际的 dll 名，处理 wstring => string
        eout.clear();
        eout
            << "[loadBaoYiDll] Failed to load DLL. "
            << "尝试加载 DLL 失败。"
            << "错误原因为：" << GetLastError()
            << std::endl;
        DLOG_F(INFO, eout.str().c_str());
        return EXIT_FAILURE;
    }

    // 加载配置文件
    ini::loadIni();

    // -- 加载函数
    boyCtrlInitialize = (BoyCtrlInitializeFunc) loadFunctionPtr("BoyCtrlInitialize");
    if (nullptr == boyCtrlInitialize) return EXIT_FAILURE;
    boyCtrlUninitialize = (BoyCtrlUninitializeFunc) loadFunctionPtr("BoyCtrlUninitialize");
    if (nullptr == boyCtrlUninitialize) return EXIT_FAILURE;
    boyCtrlSpeak = (BoyCtrlSpeakFunc)loadFunctionPtr("BoyCtrlSpeak");
    if (nullptr == boyCtrlSpeak) return EXIT_FAILURE;
    boyCtrlStopSpeaking = (BoyCtrlStopSpeakingFunc)loadFunctionPtr("BoyCtrlStopSpeaking");
    if (nullptr == boyCtrlStopSpeaking) return EXIT_FAILURE;
    boyCtrlPauseScreenReader = (BoyCtrlPauseScreenReaderFunc)loadFunctionPtr("BoyCtrlPauseScreenReader");
    if (nullptr == boyCtrlPauseScreenReader) return EXIT_FAILURE;

    // -- 初始化 DLL
    // 日志放在当前工作目录
    auto err = boyCtrlInitialize(DLL_LOG_NAME);
    if (err != e_bcerr_success)
    {
        eout.clear();
        eout << "[loadBaoYiDll] "
            << "Initialize failed. "
            << "初始化失败，调用返回值为：" << err 
            << std::endl;
        DLOG_F(INFO, eout.str().c_str());
        freeDll();
        return EXIT_FAILURE;
    }

    eout.clear();
    eout << "[loadBaoYiDll] "
        << "API Ready! " 
        << "DLL API 初始化成功。"
        << std::endl;
    DLOG_F(INFO, eout.str().c_str());
    return EXIT_SUCCESS;
}

/**
 * @brief 回调函数
 * @param reason 回调原因（调用是否成功）
 * @return void
*/
void __stdcall speakCompleteCallback(int reason)
{
    return;
}
#pragma endregion


#pragma region DLL 导出函数实现
/// 返回值转换：BoyCtrlError => error_status_t
error_status_t convertBoyCtrlError(BoyCtrlError err)
{
    switch (err)
    {
    case e_bcerr_success:
        return RPC_S_OK;
    case e_bcerr_fail:
        return RPC_S_CALL_FAILED;
    case e_bcerr_arg:
        return RPC_S_INVALID_ARG;
    case e_bcerr_unavailable:
        return RPC_S_CANNOT_SUPPORT;

    default:
        return RPC_S_CALL_FAILED;
    }
}

/// 单例模式。尝试在第一次使用时加载 DLL
void initDllIfNull()
{
    if (nullptr == dllHandle)
    {
        DLOG_F(INFO, "nullptr == dllHandle: start to loadBaoYiDll()...");
        loadBaoYiDll();
    }
}

error_status_t __stdcall testIfRunning_impl()
{
    if (nullptr == dllHandle)
    {
        DLOG_F(INFO, "[testIfRunning_impl] nullptr == dllHandle: trying to loadBaoYiDll()...");
        bool has_error = loadBaoYiDll();
        if (has_error) {
            DLOG_F(INFO, "[testIfRunning_impl] loadBaoYiDll() load error!");
            return RPC_X_SS_CONTEXT_MISMATCH;
        }
    }

    DLOG_F(INFO, "[testIfRunning_impl] loadBaoYiDll() load finished. dllHandle=%x", dllHandle);
    assert(nullptr != dllHandle);
    return RPC_S_OK;
}

error_status_t __stdcall speakText_impl(const wchar_t* text)
{
#ifdef _DEBUG
    std::wcout << "[speakText_impl] "
        << "text=" << text
        << std::endl;
#endif // def _DEBUG

    if (nullptr == boyCtrlSpeak)
    {
        bool has_error = loadBaoYiDll();
        if (has_error) {
            DLOG_F(INFO, "[speakText_impl] loadBaoYiDll() load error!");
            return RPC_X_SS_CONTEXT_MISMATCH;
        }

        DLOG_F(INFO, "[speakText_impl] loadBaoYiDll() load finished. boyCtrlSpeak=%x", boyCtrlSpeak);
        assert(nullptr != boyCtrlSpeak);
    }

    auto err = boyCtrlSpeak(text, ini::SPEAK_WITH_SLAVE, ini::SPEAK_APPEND, ini::SPEAK_ALLOW_BREAK, speakCompleteCallback);

#ifdef _DEBUG
    std::wcout << "[speakText_impl] "
        << "ret err=" << err
        << std::endl;
#endif // def _DEBUG

    return convertBoyCtrlError(err);
}

error_status_t __stdcall cancelSpeech_impl()
{
    if (nullptr == boyCtrlStopSpeaking)
    {
        bool has_error = loadBaoYiDll();
        if (has_error) {
            DLOG_F(INFO, "[cancelSpeech_impl] loadBaoYiDll() load error!");
            return RPC_X_SS_CONTEXT_MISMATCH;
        }

        DLOG_F(INFO, "[cancelSpeech_impl] loadBaoYiDll() load finished. boyCtrlStopSpeaking=%x", boyCtrlSpeak);
        assert(nullptr != boyCtrlStopSpeaking);
    }

    auto err = boyCtrlStopSpeaking(ini::SPEAK_WITH_SLAVE);
    return convertBoyCtrlError(err);
}

error_status_t __stdcall brailleMessage_impl(const wchar_t* message)
{
    return RPC_S_CANNOT_SUPPORT;
}
#pragma endregion


#ifndef BUILD_EXE
/**
 * @brief DLL 主函数
 * @param hinstDLL handle to DLL module
 * @param fdwReason reason for calling function
 * @param __lpvReserved reserved
 * @return 
*/
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID __lpvReserved)
{
    // 派发调用原因
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        {
            // Initialize once for each new process.
#ifdef _DEBUG
            loguru::add_file("fakeNvda-debug.log", loguru::Append, loguru::Verbosity_INFO);
#endif // def _DEBUG
            DLOG_F(INFO, "loguru init.");
            DLOG_F(INFO, "BaoYi Dll API Version: %s", BOY_DLL_VERSION);
            DLOG_F(INFO, "Compiled at: %s %s", __DATE__, __TIME__);

            dll::saveDllDirPath(hinstDLL);
        }
        break;

    case DLL_THREAD_ATTACH:
        // Do thread-specific initialization.
        break;

    case DLL_THREAD_DETACH:
        // Do thread-specific cleanup.
        break;

    case DLL_PROCESS_DETACH:
        {
            if (__lpvReserved != nullptr)
            {
                // do not do cleanup if process termination scenario
                break; 
            }

            // Perform any necessary cleanup.
            DLOG_F(INFO, "Perform any necessary cleanup.");
            freeDll();
        }
        break;
    }

    return TRUE;
}

#else  // defined( BUILD_EXE )
/// exe 主函数
int main()
{
    std::cout 
        << "[DllMain] BaoYi Dll API Version: " << BOY_DLL_VERSION << "\n"
        << "[DllMain] Compiled at: " << __DATE__ << " " << __TIME__
        << std::endl;

    bool has_error = loadBaoYiDll();
    if (has_error) {
        exit(EXIT_FAILURE);
    }

    std::cout << "Hello World!\n";
    return EXIT_SUCCESS;
}
#endif // BUILD_EXE
