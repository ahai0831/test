#include "nderror.h"

namespace {
#define EC_OK "OK"
#define FILEALREADYEXISTS "FileAlreadyExists"
#define FILENOTFOUND "FileNotFound"
#define FILETOOLARGE "FileTooLarge"
#define INTERNALERROR "InternalError"
#define INVALIDARGUMENT "InvalidArgument"
#define INVALIDPASSWORD "InvalidPassword"
#define INVALIDSESSIONKEY "InvalidSessionKey"
#define INVALIDSIGNATURE "InvalidSignature"
#define NOSUCHUSER "NoSuchUser"
#define PARENTNOTFOLDER "ParentNotFolder"
#define PERMISSIONDENIED "PermissionDenied"
#define CONTENTTYPEERROR "ContentTypeError"
}  // namespace

namespace Cloud189 {
namespace ErrorCode {

int int32ErrCode(const char* sErr) {
  int iErr = nderr_unknown;
  if (strcmp(sErr, FILEALREADYEXISTS) == 0) {
    iErr = nderr_already_exists;
  } else if (strcmp(sErr, FILENOTFOUND) == 0) {
    iErr = nderr_not_found;
  } else if (strcmp(sErr, FILETOOLARGE) == 0) {
    iErr = nderr_limit_exceeded;
  } else if (strcmp(sErr, "UploadSingleFileOverLimited") == 0) {
    iErr = nderr_limit_exceeded;
  } else if (strcmp(sErr, INTERNALERROR) == 0) {
    iErr = nderr_internal_error;
  } else if (strcmp(sErr, INVALIDARGUMENT) == 0) {
    iErr = nderr_invalidarg;
  } else if (strcmp(sErr, INVALIDPASSWORD) == 0) {
    iErr = nderr_password_not_match;
  } else if (strcmp(sErr, INVALIDSESSIONKEY) == 0) {
    iErr = nderr_sessionbreak;
  } else if (strcmp(sErr, INVALIDSIGNATURE) == 0) {
    iErr = nderr_invalidsign;
  } else if (strcmp(sErr, NOSUCHUSER) == 0) {
    iErr = nderr_user_not_exists;
  } else if (strcmp(sErr, PARENTNOTFOLDER) == 0) {
    iErr = nderr_parent_not_folder;
  } else if (strcmp(sErr, "InsufficientStorageSpace") == 0) {
    iErr = nderr_not_enough_quota;
  } else if (strcmp(sErr, "UploadFileVerifyFailed") == 0) {
    iErr = nderr_uploadfile_verify_failed;
  } else if (strcmp(sErr, "UploadFileNotFound") == 0) {
    iErr = nderr_uploadfile_not_found;
  } else if (strcmp(sErr, "UploadFileSaveFailed") == 0) {
    iErr = nderr_uploadfile_save_failed;
  } else if (strcmp(sErr, "UploadFileAccessViolation") == 0) {
    iErr = nderr_uploadfile_accessviolation;
  } else if (strcmp(sErr, "ClientUploadThreadNumber") == 0) {
    iErr = nderr_clientuploadthreadnumber;
  } else if (strcmp(sErr, "InvalidParentFolder") == 0) {
    iErr = nderr_invalid_parent_folder;
  } else if (strcmp(sErr, "repeatQos") == 0) {
    iErr = nderr_repeat_qos;
  } else if (strcmp(sErr, "notInQos") == 0) {
    iErr = nderr_notin_qos;
  } else if (strcmp(sErr, "canNotQos") == 0) {
    iErr = nderr_cannot_qos;
  } else if (strcmp(sErr, "platformTimeout") == 0) {
    iErr = nderr_platform_timeout_qos;
  } else if (strcmp(sErr, "platformFailure") == 0) {
    iErr = nderr_platform_failure_qos;
  } else if (strcmp(sErr, "belongCodeNotSupport") == 0) {
    iErr = nderr_belongcodenotsupport;
  } else if (strcmp(sErr, "ErrorDownloadFileNotFound") == 0) {
    iErr = nderr_errordownloadfilenotfound;
  } else if (strcmp(sErr, "ErrorDownloadFileDeleted") == 0) {
    iErr = nderr_errordownloadfiledeleted;
  } else if (strcmp(sErr, "ErrorDownloadFileInvalidParam") == 0) {
    iErr = nderr_errordownloadfileinvalidparam;
  } else if (strcmp(sErr, "ErrorDownloadFileInternalError") == 0) {
    iErr = nderr_errordownloadfileinternalerror;
  } else if (strcmp(sErr, "ErrorDownloadFileInvalidSessionKey") == 0) {
    iErr = nderr_errordownloadfileinvalidsessionkey;
  } else if (strcmp(sErr, "ErrorDownloadFileShareTimeOut") == 0) {
    iErr = nderr_errordownloadfilesharetimeout;
  } else if (strcmp(sErr, PERMISSIONDENIED) == 0) {
    iErr = nderr_permission_denied;
  } else if (strcmp(sErr, "GroupSpaceNumberOverLimit") == 0) {
    iErr = nderr_groupspacenumberoverlimit;
  } else if (strcmp(sErr, "ShareOverLimitedNumber") == 0) {
    iErr = nderr_sharefile_overlimitnumber;
  } else if (strcmp(sErr, "AccessDenyOfHighFrequency") == 0) {
    iErr = nderr_accessdenyofhighfrequency;
  } else if (strcmp(sErr, "MoveFileValidError") == 0) {
    iErr = nderr_movefilevaliderror;
  } else if (strcmp(sErr, "GroupSpaceMemberOverLimit") == 0) {
    iErr = nderr_groupspacememberoverlimit;
  } else if (strcmp(sErr, "GroupSpacePermitDenyErrorCode") == 0) {
    iErr = nderr_groupspacepermitdenyerrorCode;
  } else if (strcmp(sErr, "ShareSpecialDirError") == 0) {
    iErr = nderr_sharespecialdirerror;
  } else if (strcmp(sErr, "SpecialDirShareError") == 0) {
    iErr = nderr_specialdirshareerror;
  } else if (strcmp(sErr, "InvalidUploadFileStatus") == 0) {
    iErr = nderr_invaliduploadfilestatus;
  } else if (strcmp(sErr, "InfoSecurityErrorCode") == 0) {
    iErr = nderr_infosecurityerrorcode;
  } else if (strcmp(sErr, "InfoSecurityCheck") == 0) {
    iErr = nderr_infosecurityerrorcode;
  } else if (strcmp(sErr, "InfosecuMD5CheckError") == 0) {
    iErr = nderr_infosecuMD5checkerror;
  } else if (strcmp(sErr, "UserDayFlowOverLimited") == 0) {
    iErr = nderr_userdayflowoverlimited;
  } else if (strcmp(sErr, "UserInvalidOpenToken") == 0) {
    iErr = nderr_userinvalidopentoken;
  } else if (strcmp(sErr, "AnalysisRedirectURLAbnormal") == 0) {
    iErr = nderr_analysisredirecturlabnormal;
  } else if (strcmp(sErr, "SessionTimeout") == 0) {
    iErr = nderr_sessiontimeout;
  } else if (strcmp(sErr, "ParseLoginResponseError") == 0) {
    iErr = nderr_parseloginresponseerror;
  } else if (strcmp(sErr, "LoginRespIsNull") == 0) {
    iErr = nderr_loginrespisnull;
  } else if (strcmp(sErr, "GetUserInfoForPCError") == 0) {
    iErr = nderr_getuserinfoforpcerror;
  } else if (strcmp(sErr, "UserInfoIsNull") == 0) {
    iErr = nderr_userinfoisnull;
  } else if (strcmp(sErr, "UserNameIsNull") == 0) {
    iErr = nderr_usernameisnull;
  } else if (strcmp(sErr, "UserSessionBOIsNull") == 0) {
    iErr = nderr_usersessionboisnull;
  } else if (strcmp(sErr, "LoginFailInBusinessException") == 0) {
    iErr = nderr_loginfailinbusinessexception;
  } else if (strcmp(sErr, "MSG_InternalError") == 0) {
    iErr = nderr_msginternalerror;
  } else if (strcmp(sErr, "ResultOfRefreshTokenIsNull") == 0) {
    iErr = nderr_resultofrefreshtokenisnull;
  } else if (strcmp(sErr, "ResultOfRefreshTokenError") == 0) {
    iErr = nderr_resultofrefreshtokenerror;
  } else if (strcmp(sErr, "FamilyInfoNotExist") == 0) {
    iErr = nderr_family_info_not_exist;
  } else if (strcmp(sErr, "FamilyInfoExist") == 0) {
    iErr = nderr_family_info_exist;
  } else if (0 == strcmp(sErr, "CommonPermissionDeny")) {
    iErr = nderr_family_kicked_out_duringdowmload;
  } else if (0 == strcmp(sErr, "CommonInfoSecurityCheck ")) {
    iErr = nderr_family_kicked_out_duringdowmload;
  } else if (strcmp(sErr, "MemberInfoNotExist") == 0) {
    iErr = nderr_family_kicked_out;
  } else if (strcmp(sErr, "FamilyOperationFailed") == 0) {
    iErr = nderr_family_operation_failed;
  } else if (strcmp(sErr, "FamilyBroadbandNoBindInfoNotFound") == 0) {
    iErr = nderr_family_BindInfoNotFound;
  } else if (strcmp(sErr, "FamilyBroadbandNoInvaliad") == 0) {
    iErr = nderr_family_BindInfoNotFound;
  } else if (strcmp(sErr, "InvalidAccessToken") == 0) {
    iErr = nderr_oauth2_invalidaccesstoken;
  } else if (strcmp(sErr, "ContentTypeError") == 0) {
    iErr = nderr_content_type_error;
  }
  return iErr;
}

string strErrCode(int nErr) {
  string sErr;
  switch (nErr) {
    case 0:
      sErr = EC_OK;
      break;
    case nderr_already_exists:
      sErr = FILEALREADYEXISTS;
      break;
    case nderr_id_not_found:
      sErr = FILENOTFOUND;
      break;
    case nderr_limit_exceeded:
      sErr = FILETOOLARGE;
      break;
    case nderr_internal_error:
      sErr = INTERNALERROR;
      break;
    case nderr_invalidarg:
      sErr = INVALIDARGUMENT;
      break;
    case nderr_password_not_match:
      sErr = INVALIDPASSWORD;
      break;
    case nderr_sessionbreak:
      sErr = INVALIDSESSIONKEY;
      break;
    case nderr_invalidsign:
      sErr = INVALIDSIGNATURE;
      break;
    case nderr_user_not_exists:
      sErr = NOSUCHUSER;
      break;
    case nderr_parent_not_folder:
      sErr = PARENTNOTFOLDER;
      break;
    case nderr_permission_denied:
      sErr = PERMISSIONDENIED;
      break;
    case nderr_content_type_error:
      sErr = CONTENTTYPEERROR;
      break;
    default:
      sErr = "Unkonwn";
      break;
  };
  return sErr;
}

}  // namespace ErrorCode
}  // namespace Cloud189