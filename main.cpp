#include "boy_impl.h"
#include "BoyCtrl.h"
#include "SimpleIni.h"
#include <iostream>
#include <sstream>
#include <Windows.h>
#include "loguru.hpp"
#include <cassert>
#include <pathcch.h>
#pragma comment(lib, "pathcch.lib")
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <vector>
#include <stdio.h>
#include <strsafe.h>  // StringCchPrintfA


#pragma region 常量及全局变量定义
/// 允许的长路径长度
const int PATH_NO_LIMIT_SIZE = 1024;
/// DLL 所在文件夹路径
TCHAR DLL_DIR_PATH[MAX_PATH];

/// 保益 DLL 版本
const LPCSTR BOY_DLL_VERSION = "v1.5.2";

#ifdef _WIN64
/// 保益 DLL 文件名
const LPCWSTR BOY_DLL_NAME = L"BoyCtrl-x64.dll";
#else
/// 保益 DLL 文件名
const LPCWSTR BOY_DLL_NAME = L"BoyCtrl.dll";
#endif // def _WIN64
/// 保益 DLL 完整路径
TCHAR BOY_DLL_FULLPATH[MAX_PATH];

#ifdef _DEBUG
/// 输出日志名称
const LPCWSTR DLL_LOG_NAME = L"boyCtrl-debug.log";
#else // release mode
/// 置空，不输出日志
const LPCWSTR DLL_LOG_NAME = nullptr;
#endif // def _DEBUG

/// 配置文件对象
static CSimpleIniW ini;
/// 保益 DLL 配置 ini 文件名
const LPCWSTR INI_NAME = L"朗读配置.ini";
//const LPCWSTR INI_APP_NAME = L"朗读";
const LPCWSTR INI_APP_NAME = L"APP";
//const LPCWSTR INI_KEY_USE_SLAVE = L"独立通道";
const LPCWSTR INI_KEY_USE_SLAVE = L"USE_SLAVE";
//const LPCWSTR INI_KEY_USE_APPEND = L"排队朗读";
const LPCWSTR INI_KEY_USE_APPEND = L"USE_APPEND";
//const LPCWSTR INI_KEY_ALLOW_BREAK = L"按键打断朗读";
const LPCWSTR INI_KEY_ALLOW_BREAK = L"ALLOW_BREAK";


/// DLL 句柄
static HMODULE dllHandle;
static BoyCtrlInitializeFunc boyCtrlInitialize;
static BoyCtrlUninitializeFunc boyCtrlUninitialize;
static BoyCtrlSpeakFunc boyCtrlSpeak;
static BoyCtrlStopSpeakingFunc boyCtrlStopSpeaking;
static BoyCtrlPauseScreenReaderFunc boyCtrlPauseScreenReader;

// -- 参数常量
/// false=使用读屏通道，true=使用独立通道
bool SPEAK_WITH_SLAVE = true;
/// 是否排队朗读
bool SPEAK_APPEND = true;
/// 是否允许用户打断.使用读屏通道时该参数被忽略
bool SPEAK_ALLOW_BREAK = true;
#pragma region


/// 通过日志输出 WString 类型的变量
void logWString(LPCSTR varName, LPCWSTR inWstring)
{
    /// 输出字符串的长度，包括结尾的 null
    size_t outStringLen;
    /// 转化后的字符串，待输出
    char outString[PATH_NO_LIMIT_SIZE];
    wcstombs_s(&outStringLen, outString, (size_t)PATH_NO_LIMIT_SIZE, inWstring, (size_t)MAX_PATH - 1);
    DLOG_F(INFO, "%s[]: %s", varName, outString);
}


