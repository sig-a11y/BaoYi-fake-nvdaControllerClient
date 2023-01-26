#include <conio.h>
#include <ctype.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <Windows.h>
#include "nvda.h"

void speakPause()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

const std::wstring currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	wchar_t    buf[256];

	localtime_s(&tstruct, &now);

	// https://en.cppreference.com/w/cpp/chrono/c/wcsftime
	wcsftime(buf, sizeof(buf), L"%Y年%m月%d日.%X", &tstruct);

	return buf;
}

int main(int argc, char* argv[])
{
	// 显式加载
	auto dllHandle = LoadLibrary(L"nvdaControllerClient.dll");
	if (!dllHandle)
	{
		std::cerr << "Failed to LoadLibrary nvdaControllerClient.dll" << std::endl;
		return 1;
	}

	auto testIfRunning = (decltype(nvdaController_testIfRunning)*)GetProcAddress(dllHandle, "nvdaController_testIfRunning");
	if (!testIfRunning)
	{
		std::cerr << "Failed to get nvdaController_testIfRunning" << std::endl;
		return 1;
	}
	auto speakText = (decltype(nvdaController_speakText)*)GetProcAddress(dllHandle, "nvdaController_speakText");
	if (!speakText)
	{
		std::cerr << "Failed to get nvdaController_speakText" << std::endl;
		return 1;
	}
	auto cancelSpeech = (decltype(nvdaController_cancelSpeech)*)GetProcAddress(dllHandle, "nvdaController_cancelSpeech");
	if (!cancelSpeech)
	{
		std::cerr << "Failed to get nvdaController_cancelSpeech" << std::endl;
		return 1;
	}


	error_status_t ret = testIfRunning();
	if (ret != 0) {
		std::cout 
			<< "Error communicating with NVDA. "
			<< "ret=" << ret
			<< std::endl;
		return 1;
	}

	speakText(L"This is a test speech message");
	speakPause();

	speakText(L"中文输出测试.");
	speakPause();
	speakText(L"当前时间：");
	speakText(currentDateTime().c_str());
	speakPause();

	// ---- 循环输出测试
	int ch;
	_cputs("Press 'E' to exit.\n");
	do
	{
		ch = _getch();
		ch = toupper(ch);

		switch (ch)
		{
		case '1':
			speakText(L"中文输出测试.");
			break;

		case '2':
			speakText(L"当前时间：");
			speakText(currentDateTime().c_str());
			break;

		default:
			_cputs("Press 'E' to exit.\n");
			break;
		}

	} while (ch != 'E');
	_putch('\r');    // Carriage return
	_putch('\n');    // Line feed

	speakText(L"Test completed!");
	return 0;
}
