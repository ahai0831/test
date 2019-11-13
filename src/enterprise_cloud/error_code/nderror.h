#ifndef _NETDRIVE_ERROR_H_
#define _NETDRIVE_ERROR_H_

#include <string>
using std::string;
namespace EnterpriseCloud{
namespace ErrorCode{

enum
{
	////////////////////////////////本地错误码  以10000开始//////////////////////////////////////////
	//1X000-1X999为本地错误其他归类

	//-------10000-10999为本地错误非网络相关错误码------- start
	nderr_unknown = 10000,//-1 10000
	nderr_session_expired,//-2 10001		// 签名网络异常	
	nderr_id_not_found,//-4 10002		// 文件或文件夹不存在
	nderr_no_privilege,//-7 10003
	nderr_parent_id_not_found,//-17 10004		// 找不到父文件夹id

	nderr_state_not_match,//-998 10005
	nderr_file_is_modified,//-995 10006
	nderr_download_file_access_error,//-993 10007
	nderr_not_logged_in,//-991 10008
	nderr_response_data_exception,//-987 10009	// 响应数据异常
	nderr_no_more,//-984 10010
	nderr_unexpected,//-983 10011

	nderr_file_locked,//40000 10012	// 文件被占用
	nderr_no_diskspace,//40002 10013							// 本地磁盘空间不足	
	nderr_case_not_match,//40006 10014
	nderr_fail_to_move_file,//40008 10015					// 文件移动失败
	nderr_zero_size_file,//40009 10016
	nderr_not_initialize,//40010 10017
	nderr_path_not_exists,//40013 10018
	nderr_fail_to_del_path,//40014 10019
	nderr_not_support,//40020 10020
	nderr_transaction_not_began,//40028 10021
	nderr_operation_not_completed,//40029 10022

	nderr_exsit_same_id,//40038 10023              //数据库中存在同ID记录
	nderr_exsit_same_path,//40039 10024             //数据库中存在同path记录
	nderr_file_access_denied,//40041 10025				// 文件没有访问权限
	nderr_downLoadalikeoften,//40046 10026				//下载相同任务频繁	
	nderr_db_exception,//40048 10027                        // 数据库异常   
	nderr_ecloudfile_access_error,//40049 10028				//天翼企业云盘系统文件，禁止上传
	nderr_file_data_exception,    //40050 10029             //文件数据异常，导致上传失败（正在被使用的文件重新编辑，导致） 

	//-------10000-10999为本地错误非网络相关错误码------- end

	//-------11000-11999为本地错误网络相关错误码------- start
	nderr_transfer_data_error = 11000,//40031 11000
	//-------11000-11999为本地错误网络相关错误码------- end
	//////////////////////////////////////////////////////////////////////////

	///////////////////////////////////服务器错误码  以50000开始///////////////////////////////////////
	//5X000-5X999为服务器其他归类

	//-------50000-50999为服务器非网络错误码------- start
	nderr_invalidarg = 50000,  //-3   50000	 // 非法参数
	nderr_user_not_exists, //-8  50001	// 用户帐号不存在
	nderr_password_not_match, //- 9 -> 50002  // 密码不正确
	nderr_already_exists, //- 10 -> 50003		// 文件或文件夹已存在
	nderr_limit_exceeded, //- 11 - > 50004  // 单个文件大小超出限制
	nderr_not_enough_quota,//-13 50005		// 网盘空间不足
	nderr_parent_not_folder,//-16  50006	// 父文件夹类型不正确

	nderr_sessionbreak, //- 100 -> 50007	// 非法登录会话Key
	nderr_invalidsign,  //-101  50008				// 非法签名
	nderr_uploadfile_verify_failed,//-102  50009	// 上传文件校验失败
	nderr_uploadfile_not_found,//-103  50010		// 上传文件不存在
	nderr_uploadfile_save_failed,//-104  50011		// 上传文件保存至云存储失败
	nderr_invalid_parent_folder,//-105  50012		// 无效的父目录
	nderr_uploadfile_accessviolation,//-106  50013	// 上传文件访问冲突