#pragma region ini 配置文件加载
/// 加载配置文件
/// 非0值作为 true
void loadIni()
{
    DLOG_F(INFO, "[loadIni] begin to load ini...");
    /// 输出消息长度
    size_t i;
    /// log 消息
    char errMsg[PATH_NO_LIMIT_SIZE];
    /// ini 路径
    //TCHAR iniPath[PATH_NO_LIMIT_SIZE] = L"E:\\game\\ShadowRine_FullVoice\\朗读配置.ini";
    TCHAR iniPath[PATH_NO_LIMIT_SIZE];

    // ==== 拼接 ini 完整路径，尝试加载
    // TODO: 显式构造长路径 "\\?\"
    PathCchCombineEx(iniPath, PATH_NO_LIMIT_SIZE, DLL_DIR_PATH, INI_NAME, PATHCCH_ALLOW_LONG_PATHS);
    wcstombs_s(&i, errMsg, (size_t)PATH_NO_LIMIT_SIZE, iniPath, (size_t)PATH_NO_LIMIT_SIZE - 1);
    DLOG_F(INFO, "[loadIni] iniPath=%s", errMsg);
    bool exist = PathFileExists(iniPath);
    DLOG_F(INFO, "[loadIni] PathFileExists=%d", exist);
    SI_Error rc = ini.LoadFile(iniPath);
    if (rc < 0) 
    {
        DLOG_F(INFO, "[loadIni] ini.LoadFile rc=%x", rc);
    }
    else 
    {
        LPCWSTR pv;
        pv = ini.GetValue(INI_APP_NAME, INI_KEY_USE_SLAVE, L"1");
        DLOG_F(INFO, "[loadIni]     slave=%d; SPEAK_WITH_SLAVE=%d", pv, pv != 0);

        pv = ini.GetValue(L"朗读", L"独立通道", L"1");
        DLOG_F(INFO, "[loadIni]     朗读.独立通道=%d;", pv);
    }

    // 拼接的路径不存在，尝试直接读取 ini
    if (!exist)
    {
        PathCchCombineEx(iniPath, PATH_NO_LIMIT_SIZE, INI_NAME, NULL, PATHCCH_ALLOW_LONG_PATHS);
        wcstombs_s(&i, errMsg, (size_t)PATH_NO_LIMIT_SIZE, iniPath, (size_t)PATH_NO_LIMIT_SIZE - 1);
        DLOG_F(INFO, "[loadIni] iniPath=%s", errMsg);
        exist = PathFileExists(iniPath);
        DLOG_F(INFO, "[loadIni] INI_NAME Exists=%d", exist);
    }
    
    // ==== 读取 ini 配置
    int slave = GetPrivateProfileIntW(INI_APP_NAME, INI_KEY_USE_SLAVE, 1, iniPath);
    SPEAK_WITH_SLAVE = 0 != slave;
    DLOG_F(INFO, "[loadIni]     slave=%d; SPEAK_WITH_SLAVE=%d", slave, SPEAK_WITH_SLAVE);

    int append = GetPrivateProfileIntW(INI_APP_NAME, INI_KEY_USE_APPEND, 1, iniPath);
    SPEAK_APPEND = 0 != append;
    DLOG_F(INFO, "[loadIni]     append=%d; SPEAK_APPEND=%d", append, SPEAK_APPEND);

    int allowBreak = GetPrivateProfileIntW(INI_APP_NAME, INI_KEY_ALLOW_BREAK, 1, iniPath);
    SPEAK_ALLOW_BREAK = 0 != allowBreak;
    DLOG_F(INFO, "[loadIni]     allowBreak=%d; SPEAK_ALLOW_BREAK=%d", allowBreak, SPEAK_ALLOW_BREAK);

    DLOG_F(INFO, "[loadIni] load ini finished.");
}
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
    if (nullptr == dllHandle)
    {
        dllHandle = LoadLibrary(BOY_DLL_NAME);
    }
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
    loadIni();

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
            DLOG_F(INFO, "[cancelSpeech_impl] loadBaoYiDll() load error!");
            return RPC_X_SS_CONTEXT_MISMATCH;
        }

        DLOG_F(INFO, "[cancelSpeech_impl] loadBaoYiDll() load finished. boyCtrlStopSpeaking=%x", boyCtrlSpeak);
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
    size_t i;
    char errMsg[PATH_NO_LIMIT_SIZE];
    wcstombs_s(&i, errMsg, (size_t)PATH_NO_LIMIT_SIZE, DLL_PATH, (size_t)MAX_PATH - 1);
    DLOG_F(INFO, "DLL_PATH[]: %s", errMsg);

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
    // -- 打印文件夹路径
    char errMsg2[PATH_NO_LIMIT_SIZE];
    wcstombs_s(&i, errMsg2, (size_t)PATH_NO_LIMIT_SIZE, DLL_DIR_PATH, (size_t)MAX_PATH - 1);
    DLOG_F(INFO, "DLL_DIR_PATH[]: %s", errMsg2);

    // -- 拼接保益 DLL 完整路径
    StringCchPrintfW(BOY_DLL_FULLPATH, MAX_PATH, L"%s\\%s", DLL_DIR_PATH, BOY_DLL_NAME);
    logWString("BOY_DLL_FULLPATH", BOY_DLL_FULLPATH);
}

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

            saveDllDirPath(hinstDLL);
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
