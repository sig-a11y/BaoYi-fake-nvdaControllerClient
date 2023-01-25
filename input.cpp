#include "input.hpp"
// -- [sys] win
#include <thread>
#include <chrono>
#include <thread>
#include <cassert>
#include <conio.h> // _kbhit
// -- [proj]
#include "log.hpp" // spdlog::
#include "dll.hpp" // nvdll::

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
                // nvdll::StopSpeaking();
                SPDLOG_DEBUG("[keyboardListener]     call StopSpeaking()");
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
    HHOOK kKeyboardHook;

    /// [WH_KEYBOARD_LL] hook 回调函数
    LRESULT CALLBACK lowLevelKbHookFunc(int nCode, WPARAM wParam, LPARAM lParam)
    {
        if (
            HC_ACTION != nCode
            || (WM_KEYDOWN != wParam && WM_SYSKEYDOWN != wParam)
            )
        {
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }
        assert(HC_ACTION == nCode);
        assert(WM_KEYDOWN == wParam || WM_SYSKEYDOWN == wParam);

        // NOTE： WH_KEYBOARD_LL uses the LowLevelKeyboardProc Call Back
        //  https://msdn.microsoft.com/en-us/library/windows/desktop/ms644985(v=vs.85).aspx
        // LowLevelKeyboardProc Structure 
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

        // 返回前台窗口，获得当前窗口
        HWND currentWindow = GetForegroundWindow();
        if (nullptr != currentWindow)
        {
            // TODO: 仅处理当前窗口
        }
        // 如果有按键
        if (p->vkCode)
        {
            SPDLOG_DEBUG("[lowLevelKbHookFunc] press code={}", p->vkCode);
            // nvdll::StopSpeaking();
            SPDLOG_DEBUG("[lowLevelKbHookFunc]     call StopSpeaking()");
        }

        //  hook procedure must pass the message *Always*
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    /// [WH_KEYBOARD] hook 回调函数
    LRESULT CALLBACK kbHookFunc(int nCode, WPARAM wParam, LPARAM lParam)
    {
        if (HC_ACTION != nCode || wParam <= 0)
        {
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }
        assert(HC_ACTION == nCode);
        assert(wParam > 0);

        // ==== 按键状态
        // virtual-key code
        WORD vkCode = LOWORD(wParam);

        WORD keyFlags = HIWORD(lParam);
        WORD scanCode = LOBYTE(keyFlags);                             
        // extended-key flag, 1 if scancode has 0xE0 prefix
        BOOL isExtendedKey = (keyFlags & KF_EXTENDED) == KF_EXTENDED; 
        if (isExtendedKey)
            scanCode = MAKEWORD(scanCode, 0xE0);
        // transition-state flag, 1 on keyup
        BOOL upFlag = (keyFlags & KF_UP) == KF_UP;  

        // previous key-state flag, 1 on autorepeat
        BOOL repeatFlag = (keyFlags & KF_REPEAT) == KF_REPEAT;        
        // repeat count, > 0 if several keydown messages was combined into one message
        WORD repeatCount = LOWORD(lParam);                            

        // -- 仅在按下时静音，忽略弹起。
        BOOL keyDown = !upFlag;
        if (keyDown)
        {
            // 发布版本输出. TODO: 打断测试完成后，还原为 SPDLOG_DEBUG 宏
            spdlog::debug("[kbHookFunc] VK={} x {}; StopSpeaking()", wParam, repeatCount);
            // NOTE: 调用内部函数，与外部调用区分开来
            auto err = nvdll::StopSpeaking();
            spdlog::debug("[kbHookFunc]     StopSpeaking.ret={}", (int)err);  // 发布版本输出
        }
        
        //  hook procedure must pass the message *Always*
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    /// 设置输入 hook
    bool setInputHook()
    {
        // 尝试挂钩
        kKeyboardHook = SetWindowsHookEx(
            // (low-level) keyboard input events
            // NOTE: WH_KEYBOARD 可以当前线程或全局挂钩；WH_KEYBOARD_LL 只能全局挂钩
            WH_KEYBOARD,
            // 回调函数地址
            kbHookFunc,
            // A handle to the DLL containing the hook procedure: GetModuleHandle(NULL)
            // 设置 dwThreadId 时置空
            NULL,
            // 待挂钩的线程ID；为 NULL 则全局挂钩
            GetCurrentThreadId()
        );

        if (nullptr != kKeyboardHook)
        {
            // 挂钩成功
            spdlog::info("[setInputHook] Input hook ready.");
            // 消息循环初始化（没有会导致窗口卡死）
            // NOTE: 全局挂钩用

            //MSG Msg{};
            //while (GetMessage(&Msg, NULL, 0, 0) > 0)
            //{
            //    TranslateMessage(&Msg);
            //    DispatchMessage(&Msg);
            //}

            SPDLOG_DEBUG("[setInputHook] init finished.");
            return true;
        }

        // 挂钩失败
        spdlog::warn("[setInputHook] Failed to get handle from SetWindowsHookEx().");
        return false;
    }

    /// 移除输入 hook
    void removeInputHook()
    {
        if (nullptr != kKeyboardHook)
        {
            SPDLOG_DEBUG("[removeInputHook] trying to remove hook {}", (void*)kKeyboardHook);
            bool ret = UnhookWindowsHookEx(kKeyboardHook);
            spdlog::info("[removeInputHook] remove hook ret={}.", ret);
        }
    }
#pragma endregion

} // nvdll::input::
} // nvdll::