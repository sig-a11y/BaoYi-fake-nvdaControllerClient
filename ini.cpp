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
    bool SPEAK_WITH_SLAVE = false;
    /// 程序控制打断: false=程序控制打断，true=由 DLL 控制打断
    bool BREAK_CTRL = false;
    /// 是否排队朗读
    bool SPEAK_APPEND = true;
    /// 是否允许用户打断
    bool SPEAK_ALLOW_BREAK = true;
    /// 是否任意按键打断
    bool SPEAK_ALL_KEY_BREAK = false;
    /// 是否允许读屏打断朗读
    bool ALLOW_SR_INTERRUPT = true;

#pragma region

#pragma region 常量变量定义
    /// 保益 DLL 配置 ini 文件名. 
    LPCWSTR INI_NAME = L"nvdaCfg.ini";
    LPCWSTR INI_NAME_CN = L"朗读配置.ini";

    /* ini 配置项 */
    /* [NvdaDll] */
    LPCWSTR INI_SEC__NvdaDll = L"NvdaDll";
    //  记录调试级别日志
    LPCWSTR INI_KEY__DEBUG_LOG = L"DEBUG_LOG";
    //  独立通道: 
    LPCWSTR INI_KEY__USE_CHANNEL = L"USE_CHANNEL";
    //  打断控制: 程序控制还是 DLL 控制
    LPCWSTR INI_KEY__BREAK_CTRL = L"BREAK_CTRL";
    //  排队朗读: 
    LPCWSTR INI_KEY__USE_APPEND = L"USE_APPEND";
    //  按键打断模式: 
    LPCWSTR INI_KEY__INTERRUPT_MODE = L"INTERRUPT_MODE";
    //  允许读屏打断朗读: 
    LPCWSTR INI_KEY__ALLOW_SR_INTERRUPT = L"ALLOW_SR_INTERRUPT";

    // TODO: rm
    LPCWSTR INI_APP_NAME = L"NvdaDll";
    LPCWSTR INI_KEY_GEN_DEBUG_LOG_EN = L"DEBUG_LOG";
    LPCWSTR INI_KEY_GEN_BOY_LOG_EN = L"BOY_LOG";
    LPCWSTR INI_KEY_BREAK_CTRL_EN = L"BREAK_CTRL";
    LPCWSTR INI_KEY_USE_SLAVE = L"USE_SLAVE";
    LPCWSTR INI_KEY_USE_APPEND = L"USE_APPEND";
    LPCWSTR INI_KEY_ALLOW_BREAK = L"ALLOW_BREAK";

    /* [BoySR] */
    LPCWSTR INI_SEC__BoySR = L"BoySR";
    //  生成保益日志
    LPCWSTR INI_KEY__BOY_LOG = L"BOY_LOG";

    /* [ZDSR] */
    LPCWSTR INI_SEC__ZDSR = L"ZDSR";

    /// 配置文件对象
    static CSimpleIniW ini;
