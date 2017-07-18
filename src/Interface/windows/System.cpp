/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#include "System.h"

#include <direct.h>
#include <Shlobj.h>
#include <locale.h>
#include <string>
#include <../../Core/Path.h>

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

namespace Nidium {
namespace Interface {

static char *WideCharToUTF8(WCHAR *str)
{
    int length = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    if (length == 0) {
        return nullptr;
    }

    char* output = new char[length];
    if (WideCharToMultiByte(CP_UTF8, 0, str, -1, output , length, NULL, NULL) == 0) {
        delete[] output;
        return nullptr;
    }

    return output;
}

void OnSize(HWND hwnd, UINT flag, int width, int height)
{
    // Handle resizing
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE: {
        int width = LOWORD(lParam);  // Macro to get the low-order word.
        int height = HIWORD(lParam); // Macro to get the high-order word.
                                     // Respond to the message:
        OnSize(hwnd, (UINT)wParam, width, height);
    }
    break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(hwnd, &ps);
    }
    break;
    case WM_CLOSE:
        if (MessageBox(hwnd, "Really quit?", "Nidium", MB_OKCANCEL) == IDOK) {
            DestroyWindow(hwnd);
        }
        // Else: User canceled. Do nothing.
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Attach output of application to parent console
// Source : https://www.tillett.info/2013/05/13/how-to-create-a-windows-program-that-works-as-both-as-a-gui-and-console-application/
static BOOL AttachOutputToConsole(void) {
    HANDLE consoleHandleOut, consoleHandleError;

    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        // Redirect unbuffered STDOUT to the console
        consoleHandleOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (consoleHandleOut != INVALID_HANDLE_VALUE) {
            freopen("CONOUT$", "w", stdout);
            setvbuf(stdout, nullptr, _IONBF, 0);
        } else {
            return false;
        }

        // Redirect unbuffered STDERR to the console
        consoleHandleError = GetStdHandle(STD_ERROR_HANDLE);
        if (consoleHandleError != INVALID_HANDLE_VALUE) {
            freopen("CONOUT$", "w", stderr);
            setvbuf(stderr, nullptr, _IONBF, 0);
        } else {
            return false;
        }
        return true;
    }

    //Not a console application
    return false;
}

System::System()
{
    m_fBackingStorePixelRatio = 1.0;

    // Redirect stdout to the current terminal (if any)
    AttachOutputToConsole();

    // Get the path to %USERPROFILE%\Documents
    PWSTR userDirectoryW;
    if (SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, NULL, &userDirectoryW) == S_OK) {
         char *userDirectory = WideCharToUTF8(userDirectoryW);
         m_UserDirectory = strdup((std::string(userDirectory) + "\\").c_str());
         free(userDirectory);
    } else {
        ndm_log(NDM_LOG_ERROR, "System", "Failed to get path to user directory");
    }
}

System::~System()
{
    if (m_EmbedPath) {
        free(m_EmbedPath);
    }

    if (m_UserDirectory) {
        free(m_UserDirectory);
    }


}

void System::print(const char *buf)
{
    fwrite(buf, 1, strlen(buf), stdout);
}


float System::backingStorePixelRatio()
{
    return m_fBackingStorePixelRatio;
}

void System::initSystemUI(HINSTANCE hInstance)
{
    if (!m_SystemUIReady) {
        const LPCSTR CLASS_NAME = TEXT("Nidium");
        const LPCSTR text = TEXT("Nidium demo");
        WNDCLASS wc = {};

        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = CLASS_NAME;
        RegisterClass(&wc);

        HWND hwnd = CreateWindowEx( 0, CLASS_NAME, text, WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            NULL,       // Parent window    
            NULL,       // Menu
            hInstance,  // Instance handle
            NULL        // Additional application data
        );

        if (hwnd == NULL) {
            return;
        }

        int nCmdShow = 0;
        ShowWindow(hwnd, nCmdShow);
    }
}

