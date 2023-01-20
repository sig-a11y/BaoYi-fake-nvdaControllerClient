#include <iostream>
#include "boy_impl.h"
#include "nvda.h"


int main(int argc, char* argv[]) {
	error_status_t ret = nvdaController_testIfRunning();
	if (ret != 0) {
		std::cout 
			<< "Error communicating with NVDA. "
			<< "ret=" << ret
			<< std::endl;
		return 1;
	}

	nvdaController_speakText(L"This is a test speech message");
	nvdaController_speakText(L"Test completed!");
	return 0;
}
