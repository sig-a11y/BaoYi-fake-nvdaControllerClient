#include "log.hpp"

namespace nvdll {
namespace log {

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