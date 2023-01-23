#pragma once
#ifndef __BOY_INI_H
#define __BOY_INI_H
#include <wtypes.h>  // LPCWSTR


namespace ini {
    // TODO: 使用枚举

    /// 保益 DLL 配置 ini 文件名. 
    LPCWSTR INI_NAME = L"nvdaCfg.ini";
    LPCWSTR INI_NAME_CN = L"朗读配置.ini";
    LPCWSTR INI_APP_NAME = L"APP";
    LPCWSTR INI_APP_NAME_CN = L"朗读";
    LPCWSTR INI_KEY_USE_SLAVE = L"USE_SLAVE";
    LPCWSTR INI_KEY_USE_SLAVE_CN = L"独立通道";
    LPCWSTR INI_KEY_USE_APPEND = L"USE_APPEND";
    LPCWSTR INI_KEY_USE_APPEND_CN = L"排队朗读";
    LPCWSTR INI_KEY_ALLOW_BREAK = L"ALLOW_BREAK";
    LPCWSTR INI_KEY_ALLOW_BREAK_CN = L"按键打断朗读";

    void loadIni();
} // ini::

#endif // __BOY_INI_H