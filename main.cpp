#include "nvda.h"
#include <iostream>
#include "BoyCtrl.h"
#include <Windows.h>


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
/// 输出日志名称
const LPCWSTR DLL_LOG_NAME = L"boyCtrl.log";

/// DLL 句柄
HMODULE dllHandle;
BoyCtrlInitializeFunc boyCtrlInitialize;
BoyCtrlUninitializeFunc boyCtrlUninitialize;
BoyCtrlSpeakFunc boyCtrlSpeak;
BoyCtrlStopSpeakingFunc boyCtrlStopSpeaking;
BoyCtrlPauseScreenReaderFunc boyCtrlPauseScreenReader;

// -- 参数常量
/// false=使用读屏通道，true=使用独立通道
const bool SPEAK_WITH_SLAVE = true;
/// 是否排队朗读
const bool SPEAK_APPEND = false;
/// 是否允许用户打断.使用读屏通道时该参数被忽略
const bool SPEAK_ALLOW_BREAK = true;


/// 加载函数指针
FARPROC loadFunctionPtr(_In_ LPCSTR lpProcName)
{
    if (nullptr == dllHandle)
    {
        std::cerr << "[loadFunctionPtr] "
            << "DLL dllHandle==nullptr. "
            << "DLL 句柄为空。"
            << std::endl;
        return nullptr;
    }

    auto funcHandle = GetProcAddress(dllHandle, lpProcName);
    if (!funcHandle)
    {
        std::cerr << "[loadFunctionPtr] "
            << "Failed to get '" << lpProcName << "'. " 
            << "获取函数指针失败：'" << lpProcName << "'" 
            << std::endl;
        FreeLibrary(dllHandle);
        return nullptr;
    }

    return funcHandle;
}

/// 加载 DLL 及导入函数
bool loadBaoYiDll()
{
    // -- 加载 DLL
    dllHandle = LoadLibrary(DLL_NAME);
    if (!dllHandle)
    {
        std::wcout 
            << "[loadBaoYiDll] Failed to load " << DLL_NAME << ". "
            << "尝试加载 " << DLL_NAME << " 失败。"
            << "错误原因为：" << GetLastError()
            << std::endl;
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
        std::cerr << "[loadBaoYiDll] "
            << "Initialize failed. "
            << "初始化失败，调用返回值为：" << err 
            << std::endl;
        FreeLibrary(dllHandle);
        return EXIT_FAILURE;
    }

    std::cout << "[loadBaoYiDll] "
        << "API Ready! " 
        << "DLL API 初始化成功。"
        << std::endl;
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

error_status_t __stdcall nvdaController_testIfRunning()
{
    if (nullptr == dllHandle)
    {
        return RPC_S_CALL_FAILED;
    }
    else
    {
        return RPC_S_OK;
    }
}

error_status_t __stdcall nvdaController_speakText(const wchar_t* text)
{
#ifdef _DEBUG
    std::wcout << "[nvdaController_speakText] "
        << "text=" << text
        << std::endl;
#endif // def _DEBUG

    auto err = boyCtrlSpeak(text, SPEAK_WITH_SLAVE, SPEAK_APPEND, SPEAK_ALLOW_BREAK, speakCompleteCallback);

#ifdef _DEBUG
    std::wcout << "[nvdaController_speakText] "
        << "ret err=" << err
        << std::endl;
#endif // def _DEBUG

    return convertBoyCtrlError(err);
}

error_status_t __stdcall nvdaController_cancelSpeech()
{
    auto err = boyCtrlStopSpeaking(SPEAK_WITH_SLAVE);
    return convertBoyCtrlError(err);
}

error_status_t __stdcall nvdaController_brailleMessage(const wchar_t* message)
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
    std::cout
        << "[DllMain] BaoYi Dll API Version: " << BOY_DLL_VERSION << "\n"
        << "[DllMain] Compiled at: " << __DATE__ << " " << __TIME__
        << std::endl;

    // 派发调用原因
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        {
            // Initialize once for each new process.
            bool has_error = loadBaoYiDll();
            if (has_error) {
                // Return FALSE to fail DLL load.
                return FALSE;
            }
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
            FreeLibrary(dllHandle);
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
