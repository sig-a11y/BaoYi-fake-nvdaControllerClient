#include <iostream>
#include <chrono>
#include <thread>
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
	error_status_t ret = nvdaController_testIfRunning();
	if (ret != 0) {
		std::cout 
			<< "Error communicating with NVDA. "
			<< "ret=" << ret
			<< std::endl;
		return 1;
	}

	nvdaController_speakText(L"This is a test speech message");
	speakPause();

	nvdaController_speakText(L"中文输出测试.");
	speakPause();
	nvdaController_speakText(L"当前时间：");
	nvdaController_speakText(currentDateTime().c_str());
	speakPause();

	nvdaController_speakText(L"Test completed!");
	return 0;
}
