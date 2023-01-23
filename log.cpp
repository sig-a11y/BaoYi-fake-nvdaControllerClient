#include "log.hpp"
#include "dll.hpp"
#include "spdlog/sinks/basic_file_sink.h"

namespace nvdll {
namespace log {

    void init()
    {
#ifdef _DEBUG
        loguru::add_file("fakeNvda-debug.log", loguru::Truncate, loguru::Verbosity_INFO);
        auto log = spdlog::basic_logger_mt("basic_logger", "fakeNvda-debug.txt");
        log->set_level(spdlog::level::debug);
        spdlog::flush_every(std::chrono::seconds(1));
#endif // def _DEBUG
        DLOG_F(INFO, "loguru init.");
        DLOG_F(INFO, "BaoYi Dll API Version: %s", dll::boy::BOY_DLL_VERSION);
        DLOG_F(INFO, "Compiled at: %s %s", __DATE__, __TIME__);
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