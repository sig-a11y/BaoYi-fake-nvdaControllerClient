#include "log.hpp"
// -- [3rd]
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
// -- [proj]
#include "dll.hpp"

namespace nvdll {
namespace log {

    constexpr LPCSTR DEBUG_LOG_NAME = "nvdaDll-debug.log";
    constexpr LPCSTR LOG_NAME = "nvdaDll.log";

    /// 初始化 spdlog
    void spdlog_init()
    {
        // -- 彩色终端输出
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);

        // -- release 模式：日志默认输出 debug 以上，在输入端过滤
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(LOG_NAME, true);
        file_sink->set_level(spdlog::level::debug);
#ifndef _DEBUG
        spdlog::logger logger("release_sink", { console_sink, file_sink });
        logger.set_level(spdlog::level::info);
        logger.flush_on(spdlog::level::info);
#else
        // -- debug 模式：日志文件输出 warn+debug
        auto debug_file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(DEBUG_LOG_NAME, true);
        debug_file_sink->set_level(spdlog::level::debug);
        spdlog::logger logger("debug_sink", { console_sink, file_sink, debug_file_sink });
        logger.set_level(spdlog::level::debug);
        logger.flush_on(spdlog::level::debug);
#endif // ndef _DEBUG

        // 注册为默认日志
        spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));

        // ==== 初始化完毕
        SPDLOG_DEBUG("spdlog init finished.");
    }

    /// 初始化全局日志
    void init()
    {
        spdlog_init();
        nvdll::printDllInfo();
    }

} // nvdll::log::
} // nvdll::