#include"electron-dll-proj.h"

extern "C" int64_t CreateAst()
{
	int64_t a = -1;
	return a;
}

extern "C" char * StartSliceDownload(char * sDownloadURL, char * sDownloadPath, int64_t nAst)
{
	return "";
}

extern "C" char * RegisterSliceDownloadSubscription(f4download::OnNext fNext, f4download::OnComplete fComplete, char * sDownloadID, int64_t nAst)
{
	return "";
}

extern "C" bool CancelSubscription(char * sRegisterID, int64_t nAst)
{
	bool bSucceed = false;
	return bSucceed;
}

extern "C" void StopDownload(char * sDownloadID, int64_t nAst)
{

}

extern "C" void DestroyAst(int64_t nAst)
{

}