	nderr_not_found,  //40018 -> 50014
	nderr_repeat_qos,//40022  50015		//当前线路已在提速中，不能重复提速
	nderr_notin_qos,//40023  50016  用户未在提速中，不能关闭
	nderr_cannot_qos,//40024  50017 //当前线路不具备提速条件
	nderr_platform_timeout_qos,//40025  50018  //归属地提速平台响应超时
	nderr_platform_failure_qos,//40026  50019  //归属地提速平台操作失败
	nderr_belongcodenotsupport,//40027  50020  //用户地区不支持智能提速
	nderr_errordownloadfilenotfound,//40032  50021
	nderr_errordownloadfiledeleted,//40033  50022
	nderr_errordownloadfileinvalidparam,//40034  50023
	nderr_errordownloadfileinvalidsessionkey,//40036  50024
	nderr_errordownloadfilesharetimeout,//40037  50025
	nderr_permission_denied,//40040  50026// 没有操作的权限
	nderr_groupspacenumberoverlimit,//40047 50027//群空间超过数目限制
	nderr_sharefile_overlimitnumber,//40050 50028//私密分享文件超出限制数目

	//www 2015.09.14 补充完善服务器错误码  //可查看云存储接口文档对应意义
	nderr_infosecurityerrorcode,//40051  50029  //401HTTP状态码
	nderr_infosecuMD5checkerror,//40052  50030
	nderr_accessdenyofhighfrequency,//40053  50031
	nderr_movefilevaliderror,//40054  50032
	nderr_corpuserbeinglocked,//40055  50033 -> 修改为企业用户被锁定错误码；
	nderr_groupspacepermitdenyerrorCode,//40056  50034
	nderr_sharespecialdirerror,//40057  50035
	nderr_specialdirshareerror,//40058  50036
	nderr_invaliduploadfilestatus,//40059  50037
	nderr_clientuploadthreadnumber, // 50038 
	nderr_corpusernotbelongcorp,//40050 50039//私密分享文件超出限制数目,//  50039//私密分享文件超出限制数目
	nderr_corpauthtimeexpire,	//50040   //企业空间已过期

	//--------50041-50052为文件权限相关的错误码

	nderr_corpauth_nouploadr, //50041 无上传权限
	nderr_corpauth_nocreatefolderr, // 50042 无创建文件夹权限
	nderr_corpauth_nodownloadr, // 50043 无下载权限
	nderr_corpauth_nosharer, // 50044 无分享权限
	nderr_corpauth_nosaver, // 50045 无转存权限
	nderr_corpauth_nomover, // 50046 无移动权限
	nderr_corpauth_nocopyr, // 50047 无复制权限
	nderr_corpauth_norenamer,// 50048 无重命名权限
	nderr_corpauth_nodelr, // 50049 无删除权限
	nderr_corpauth_noonliner, // 50050 无在线编辑权限
	nderr_corpauth_nosetrr, // 50051 无设置权限
	nderr_corpauth_noviewr, // 50052 无预览权限

	//--------50100-50199为新增的登录流程相关的错误码（预留一部分错误码的位置给权限精细化2.0）
	nderr_userinvalidopentoken = 50100, // 50100 综合平台token失效

	//--------50200-50299为新增的工作空间相关的错误码
	nderr_corpfilenotinwork = 50200, // 50200 文件不在工作空间
	nderr_corpworkspacenotenoughspace, // 50201 工作空间不足

	//-------50000-50999为服务器非网络错误码------- end

	//-------51000-51999为服务器网络相关错误码------- start
	nderr_internal_error = 51000,//-989 51000		// 内部错误
	nderr_errordownloadfileinternalerror,//40035 51001
	//-------51000-51999为服务器网络相关错误码------- end

	//////////////////////////////////////////////////////////////////////////

	////////////////////////////////Curl库错误码  以60000开始//////////////////////////////////////////
	//6X000-6X999为Curl库其他归类

	//-------60000-60999为Curl库非网络相关错误码------- start
	nderr_usercanceled = 60000,//-1000  60000	
	nderr_file_access_error,//-994  60001		// 文件访问失败
	nderr_no_memory,//40001  60002
	nderr_cant_take_operation,//40007  60003
	nderr_corrupted,//40019  60004		
	nderr_transfer_error,//40060  60005  //文件传输，短于或大于预期
	nderr_url_error,//40061  60006//被拒绝访问的资源的URL
	nderr_download_resume_error,//40062  60007//下载无法恢复，因为指定的偏移量为文件的边界
	nderr_over_filesize_error,//40063  60008//最大文件大小超过
	nderr_rewind_error,	//40064  60009
	

