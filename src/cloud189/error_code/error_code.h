#ifndef _ERROR_CODE
#define _ERROR_CODE

#include <string>
using std::string;
namespace Cloud189 {
namespace ErrorCode {

enum nderr {
  ////////////////////////////////���ش�����
  ///��10000��ʼ//////////////////////////////////////////
  // 1X000-1X999Ϊ���ش�����������

  //-------10000-10999Ϊ���ش����������ش�����------- start
  nderr_unknown = 10000,      //-1 10000
  nderr_session_expired,      //-2 10001		// ǩ�������쳣
  nderr_id_not_found,         //-4 10002		// �ļ����ļ��в�����
  nderr_no_privilege,         //-7 10003
  nderr_parent_id_not_found,  //-17 10004		// �Ҳ������ļ���id

  nderr_state_not_match,             //-998 10005
  nderr_file_is_modified,            //-995 10006
  nderr_download_file_access_error,  //-993 10007
  nderr_not_logged_in,               //-991 10008
  nderr_response_data_exception,     //-987 10009	// ��Ӧ�����쳣
  nderr_no_more,                     //-984 10010
  nderr_unexpected,                  //-983 10011

  nderr_file_locked,              // 40000 10012	// �ļ���ռ��
  nderr_no_diskspace,             // 40002 10013
                                  // // ���ش��̿ռ䲻��
  nderr_case_not_match,           // 40006 10014
  nderr_fail_to_move_file,        // 40008 10015 // �ļ��ƶ�ʧ��
  nderr_zero_size_file,           // 40009 10016
  nderr_not_initialize,           // 40010 10017
  nderr_path_not_exists,          // 40013 10018
  nderr_fail_to_del_path,         // 40014 10019
  nderr_not_support,              // 40020 10020
  nderr_transaction_not_began,    // 40028 10021
  nderr_operation_not_completed,  // 40029 10022

  nderr_exsit_same_id,  // 40038 10023              //���ݿ��д���ͬID��¼
  nderr_exsit_same_path,  // 40039 10024             //���ݿ��д���ͬpath��¼
  nderr_file_access_denied,  // 40041 10025				//
                             // �ļ�û�з���Ȩ��
  nderr_downLoadalikeoften,  // 40046 10026
                             // //������ͬ����Ƶ��
  nderr_db_exception,  // 40048 10027                        // ���ݿ��쳣
  nderr_ecloudfile_access_error,  // 40049 10028
                                  // //��������ϵͳ�ļ�����ֹ�ϴ�
  nderr_file_data_exception,      // 40050 10029
                              // //�ļ������쳣�������ϴ�ʧ�ܣ����ڱ�ʹ�õ��ļ����±༭�����£�
                              //-------10000-10999Ϊ���ش����������ش�����-------
                              // end

  //-------11000-11999Ϊ���ش���������ش�����------- start
  nderr_transfer_data_error =
      11000,  // 40031 11000
              //-------11000-11999Ϊ���ش���������ش�����------- end
              //////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////������������
  ///��50000��ʼ///////////////////////////////////////
  // 5X000-5X999Ϊ��������������

  //-------50000-50999Ϊ�����������������------- start
  nderr_invalidarg = 50000,  //-3   50000	 // �Ƿ�����
  nderr_user_not_exists,     //-8  50001	// �û��˺Ų�����
  nderr_password_not_match,  //- 9 -> 50002  // ���벻��ȷ
  nderr_already_exists,      //- 10 -> 50003		// �ļ����ļ����Ѵ���
  nderr_limit_exceeded,      //- 11 - > 50004  // �����ļ���С��������
  nderr_not_enough_quota,    //-13 50005		// ���̿ռ䲻��
  nderr_parent_not_folder,   //-16  50006	// ���ļ������Ͳ���ȷ

  nderr_sessionbreak,                //- 100 -> 50007	// �Ƿ���¼�ỰKey
  nderr_invalidsign,                 //-101  50008				// �Ƿ�ǩ��
  nderr_uploadfile_verify_failed,    //-102  50009	// �ϴ��ļ�У��ʧ��
  nderr_uploadfile_not_found,        //-103  50010		// �ϴ��ļ�������
  nderr_uploadfile_save_failed,      //-104  50011		//
                                     //�ϴ��ļ��������ƴ洢ʧ��
  nderr_invalid_parent_folder,       //-105  50012		// ��Ч�ĸ�Ŀ¼
  nderr_uploadfile_accessviolation,  //-106  50013	// �ϴ��ļ����ʳ�ͻ

