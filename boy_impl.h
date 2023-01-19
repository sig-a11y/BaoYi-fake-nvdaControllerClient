#pragma once
#include <winerror.h>

#ifdef BOY_IMPL_IMPORTS
#define BOY_IMPL_API __declspec(dllimport)
#else
#define BOY_IMPL_API 
#endif

/// 标准的错误返回值
typedef unsigned long error_status_t;
#define WIN_RPC_RET error_status_t __stdcall


/**
 * @brief 检查读屏器是否在运行
 * @return 错误码
*/
BOY_IMPL_API WIN_RPC_RET testIfRunning_impl();

/**
 * @brief 向读屏器输出文本
 * @param text 待输出的文本
 * @return 错误码
*/
BOY_IMPL_API WIN_RPC_RET speakText_impl(const wchar_t* text);

/**
 * @brief 静音当前输出
 * @return 错误码
*/
BOY_IMPL_API WIN_RPC_RET cancelSpeech_impl();

/**
 * @brief 【暂不支持】输出盲文
 * @param message 待输出的文本
 * @return 错误码
*/
BOY_IMPL_API WIN_RPC_RET brailleMessage_impl(const wchar_t* message);