// FIXME : getCacheDirectory() should point to AppData folder
const char *System::getCacheDirectory()
{
    const char *homedir = getUserDirectory();
    char nHome[4096];

    snprintf(nHome, 4096, "%s.config/nidium/", homedir);

    if (_mkdir(nHome) == -1 && errno != EEXIST) {
        ndm_logf(NDM_LOG_ERROR, "System", "Can't create cache directory %s", nHome);
        return NULL;
    }

    return strdup(nHome);
}

const char *System::getEmbedDirectory()
{
    if (!m_EmbedPath) {
        WCHAR nidiumPathW[MAX_PATH];
        if (!GetModuleFileNameW(NULL, nidiumPathW, MAX_PATH) != 0) {
            ndm_log(NDM_LOG_ERROR, "System", "Failed to get module filename");
            return nullptr;
        }

        const char *embed = "../src/Embed/";
        char *nidiumPath = WideCharToUTF8(nidiumPathW);
        char dir[MAX_PATH];
        char driveLetter[3];
        std::string path;

        _splitpath(nidiumPath, &driveLetter[0], &dir[0], NULL, NULL);

        path += std::string(driveLetter) + std::string(dir) + "/" + std::string(embed);

        m_EmbedPath = Core::Path::Sanitize(path.c_str());

        delete[] nidiumPath;
    }

    return m_EmbedPath;
}

const char* System::getUserDirectory() {
    return m_UserDirectory;
};

const char *System::getLanguage()
{

    const char *lang;

    lang = setlocale(LC_COLLATE, NULL);

    return lang;
}


const char *System::cwd()
{
    static char dir[MAXPATHLEN];

    getcwd(dir, MAXPATHLEN);

    return dir;
}

void System::alert(const char *message, Nidium::Interface::SystemInterface::AlertType type)
{

    HWND hWnd = NULL;
    LPCTSTR lpText = message;
    LPCTSTR lpCaption = "nidium";
    UINT uType = MB_OK | MB_ICONSTOP;

    MessageBox(hWnd, lpText, lpCaption, uType);

}

void System::sendNotification(const char *title, const char *content, bool sound)
{
    HWND hWnd = NULL;
    LPCTSTR lpText = content;
    LPCTSTR lpCaption = title;
    UINT uType = MB_OK;

    MessageBox(hWnd, lpText, lpCaption, uType);
}

const char *System::execute(const char *cmd)
{
    // http://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix
    std::string strResult;
    HANDLE hPipeRead, hPipeWrite;

    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
    saAttr.bInheritHandle = TRUE;   //Pipe handles are inherited by child process.
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe to get results from child's stdout.
    if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0)) {
        return strResult.c_str();
    }

    STARTUPINFOW si = {sizeof(STARTUPINFOW)};
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.hStdOutput = hPipeWrite;
    si.hStdError = hPipeWrite;
    si.wShowWindow = SW_HIDE;       // Prevents cmd window from flashing. Requires STARTF_USESHOWWINDOW in dwFlags.

    PROCESS_INFORMATION pi = { 0 };

    BOOL fSuccess = CreateProcessW(NULL, (LPWSTR)cmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    if (!fSuccess) {
       CloseHandle(hPipeWrite);
        CloseHandle(hPipeRead);
        return strResult.c_str();
    }

    bool bProcessEnded = false;
    for (; !bProcessEnded;) {
        // Give some timeslice (50ms), so we won't waste 100% cpu.
        bProcessEnded = WaitForSingleObject(pi.hProcess, 50) == WAIT_OBJECT_0;

        // Even if process exited - we continue reading, if there is some data available over pipe.
        for (;;) {
            char buf[1024];
            DWORD dwRead = 0;
            DWORD dwAvail = 0;

            if (!::PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL))
                break;

            if (!dwAvail) // no data available, return
                break;

            if (!::ReadFile(hPipeRead, buf, MIN(sizeof(buf) - 1, dwAvail), &dwRead, NULL) || !dwRead)
                // error, the child process might ended
                break;

            buf[dwRead] = 0;
            strResult += buf;
        }
    } //for

    CloseHandle(hPipeWrite);
    CloseHandle(hPipeRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return strResult.c_str();

}

} // namespace Interface
} // namespace Nidium

