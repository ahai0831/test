#ifndef GENERAL_RESTFUL_SDK_AST_SOLVE_CLOUD189_FOLDER_UPLOADER
#define GENERAL_RESTFUL_SDK_AST_SOLVE_CLOUD189_FOLDER_UPLOADER

#include <cinttypes>
#include <string>

namespace general_restful_sdk_ast {

namespace Cloud189 {
/// 返回0，代表创建任务成功，on_callback将得到至少一次调用
/// 返回非0，代表创建任务失败
///
/// 返回1，代表总控初始化过程中，初始化失败，
/// 说明相关业务字段不完全合法（如待上传的文件不存在或无法访问）
int32_t DoFolderUpload(const std::string& folder_info,
                       void (*on_callback)(const char*));
}  // namespace Cloud189

}  // namespace general_restful_sdk_ast
#endif