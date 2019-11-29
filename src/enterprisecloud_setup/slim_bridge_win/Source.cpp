#include <windows.h>

#include <string>

std::wstring GetCurrentProcessName() { return L"3.0.0.0\\������ҵ����.exe"; }

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PWSTR pCmdLine, int nCmdShow) {
  WCHAR lpFilename[MAX_PATH] = {'\0'};
  // Retrieves the fully qualified path for the file that contains the
  // specified module the first parameter is a handle to the loaded module
  // whose path is being requested. If this parameter is NULL,
  // GetModuleFileName retrieves the path of the executable file of the
  // current process.
  do {
    auto filename_len = GetModuleFileNameW(NULL, lpFilename, MAX_PATH);
    if (filename_len < 1 || filename_len >= MAX_PATH) {
      break;
    }
    std::wstring file_path = lpFilename;
    auto iter = file_path.rfind(L"\\");
    if (std::wstring::npos == iter) {
      break;
    }
    file_path.resize(iter + 1);
    file_path.append(GetCurrentProcessName());

    /// TODO: ��Ҫ����Ŀ�������ǩ��֤�飬��У�鲻�Ϸ���������ִ��

    /// TODO: ��Ҫ�ԺڿƼ���ʽ��Ȩ���û�ִ̬��Ŀ�����

    STARTUPINFO si{0};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{0};
    CreateProcess(file_path.c_str(),  // module name
                  pCmdLine,           // Command line
                  NULL,               // Process handle not inheritable
                  NULL,               // Thread handle not inheritable
                  FALSE,              // Set handle inheritance to FALSE
                  0,                  // No creation flags
                  NULL,               // Use parent's environment block
                  NULL,               // Use parent's starting directory
                  &si,                // Pointer to STARTUPINFO structure
                  &pi);  // Pointer to PROCESS_INFORMATION structure

  } while (false);
  return 0;
}
