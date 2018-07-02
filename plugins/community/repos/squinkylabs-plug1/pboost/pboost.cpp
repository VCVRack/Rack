/**
 * This is an unfinished experiment. 
 * It's a windows only command line program that
 * find the Rack process in memory, and sets its 
 * priority class to realtime.
 */

#include <stdio.h>
#include <windows.h>
#include <psapi.h>
#include <string>

class ProcessNameAndHandle
{
public:
    std::string name;
    HANDLE handle = INVALID_HANDLE_VALUE;
};

ProcessNameAndHandle  getProcessNameAndHandle(DWORD pid);
bool setRealtimePriority(HANDLE pHandle);
bool enablePrivilege(HANDLE hProcess, const char * privilege);
std::string GetLastErrorAsString();

int main(int argc, char** argv)
{

    HANDLE realHandle = GetCurrentProcess();
    bool b = enablePrivilege(realHandle, SE_DEBUG_NAME);
    printf("try set debug on us: %d\n", b);
    if (!b) {
        // TODO: we should re-run here
        printf("can't get debug right from system. Try running as admin\n");
        fflush(stdout);

        // Spawn a copy of ourselves, via ShellExecuteEx().
        // The "runas" verb is important because that's what
        // internally triggers Windows to open up a UAC prompt.
       // HANDLE child = ShellExecuteEx(argc, argv, "runas");
        SHELLEXECUTEINFO sinfo;
        memset(&sinfo, 0, sizeof(SHELLEXECUTEINFO));
        sinfo.cbSize       = sizeof(SHELLEXECUTEINFO);
        sinfo.fMask        = SEE_MASK_FLAG_DDEWAIT |
                        SEE_MASK_NOCLOSEPROCESS;
        sinfo.hwnd         = NULL;
        sinfo.lpFile       = argv[0];
        sinfo.lpParameters = "";
        sinfo.lpVerb       = "runas"; // <<-- this is what makes a UAC prompt show up
        sinfo.nShow        = SW_SHOWMAXIMIZED;

        // The only way to get a UAC prompt to show up
        // is by calling ShellExecuteEx() with the correct
        // SHELLEXECUTEINFO struct.  Non privlidged applications
        // cannot open/start a UAC prompt by simply spawning
        // a process that has the correct XML manifest.
        BOOL result = ShellExecuteEx(&sinfo);
        if (!result) {
            printf("re-exec as admin failed\n");
            return -1;
        }

       // HINSTANCE appInstance = sinfo.hInstApp;

    
        printf("exec worked. hp=%x\n", sinfo.hProcess);
        fflush(stdout);
        // User accepted UAC prompt (gave permission).
        // The unprivileged parent should wait for
        // the privileged child to finish.
        WaitForSingleObject(sinfo.hProcess, INFINITE);
        printf("orig proc finished waiting\n");

        DWORD exitCode=666;
        GetExitCodeProcess(sinfo.hProcess, &exitCode);
        printf("EXIT CODE %d\n", exitCode);
        CloseHandle(sinfo.hProcess);
    }

    // here were able to set debug
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        printf("enum proc failed\n");
        return 1;
    }


    // Calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);

    // find the pid for Rack.
    HANDLE rackProcessHandle = INVALID_HANDLE_VALUE;
    std::string rackName("Rack.exe");
    for (i = 0; i < cProcesses; i++) {
        if (aProcesses[i] != 0) {
            //PrintProcessNameAndID( aProcesses[i] );
            ProcessNameAndHandle proc = getProcessNameAndHandle(aProcesses[i]);
            if (proc.name == rackName) {
                rackProcessHandle = proc.handle;
            } else {
                CloseHandle(proc.handle);
            }
        }
    }

    printf("rack pid = %d\n", rackProcessHandle);
    if (rackProcessHandle == INVALID_HANDLE_VALUE) {
        printf("could not find rack process\n");
        return -1;
    }

    bool bSet = setRealtimePriority(rackProcessHandle);
    CloseHandle(rackProcessHandle);
}



/**
 *
 */
ProcessNameAndHandle getProcessNameAndHandle(DWORD processID)
{
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

    // Get a handle to the process.
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE, processID);

    // Get the process name
    if (NULL != hProcess) {
        HMODULE hMod;
        DWORD cbNeeded;

        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
            &cbNeeded)) {
            GetModuleBaseName(hProcess, hMod, szProcessName,
                sizeof(szProcessName) / sizeof(TCHAR));
        } else printf("could not enum proc\n");
    } else {
        return {};
    }

    // Release the handle to the process.
    // CloseHandle(hProcess);
    return {szProcessName, hProcess};
}

#if 0
// Returns the last Win32 error, in string format. Returns an empty string if there is no error.//Returns 
std::string GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0)
        return std::string(); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}
#endif

bool setClassRealtime(HANDLE h)
{
    auto x = GetPriorityClass(h);
    auto b = SetPriorityClass(h, REALTIME_PRIORITY_CLASS);
    auto c = GetPriorityClass(h);

#if 0
    printf("setrealtime(%d) %d, %d, %d\n", REALTIME_PRIORITY_CLASS, x, b, c);
    if (!b) {
        printf("SetP call failed with %d (%s)\n", GetLastError(), GetLastErrorAsString().c_str());
    }
 #endif

    return c == REALTIME_PRIORITY_CLASS;
}


bool enablePrivilege(HANDLE hProcess, const char * privilege)
{
    printf("called ep with %s\n", privilege);
    struct
    {
        DWORD Count;
        LUID_AND_ATTRIBUTES Privilege[1];
    } Info;

    HANDLE Token;
    BOOL Result;

    // Open the token.

    Result = OpenProcessToken(hProcess,
        TOKEN_ADJUST_PRIVILEGES,
        &Token);

    if (Result != TRUE) {
        printf("ep Cannot open process token.\n");
        return FALSE;
    }


    Info.Count = 1;
    Info.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;


    // Get the LUID.
    Result = LookupPrivilegeValue(NULL,
        privilege,
        &(Info.Privilege[0].Luid));

    if (Result != TRUE) {
        printf("ep Cannot get privilege for %s.\n", privilege);
        return FALSE;
    }

    // Adjust the privilege.
    Result = AdjustTokenPrivileges(Token, FALSE,
        (PTOKEN_PRIVILEGES) &Info,
        0, NULL, NULL);

    CloseHandle(Token);

    // Check the result.

    if (Result != TRUE) {
        printf("ep Cannot adjust token privileges (%u)\n", GetLastError());
        return FALSE;
    } else {
        if (GetLastError() != ERROR_SUCCESS) {
         //   printf("getlasterror = %d, %s\n", GetLastError(), GetLastErrorAsString().c_str());
            printf("Cannot enable the %s privilege; ", privilege
            );
            printf("please check the local policy.\n");
            return FALSE;
        }
    }

    return TRUE;
}

/**
 *
 */
bool setRealtimePriority(HANDLE hRackProcess)
{
    auto set = setClassRealtime(hRackProcess);
    printf("set pri class at start ret %d\n", set);
    return set;
}

