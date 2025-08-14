#include "nvdll_impl.h" // API 导出
#include "dll.hpp" // 项目内导出
// -- [sys] win 
#include <sstream> // stringstream
#include <cassert> // assert
#include <iostream> // wcout
// -- [3rd]
#include "zdsr.hpp"
// -- [proj]
#include "log.hpp" // nvdll::log::; DLOG_F
#include "ini.hpp" // ini:: loadIni; SPEAK_WITH_SLAVE, SPEAK_APPEND, SPEAK_ALLOW_BREAK
#include "input.hpp"

namespace nvdll
{
namespace zdsr
{
    TCHAR DLL_FULLPATH[MAX_PATH];
    HMODULE dllHandle;

    /// 加载 DLL 及导入函数
    bool loadDLL()
    {
        std::stringstream eout;

        // -- 加载 DLL
        // 使用完整路径加载：DLL 和 nvda 放在一起
        if (nullptr == dllHandle)
        {
            spdlog::info(L"[loadDLL] trying to load dll: {}", zdsr_api::DLL_NAME);
            dllHandle = LoadLibrary(zdsr_api::DLL_NAME);
            SPDLOG_DEBUG(L"[loadDLL]   dllHandle={}", (void*)dllHandle);
        }
        // 仅使用 DLL 名加载：DLL 和主程序 exe 放在一起
        if (nullptr == dllHandle)
        {
            spdlog::info(L"[loadDLL] trying to load dll: {}", zdsr_api::DLL_NAME);
            dllHandle = LoadLibrary(zdsr_api::DLL_NAME);
            SPDLOG_DEBUG(L"[loadDLL]   dllHandle={}", (void*)dllHandle);
        }
        // 检查 DLL 是否成功加载
        if (!dllHandle)
        {
            spdlog::error(L"[loadDLL] Failed to load DLL '{}'. 尝试加载 DLL 失败。", zdsr_api::DLL_NAME);
            //spdlog::error(L"[loadDLL] GetLastError={}", GetLastError());
            return EXIT_FAILURE;
        }

        // 加载配置文件
        /* NOTE: ZDSR 自带配置无需加载。这里主要用作日志控制 */
        nvdll::ini::loadIni(DLL_DIR_PATH);

        // -- 加载函数
        //boyCtrlInitialize = (BoyCtrlInitializeFunc)loadFunctionPtr("BoyCtrlInitialize");
        //if (nullptr == boyCtrlInitialize) return EXIT_FAILURE;
        //boyCtrlUninitialize = (BoyCtrlUninitializeFunc)loadFunctionPtr("BoyCtrlUninitialize");
        //if (nullptr == boyCtrlUninitialize) return EXIT_FAILURE;
        //boyCtrlSpeak = (BoyCtrlSpeakFunc)loadFunctionPtr("BoyCtrlSpeak");
        //if (nullptr == boyCtrlSpeak) return EXIT_FAILURE;
        //boyCtrlStopSpeaking = (BoyCtrlStopSpeakingFunc)loadFunctionPtr("BoyCtrlStopSpeaking");
        //if (nullptr == boyCtrlStopSpeaking) return EXIT_FAILURE;
        //boyCtrlPauseScreenReader = (BoyCtrlPauseScreenReaderFunc)loadFunctionPtr("BoyCtrlPauseScreenReader");
        //if (nullptr == boyCtrlPauseScreenReader) return EXIT_FAILURE;
        //boyCtrlIsReaderRunning = (BoyCtrlIsReaderRunningFunc)loadFunctionPtr("BoyCtrlIsReaderRunning");
        //if (nullptr == boyCtrlIsReaderRunning) return EXIT_FAILURE;
        //boyCtrlSetAnyKeyStopSpeaking = (BoyCtrlSetAnyKeyStopSpeakingFunc)loadFunctionPtr("BoyCtrlSetAnyKeyStopSpeaking");
        //if (nullptr == boyCtrlSetAnyKeyStopSpeaking) return EXIT_FAILURE;

        // -- 初始化 DLL
        // 开启调试日志(DEBUG_LOG=1)：生成保益的日志
  /*      auto logName = ini::GEN_BOY_LOG ? DLL_LOG_NAME : nullptr;
        spdlog::info(L"[loadDLL] boyCtrlInitialize(logName={})", logName ? logName : L"nullptr");
        auto err = boyCtrlInitialize(logName);
        if (err != e_bcerr_success)
        {
            spdlog::error(L"[loadDLL] Initialize failed. 初始化失败。");
            spdlog::error(L"[loadDLL]   ret={}", (int)err);
            freeDll();
            return EXIT_FAILURE;
        }*/

        // 由 DLL 控制 && 开启按任意键打断
        if (nvdll::ini::BREAK_CTRL && ini::SPEAK_ALL_KEY_BREAK)
        {
            // input::runInputListener();
            // input::setInputHook();
        }

        spdlog::info(L"[loadDLL] ZDSR API Ready! DLL initialize successful");
        return EXIT_SUCCESS;
    }

#pragma region DLL 导出函数实现
    error_status_t __stdcall testIfRunning_impl()
    {
        spdlog::debug(L"[testIfRunning_impl]");  // 发布版本输出
        return RPC_S_CANNOT_SUPPORT;
    }

    error_status_t __stdcall speakText_impl(const wchar_t* text)
    {
        spdlog::debug(L"[speakText_impl] text={}", text);  // 发布版本输出

        return RPC_S_CANNOT_SUPPORT;
    }

    error_status_t __stdcall cancelSpeech_impl()
    {
        spdlog::debug(L"[cancelSpeech_impl]");  // 发布版本输出

        return RPC_S_CANNOT_SUPPORT;
    }

#pragma endregion

} // nvdll::zdsr::

} // nvdll::