	//nderr_invalidarg,// 与服务器重复
	//-------60000-60999为Curl库非网络相关错误码------- end

	//-------61000-61999为Curl库网络相关错误码------- start
	nderr_timeout = 61000,//-985 61000
	nderr_bad_request,//-988 61001
	nderr_fail_to_connect,//-996 61002// 连接服务器失败
	nderr_http_response_error,//40011 61003
	nderr_network_io_failed,//40012 61004
	nderr_socket_waiting_error,//40065 61005//Socket是没有准备好发送/接收等待，直到它准备好了，然后再试一次
	nderr_connection_error,//40066 61006//No connection available, the session will be queued
	//-------61000-61999为Curl库网络相关错误码------- end

	//////////////////////////////////////////////////////////////////////////

	////////////////////////////////特殊暂未使用错误码  保留原样//////////////////////////////////////////	
	//2X000-2X999为未使用错误码其他归类

	//-------20000-20999为特殊暂未使用错误码------- start
	nderr_target_id_not_found = 20000,	// not used
	nderr_id_type_error,			// not used
	nderr_expired,					// not used
	nderr_account_freezed,			// 用户帐号已冻结 not used
	nderr_user_arrears,				// 用户欠费		not used
	nderr_request_not_found,		// not used
	nderr_can_not_cancel,			// not used	
	nderr_need_refresh,				// not used
	nderr_connection_break,			// not used
	nderr_server_exception,			// not used
	nderr_fail_to_create_tmpfile,	// 创建临时文件失败 not used
	nderr_file_is_used,				// 文件在使用中 not used
	nderr_localfolder_not_exists,	// 本地文件夹不存在
	nderr_localfolder_err,			// 部分子文件传输错误 not used
	nderr_transferfolder_delete,	// 传输文件夹任务已经删除 not used
	nderr_downloadpicture_err,		// 下载图片数据有错误 not used
	//-------20000-20999为特殊暂未使用错误码------- end

// 留作以后参照用
// 	nderr_target_id_not_found = -5,			// not used
// 	nderr_id_type_error = -6,				// not used
// 	nderr_expired = -12,					// not used
// 	nderr_account_freezed = -14,			// 用户帐号已冻结 not used
// 	nderr_user_arrears = -15,				// 用户欠费		not used
// 	nderr_request_not_found = -999,			// not used
// 	nderr_can_not_cancel = -997,			// not used	
// 	nderr_need_refresh = -992,				// not used
// 	nderr_connection_break = 40003,			// not used
// 	nderr_server_exception = 40004,			// not used
// 	nderr_fail_to_create_tmpfile = 40005,	// 创建临时文件失败 not used
// 	nderr_file_is_used = 40030,				// 文件在使用中 not used
// 	nderr_localfolder_not_exists = 40042,	// 本地文件夹不存在 not used
// 	nderr_localfolder_err = 40043,			// 部分子文件传输错误 not used
// 	nderr_transferfolder_delete = 40044,	// 传输文件夹任务已经删除 not used
// 	nderr_downloadpicture_err = 40045,		// 下载图片数据有错误 not used
	//////////////////////////////////////////////////////////////////////////

	////////////////////////////////虚拟磁盘独占错误码  保留原样//////////////////////////////////////////	
	nderr_already_done = 100000,		// 挂载设备相关	
	nderr_already_initialized,			// part used int VDiskManager or not used becareful
	nderr_fail_to_load_driver,			// 加载驱动程序失败
	nderr_fail_to_mount_device,			// 挂载设备失败
	nderr_fail_to_unmount_device,		// 卸载设备失败
	nderr_no_available_drive,

// 留作以后参照用
// 	nderr_already_done = -986,    // 挂载设备相关	
// 	nderr_already_initialized = -990,		// part used int VDiskManager or not used becareful
// 	nderr_fail_to_load_driver = 40015,					// 加载驱动程序失败
// 	nderr_fail_to_mount_device = 40016,					// 挂载设备失败
// 	nderr_fail_to_unmount_device = 40017,				// 卸载设备失败
// 	nderr_no_available_drive = 40021,
	//////////////////////////////////////////////////////////////////////////
};

int int32ErrCode(const char * sErr);

string strErrCode(int nErr);

}
}

#endif
