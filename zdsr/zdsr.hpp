#pragma once

#ifndef __ZDSR_API__
#define __ZDSR_API__

namespace ZDSRAPI;

#if _WIN64
    const string DLL_FNAME = "ZDSRAPI_x64.dll";
#else
    const string DLL_FNAME = "ZDSRAPI.dll";
#endif

/**
 * @brief 初始化语音接口
 * 
 * @param channelType 0=读屏通道，1=独立通道
 * @param channelName 独立通道名称, type为0时忽略,传入NULL; type为1时:独立通道名称, NULL或空字符串时,将使用默认名称"API"
 * @param bKeyDownInterrupt true 按键打断朗读, false 按键不打断朗读
 * @return int 0:成功; 1:版本不匹配
 */
int __stdcall InitTTS(int channelType, const wchar_t *channelName, bool bKeyDownInterrupt);
typedef int (__stdcall *InitTTSFunc)(int channelType, const wchar_t *channelName, bool bKeyDownInterrupt);

/**
 * @brief 朗读文本
 * 
 * @param text 要朗读的文本, UTF-16编码
 * @param bInterrupt true:打断朗读, false:等待空闲时朗读
 * @return int 0:成功; 1:版本不匹配; 2:ZDSR没有运行
 */
int __stdcall Speak(const wchar_t *text, bool bInterrupt);
typedef int (__stdcall *SpeakFunc)(const wchar_t *text, bool bInterrupt);

/**
 * @brief 获取读屏状态
 * 
 * @return int 1:版本不匹配; 2:ZDSR没有运行或没有授权; 3:正在朗读; 4:没有朗读
 */
int __stdcall GetSpeakState();
typedef int (__stdcall *GetSpeakStateFunc)();

/**
 * @brief 停止朗读
 */
void __stdcall StopSpeak();
typedef void (__stdcall *StopSpeakFunc)();

#endif // __ZDSR_API__