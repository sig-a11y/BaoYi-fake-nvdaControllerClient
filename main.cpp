// ---- 私有头文件
#include "log.hpp"
#include "dll.hpp"  // dll::


#ifndef BUILD_EXE
/**
 * @brief DLL 主函数
 * @param hinstDLL handle to DLL module
 * @param fdwReason reason for calling function
 * @param __lpvReserved reserved
 * @return 
*/
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID __lpvReserved)
{
    // 派发调用原因
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        {
            // Initialize once for each new process.
#ifdef _DEBUG
            loguru::add_file("fakeNvda-debug.log", loguru::Append, loguru::Verbosity_INFO);
#endif // def _DEBUG
            DLOG_F(INFO, "loguru init.");
            DLOG_F(INFO, "BaoYi Dll API Version: %s", dll::BOY_DLL_VERSION);
            DLOG_F(INFO, "Compiled at: %s %s", __DATE__, __TIME__);

            dll::saveDllDirPath(hinstDLL);
        }
        break;

    case DLL_THREAD_ATTACH:
        // Do thread-specific initialization.
        break;

    case DLL_THREAD_DETACH:
        // Do thread-specific cleanup.
        break;

    case DLL_PROCESS_DETACH:
        {
            if (__lpvReserved != nullptr)
            {
                // do not do cleanup if process termination scenario
                break; 
            }

            // Perform any necessary cleanup.
            DLOG_F(INFO, "Perform any necessary cleanup.");
            dll::freeDll();
        }
        break;
    }

    return TRUE;
}

#else  // defined( BUILD_EXE )
/// exe 主函数
int main()
{
    std::cout 
        << "[DllMain] BaoYi Dll API Version: " << BOY_DLL_VERSION << "\n"
        << "[DllMain] Compiled at: " << __DATE__ << " " << __TIME__
        << std::endl;

    bool has_error = loadBaoYiDll();
    if (has_error) {
        exit(EXIT_FAILURE);
    }

    std::cout << "Hello World!\n";
    return EXIT_SUCCESS;
}
#endif // BUILD_EXE
