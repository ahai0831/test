// electron-dll-test.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <future>
#include <iostream>
#include <memory>
#include <string>

namespace f0fdownload {
typedef void (*OnNext)(const char *);
typedef void (*OnComplete)();
}  // namespace f0fdownload

typedef char *(__cdecl *Start)(const char *, const char *, int64_t);
typedef int64_t(__cdecl *Creat)();
typedef bool(__cdecl *Cancel)(const char *, int64_t);
typedef char *(__cdecl *Register)(f0fdownload::OnNext fNext,
                                  f0fdownload::OnComplete fComplete,
                                  char *sDownloadID, int64_t nAst);

std::promise<void> finish_promise = std::promise<void>();

int main(int argc, char *argv[]) {
  HINSTANCE dllLoad = LoadLibrary(L"electron-dll-project.dll");
  Creat cst = nullptr;
  Start startDownload = nullptr;
  Register reg = nullptr;
  if (dllLoad) {
    cst = (Creat)GetProcAddress(dllLoad, "CreateAst");
    startDownload = (Start)GetProcAddress(dllLoad, "StartSliceDownload");
    reg =
        (Register)GetProcAddress(dllLoad, "RegisterSliceDownloadSubscription");
    auto ast = cst();
    auto sDownloadID = startDownload(
        "http://download.cloud.189.cn/download/client/android/"
        "cloud189_v8.1.2_1564572280003.apk",
        "D:/1.apk", ast);
    auto fNext1 = [](const char *a) { printf("a is %s\n", a); };
    auto fComp1 = []() { finish_promise.set_value(); };
    auto finish_future = finish_promise.get_future();
    auto sRegID = reg(fNext1, fComp1, sDownloadID, ast);
    finish_future.get();
  } else {
    std::cout << "fail to load dll file.\n";
  }
  return 0;
}
