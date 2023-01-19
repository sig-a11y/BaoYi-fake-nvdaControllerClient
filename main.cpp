#include "boy_impl.h"
#include "BoyCtrl.h"
#include <iostream>
#include <sstream>
#include <Windows.h>
#include "loguru.hpp"
#include <cassert>


#pragma region 加载保益 DLL
/// 保益 DLL 版本
const LPCSTR BOY_DLL_VERSION = "v1.5.2";

#ifdef _WIN64
/// 保益 DLL 文件名
const LPCWSTR DLL_NAME = L"BoyCtrl-x64.dll";
#else
/// 保益 DLL 文件名
const LPCWSTR DLL_NAME = L"BoyCtrl.dll";
#endif // def _WIN64

#ifdef _DEBUG
/// 输出日志名称
const LPCWSTR DLL_LOG_NAME = L"boyCtrl-debug.log";
#else // release mode
/// 置空，不输出日志
const LPCWSTR DLL_LOG_NAME = nullptr;
#endif // def _DEBUG

/// DLL 句柄
static HMODULE dllHandle;
static BoyCtrlInitializeFunc boyCtrlInitialize;
static BoyCtrlUninitializeFunc boyCtrlUninitialize;
static BoyCtrlSpeakFunc boyCtrlSpeak;
static BoyCtrlStopSpeakingFunc boyCtrlStopSpeaking;
static BoyCtrlPauseScreenReaderFunc boyCtrlPauseScreenReader;

// -- 参数常量
/// false=使用读屏通道，true=使用独立通道
const bool SPEAK_WITH_SLAVE = true;
/// 是否排队朗读
const bool SPEAK_APPEND = false;
/// 是否允许用户打断.使用读屏通道时该参数被忽略
const bool SPEAK_ALLOW_BREAK = true;


/// 释放 DLL
void freeDll()
{
    if (nullptr != dllHandle)
    {
        bool freeRet = FreeLibrary(dllHandle);
        DLOG_F(INFO, "FreeLibrary ret=%d", freeRet);
    }
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

    return funcHandle;
}


/// 加载 DLL 及导入函数
bool loadBaoYiDll()
{
    std::stringstream eout;

    // -- 加载 DLL
    dllHandle = LoadLibrary(DLL_NAME);
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
        DLOG_F(INFO, "nullptr == dllHandle: trying to loadBaoYiDll()...");
        bool has_error = loadBaoYiDll();
        if (has_error) {
            return RPC_X_SS_CONTEXT_MISMATCH;
        }

        DLOG_F(INFO, "loadBaoYiDll() load finished. dllHandle=%d", dllHandle);
    }

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
            return RPC_X_SS_CONTEXT_MISMATCH;
        }
        assert(nullptr != boyCtrlSpeak);
    }

    auto err = boyCtrlSpeak(text, SPEAK_WITH_SLAVE, SPEAK_APPEND, SPEAK_ALLOW_BREAK, speakCompleteCallback);

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
            return RPC_X_SS_CONTEXT_MISMATCH;
        }
        assert(nullptr != boyCtrlStopSpeaking);
    }

    auto err = boyCtrlStopSpeaking(SPEAK_WITH_SLAVE);
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
