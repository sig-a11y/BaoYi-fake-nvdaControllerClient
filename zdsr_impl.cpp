#include "nvdll_impl.h" // API 导出
#include "dll.hpp" // 项目内导出
// -- [sys] win 
#include <cassert> // assert
// -- [3rd]
#include "zdsr.hpp"
// -- [proj]
#include "log.hpp" // nvdll::log::; DLOG_F
#include "ini.hpp" // ini:: loadIni; SPEAK_WITH_SLAVE, SPEAK_APPEND, SPEAK_ALLOW_BREAK

namespace nvdll
{
namespace zdsr
{
    using namespace zdsr_api;

    TCHAR DLL_FULLPATH[MAX_PATH];
    //HMODULE dllHandle;

    InitTTSFunc InitTTS;
    SpeakFunc Speak;
    GetSpeakStateFunc GetSpeakState;
    StopSpeakFunc StopSpeak;

    /// 加载 DLL 及导入函数
    bool loadDLL()
    {
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
        InitTTS = (InitTTSFunc)loadFunctionPtr("InitTTS");
        if (nullptr == InitTTS) return EXIT_FAILURE;
        Speak = (SpeakFunc)loadFunctionPtr("Speak");
        if (nullptr == Speak) return EXIT_FAILURE;
        GetSpeakState = (GetSpeakStateFunc)loadFunctionPtr("GetSpeakState");
        if (nullptr == GetSpeakState) return EXIT_FAILURE;
        StopSpeak = (StopSpeakFunc)loadFunctionPtr("StopSpeak");
        if (nullptr == StopSpeak) return EXIT_FAILURE;

        // -- 初始化 DLL
        // 开启调试日志(DEBUG_LOG=1)：生成保益的日志
        int channelType = nvdll::ini::SPEAK_WITH_SLAVE ? 1 : 0;
        const wchar_t *channelName = nullptr;
        bool bKeyDownInterrupt = nvdll::ini::SPEAK_ALLOW_BREAK;
        auto err = InitTTS(channelType, channelName, bKeyDownInterrupt);
        if (err != 0)
        {
            spdlog::error(L"[loadDLL] ZDSR.dll Initialize failed. 初始化失败。");
            spdlog::error(L"[loadDLL]   ret={}", (int)err);
            FreeLibrary(dllHandle);
            return EXIT_FAILURE;
        }

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
        spdlog::debug(L"[testIfRunning_impl]");

        if (nullptr == GetSpeakState)
        {
            spdlog::warn(L"[testIfRunning_impl] GetSpeakState is null");
            return ERROR_INVALID_HANDLE;
        }
        assert(nullptr != GetSpeakState);

        int state = GetSpeakState();
        // 1:版本不匹配; 2:ZDSR没有运行或没有授权; 3:正在朗读; 4:没有朗读
        bool isRunning = (state == 3) || (state == 4);

        return isRunning ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    error_status_t __stdcall speakText_impl(const wchar_t* text)
    {
        spdlog::debug(L"[speakText_impl] text={}", text);
        if (nullptr == Speak)
        {
            spdlog::warn(L"[cancelSpeech_impl] Speak is null");
            return ERROR_INVALID_HANDLE;
        }
        assert(nullptr != Speak);

        // 默认排队
        bool bAppend = true;
        //if (nvdll::ini::BREAK_CTRL)
        //{
        //    // 仅在 BREAK_CTRL=1 时，使用 ini 中的打断设置
        //    bAppend = nvdll::ini::SPEAK_APPEND;
        //}
        bool interrupt = !bAppend;

        int ret = Speak(text, interrupt);
        spdlog::debug(L"[speakText_impl]   interrupt={}: ret={}",
            interrupt, ret);

        return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    error_status_t __stdcall cancelSpeech_impl()
    {
        spdlog::debug(L"[cancelSpeech_impl]");
        if (nullptr == StopSpeak)
        {
            spdlog::warn(L"[cancelSpeech_impl] StopSpeak is null");
            return ERROR_INVALID_HANDLE;
        }
        assert(nullptr != StopSpeak);

        // 由 DLL 控制 && 并且不允许打断
        //if (nvdll::ini::BREAK_CTRL && !nvdll::ini::SPEAK_ALLOW_BREAK)
        //{
        //    return RPC_S_CANNOT_SUPPORT;
        //}

        StopSpeak();

        return EXIT_SUCCESS;
    }

#pragma endregion

} // nvdll::zdsr::

} // nvdll::
