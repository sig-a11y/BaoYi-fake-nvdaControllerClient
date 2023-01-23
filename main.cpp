// -- [proj]
#include "log.hpp" // loguru::; DLOG_F
#include "dll.hpp" // dll::


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
            nvdll::log::init();
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
