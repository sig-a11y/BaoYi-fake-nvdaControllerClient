#include "ini.hpp"
#include "loguru.hpp" // 日志记录
#include "boy_global.hpp"
#include <pathcch.h> // PathCchCombineEx
#pragma comment(lib, "pathcch.lib")
#include <Shlwapi.h> // PathFileExists
#pragma comment(lib, "shlwapi.lib")


namespace ini {
    using boy::logWString;

    /// 配置文件对象
    static CSimpleIniW ini;

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
        PathCchCombineEx(iniPath, MAX_PATH, boy::DLL_DIR_PATH, INI_NAME_CN, PATHCCH_ALLOW_LONG_PATHS);
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
        boy::SPEAK_WITH_SLAVE = 0 != slave;
        DLOG_F(INFO, "[loadIni]     slave=%d; SPEAK_WITH_SLAVE=%d", slave, boy::SPEAK_WITH_SLAVE);

        int append = GetPrivateProfileIntW(INI_APP_NAME, INI_KEY_USE_APPEND, 1, iniPath);
        boy::SPEAK_APPEND = 0 != append;
        DLOG_F(INFO, "[loadIni]     append=%d; SPEAK_APPEND=%d", append, boy::SPEAK_APPEND);

        int allowBreak = GetPrivateProfileIntW(INI_APP_NAME, INI_KEY_ALLOW_BREAK, 1, iniPath);
        boy::SPEAK_ALLOW_BREAK = 0 != allowBreak;
        DLOG_F(INFO, "[loadIni]     allowBreak=%d; SPEAK_ALLOW_BREAK=%d", allowBreak, boy::SPEAK_ALLOW_BREAK);

        DLOG_F(INFO, "[loadIni] load ini finished.");
    }

} // ini::
