#pragma once
#ifndef __NVDLL_LOG_H
#define __NVDLL_LOG_H
// -- [sys] win
#include <wtypes.h> // LPCSTR; LPCWSTR
// -- [3rd]
// NOTE: 日志库配置在 "tweakme.h" 文件中
// Workaround:  https://github.com/gabime/spdlog/issues/3251
#define FMT_UNICODE 0
#include "spdlog/spdlog.h"

namespace nvdll {
namespace log {
#pragma region 全局变量定义
    // TODO: 提供初始化函数与指定日志名称、日志级别
#pragma region

    void init();

} // nvdll::log::
} // nvdll::
#endif // __NVDLL_LOG_H