#pragma once
#ifndef __BOYSR_HPP
#define __BOYSR_HPP

namespace boysr_api
{

/// DLL 版本
constexpr wchar_t DLL_VERSION[] = L"v1.7";

#if _WIN64
constexpr wchar_t DLL_NAME[] = L"BoyCtrl-x64.dll";
#else
constexpr wchar_t DLL_NAME[] = L"BoyCtrl.dll";
#endif

// 导入纯 C 头文件
#include "BoyCtrl.h"

}
#endif // __BOYSR_HPP