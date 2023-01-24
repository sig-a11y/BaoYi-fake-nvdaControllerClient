#include "nvdll_impl.h" // API 导出
#include "dll.hpp" // 项目内导出
// -- [sys] win 
#include <sstream> // stringstream
#include <cassert> // assert
#include <iostream> // wcout
// -- [3rd]
#include "BoyCtrl.h" // 保益 DLL 接口导入
// -- [proj]
#include "log.hpp" // nvdll::log::; DLOG_F
#include "ini.hpp" // ini:: loadIni; SPEAK_WITH_SLAVE, SPEAK_APPEND, SPEAK_ALLOW_BREAK


using namespace dll;
using namespace dll::boy;


#pragma region 局部变量定义
namespace dll {
    namespace boy
    {
        /// 保益 DLL 完整路径
        TCHAR BOY_DLL_FULLPATH[MAX_PATH];
    } // dll::boy::
} // dll::

static BoyCtrlInitializeFunc boyCtrlInitialize;
static BoyCtrlUninitializeFunc boyCtrlUninitialize;
static BoyCtrlSpeakFunc boyCtrlSpeak;
static BoyCtrlStopSpeakingFunc boyCtrlStopSpeaking;
static BoyCtrlPauseScreenReaderFunc boyCtrlPauseScreenReader;
#pragma region

#pragma region 加载保益 DLL
/// 加载 DLL 及导入函数
bool loadBaoYiDll()
{
    std::stringstream eout;

    // -- 加载 DLL
    // 使用完整路径加载：保益 DLL 和 nvda 放在一起
    if (nullptr == dllHandle)
    {
        SPDLOG_DEBUG(L"[loadBaoYiDll] trying to load dll: {}", BOY_DLL_FULLPATH);
        dllHandle = LoadLibrary(BOY_DLL_FULLPATH);
        SPDLOG_DEBUG(L"[loadBaoYiDll]   dllHandle={}", (void*)dllHandle);
    }
    // 仅使用 DLL 名加载：保益 DLL 和主程序 exe 放在一起
    if (nullptr == dllHandle)
    {
        SPDLOG_DEBUG(L"[loadBaoYiDll] trying to load dll: {}", BOY_DLL_FULLPATH);
        dllHandle = LoadLibrary(BOY_DLL_NAME);
        SPDLOG_DEBUG(L"[loadBaoYiDll]   dllHandle={}", (void*)dllHandle);
    }
    // 检查 DLL 是否成功加载
    if (!dllHandle)
    {
        spdlog::error(L"[loadBaoYiDll] Failed to load DLL '{}'. 尝试加载 DLL 失败。", BOY_DLL_NAME);
        spdlog::error(L"[loadBaoYiDll] GetLastError={}", GetLastError());
        return EXIT_FAILURE;
    }

    // 加载配置文件
    ini::loadIni(DLL_DIR_PATH);

    // -- 加载函数
    boyCtrlInitialize = (BoyCtrlInitializeFunc)loadFunctionPtr("BoyCtrlInitialize");
    if (nullptr == boyCtrlInitialize) return EXIT_FAILURE;
    boyCtrlUninitialize = (BoyCtrlUninitializeFunc)loadFunctionPtr("BoyCtrlUninitialize");
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
        spdlog::error(L"[loadBaoYiDll] Initialize failed. 初始化失败。");
        spdlog::error(L"[loadBaoYiDll]   ret={}", (int)err);
        freeDll();
        return EXIT_FAILURE;
    }

    SPDLOG_DEBUG(L"API Ready! DLL API 初始化成功。");
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
        spdlog::info("nullptr==dllHandle: trying to load dll...");
        loadBaoYiDll();
    }
}

error_status_t __stdcall testIfRunning_impl()
{
    if (nullptr == dllHandle)
    {
        bool has_error = loadBaoYiDll();
        if (has_error) {
            spdlog::error("[testIfRunning_impl] loadBaoYiDll() load error!");
            return RPC_X_SS_CONTEXT_MISMATCH;
        }
    }

    SPDLOG_DEBUG("[testIfRunning_impl] loadBaoYiDll() load finished. dllHandle={}", (void*)dllHandle);
    assert(nullptr != dllHandle);
    return RPC_S_OK;
}

error_status_t __stdcall speakText_impl(const wchar_t* text)
{
    SPDLOG_DEBUG(L"[speakText_impl] text={}", text);

    if (nullptr == boyCtrlSpeak)
    {
        bool has_error = loadBaoYiDll();
        if (has_error) {
            spdlog::error("[speakText_impl] loadBaoYiDll() load error!");
            return RPC_X_SS_CONTEXT_MISMATCH;
        }

        SPDLOG_DEBUG("[speakText_impl] loadBaoYiDll() load finished. boyCtrlSpeak={}", (void*)boyCtrlSpeak);
        assert(nullptr != boyCtrlSpeak);
    }

    auto err = boyCtrlSpeak(text, ini::SPEAK_WITH_SLAVE, ini::SPEAK_APPEND, ini::SPEAK_ALLOW_BREAK, speakCompleteCallback);
    SPDLOG_DEBUG("[speakText_impl] ret={}", (int)err);

    return convertBoyCtrlError(err);
}

error_status_t __stdcall cancelSpeech_impl()
{
    if (nullptr == boyCtrlStopSpeaking)
    {
        bool has_error = loadBaoYiDll();
        if (has_error) {
            spdlog::error("[cancelSpeech_impl] loadBaoYiDll() load error!");
            return RPC_X_SS_CONTEXT_MISMATCH;
        }

        SPDLOG_DEBUG("[cancelSpeech_impl] loadBaoYiDll() load finished. boyCtrlStopSpeaking={}", (void*)boyCtrlStopSpeaking);
        assert(nullptr != boyCtrlStopSpeaking);
    }

    auto err = boyCtrlStopSpeaking(ini::SPEAK_WITH_SLAVE);
    SPDLOG_DEBUG("[cancelSpeech_impl] ret={}", (int)err);
    return convertBoyCtrlError(err);
}

error_status_t __stdcall brailleMessage_impl(const wchar_t* message)
{
    return RPC_S_CANNOT_SUPPORT;
}
#pragma endregion
