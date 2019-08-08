#include<memory>

namespace f4download
{
	typedef void(*OnNext)(const char*);
	typedef void(*OnComplete)();
}

//新建Ast对象句柄；<句柄><void>
extern "C" int64_t CreateAst();

//启动分片下载流程；
//<下载流程uuid><下载链接，下载路径，Ast句柄>
extern "C" char * StartSliceDownload(char * sDownloadURL, char * sDownloadPath, int64_t nAst);

//注册下载回调
//<订阅的uuid><OnNext回调，OnComplete回调，下载流程uuid，Ast句柄>
extern "C" char * RegisterSliceDownloadSubscription(f4download::OnNext fNext, f4download::OnComplete fComplete, char * sDownloadID, int64_t nAst);

//取消回调订阅
//<返回取消结果><订阅uuid，Ast句柄>
extern "C" bool CancelSubscription(char * sRegisterID, int64_t nAst);

//停止下载流程
//<void><下载uuid，Ast句柄>
extern "C" void StopDownload(char * sDownloadID, int64_t nAst);

//销毁Ast句柄
//<void><Ast句柄>
extern "C" void DestroyAst(int64_t nAst);
