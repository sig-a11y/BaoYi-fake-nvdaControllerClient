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
static BoyCtrlSetAnyKeyStopSpeakingFunc boyCtrlSetAnyKeyStopSpeaking;
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
            spdlog::info(L"[loadBaoYiDll] trying to load dll: {}", BOY_DLL_FULLPATH);
            dllHandle = LoadLibrary(BOY_DLL_FULLPATH);
            SPDLOG_DEBUG(L"[loadBaoYiDll]   dllHandle={}", (void*)dllHandle);
        }
        // 仅使用 DLL 名加载：保益 DLL 和主程序 exe 放在一起
        if (nullptr == dllHandle)
        {
            spdlog::info(L"[loadBaoYiDll] trying to load dll: {}", BOY_DLL_FULLPATH);
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
        boyCtrlSetAnyKeyStopSpeaking = (BoyCtrlSetAnyKeyStopSpeakingFunc)loadFunctionPtr("BoyCtrlSetAnyKeyStopSpeaking");
        if (nullptr == boyCtrlSetAnyKeyStopSpeaking) return EXIT_FAILURE;

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

        // 由 DLL 控制 && 开启按任意键打断
        if (nvdll::ini::BREAK_CTRL && ini::SPEAK_ALL_KEY_BREAK)
        {
            boyCtrlSetAnyKeyStopSpeaking(nvdll::ini::SPEAK_WITH_SLAVE);
            // input::runInputListener();
            //input::setInputHook();
        }

        spdlog::info(L"[loadBaoYiDll] BouAPI Ready! DLL initialize successful");
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

error_status_t __stdcall nvdll::boy::testIfRunning_impl()
{
    spdlog::debug(L"[testIfRunning_impl]");  // 发布版本输出
    if (nullptr == boyCtrlIsReaderRunning)
    {
        bool has_error = nvdll::boy::loadBaoYiDll();
        if (has_error) {
            spdlog::error("[testIfRunning_impl] loadBaoYiDll() load error!");
            return ERROR_INVALID_HANDLE;
        }
    }
    assert(nullptr != boyCtrlIsReaderRunning);
    bool isRunning = boyCtrlIsReaderRunning();

    return isRunning ? e_bcerr_success : e_bcerr_fail;
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

error_status_t __stdcall nvdll::boy::speakText_impl(const wchar_t* text)
{
    spdlog::debug(L"[speakText_impl] text={}", text);  // 发布版本输出

    if (nullptr == boyCtrlSpeak)
    {
        bool has_error = nvdll::boy::loadBaoYiDll();
        if (has_error) {
            spdlog::error("[speakText_impl] loadBaoYiDll() load error!");
            return ERROR_INVALID_HANDLE;
        }

        SPDLOG_DEBUG("[speakText_impl] loadBaoYiDll() load finished. boyCtrlSpeak={}", (void*)boyCtrlSpeak);
    }
    assert(nullptr != boyCtrlSpeak);

    // 默认排队
    bool bAppend = true;
    if (nvdll::ini::BREAK_CTRL)
    {
        // 仅在 BREAK_CTRL=1 时，使用 ini 中的打断设置
        bAppend = nvdll::ini::SPEAK_APPEND;
    }

    auto err = boyCtrlSpeak(text,
        nvdll::ini::SPEAK_WITH_SLAVE,
        bAppend,
        nvdll::ini::ALLOW_SR_INTERRUPT,
        speakCompleteCallback);
    spdlog::debug(L"[speakText_impl]     ret={}", (int)err);  // 发布版本输出

    return convertBoyCtrlError(err);
}

error_status_t __stdcall nvdll::boy::cancelSpeech_impl()
{
    if (nullptr == boyCtrlStopSpeaking)
    {
        bool has_error = nvdll::boy::loadBaoYiDll();
        if (has_error) {
            spdlog::error("[cancelSpeech_impl] loadBaoYiDll() load error!");
            return ERROR_INVALID_HANDLE;
        }

        SPDLOG_DEBUG("[cancelSpeech_impl] loadBaoYiDll() load finished. boyCtrlStopSpeaking={}", (void*)boyCtrlStopSpeaking);
    }
    assert(nullptr != boyCtrlStopSpeaking);

    // 由 DLL 控制 && 并且不允许打断
    if (nvdll::ini::BREAK_CTRL && !nvdll::ini::SPEAK_ALLOW_BREAK)
    {
        return RPC_S_CANNOT_SUPPORT;
    }

    // 正常打断
    auto err = boyCtrlStopSpeaking(nvdll::ini::SPEAK_WITH_SLAVE);
    spdlog::debug("[cancelSpeech_impl] ret={}", (int)err);  // 发布版本输出
    return convertBoyCtrlError(err);
}
#pragma endregion
