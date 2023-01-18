#pragma once

// 朗读完成回调
// reason: 回调原因，1=朗读完成，2=被新的朗读打断，3=被停止调用打断
typedef void (__stdcall*BoyCtrlSpeakCompleteFunc)(int reason);

// 错误码，要查看具体错误原因请在初始化时传入日志输出路径
typedef enum
{
	// 成功
	e_bcerr_success = 0,
	// 一般错误
	e_bcerr_fail = 1,
	// 参数错误（如字符串为空或过长）
	e_bcerr_arg = 2,
	// 功能不可用
	e_bcerr_unavailable = 3,
} BoyCtrlError;


// 初始化接口
// logPath: 日志输出路径，UTF-16le编码，NULL或空字符串=无日志输出
BoyCtrlError __stdcall BoyCtrlInitialize(const wchar_t* logPath);
typedef BoyCtrlError(__stdcall *BoyCtrlInitializeFunc)(const wchar_t *logPath);

// 初始化接口
// logPath: 日志输出路径，ANSI编码，NULL或空字符串=无日志输出
BoyCtrlError __stdcall BoyCtrlInitializeAnsi(const char* logPath);

// 初始化接口
// logPath: 日志输出路径，UTF-8编码，NULL或空字符串=无日志输出
BoyCtrlError __stdcall BoyCtrlInitializeU8(const char* logPath);

// 朗读文本
// text: 朗读的文本，UTF-16le编码
// withSlave: false=使用读屏通道，true=使用独立通道
// append: 是否排队朗读
// allowBreak: 是否允许用户打断，使用读屏通道时该参数被忽略
// onCompletion: 朗读完成回调，NULL=完成不需要通知，使用读屏通道时该参数被忽略
BoyCtrlError __stdcall BoyCtrlSpeak(const wchar_t* text, bool withSlave, bool append, bool allowBreak, BoyCtrlSpeakCompleteFunc onCompletion);
typedef BoyCtrlError(__stdcall *BoyCtrlSpeakFunc)(const wchar_t *text, bool withSlave, bool append, bool allowBreak, BoyCtrlSpeakCompleteFunc onCompletion);

// 朗读文本(ANSI)
// text: 朗读的文本，ANSI编码
// withSlave: false=使用读屏通道，true=使用独立通道
// append: 是否排队朗读
// allowBreak: 是否允许用户打断，使用读屏通道时该参数被忽略
// onCompletion: 朗读完成回调，NULL=完成不需要通知，当使用读屏通道时该参数被忽略
BoyCtrlError __stdcall BoyCtrlSpeakAnsi(const char* text, bool withSlave, bool append, bool allowBreak, BoyCtrlSpeakCompleteFunc onCompletion);

// 朗读文本(UTF-8)
// text: 朗读的文本，UTF-8编码
// withSlave: false=使用读屏通道，true=使用独立通道
// append: 是否排队朗读
// allowBreak: 是否允许用户打断，使用读屏通道时该参数被忽略
// onCompletion: 朗读完成回调，NULL=完成不需要通知，当使用读屏通道时该参数被忽略
BoyCtrlError __stdcall BoyCtrlSpeakU8(const char* text, bool withSlave, bool append, bool allowBreak, BoyCtrlSpeakCompleteFunc onCompletion);

// 停止朗读
// withSlave: false=停止读屏通道，true=停止独立通道
BoyCtrlError __stdcall BoyCtrlStopSpeaking(bool withSlave);
typedef BoyCtrlError(__stdcall *BoyCtrlStopSpeakingFunc)(bool withSlave);

// 暂停指定时间的读屏朗读
// ms: 暂停时间，单位毫秒，10~10000ms
BoyCtrlError __stdcall BoyCtrlPauseScreenReader(int ms);
typedef BoyCtrlError(__stdcall *BoyCtrlPauseScreenReaderFunc)(int ms);

// 退出清理
void __stdcall BoyCtrlUninitialize();
typedef void(__stdcall *BoyCtrlUninitializeFunc)();
