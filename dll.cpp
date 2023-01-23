#include "dll.hpp"
#include <string> // wstring
#include <vector> // vector
#include <strsafe.h> // StringCchPrintf
#include <stdlib.h> // _wsplitpath_s, _wmakepath_s


namespace dll {
    using boy::logWString;

#pragma region 全局变量定义
    /// DLL 句柄
    static HMODULE dllHandle;
    static BoyCtrlInitializeFunc boyCtrlInitialize;
    static BoyCtrlUninitializeFunc boyCtrlUninitialize;
    static BoyCtrlSpeakFunc boyCtrlSpeak;
    static BoyCtrlStopSpeakingFunc boyCtrlStopSpeaking;
    static BoyCtrlPauseScreenReaderFunc boyCtrlPauseScreenReader;
#pragma region


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
        logWString("saveDllDirPath", "DLL_PATH", DLL_PATH);

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
        _wmakepath_s(boy::DLL_DIR_PATH, disk.data(), dirname.data(), NULL, NULL);
        // 打印文件夹路径
        logWString("saveDllDirPath", "DLL_DIR_PATH", boy::DLL_DIR_PATH);

        // -- 拼接保益 DLL 完整路径
        StringCchPrintfW(boy::BOY_DLL_FULLPATH, MAX_PATH, L"%s\\%s", boy::DLL_DIR_PATH, boy::BOY_DLL_NAME);
        logWString("saveDllDirPath", "BOY_DLL_FULLPATH", boy::BOY_DLL_FULLPATH);
    }

} // dll::
