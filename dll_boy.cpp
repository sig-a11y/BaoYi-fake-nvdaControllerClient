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


namespace dll 
{
namespace boy
{
#pragma region 局部变量定义
    /// 保益 DLL 完整路径
    TCHAR BOY_DLL_FULLPATH[MAX_PATH];

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

} // dll::boy::
} // dll::