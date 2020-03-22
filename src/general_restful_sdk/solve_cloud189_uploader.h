#ifndef GENERAL_RESTFUL_SDK_AST_SOLVE_CLOUD189_UPLOADER
#define GENERAL_RESTFUL_SDK_AST_SOLVE_CLOUD189_UPLOADER

#include <cinttypes>
#include <string>

namespace general_restful_sdk_ast {

namespace Cloud189 {
/// 返回0，代表创建任务成功，on_callback将得到至少一次调用
/// 返回非0，代表创建任务失败
/// 返回-1，代表总控未初始化，说明传入的信息不完全合法
/// 返回1，代表总控初始化过程中，初始化失败，
/// 说明相关业务字段不完全合法（如待上传的文件不存在或无法访问）
/// 在返回中增加一个输出字段，用于表达创建成功的UUID。
int32_t CreateUpload(const std::string& upload_info,
                     void (*on_callback)(const char*),
                     std::string& success_uuid);

/// 传入CreateUpload返回的uuid；如果传入的uuid无效，什么也不会发生
void StartUpload(const std::string& cancel_uuid);

/// 返回0，代表传入的uuid合法，因此成功地发出了取消请求
/// 但此时不代表已经立即停止，仅仅是请求发出。
/// 返回-1，代表传入的uuid不合法或不存在
int32_t UserCancelUpload(const std::string& cancel_uuid);

}  // namespace Cloud189

}  // namespace general_restful_sdk_ast
#endif