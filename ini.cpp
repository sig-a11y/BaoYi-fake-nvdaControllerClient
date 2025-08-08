#include "ini.hpp"
// -- [sys] win 
#include <pathcch.h> // PathCchCombineEx
#pragma comment(lib, "pathcch.lib")
#include <Shlwapi.h> // PathFileExists
#pragma comment(lib, "shlwapi.lib")
#include "SimpleIni.h" // [3rd] ini 实现
// -- [proj]
#include "log.hpp" // log

namespace nvdll {
namespace ini {

#pragma region 全局变量定义
    /// 是否生成调试日志。默认生成 INFO 级别日志。
    bool GEN_DEBUG_LOG = false;
    /// 是否生成【保益】调用日志。
    bool GEN_BOY_LOG = false;

    /// false=使用读屏通道，true=使用独立通道
    bool SPEAK_WITH_SLAVE = true;
    /// 是否排队朗读
    bool SPEAK_APPEND = true;
    /// 是否允许用户打断.使用读屏通道时该参数被忽略
    bool SPEAK_ALLOW_BREAK = true;
    /// 是否任意按键打断（NVDLL 新增）
    bool SPEAK_ALL_KEY_BREAK = true;

#pragma region

#pragma region 常量变量定义
    // TODO: 使用枚举

    /// 保益 DLL 配置 ini 文件名. 
    LPCWSTR INI_NAME = L"nvdaCfg.ini";
    LPCWSTR INI_NAME_CN = L"朗读配置.ini";

