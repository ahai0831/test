#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

#include <cstdio>


/// defines some symbol for dll dynamic loading.
namespace {
typedef void (*OnProcessStart)(const char *);
typedef void (*OnProcessCallback)(const char *);
typedef void (*OnConfigFinished)(const char *);

typedef void (*pAstConfig)(const char *, OnConfigFinished);
typedef void (*pAstProcess)(const char *, OnProcessStart, OnProcessCallback);

pAstConfig ast_config__ = nullptr;
pAstProcess ast_process__ = nullptr;

#ifdef _WIN32
const char *dll_name = "general_restful_sdk";
#else
const char *dll_name = "libgeneral_restful_sdk.dylib";
#endif

/// dynamic loading dll or dylib here
static struct DynamicLoading {
#ifdef _WIN32
  HINSTANCE instance = nullptr;
#else
  void *instance = nullptr;
#endif
  DynamicLoading() {
#ifdef _WIN32
    instance = LoadLibraryA(dll_name);
    ast_config__ = (pAstConfig)GetProcAddress(instance, "AstConfig");
    ast_process__ = (pAstProcess)GetProcAddress(instance, "AstProcess");
#else
    char *error_info = nullptr;
    instance = dlopen(dll_name, RTLD_NOW);
    error_info = dlerror();
    if (nullptr != error_info) {
      fprintf(stderr, "Error when loading pAstConfig: %s\n", error_info);
      error_info = nullptr;
    }
    ast_config__ = (pAstConfig)dlsym(instance, "AstConfig");
    error_info = dlerror();
    if (nullptr != error_info) {
      fprintf(stderr, "Error when loading pAstConfig: %s\n", error_info);
      error_info = nullptr;
    }
    ast_process__ = (pAstProcess)dlsym(instance, "AstProcess");
    error_info = dlerror();
    if (nullptr != error_info) {
      fprintf(stderr, "Error when loading pAstProcess: %s\n", error_info);
      error_info = nullptr;
    }
#endif
  }
  ~DynamicLoading() {
    ast_process__ = nullptr;
    ast_config__ = nullptr;
#ifdef _WIN32
    FreeLibrary(instance);
#else
    dlclose(instance);
#endif
  }
} _dynamic_loading;
}  // namespace

void AstConfig(const char *info, void(*on_config)(const char *))
{
	if (nullptr == ast_config__)
	{
		printf("Error when loading `AstConfig` from %s\n", dll_name);
		return;
	}
	return ast_config__(info, on_config);
}

void AstProcess(const char *info, void(*on_start)(const char *), void(*on_callback)(const char *))
{
	if (nullptr == ast_process__)
	{
		printf("Error when loading `AstProcess` from %s\n", dll_name);
		return;
	}
	return ast_process__(info, on_start,on_callback);
}
