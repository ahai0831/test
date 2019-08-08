#include<memory>

namespace f4download
{
	typedef void(*OnNext)(const char*);
	typedef void(*OnComplete)();
}

//�½�Ast��������<���><void>
extern "C" int64_t CreateAst();

//������Ƭ�������̣�
//<��������uuid><�������ӣ�����·����Ast���>
extern "C" char * StartSliceDownload(char * sDownloadURL, char * sDownloadPath, int64_t nAst);

//ע�����ػص�
//<���ĵ�uuid><OnNext�ص���OnComplete�ص�����������uuid��Ast���>
extern "C" char * RegisterSliceDownloadSubscription(f4download::OnNext fNext, f4download::OnComplete fComplete, char * sDownloadID, int64_t nAst);

//ȡ���ص�����
//<����ȡ�����><����uuid��Ast���>
extern "C" bool CancelSubscription(char * sRegisterID, int64_t nAst);

//ֹͣ��������
//<void><����uuid��Ast���>
extern "C" void StopDownload(char * sDownloadID, int64_t nAst);

//����Ast���
//<void><Ast���>
extern "C" void DestroyAst(int64_t nAst);
