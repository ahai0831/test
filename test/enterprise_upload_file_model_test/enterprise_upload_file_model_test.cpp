#include <iostream>
#include <string>

#include "enterprise_cloud/apis/comfirm_corp_upload_file_complete.h"
#include "enterprise_cloud/apis/create_corp_upload_file.h"
#include "enterprise_cloud/apis/get_corp_upload_status.h"
#include "enterprise_cloud/apis/upload_corp_file_data.h"
#include "enterprise_cloud/session_helper/session_helper.h"

int main() {
  std::string session_key = "91761316-18b3-4929-849d-5ad7a87ed88a";
  std::string session_secret = "944F880085380D93B3B4F4F6F118B57C";
  std::string json_str;
  std::string localPath = "E:\\\\Desktop\\\\StudyNotes.docx";
  int64_t corpId = 114131189491;
  int64_t parentId = -683279323;
  std::string md5 = "5429B146F082A63F19CA5670C61FD7D9";
  int32_t fileSource = 3;
  std::string coshareId = "";
  int32_t isLog = 0;

  EnterpriseCloud::SessionHelper::EnterpriseCloudLogin(session_key,
                                                       session_secret);

  json_str = EnterpriseCloud::Apis::CreateUploadFile::JsonStringHelper(
      localPath, corpId, parentId, md5, fileSource, coshareId, isLog);
  assistant::HttpRequest create_corp_upload_request("");
  EnterpriseCloud::Apis::CreateUploadFile::HttpRequestEncode(
      json_str, create_corp_upload_request);

  json_str = EnterpriseCloud::Apis::GetUploadFileStatus::JsonStringHelper(
      12345, corpId, isLog);
  assistant::HttpRequest get_upload_file_status_request("");
  EnterpriseCloud::Apis::GetUploadFileStatus::HttpRequestEncode(
      json_str, get_upload_file_status_request);

  std::string upload_data_url =
      "https://hn01.upload.cloud.189.cn/uploadDataClient4DCI.action";
  json_str = EnterpriseCloud::Apis::UploadFileData::JsonStringHelper(
      upload_data_url, localPath, 12345, corpId, fileSource, 0, 1000, isLog);
  assistant::HttpRequest upload_file_data_request("");
  EnterpriseCloud::Apis::UploadFileData::HttpRequestEncode(
      json_str, upload_file_data_request);

  std::string commit_corp_file_url =
      "https://hn01upload-b.cloud.189.cn/dciCorp/commitCorpFile.action";
  json_str = EnterpriseCloud::Apis::ComfirmUploadFileComplete::JsonStringHelper(
      commit_corp_file_url, 12345, corpId, fileSource, 1, coshareId, isLog);
  assistant::HttpRequest comfirm_upload_request("");
  EnterpriseCloud::Apis::ComfirmUploadFileComplete::HttpRequestEncode(
      json_str, comfirm_upload_request);

  EnterpriseCloud::SessionHelper::EnterpriseCloudLogout();

  return 0;
}