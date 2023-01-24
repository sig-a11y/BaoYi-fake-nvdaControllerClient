#include "input.hpp"
// -- [sys] win
#include <thread>
#include <chrono>
#include <thread>
#include <conio.h> // _kbhit
// -- [proj]
#include "log.hpp" // spdlog::
#include "nvdll_impl.h" // cancelSpeech_impl

namespace nvdll
{
namespace input
{

#pragma region 输入监听线程
    std::thread inputListenerThread;

    // TODO: 换用输入钩子
    void keyboardListener()
    {
        while (true)
        {
            // NOTE: 在 release 中反应较慢、debug 中正常
            if (_kbhit()) {
                SPDLOG_DEBUG("[keyboardListener] keydown");
                cancelSpeech_impl();
            }

            // 每 100ms 检查一次
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    /// 启动输入监听线程
    void runInputListener()
    {
        // 避免重复初始化
        killListenerThread();

        SPDLOG_DEBUG("[runInputListener] theard start...");

        inputListenerThread = std::thread(keyboardListener);

        // NOTE: get int thread id.
        spdlog::info("[runInputListener] theard({}) running.", 
            std::hash<std::thread::id>{}(inputListenerThread.get_id()));
    }

    /// 停止输入监听线程
    void killListenerThread()
    {
        if (inputListenerThread.joinable())
        {
            SPDLOG_DEBUG("[killListenerThread] waiting for theard to exit...");
            inputListenerThread.join();
            spdlog::info("[killListenerThread] theard exited.");
        }
    }
#pragma endregion


#pragma region 输入监听 hook

#pragma endregion

} // nvdll::input::
} // nvdll::