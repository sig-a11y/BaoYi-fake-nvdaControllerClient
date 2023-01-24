#pragma once
#ifndef __NVDLL_LOG_H
#define __NVDLL_LOG_H
// -- [sys] win
#include <wtypes.h> // LPCSTR; LPCWSTR
// -- [3rd]
#include "loguru.hpp" // 日志记录框架
// NOTE: 日志库配置在 "tweakme.h" 文件中
#include "spdlog/spdlog.h"

namespace nvdll {
namespace log {
#pragma region 全局变量定义
    // TODO: 提供初始化函数与指定日志名称、日志级别
    extern spdlog::logger logger;
#pragma region

    void init();
    void logWString(LPCSTR prefix, LPCSTR varName, LPCWSTR inWstring);

} // nvdll::log::
} // nvdll::
#endif // __NVDLL_LOG_H