#ifndef GENERAL_RESTFUL_SDK_AST_SOLVE_CLOUD189_DOWNLOADER
#define GENERAL_RESTFUL_SDK_AST_SOLVE_CLOUD189_DOWNLOADER

#include <cinttypes>
#include <string>

namespace general_restful_sdk_ast {

namespace Cloud189 {
/// 返回0，代表创建任务成功，on_callback将得到至少一次调用
/// 返回非0，代表创建任务失败
/// 返回-1，代表总控未初始化，说明传入的信息不完全合法
int32_t DoDownload(const std::string& download_info,
                   void (*on_callback)(const char*));
}  // namespace Cloud189

}  // namespace general_restful_sdk_ast
#endif