    /* ini 配置项 */
    // 记录调试级别日志
    LPCWSTR INI_KEY_GEN_DEBUG_LOG_EN = L"DEBUG_LOG";
    // 生成保益日志
    LPCWSTR INI_KEY_GEN_BOY_LOG_EN = L"BOY_LOG";

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
     * @param pszBaseDirIn 配置文件所在的文件夹
     */
    void loadIni(PCWSTR pszBaseDirIn)
    {
        SPDLOG_DEBUG("[loadIni] begin to load ini...");
        /// ini 路径
        //TCHAR iniPath[MAX_PATH] = L"E:\\game\\ShadowRine_FullVoice\\朗读配置.ini";
        TCHAR iniPath[MAX_PATH];

        // ==== 拼接 ini 完整路径，尝试加载
        // TODO: 显式构造长路径 "\\?\"
        PathCchCombineEx(iniPath, MAX_PATH, pszBaseDirIn, INI_NAME, PATHCCH_ALLOW_LONG_PATHS);
        bool exist = PathFileExists(iniPath);
        spdlog::info(L"[loadIni] FileExists={}; iniPath={}", exist, iniPath);

        SI_Error rc = ini.LoadFile(iniPath);
        if (rc < 0)
        {
            // 出错
            spdlog::warn("[loadIni] ini.LoadFile error: rc={}", rc);
        }
        else
        {
            SPDLOG_DEBUG("[loadIni] ini.LoadFile rc={}", rc);
            LPCWSTR pv;

#ifdef _DEBUG
            std::list<CSimpleIniW::Entry> allSec;
            ini.GetAllSections(allSec);
            SPDLOG_DEBUG("[loadIni] print all item in ini...");
            for (CSimpleIniW::Entry s : allSec)
            {
                SPDLOG_DEBUG(L"[loadIni] s.pItem[{}]={}", s.nOrder, std::wstring(s.pItem));
                // TODO: 输出所有选项
            }
#endif // def _DEBUG

            pv = ini.GetValue(INI_APP_NAME, INI_KEY_USE_SLAVE, L"-1");
            SPDLOG_DEBUG(L"[loadIni] INI_KEY_USE_SLAVE={}", pv);
            pv = ini.GetValue(INI_APP_NAME_CN, INI_KEY_USE_SLAVE_CN, L"-1");
            SPDLOG_DEBUG(L"[loadIni] INI_KEY_USE_SLAVE_CN={}", pv);

            pv = ini.GetValue(INI_APP_NAME, INI_KEY_USE_APPEND, L"-1");
            SPDLOG_DEBUG(L"[loadIni] INI_KEY_USE_APPEND={}", pv);
            pv = ini.GetValue(INI_APP_NAME_CN, INI_KEY_USE_APPEND_CN, L"-1");
            SPDLOG_DEBUG(L"[loadIni] INI_KEY_USE_APPEND_CN={}", pv);

            pv = ini.GetValue(INI_APP_NAME, INI_KEY_ALLOW_BREAK, L"-1");
            SPDLOG_DEBUG(L"[loadIni] INI_KEY_ALLOW_BREAK={}", pv);
            pv = ini.GetValue(INI_APP_NAME_CN, INI_KEY_ALLOW_BREAK_CN, L"-1");
            SPDLOG_DEBUG(L"[loadIni] INI_KEY_ALLOW_BREAK_CN={}", pv);
        }

        // 拼接的路径不存在，尝试直接读取 ini
        if (!exist)
        {
            PathCchCombineEx(iniPath, MAX_PATH, INI_NAME, NULL, PATHCCH_ALLOW_LONG_PATHS);
            exist = PathFileExists(iniPath);
            spdlog::warn(L"[loadIni] Reload init: FileExists={}; iniPath={}", exist, iniPath);
        }

        // ==== 读取 ini 配置
        int slave = GetPrivateProfileIntW(INI_APP_NAME, INI_KEY_USE_SLAVE, 1, iniPath);
        SPEAK_WITH_SLAVE = 0 != slave;
        SPDLOG_DEBUG("[loadIni]     slave={}; SPEAK_WITH_SLAVE={}", slave, SPEAK_WITH_SLAVE);

        int append = GetPrivateProfileIntW(INI_APP_NAME, INI_KEY_USE_APPEND, 1, iniPath);
        SPEAK_APPEND = 0 != append;
        SPDLOG_DEBUG("[loadIni]     append={}; SPEAK_APPEND={}", append, SPEAK_APPEND);

        int allowBreak = GetPrivateProfileIntW(INI_APP_NAME, INI_KEY_ALLOW_BREAK, 1, iniPath);
        SPEAK_ALLOW_BREAK = 0 != allowBreak;
        SPEAK_ALL_KEY_BREAK = 2 == allowBreak;
        SPDLOG_DEBUG("[loadIni]     allowBreak={}; ALLOW_BREAK={}; ALL_KEY_BREAK={}", 
            allowBreak, SPEAK_ALLOW_BREAK, SPEAK_ALL_KEY_BREAK);
        
        int debug_log = GetPrivateProfileIntW(INI_APP_NAME, INI_KEY_GEN_DEBUG_LOG_EN, 0, iniPath);
        GEN_DEBUG_LOG = 0 != debug_log;
        SPDLOG_DEBUG("[loadIni]     debug_log={}; GEN_DEBUG_LOG={}", debug_log, GEN_DEBUG_LOG);
        // NOTE: 动态设置 log 级别
        if (GEN_DEBUG_LOG)
        {
            spdlog::set_level(spdlog::level::debug);
            spdlog::flush_on(spdlog::level::debug);
            spdlog::debug("[loadIni] set dyn log level to ::debug");
        }

        SPDLOG_DEBUG("[loadIni] load ini finished.");
        spdlog::info("[loadIni] SPEAK_WITH_SLAVE={}", SPEAK_WITH_SLAVE);
        spdlog::info("[loadIni] SPEAK_APPEND={}", SPEAK_APPEND);
        spdlog::info("[loadIni] SPEAK_ALLOW_BREAK={}", SPEAK_ALLOW_BREAK);
        spdlog::info("[loadIni] SPEAK_ALL_KEY_BREAK={}", SPEAK_ALL_KEY_BREAK);
        spdlog::info("[loadIni] GEN_DEBUG_LOG={}", GEN_DEBUG_LOG);
    }

} // nvdll::ini::
} // nvdll::