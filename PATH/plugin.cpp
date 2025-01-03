#include "plugin.h"
#include <Windows.h>

// Examples: https://github.com/x64dbg/x64dbg/wiki/Plugins
// References:
// - https://help.x64dbg.com/en/latest/developers/plugins/index.html
// - https://x64dbg.com/blog/2016/10/04/architecture-of-x64dbg.html
// - https://x64dbg.com/blog/2016/10/20/threading-model.html
// - https://x64dbg.com/blog/2016/07/30/x64dbg-plugin-sdk.html

BOOL AddDllDirectoryWithEnv(wchar_t* path)
{
    //_plugin_logprintf("Add '%ls' path\n", path);
    wchar_t pathContent[4096];
    if (!ExpandEnvironmentStrings(path, pathContent, sizeof(pathContent) / sizeof(wchar_t)))
    {
        return FALSE;
    }
    return (AddDllDirectory(pathContent) != 0);
}

bool loadPathFromRegistry(HKEY hKeyType, const wchar_t* registryPath)
{
    HKEY hKey;
    LONG lResult;

    // ChatGPT....
    // Open the registry key
    lResult = RegOpenKeyEx(hKeyType, registryPath, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS) 
    {
        _plugin_logprint("RegOpenKeyEx failed\n");
        return false;
    }

    const DWORD bufferSize = 2048;
    wchar_t buffer[bufferSize];
    DWORD dataSize = bufferSize;

    // Read the registry value
    lResult = RegQueryValueEx(hKey, L"Path", NULL, NULL, reinterpret_cast<LPBYTE>(buffer), &dataSize);

    if (lResult != ERROR_SUCCESS) 
    {
        _plugin_logprint("Error reading registry value\n");
        RegCloseKey(hKey);
        return true;
    }

    wchar_t* ptr = buffer;
    while (true)
    {
        wchar_t* nextPtr = wcsstr(ptr, L";");
        if (nextPtr == NULL && ptr)
        {
            AddDllDirectoryWithEnv(ptr);
            break;
        }

        nextPtr[0] = L'\x00';
        if (!AddDllDirectoryWithEnv(ptr))
            _plugin_logprintf("AddDllDirectory failed. Directory: '%ls'. Error = 0x%X\n", ptr, GetLastError());

        ptr = nextPtr + 1;
    }

    // Close the registry key
    RegCloseKey(hKey);
    return true;
}

// Initialize your plugin data here.
bool pluginInit(PLUG_INITSTRUCT* initStruct)
{
    loadPathFromRegistry(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");
    loadPathFromRegistry(HKEY_CURRENT_USER, L"Environment");
    return true;
}

// Deinitialize your plugin data here.
// NOTE: you are responsible for gracefully closing your GUI
// This function is not executed on the GUI thread, so you might need
// to use WaitForSingleObject or similar to wait for everything to close.
void pluginStop()
{
    // Prefix of the functions to call here: _plugin_unregister
}

// Do GUI/Menu related things here.
// This code runs on the GUI thread: GetCurrentThreadId() == GuiGetMainThreadId()
// You can get the HWND using GuiGetWindowHandle()
void pluginSetup()
{
    // Prefix of the functions to call here: _plugin_menu
}