  nderr_not_found,   // 40018 -> 50014
  nderr_repeat_qos,  // 40022  50015
                     // //��ǰ��·���������У������ظ�����
  nderr_notin_qos,   // 40023  50016  �û�δ�������У����ܹر�
  nderr_cannot_qos,  // 40024  50017 //��ǰ��·���߱���������
  nderr_platform_timeout_qos,  // 40025  50018  //����������ƽ̨��Ӧ��ʱ
  nderr_platform_failure_qos,  // 40026  50019  //����������ƽ̨����ʧ��
  nderr_belongcodenotsupport,  // 40027  50020  //�û�������֧����������
  nderr_errordownloadfilenotfound,           // 40032  50021
  nderr_errordownloadfiledeleted,            // 40033  50022
  nderr_errordownloadfileinvalidparam,       // 40034  50023
  nderr_errordownloadfileinvalidsessionkey,  // 40036  50024
  nderr_errordownloadfilesharetimeout,       // 40037  50025
  nderr_permission_denied,          // 40040  50026// û�в�����Ȩ��
  nderr_groupspacenumberoverlimit,  // 40047 50027//Ⱥ�ռ䳬����Ŀ����
  nderr_sharefile_overlimitnumber,  // 40050 50028//˽�ܷ����ļ�����������Ŀ

  // www 2015.09.14 �������Ʒ�����������  //�ɲ鿴�ƴ洢�ӿ��ĵ���Ӧ����
  nderr_infosecurityerrorcode,          // 40051  50029  //401HTTP״̬��
  nderr_infosecuMD5checkerror,          // 40052  50030
  nderr_accessdenyofhighfrequency,      // 40053  50031
  nderr_movefilevaliderror,             // 40054  50032
  nderr_groupspacememberoverlimit,      // 40055  50033
  nderr_groupspacepermitdenyerrorCode,  // 40056  50034
  nderr_sharespecialdirerror,           // 40057  50035
  nderr_specialdirshareerror,           // 40058  50036
  nderr_invaliduploadfilestatus,        // 40059  50037
  nderr_clientuploadthreadnumber,       // 50038
  nderr_userdayflowoverlimited,         // �û������ϴ���������
  nderr_qrcodeinvalid,                  // ��ά�������Чʱ��
  nderr_qrcodeloginfailed,

  // 2017.9.26 ����Ƿ�Token�Ĵ�����
  nderr_userinvalidopentoken,  // 50042 // �Ƿ�AccessToken
  nderr_refreshtokenfailed,    // 50043 // ˢ��tokenʧ��

  // 2017.11.21 �������RedirectURL��¼�ӿڵĴ�����
  nderr_analysisredirecturlabnormal,
  nderr_sessiontimeout,
  nderr_parseloginresponseerror,
  nderr_loginrespisnull,
  nderr_getuserinfoforpcerror,
  nderr_userinfoisnull,
  nderr_usernameisnull,
  nderr_usersessionboisnull,
  nderr_loginfailinbusinessexception,

  // 2017.11.21 �����Ż���¼�ӿڵ����еĴ�����
  nderr_msginternalerror,

  // 2017.11.21 �����Ż���ˢ��Token�ӿڵĴ�����
  nderr_resultofrefreshtokenisnull,
  nderr_resultofrefreshtokenerror,

  // 218.12.25 ��ͥ����ش�����
  nderr_family_info_not_exist,
  nderr_family_info_exist,
  nderr_family_kicked_out,
  nderr_family_kicked_out_duringdowmload,
  nderr_family_operation_failed,
  nderr_family_BindInfoNotFound,

  // 2019.8.14 �����������ţ�accessToken��sessionKey�ӿڣ���ش�����
  /// ���������Խ��������������ź����ܵ�¼ʹ��
  nderr_oauth2_invalidaccesstoken,
  nderr_auditerrorcode,// 50063 ���дʼ�鲻ͨ��
  //-------50000-50999Ϊ�����������������------- end

  //-------51000-51999Ϊ������������ش�����------- start
  nderr_internal_error = 51000,          //-989 51000		// �ڲ�����
  nderr_errordownloadfileinternalerror,  // 40035 51001
                                         //-------51000-51999Ϊ������������ش�����-------
                                         // end

  //////////////////////////////////////////////////////////////////////////

  ////////////////////////////////Curl�������
  ///��60000��ʼ//////////////////////////////////////////
  // 6X000-6X999ΪCurl����������

  //-------60000-60999ΪCurl���������ش�����------- start
  nderr_usercanceled = 60000,  //-1000  60000
  nderr_file_access_error,     //-994  60001		// �ļ�����ʧ��
  nderr_no_memory,             // 40001  60002
  nderr_cant_take_operation,   // 40007  60003
  nderr_corrupted,             // 40019  60004
  nderr_transfer_error,  // 40060  60005  //�ļ����䣬���ڻ����Ԥ��
  nderr_url_error,       // 40061  60006//���ܾ����ʵ���Դ��URL
  nderr_download_resume_error,  // 40062
                                // 60007//�����޷��ָ�����Ϊָ����ƫ����Ϊ�ļ��ı߽�
  nderr_over_filesize_error,  // 40063  60008//����ļ���С����
  nderr_rewind_error,         // 40064  60009

