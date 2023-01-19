#pragma once
#include <winerror.h>

#ifdef BOY_IMPL_IMPORTS
#define BOY_IMPL_API __declspec(dllimport)
#else
#define BOY_IMPL_API 
#endif
typedef unsigned long error_status_t;


/**
 * @brief 检查读屏器是否在运行
 * @return 错误码
*/
BOY_IMPL_API error_status_t __stdcall testIfRunning_impl();

/**
 * @brief 向读屏器输出文本
 * @param text 待输出的文本
 * @return 错误码
*/
BOY_IMPL_API error_status_t __stdcall speakText_impl(const wchar_t* text);

/**
 * @brief 静音当前输出
 * @return 错误码
*/
BOY_IMPL_API error_status_t __stdcall cancelSpeech_impl();

/**
 * @brief 【暂不支持】输出盲文
 * @param message 待输出的文本
 * @return 错误码
*/
BOY_IMPL_API error_status_t __stdcall brailleMessage_impl(const wchar_t* message);
