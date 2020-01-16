#ifdef WIN32
#ifdef GENERAL_RESTFUL_SDK_EXPORTS
#define AST_API __declspec(dllexport)
#else
#define AST_API __declspec(dllimport)
#endif

#elif __APPLE__
#ifdef GENERAL_RESTFUL_SDK_EXPORTS
#define AST_API __attribute__ ((visibility("default")))
#else
#define AST_API __attribute__ ((visibility("hidden")))
#endif

#else
#define AST_API
#endif  // WIN32

typedef void (*OnProcessStart)(const char *);

typedef void (*OnProcessCallback)(const char *);

typedef void (*OnConfigFinished)(const char *);

#ifdef __cplusplus
extern "C" {
#endif
/// Provide a general restful SDK for Cloud189, FamilyCloud, EnterpriseCloud and
/// so on.

/// The fields "process_info" (json string) must contain are belowed:
/// "domain": "Cloud189", "FamilyCloud", "EnterpriseCloud"
/// operation: "DoUpload", "UserCancelUpload", "DoDownload",
/// "UserCancelDownload"
/// uuid: if empty, it would be generated.

/// on_start is a C funtion that would be called synchronously once when the
/// process start.

/// on_callback is a C funtion that would be called asynchronously per 1000
/// milliseconds; and it would be called once when the process get completed.

AST_API void AstProcess(const char *process_info, OnProcessStart on_start,
                        OnProcessCallback on_callback);

AST_API void AstConfig(const char *json_str,
                       OnConfigFinished on_config_finished);
/// Note: A general info block per progress would be create when the
/// "AstProcess" called the first time automatically.

/// Note: All json string shouldn't be keeped outside the callback while its
/// memory would be recycled.

/// Note: To avoid garble, the header file would not use chinese.
#ifdef __cplusplus
}
#endif