#pragma region

    /// <summary>
    /// 使用 CSimpleIniW 读取 ini 文件。
    /// </summary>
    /// <param name="iniPath">ini 完整路径</param>
    /// <returns>是否成功读取</returns>
    bool CSimpleIni_ReadIni(PWSTR iniPath)
    {
        spdlog::info("[loadIni] Reading ini using CSimpleIniW.LoadFile");
        SI_Error rc = ini.LoadFile(iniPath);
        if (rc < 0)
        {
            // 出错
            spdlog::warn("[loadIni] CSimpleIniW.LoadFile error: rc={}", rc);
            return false;
        }

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

        pv = ini.GetValue(INI_APP_NAME, INI_KEY_USE_APPEND, L"-1");
        SPDLOG_DEBUG(L"[loadIni] INI_KEY_USE_APPEND={}", pv);

        pv = ini.GetValue(INI_APP_NAME, INI_KEY_ALLOW_BREAK, L"-1");
        SPDLOG_DEBUG(L"[loadIni] INI_KEY_ALLOW_BREAK={}", pv);
        
        return true;
    }

    /* 废弃不使用 */
    bool WinAPI_ReadIni(PWSTR iniPath)
    {
        spdlog::info(L"[loadIni] Reading ini file use GetPrivateProfileIntW");
        /* [DEBUG_LOG] */
        int debug_log = GetPrivateProfileIntW(INI_APP_NAME, INI_KEY_GEN_DEBUG_LOG_EN, 0, iniPath);
        // 仅 1 生成调试日志；其他均保持默认。
        GEN_DEBUG_LOG = 1 == debug_log;
        SPDLOG_DEBUG("[loadIni]     debug_log={}; GEN_DEBUG_LOG={}", debug_log, GEN_DEBUG_LOG);
        // NOTE: 动态设置 log 级别
        if (GEN_DEBUG_LOG)
        {
            spdlog::set_level(spdlog::level::debug);
            spdlog::flush_on(spdlog::level::debug);
            spdlog::debug("[loadIni] set dyn log level to ::debug");
        }
        /* [BOY_LOG] */
        int boy_log = GetPrivateProfileIntW(INI_APP_NAME, INI_KEY_GEN_BOY_LOG_EN, 0, iniPath);
        // 仅 1 生成日志；其他均不生成
        GEN_BOY_LOG = 1 == boy_log;
        SPDLOG_DEBUG("[loadIni]     boy_log={}; GEN_BOY_LOG={}", boy_log, GEN_BOY_LOG);

        /* [BREAK_CTRL] */
        int break_ctrl = GetPrivateProfileIntW(INI_APP_NAME, INI_KEY_BREAK_CTRL_EN, 1, iniPath);
        // 仅 0 时才由程序控制，1和其他(非0)由 DLL 控制。
        BREAK_CTRL = 0 != break_ctrl;
        SPDLOG_DEBUG("[loadIni]     break_ctrl={}; BREAK_CTRL={}", break_ctrl, BREAK_CTRL);

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

        return true;
    }

    /**
     * @brief 加载配置文件。非0值作为 true
     * @param pszBaseDirIn 配置文件所在的文件夹
     */
    void loadIni(PCWSTR pszBaseDirIn)
    {
        SPDLOG_DEBUG("[loadIni] begin to load ini...");
        /// ini 路径
        TCHAR iniPath[MAX_PATH];

        // ==== 拼接 ini 完整路径，尝试加载
        // 从 DLL 旁完整路径读取 ini
        PathCchCombineEx(iniPath, MAX_PATH, pszBaseDirIn, INI_NAME, PATHCCH_ALLOW_LONG_PATHS);
        bool exist = PathFileExists(iniPath);
        // 拼接的路径不存在，尝试直接读取 ini
        if (!exist)
        {
            spdlog::warn(L"[loadIni] .ini file not exist: iniPath={}", iniPath);
            PathCchCombineEx(iniPath, MAX_PATH, INI_NAME, NULL, PATHCCH_ALLOW_LONG_PATHS);
        }
        exist = PathFileExists(iniPath);
        if (!exist)
        {
            spdlog::warn(L"[loadIni] .ini file not exist: iniPath={}", iniPath);
            spdlog::error(L"[loadIni] stop read ini.");
            return;
        }

        /* 开始读取 ini */
        spdlog::info(L"[loadIni] Reading ini file: {}", iniPath);

        bool iniLoaded = false;
        // 使用 CSimpleIniW 读取 ini 文件
        iniLoaded = CSimpleIni_ReadIni(iniPath);
        if (!iniLoaded)
        {
            // 使用系统 API 重新读取 ini
            //iniLoaded = WinAPI_ReadIni(iniPath);
        }
        if (!iniLoaded)
        {
            spdlog::error(L"[loadIni] ini not loaded, stop read ini.");
            return;
        }
        spdlog::info("[loadIni] load ini finished.");

        /* 输出当前配置 */
        spdlog::info(L"[loadIni] Section: [{}]", INI_SEC__NvdaDll);
        spdlog::info(L"[loadIni]    {}={}", INI_KEY__DEBUG_LOG,     GEN_DEBUG_LOG);
        spdlog::info(L"[loadIni]    {}={}", INI_KEY__BOY_LOG,       GEN_BOY_LOG);
        spdlog::info(L"[loadIni]    {}={}", INI_KEY__USE_CHANNEL,   SPEAK_WITH_SLAVE);
        spdlog::info(L"[loadIni]    {}={}", INI_KEY__BREAK_CTRL,    BREAK_CTRL);
        spdlog::info(L"[loadIni]    {}={}", INI_KEY__USE_APPEND,    SPEAK_APPEND);
        // TODO: INI_KEY__INTERRUPT_MODE
        spdlog::info(L"[loadIni]    {}={}", INI_KEY__INTERRUPT_MODE, L"");
        spdlog::info(L"[loadIni]      ALLOW_BREAK={}", SPEAK_ALLOW_BREAK);
        spdlog::info(L"[loadIni]      ALL_KEY_BREAK={}", SPEAK_ALL_KEY_BREAK);
        spdlog::info(L"[loadIni]    {}={}", INI_KEY__ALLOW_SR_INTERRUPT, ALLOW_SR_INTERRUPT);
    }

} // nvdll::ini::
} // nvdll::