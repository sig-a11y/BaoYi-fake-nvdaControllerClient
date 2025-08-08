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
#include "input.hpp"


#pragma region 局部变量定义
static BoyCtrlInitializeFunc boyCtrlInitialize;
static BoyCtrlUninitializeFunc boyCtrlUninitialize;
static BoyCtrlSpeakFunc boyCtrlSpeak;
static BoyCtrlStopSpeakingFunc boyCtrlStopSpeaking;
static BoyCtrlPauseScreenReaderFunc boyCtrlPauseScreenReader;
static BoyCtrlIsReaderRunningFunc boyCtrlIsReaderRunning;
#pragma region


namespace nvdll 
{
namespace boy
{
#pragma region 加载保益 DLL
    /// 保益 DLL 完整路径
    TCHAR BOY_DLL_FULLPATH[MAX_PATH];

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
        nvdll::ini::loadIni(DLL_DIR_PATH);

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
        boyCtrlIsReaderRunning = (BoyCtrlIsReaderRunningFunc)loadFunctionPtr("BoyCtrlIsReaderRunning");
        if (nullptr == boyCtrlIsReaderRunning) return EXIT_FAILURE;

        // -- 初始化 DLL
        // 开启调试日志(DEBUG_LOG=1)：生成保益的日志
        auto logName = ini::GEN_BOY_LOG ? DLL_LOG_NAME : nullptr;
        spdlog::info(L"[loadBaoYiDll] boyCtrlInitialize(logName={})", logName ? logName : L"nullptr");
        auto err = boyCtrlInitialize(logName);
        if (err != e_bcerr_success)
        {
            spdlog::error(L"[loadBaoYiDll] Initialize failed. 初始化失败。");
            spdlog::error(L"[loadBaoYiDll]   ret={}", (int)err);
            freeDll();
            return EXIT_FAILURE;
        }

        // 开启按任意键打断
        if (ini::SPEAK_ALL_KEY_BREAK)
        {
            // input::runInputListener();
            input::setInputHook();
        }

        SPDLOG_DEBUG(L"API Ready! DLL API 初始化成功。");
        return EXIT_SUCCESS;
    }
#pragma endregion
} // nvdll::boy::

#pragma region nvdll:: 导出
    /** 【DLL 内部函数】
     * @brief 获取读屏器状态
     * @param -
     * @return 返回读屏是否在运行
     */
    bool IsScreenReaderRunning()
    {
        // NOTE: 调用者确保 DLL 已经初始化
        assert(nullptr != boyCtrlIsReaderRunning);
        return boyCtrlIsReaderRunning();
    }

    /** 【DLL 内部函数】
     * @brief 打断并终止读屏输出
     * @param [全局变量] nvdll::ini::SPEAK_WITH_SLAVE 
     * @return 调用状态码
     */
    error_status_t StopSpeaking()
    {
        // NOTE: 调用者确保 DLL 已经初始化
        assert(nullptr != boyCtrlStopSpeaking);
        return boyCtrlStopSpeaking(nvdll::ini::SPEAK_WITH_SLAVE);
    }
#pragma endregion
} // nvdll::


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
    if (nullptr == nvdll::dllHandle)
    {
        spdlog::info("nullptr==dllHandle: trying to load dll...");
        nvdll::boy::loadBaoYiDll();
    }
}

error_status_t __stdcall testIfRunning_impl()
{
    if (nullptr == nvdll::dllHandle)
    {
        bool has_error = nvdll::boy::loadBaoYiDll();
        if (has_error) {
            spdlog::error("[testIfRunning_impl] loadBaoYiDll() load error!");
            return RPC_X_SS_CONTEXT_MISMATCH;
        }
    }

    SPDLOG_DEBUG("[testIfRunning_impl] loadBaoYiDll() load finished. dllHandle={}", (void*)nvdll::dllHandle);
    assert(nullptr != nvdll::dllHandle);
    return RPC_S_OK;
}

/**
 * @brief 回调函数
 * @param reason 回调原因（调用是否成功）
 * @return void
 */
void __stdcall speakCompleteCallback(int reason)
{

#ifdef _DEBUG
    // 仅调试
    switch (reason) {
    case 1:
        //SPDLOG_DEBUG(L"[Callback] reason=朗读完成");
        std::cerr << "[Callback] reason=finished" << std::endl;
        break;
    case 2:
        //SPDLOG_DEBUG(L"[Callback] reason=新朗读打断");
        std::cerr << "[Callback] reason=new_speak_breeak" << std::endl;
        break;
    case 3:
        //SPDLOG_DEBUG(L"[Callback] reason=停止调用打断");
        std::cerr << "[Callback] reason=stop_break" << std::endl;
        break;

    default:
        //SPDLOG_DEBUG(L"[Callback] reason=未知的原因({})", reason);
        std::cerr << "[Callback] reason=unknown-" << reason << std::endl;
        break;
    }
#endif // def _DEBUG

    return;
}

error_status_t __stdcall speakText_impl(const wchar_t* text)
{
    spdlog::debug(L"[speakText_impl] text={}", text);  // 发布版本输出

    if (nullptr == boyCtrlSpeak)
    {
        bool has_error = nvdll::boy::loadBaoYiDll();
        if (has_error) {
            spdlog::error("[speakText_impl] loadBaoYiDll() load error!");
            return RPC_X_SS_CONTEXT_MISMATCH;
        }

        SPDLOG_DEBUG("[speakText_impl] loadBaoYiDll() load finished. boyCtrlSpeak={}", (void*)boyCtrlSpeak);
        assert(nullptr != boyCtrlSpeak);
    }

    auto err = boyCtrlSpeak(text, 
        nvdll::ini::SPEAK_WITH_SLAVE, 
        nvdll::ini::SPEAK_APPEND, 
        nvdll::ini::SPEAK_ALLOW_BREAK, 
        speakCompleteCallback);
    spdlog::debug(L"[speakText_impl]     ret={}", (int)err);  // 发布版本输出

    return convertBoyCtrlError(err);
}

error_status_t __stdcall cancelSpeech_impl()
{
// NOTE: 不允许应用程序打断。由 DLL 处理打断逻辑
#ifdef ALLOW_PROGRAM_BREAK
    if (nullptr == boyCtrlStopSpeaking)
    {
        bool has_error = nvdll::boy::loadBaoYiDll();
        if (has_error) {
            spdlog::error("[cancelSpeech_impl] loadBaoYiDll() load error!");
            return RPC_X_SS_CONTEXT_MISMATCH;
        }

        SPDLOG_DEBUG("[cancelSpeech_impl] loadBaoYiDll() load finished. boyCtrlStopSpeaking={}", (void*)boyCtrlStopSpeaking);
        assert(nullptr != boyCtrlStopSpeaking);
    }

    auto err = boyCtrlStopSpeaking(nvdll::ini::SPEAK_WITH_SLAVE);
    spdlog::debug("[cancelSpeech_impl] ret={}", (int)err);  // 发布版本输出
    return convertBoyCtrlError(err);
#endif // def _DEBUG

    spdlog::debug("[cancelSpeech_impl] ignore!");  // 发布版本输出
    return RPC_S_OK;
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
