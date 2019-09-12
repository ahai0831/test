#include <memory>

namespace f4download {
typedef void (*OnNext)(const char *);
typedef void (*OnComplete)();
typedef void (*OnCheck)(const char *);
}  // namespace f4download

typedef void (*OnFinal)(const char *);

//�½�Ast��������<���><void>
extern "C" int64_t CreateAst(const char *sSessionInfo);

//��ʼ�ļ����أ�
//<���ĵ�uuid><�����ļ���Ϣ�� ast���>
extern "C" char *DoDownload(const char *sFileInfo, f4download::OnNext fNext,
                            f4download::OnComplete fComplete, int64_t nAst);

//������Ƭ�������̣�
//<��������uuid><�������ӣ�����·����Ast���>
extern "C" char *StartSliceDownload(char *sDownloadURL, char *sDownloadPath,
                                    int64_t nAst);

//ע�����ػص�
//<���ĵ�uuid><OnNext�ص���OnComplete�ص�����������uuid��Ast���>
extern "C" char *RegisterSliceDownloadSubscription(
    f4download::OnNext fNext, f4download::OnComplete fComplete,
    char *sDownloadID, int64_t nAst);

//ȡ���ص�����
//<����ȡ�����><����uuid��Ast���>
extern "C" bool CancelSubscription(char *sRegisterID, int64_t nAst);

//ֹͣ��������
//<void><����uuid��Ast���>
extern "C" void StopDownload(char *sDownloadID, int64_t nAst);

//��ȡ�����ļ����״̬��
//
extern "C" void GetFinalResult(OnFinal fResult, const char *sDownloadID,
                               int64_t nAst);

//У�������ļ���
//
extern "C" void CheckDLFile(const char *sFileInfo,
                            f4download::OnCheck fOnCheck);

//����Ast���
//<void><Ast���>
extern "C" void DestroyAst(int64_t nAst);
