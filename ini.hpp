#pragma once
#ifndef __BOY_INI_H
#define __BOY_INI_H
#include "SimpleIni.h"


namespace ini {
    /// 保益 DLL 配置 ini 文件名. TODO: 使用枚举
    const LPCWSTR INI_NAME = L"nvdaCfg.ini";
    const LPCWSTR INI_NAME_CN = L"朗读配置.ini";
    const LPCWSTR INI_APP_NAME = L"APP";
    const LPCWSTR INI_APP_NAME_CN = L"朗读";
    const LPCWSTR INI_KEY_USE_SLAVE = L"USE_SLAVE";
    const LPCWSTR INI_KEY_USE_SLAVE_CN = L"独立通道";
    const LPCWSTR INI_KEY_USE_APPEND = L"USE_APPEND";
    const LPCWSTR INI_KEY_USE_APPEND_CN = L"排队朗读";
    const LPCWSTR INI_KEY_ALLOW_BREAK = L"ALLOW_BREAK";
    const LPCWSTR INI_KEY_ALLOW_BREAK_CN = L"按键打断朗读";

    void loadIni();
} // ini::

#endif // __BOY_INI_H