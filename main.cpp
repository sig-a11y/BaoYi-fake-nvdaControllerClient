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
