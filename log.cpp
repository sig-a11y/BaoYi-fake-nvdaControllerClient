#include "log.hpp"
#include "dll.hpp"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace nvdll {
namespace log {

    constexpr LPCSTR DEBUG_LOG_NAME = "nvdaDll-debug.log";
    constexpr LPCSTR WARN_LOG_NAME = "nvdaDll.log";

    /// 初始化 loguru 日志库
    void loguru_init()
    {
#ifdef _DEBUG
        loguru::add_file("fakeNvda-debug.log", loguru::Truncate, loguru::Verbosity_INFO);
#endif // def _DEBUG

        DLOG_F(INFO, "loguru init.");
        DLOG_F(INFO, "BaoYi Dll API Version: %s", dll::boy::BOY_DLL_VERSION);
        DLOG_F(INFO, "Compiled at: %s %s", __DATE__, __TIME__);
    }

    /// 初始化 spdlog
    void spdlog_init()
    {
        // -- 彩色终端输出
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);

        // -- release 模式：日志文件仅输出 warn
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(WARN_LOG_NAME, true);
        file_sink->set_level(spdlog::level::warn);
#ifndef _DEBUG
        spdlog::logger logger("release_sink", { console_sink, file_sink });
        logger.set_level(spdlog::level::warn);
#else
        // -- debug 模式：日志文件输出 warn+debug
        auto debug_file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(DEBUG_LOG_NAME, true);
        debug_file_sink->set_level(spdlog::level::debug);
        spdlog::logger logger("debug_sink", { console_sink, file_sink, debug_file_sink });
        logger.set_level(spdlog::level::debug);
#endif // ndef _DEBUG

        // 注册为默认日志
        spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));

        // ==== 初始化完毕
        spdlog::info("spdlog init finished.");
        SPDLOG_INFO("Compiled at: {} {}", __DATE__, __TIME__);
    }

    /// 初始化全局日志
    void init()
    {
        // TODO: 移除 loguru
        loguru_init();
        spdlog_init();
    }

    /// 通过日志输出 WString 类型的变量
    void logWString(LPCSTR prefix, LPCSTR varName, LPCWSTR inWstring)
    {
        /// 输出字符串的长度，包括结尾的 null
        size_t outStringLen;
        /// 转化后的字符串，待输出
        char outString[MAX_PATH];
        wcstombs_s(&outStringLen, outString, (size_t)MAX_PATH, inWstring, (size_t)MAX_PATH - 1);
        DLOG_F(INFO, "[%s] %s[]: %s", prefix, varName, outString);
    }

} // nvdll::log::
} // nvdll::