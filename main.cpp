// -- [proj]
#include "log.hpp" // nvdll::log::
#include "dll.hpp" // nvdll::
#include "input.hpp"

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
            nvdll::saveDllDirPath(hinstDLL);
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
            spdlog::info("[DLL_PROCESS_DETACH]");
            // nvdll::input::killListenerThread();
            nvdll::input::removeInputHook();

            if (__lpvReserved != nullptr)
            {
                // do not do cleanup if process termination scenario
                spdlog::warn("Process termination scenario, will not do cleanup.");
                break; 
            }

            // Perform any necessary cleanup.
            spdlog::info("Perform any necessary cleanup.");
            nvdll::freeDll();
        }
        break;
    }

    return TRUE;
}
