#include <iostream>
#include <memory>
#include <string>

#include <json/json.h>

#include <Assistant_v3.hpp>

#include "enterprise_cloud/apis/comfirm_corp_upload_file_complete.h"
#include "enterprise_cloud/apis/create_corp_upload_file.h"
#include "enterprise_cloud/apis/get_corp_upload_status.h"
#include "enterprise_cloud/apis/upload_corp_file_data.h"
#include "enterprise_cloud/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

int main() {
  auto assist = std::make_unique<assistant::Assistant_v3>();

  std::string session_key = "da09ee8c-ce44-4301-8b79-dd932270bedb";
  std::string session_secret = "0041A82D3C62770DF3140F81431240A4";
  std::string json_str;
  std::string localPath = "C:\\Users\\TY-PC\\Desktop\\8899.rar";
  std::string md5 = "7c797f90e9bd79c781588d398f452fb6";
  int64_t corpId = 114131189491;
  int64_t parentId = 6143233290222995;
  int64_t uploadFileId = 1385315728603025298;
  int32_t operType = 1;
  int32_t fileSource = 3;
  std::string coshareId = "";
  int32_t isLog = 0;

  EnterpriseCloud::SessionHelper::EnterpriseCloudLogin(session_key,
                                                       session_secret);

  /// create_corp_upload_file
  assistant::HttpRequest create_corp_upload_file("");
  json_str = EnterpriseCloud::Apis::CreateUploadFile::JsonStringHelper(
      localPath, corpId, parentId, md5, fileSource, coshareId, isLog);
  EnterpriseCloud::Apis::CreateUploadFile::HttpRequestEncode(
      json_str, create_corp_upload_file);
  std::string result_1;
  EnterpriseCloud::Apis::CreateUploadFile::HttpResponseDecode(
      assist->SyncHttpRequest(create_corp_upload_file), create_corp_upload_file,
      result_1);
  printf("%s\n", result_1.c_str());

  /// get create_corp_upload_file response param reUploadFileId
  Json::CharReaderBuilder char_reader_builder;
  std::unique_ptr<Json::CharReader> char_reader(
      char_reader_builder.newCharReader());
  if (nullptr == char_reader) {
    return 0;
  }
  Json::Value json_1;
  if (!char_reader->parse(result_1.c_str(),
                          result_1.c_str() + result_1.length(), &json_1,
                          nullptr)) {
    return 0;
  }
  std::string re =
      restful_common::jsoncpp_helper::GetString(json_1["uploadFileId"]);
  int64_t reUploadFileId = atoll(re.c_str());

  assistant::HttpRequest get_corp_upload_status("");
  json_str = EnterpriseCloud::Apis::GetUploadFileStatus::JsonStringHelper(
      reUploadFileId, corpId, isLog);
  EnterpriseCloud::Apis::GetUploadFileStatus::HttpRequestEncode(
      json_str, get_corp_upload_status);
  std::string result_2;
  EnterpriseCloud::Apis::GetUploadFileStatus::HttpResponseDecode(
      assist->SyncHttpRequest(get_corp_upload_status), get_corp_upload_status,
      result_2);
  printf("%s\n", result_2.c_str());

  /// upload_corp_file_data
  std::string upload_data_url =
      "https://hn01upload-b.cloud.189.cn/dciCorp/uploadCorpFile.action";
  assistant::HttpRequest upload_corp_file_data("");
  json_str = EnterpriseCloud::Apis::UploadFileData::JsonStringHelper(
      upload_data_url, localPath, reUploadFileId, corpId, fileSource, 0,
      14369994, isLog);
  EnterpriseCloud::Apis::UploadFileData::HttpRequestEncode(
      json_str, upload_corp_file_data);
  const auto kProxy = "http://127.0.0.1:9999";
  upload_corp_file_data.extends.Set("proxy", kProxy);

  std::string result_3;
  EnterpriseCloud::Apis::UploadFileData::HttpResponseDecode(
      assist->SyncHttpRequest(upload_corp_file_data), upload_corp_file_data,
      result_3);
  printf("%s\n", result_3.c_str());

  /// comfirm_corp_upload_file_complete
  std::string commit_corp_file_url =
      "https://hn01upload-b.cloud.189.cn/dciCorp/commitCorpFile.action";
  assistant::HttpRequest comfirm_corp_upload_file_complete("");
  json_str = EnterpriseCloud::Apis::ComfirmUploadFileComplete::JsonStringHelper(
      commit_corp_file_url, reUploadFileId, corpId, fileSource, operType,
      coshareId, isLog);
  EnterpriseCloud::Apis::ComfirmUploadFileComplete::HttpRequestEncode(
      json_str, comfirm_corp_upload_file_complete);
  std::string result_4;
  EnterpriseCloud::Apis::ComfirmUploadFileComplete::HttpResponseDecode(
      assist->SyncHttpRequest(comfirm_corp_upload_file_complete),
      comfirm_corp_upload_file_complete, result_4);
  printf("%s\n", result_4.c_str());

  EnterpriseCloud::SessionHelper::EnterpriseCloudLogout();

  return 0;
}