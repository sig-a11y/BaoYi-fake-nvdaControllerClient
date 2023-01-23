#pragma once
#ifndef __NVDLL_LOG_H
#define __NVDLL_LOG_H
#include <wtypes.h>
#include "loguru.hpp" // 日志记录框架

namespace nvdll {
namespace log {
#pragma region 全局变量定义
    // TODO: 提供初始化函数与指定日志名称、日志级别
#pragma region


    void logWString(LPCSTR prefix, LPCSTR varName, LPCWSTR inWstring);

} // nvdll::log::
} // nvdll::
#endif // __NVDLL_LOG_H