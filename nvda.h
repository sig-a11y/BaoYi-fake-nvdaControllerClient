#pragma once
#if !defined(__NVDA_H) && !defined(__nvdaController_h__)
/// NVDA 函数名兼容
#define __NVDA_H
#include "nvdll_impl.h"


#define nvdaController_testIfRunning    testIfRunning_impl
#define nvdaController_speakText        speakText_impl
#define nvdaController_cancelSpeech     cancelSpeech_impl
#define nvdaController_brailleMessage   brailleMessage_impl
#define nvdaController_getProcessId     getProcessId_impl
#define nvdaController_speakSsml        speakSsml_impl
#define nvdaController_setOnSsmlMarkReachedCallback setOnSsmlMarkReachedCallback_impl

// ---- 导出函数类型定义

typedef error_status_t(__stdcall *TestIfRunningFunc)();
typedef error_status_t(__stdcall *SpeakTextFunc)(const wchar_t* text);
typedef error_status_t(__stdcall *CancelSpeechFunc)();
typedef error_status_t(__stdcall *BrailleMessageFunc)(const wchar_t* message);
typedef error_status_t(__stdcall* GetProcessIdFunc)(unsigned long* pid);
typedef error_status_t(__stdcall* SpeakSsmlFunc)(const wchar_t* ssml, const enum SYMBOL_LEVEL symbolLevel, const enum SPEECH_PRIORITY priority, const boolean asynchronous);
typedef error_status_t(__stdcall* SetOnSsmlMarkReachedCallbackFunc)(onSsmlMarkReachedFuncType callback);

#endif // __NVDA_H