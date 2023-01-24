// BoyCtrlTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include <iostream>
#include <string>
#include <sstream>
#include <Windows.h>
#include "BoyCtrl.h"

using namespace std;
/// 当前支持的保益 API 版本号
const string BOY_API_VERSION = "v1.5.3";


/**
 * @brief 朗读完成回调函数
 * @param reason 回调原因。
 *  1=朗读完成；
 *  2=被新的朗读打断；
 *  3=被停止调用打断
 */
void __stdcall speakCompleteCallback(int reason)
{
    cerr << "Callback=" << reason << endl;
    MessageBeep(0xffffffff);
}

/// 主函数
int main()
{
    // 加载保益 DLL...
    // 注意：DLL 需要放在可被搜索到的路径中，一般和主程序放在一起。
#ifndef BOYCTRLTEST_X64
    auto dllHandle = LoadLibrary(L"BoyCtrl.dll");
#else
    auto dllHandle = LoadLibrary(L"BoyCtrl-x64.dll");
#endif
    if (!dllHandle)
    {
#ifndef BOYCTRLTEST_X64
        cerr << "Failed to load BoyCtrl.dll error = " << GetLastError() << endl;
#else
        cerr << "Failed to load BoyCtrl-x64.dll error = " << GetLastError() << endl;
#endif
        return 1;
    }

    // 开始加载 DLL 的导出函数...
    /// 初始化接口
    auto initFunc = (decltype(BoyCtrlInitialize) *)GetProcAddress(dllHandle, "BoyCtrlInitialize");
    if (!initFunc)
    {
        cerr << "Failed to get BoyCtrlInitialize" << endl;
        FreeLibrary(dllHandle);
        return 1;
    }
    /// 退出清理
    auto uninitFunc = (decltype(BoyCtrlUninitialize) *)GetProcAddress(dllHandle, "BoyCtrlUninitialize");
    if (!uninitFunc)
    {
        cerr << "Failed to get BoyCtrlUninitialize" << endl;
        FreeLibrary(dllHandle);
        return 1;
    }
    // 初始化接口，并输出日志
    auto err = initFunc(L"boyCtrl.log");
    if (err != e_bcerr_success)
    {
        cerr << "init failed error = " << err << endl;
        FreeLibrary(dllHandle);
        return 1;
    }
    cout << "ready" << endl;

    /// 朗读文本(UTF-16le)
    auto speakFunc = (decltype(BoyCtrlSpeak) *)GetProcAddress(dllHandle, "BoyCtrlSpeak");
    if (!speakFunc)
    {
        cerr << "Failed to get BoyCtrlSpeak" << endl;
        uninitFunc();
        FreeLibrary(dllHandle);
        return 1;
    }
    /// 朗读文本(ANSI)
    auto speakAnsiFunc = (decltype(BoyCtrlSpeakAnsi) *)GetProcAddress(dllHandle, "BoyCtrlSpeakAnsi");
    if (!speakAnsiFunc)
    {
        cerr << "Failed to get BoyCtrlSpeakAnsi" << endl;
        uninitFunc();
        FreeLibrary(dllHandle);
        return 1;
    }
    /// 朗读文本(UTF-8)
    auto speakU8Func = (decltype(BoyCtrlSpeakU8) *)GetProcAddress(dllHandle, "BoyCtrlSpeakU8");
    if (!speakU8Func)
    {
        cerr << "Failed to get BoyCtrlSpeakU8" << endl;
        uninitFunc();
        FreeLibrary(dllHandle);
        return 1;
    }
    /// 停止朗读
    auto stopFunc = (decltype(BoyCtrlStopSpeaking) *)GetProcAddress(dllHandle, "BoyCtrlStopSpeaking");
    if (!stopFunc)
    {
        cerr << "Failed to get BoyCtrlStopSpeaking" << endl;
        uninitFunc();
        FreeLibrary(dllHandle);
        return 1;
    }
    /// 暂停指定时间的读屏朗读
    auto pauseFunc = (decltype(BoyCtrlPauseScreenReader)*)GetProcAddress(dllHandle, "BoyCtrlPauseScreenReader");
    if (!pauseFunc)
    {
        cerr << "Failed to get BoyCtrlPauseScreenReader" << endl;
        uninitFunc();
        FreeLibrary(dllHandle);
        return 1;
    }
    /// 读屏是否正在运行
    auto isReaderRunningFunc = (decltype(BoyCtrlIsReaderRunning)*)GetProcAddress(dllHandle, "BoyCtrlIsReaderRunning");
    if (!isReaderRunningFunc)
    {
        cerr << "Failed to get BoyCtrlIsReaderRunning" << endl;
        uninitFunc();
        FreeLibrary(dllHandle);
        return 1;
    }
    // 函数加载完毕。
    

    // 开始测试接口...
    for (int i = 1; i <= 4; ++i)
    {
        cout << i << " Press <Enter> to speak" << endl;
        getchar();

        if (i < 3)
        {
            wostringstream oss;
            oss << i << L"使用独立语音朗读，打断模式，有完成通知";
            err = speakFunc(oss.str().c_str(), true, false, true, speakCompleteCallback);
        }
        else
        {
            err = speakU8Func(u8"朗读字符串使用UTF-8", true, false, true, speakCompleteCallback);
        }

        if (err != e_bcerr_success)
        {
            cerr << "error = " << err << endl;
        }
    }

    // 程序退出，开始清理工作...
    uninitFunc();
    FreeLibrary(dllHandle);

    cout << "Press <Enter> to exit" << endl;
    getchar();

    return 0;
}