  // nderr_invalidarg,// ��������ظ�
  //-------60000-60999ΪCurl���������ش�����------- end

  //-------61000-61999ΪCurl��������ش�����------- start
  nderr_timeout = 61000,       //-985 61000
  nderr_bad_request,           //-988 61001
  nderr_fail_to_connect,       //-996 61002// ���ӷ�����ʧ��
  nderr_http_response_error,   // 40011 61003
  nderr_network_io_failed,     // 40012 61004
  nderr_socket_waiting_error,  // 40065
                               // 61005//Socket��û��׼���÷���/���յȴ���ֱ����׼�����ˣ�Ȼ������һ��
  nderr_connection_error,  // 40066 61006//No connection available, the session
                           // will be queued
  nderr_content_type_error,  // 40066 61007
                             //-------61000-61999ΪCurl��������ش�����-------
                             //end

  //////////////////////////////////////////////////////////////////////////

  ////////////////////////////////������δʹ�ô�����
  ///����ԭ��//////////////////////////////////////////
  // 2X000-2X999Ϊδʹ�ô�������������

  //-------20000-20999Ϊ������δʹ�ô�����------- start
  nderr_target_id_not_found = 20000,  // not used
  nderr_id_type_error,                // not used
  nderr_expired,                      // not used
  nderr_account_freezed,              // �û��˺��Ѷ��� not used
  nderr_user_arrears,                 // �û�Ƿ��		not used
  nderr_request_not_found,            // not used
  nderr_can_not_cancel,               // not used
  nderr_need_refresh,                 // not used
  nderr_connection_break,             // not used
  nderr_server_exception,             // not used
  nderr_fail_to_create_tmpfile,       // ������ʱ�ļ�ʧ�� not used
  nderr_file_is_used,                 // �ļ���ʹ���� not used
  nderr_localfolder_not_exists,       // �����ļ��в�����
  nderr_localfolder_err,              // �������ļ�������� not used
  nderr_transferfolder_delete,  // �����ļ��������Ѿ�ɾ�� not used
  nderr_downloadpicture_err,    // ����ͼƬ�����д��� not used
                              //-------20000-20999Ϊ������δʹ�ô�����-------
                              // end

  // �����Ժ������
  // 	nderr_target_id_not_found = -5,			// not used
  // 	nderr_id_type_error = -6,				// not used
  // 	nderr_expired = -12,					// not used
  // 	nderr_account_freezed = -14,			// �û��˺��Ѷ��� not
  // used
  // 	nderr_user_arrears = -15,				// �û�Ƿ��
  // not
  // used
  // 	nderr_request_not_found = -999,			// not used
  // 	nderr_can_not_cancel = -997,			// not used
  // 	nderr_need_refresh = -992,				// not used
  // 	nderr_connection_break = 40003,			// not used
  // 	nderr_server_exception = 40004,			// not used
  // 	nderr_fail_to_create_tmpfile = 40005,	// ������ʱ�ļ�ʧ�� not used
  // 	nderr_file_is_used = 40030,				// �ļ���ʹ����
  // not
  // used
  // 	nderr_localfolder_not_exists = 40042,	// �����ļ��в����� not used
  // 	nderr_localfolder_err = 40043,			// �������ļ��������
  // not
  // used
  // 	nderr_transferfolder_delete = 40044,	// �����ļ��������Ѿ�ɾ�� not
  // used
  // 	nderr_downloadpicture_err = 40045,		// ����ͼƬ�����д���
  // not
  // used
  //////////////////////////////////////////////////////////////////////////

  ////////////////////////////////������̶�ռ������
  ///����ԭ��//////////////////////////////////////////
  nderr_already_done = 100000,   // �����豸���
  nderr_already_initialized,     // part used int VDiskManager or not used
                                 // becareful
  nderr_fail_to_load_driver,     // ������������ʧ��
  nderr_fail_to_mount_device,    // �����豸ʧ��
  nderr_fail_to_unmount_device,  // ж���豸ʧ��
  nderr_no_available_drive,

  // �����Ժ������
  // 	nderr_already_done = -986,    // �����豸���
  // 	nderr_already_initialized = -990,		// part used int
  // VDiskManager
  // or
  // not used becareful
  // 	nderr_fail_to_load_driver = 40015, //
  // ������������ʧ��
  // 	nderr_fail_to_mount_device = 40016, //
  // �����豸ʧ��
  // 	nderr_fail_to_unmount_device = 40017,				//
  // ж���豸ʧ��
  // 	nderr_no_available_drive = 40021,
  //////////////////////////////////////////////////////////////////////////
};

int int32ErrCode(const char* sErr);
string strErrCode(int nErr);
}  // namespace ErrorCode
}  // namespace Cloud189
#endif