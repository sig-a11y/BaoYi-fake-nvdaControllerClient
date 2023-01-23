#include "ini.hpp"
// -- [sys] win 
#include <pathcch.h> // PathCchCombineEx
#pragma comment(lib, "pathcch.lib")
#include <Shlwapi.h> // PathFileExists
#pragma comment(lib, "shlwapi.lib")
#include "SimpleIni.h" // [3rd] ini 实现
// -- [proj]
#include "log.hpp" // log
#include "dll.hpp" // dll::DLL_DIR_PATH


namespace ini {
    using nvdll::log::logWString;

#pragma region 全局变量定义
    /// false=使用读屏通道，true=使用独立通道
    bool SPEAK_WITH_SLAVE = true;
    /// 是否排队朗读
    bool SPEAK_APPEND = true;
    /// 是否允许用户打断.使用读屏通道时该参数被忽略
    bool SPEAK_ALLOW_BREAK = true;
#pragma region

#pragma region 常量变量定义
    // TODO: 使用枚举

    /// 保益 DLL 配置 ini 文件名. 
    LPCWSTR INI_NAME = L"nvdaCfg.ini";
    LPCWSTR INI_NAME_CN = L"朗读配置.ini";

    LPCWSTR INI_APP_NAME = L"APP";
    LPCWSTR INI_APP_NAME_CN = L"朗读";
    LPCWSTR INI_KEY_USE_SLAVE = L"USE_SLAVE";
    LPCWSTR INI_KEY_USE_SLAVE_CN = L"独立通道";
    LPCWSTR INI_KEY_USE_APPEND = L"USE_APPEND";
    LPCWSTR INI_KEY_USE_APPEND_CN = L"排队朗读";
    LPCWSTR INI_KEY_ALLOW_BREAK = L"ALLOW_BREAK";
    LPCWSTR INI_KEY_ALLOW_BREAK_CN = L"按键打断朗读";

    /// 配置文件对象
    static CSimpleIniW ini;
#pragma region


    /// 非0值作为 true
    /**
     * @brief 加载配置文件。非0值作为 true
     */
    void loadIni()
    {
        DLOG_F(INFO, "[loadIni] begin to load ini...");
        /// ini 路径
        //TCHAR iniPath[MAX_PATH] = L"E:\\game\\ShadowRine_FullVoice\\朗读配置.ini";
        TCHAR iniPath[MAX_PATH];

        // ==== 拼接 ini 完整路径，尝试加载
        // TODO: 显式构造长路径 "\\?\"
        PathCchCombineEx(iniPath, MAX_PATH, dll::DLL_DIR_PATH, INI_NAME_CN, PATHCCH_ALLOW_LONG_PATHS);
        logWString("loadIni", "iniPath", iniPath);
        bool exist = PathFileExists(iniPath);
        DLOG_F(INFO, "[loadIni] PathFileExists=%d", exist);
        SI_Error rc = ini.LoadFile(iniPath);
        if (rc < 0)
        {
            DLOG_F(INFO, "[loadIni] ini.LoadFile rc=%x", rc);
        }
        else
        {
            LPCWSTR pv;
            DLOG_F(INFO, "[loadIni] ini.LoadFile rc=%x", rc);

            std::list<CSimpleIniW::Entry> allSec;
            ini.GetAllSections(allSec);
            for (CSimpleIniW::Entry s : allSec)
            {
                logWString("loadIni", "s.pItem", s.pItem);
                //logWString("loadIni", "s.pComment", s.pComment);
            }

            pv = ini.GetValue(INI_APP_NAME, INI_KEY_USE_SLAVE, L"-1");
            logWString("loadIni", "INI_KEY_USE_SLAVE", pv);
            pv = ini.GetValue(INI_APP_NAME_CN, INI_KEY_USE_SLAVE_CN, L"-1");
            logWString("loadIni", "INI_KEY_USE_SLAVE_CN", pv);

            pv = ini.GetValue(INI_APP_NAME, INI_KEY_USE_APPEND, L"-1");
            logWString("loadIni", "INI_KEY_USE_APPEND", pv);
            pv = ini.GetValue(INI_APP_NAME_CN, INI_KEY_USE_APPEND_CN, L"-1");
            logWString("loadIni", "INI_KEY_USE_APPEND_CN", pv);

            pv = ini.GetValue(INI_APP_NAME, INI_KEY_ALLOW_BREAK, L"-1");
            logWString("loadIni", "INI_KEY_ALLOW_BREAK", pv);
            pv = ini.GetValue(INI_APP_NAME_CN, INI_KEY_ALLOW_BREAK_CN, L"-1");
            logWString("loadIni", "INI_KEY_ALLOW_BREAK_CN", pv);

        }

        // 拼接的路径不存在，尝试直接读取 ini
        if (!exist)
        {
            PathCchCombineEx(iniPath, MAX_PATH, INI_NAME, NULL, PATHCCH_ALLOW_LONG_PATHS);
            logWString("loadIni", "iniPath", iniPath);
            exist = PathFileExists(iniPath);
            DLOG_F(INFO, "[loadIni] INI_NAME Exists=%d", exist);
        }

        // ==== 读取 ini 配置
        int slave = GetPrivateProfileIntW(INI_APP_NAME, INI_KEY_USE_SLAVE, 1, iniPath);
        SPEAK_WITH_SLAVE = 0 != slave;
        DLOG_F(INFO, "[loadIni]     slave=%d; SPEAK_WITH_SLAVE=%d", slave, SPEAK_WITH_SLAVE);

        int append = GetPrivateProfileIntW(INI_APP_NAME, INI_KEY_USE_APPEND, 1, iniPath);
        SPEAK_APPEND = 0 != append;
        DLOG_F(INFO, "[loadIni]     append=%d; SPEAK_APPEND=%d", append, SPEAK_APPEND);

        int allowBreak = GetPrivateProfileIntW(INI_APP_NAME, INI_KEY_ALLOW_BREAK, 1, iniPath);
        SPEAK_ALLOW_BREAK = 0 != allowBreak;
        DLOG_F(INFO, "[loadIni]     allowBreak=%d; SPEAK_ALLOW_BREAK=%d", allowBreak, SPEAK_ALLOW_BREAK);

        DLOG_F(INFO, "[loadIni] load ini finished.");
    }

} // ini::
