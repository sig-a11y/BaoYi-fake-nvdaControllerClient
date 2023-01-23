#pragma once
#ifndef __NVDLL_IMPL_H
#define __NVDLL_IMPL_H
// -- [sys] win
#include <winerror.h>


#ifdef NVDLL_IMPL_IMPORTS
#define NVDLL_IMPL_API __declspec(dllimport)
#else // 导出函数
#ifdef _DEBUG
/// 调试模式时，导出实现 impl 函数
#define NVDLL_IMPL_API __declspec(dllexport)
#else // REL mode
/// 发布模式时，不导出
#define NVDLL_IMPL_API
#endif // def _DEBUG
#endif // def NVDLL_IMPL_IMPORTS

/// 标准的错误返回值
typedef unsigned long error_status_t;
#define WIN_RPC_RET error_status_t __stdcall


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    /**
        * @brief 检查读屏器是否在运行
        * @return 错误码
    */
    NVDLL_IMPL_API WIN_RPC_RET testIfRunning_impl();

    /**
        * @brief 向读屏器输出文本
        * @param text 待输出的文本
        * @return 错误码
    */
    NVDLL_IMPL_API WIN_RPC_RET speakText_impl(const wchar_t* text);

    /**
        * @brief 静音当前输出
        * @return 错误码
    */
    NVDLL_IMPL_API WIN_RPC_RET cancelSpeech_impl();

    /**
        * @brief 【暂不支持】输出盲文
        * @param message 待输出的文本
        * @return 错误码
    */
    NVDLL_IMPL_API WIN_RPC_RET brailleMessage_impl(const wchar_t* message);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __NVDLL_IMPL_H