#include <iostream>
#include <memory>

#include <json/json.h>

#include <Assistant_v2.h>

#include "cloud189/apis/comfirm_upload_file_complete.h"
#include "cloud189/apis/create_upload_file.h"
#include "cloud189/apis/get_upload_status.h"
#include "cloud189/apis/upload_file_data.h"
#include "cloud189/session_helper/session_helper.h"

int main() {
  auto assistant_ptr = std::make_unique<assistant::Assistant_v2>();

  std::string psk = "6ca144ee-5ba5-4d28-973b-f4d79ed9ccec";
  std::string pss = "2F64B5DB552D6DD215053382ED52A4D1";
  std::string fsk = "c4726d6e-5c66-4dca-a9b5-33597121a4b1_family";
  std::string fss = "2F64B5DB552D6DD215053382ED52A4D1";
  Cloud189::SessionHelper::Cloud189Login(psk, pss, fsk, fss);

  std::string json_str;
  std::string file_local_path =
      "E:\\\\Desktop\\\\Snipaste_2019-10-25_09-47-36.png";
  std::string file_md5 = "11459464ef87eebab3c93dd749f20581";

  // test create upload file
  assistant::HttpRequest create_upload_file_request("");
  json_str = Cloud189::Apis::CreateUploadFile::JsonStringHelper(
      -11, file_local_path, file_md5, 1, 0);
  Cloud189::Apis::CreateUploadFile::HttpRequestEncode(
      json_str, create_upload_file_request);
  std::string result;
  Cloud189::Apis::CreateUploadFile::HttpResponseDecode(
      assistant_ptr->SyncHttpRequest(create_upload_file_request),
      create_upload_file_request, result);

  Json::Value result_json;
  Json::Reader result_reader;
  if (!result_reader.parse(result, result_json)) {
    return 0;
  }
  // test get upload file status
  assistant::HttpRequest get_upload_file_request("");
  json_str = Cloud189::Apis::GetUploadFileStatus::JsonStringHelper(
      result_json["uploadFileId"].asInt64());
  Cloud189::Apis::GetUploadFileStatus::HttpRequestEncode(
      json_str, get_upload_file_request);
  Cloud189::Apis::GetUploadFileStatus::HttpResponseDecode(
      assistant_ptr->SyncHttpRequest(get_upload_file_request),
      get_upload_file_request, result);

  // test upload file data
  assistant::HttpRequest upload_file_data_request("");
  json_str = Cloud189::Apis::UploadFileData::JsonStringHelper(
      result_json["fileUploadUrl"].asString(), file_local_path,
      result_json["uploadFileId"].asInt64(), 0, 84853);
  Cloud189::Apis::UploadFileData::HttpRequestEncode(json_str,
                                                    upload_file_data_request);
  Cloud189::Apis::UploadFileData::HttpResponseDecode(
      assistant_ptr->SyncHttpRequest(upload_file_data_request),
      upload_file_data_request, result);

  // test comfirm upload file
  assistant::HttpRequest comfirm_upload_file_request("");
  json_str = Cloud189::Apis::ComfirmUploadFileComplete::JsonStringHelper(
      result_json["fileCommitUrl"].asString(),
      result_json["uploadFileId"].asInt64(), 1, 0);
  Cloud189::Apis::ComfirmUploadFileComplete::HttpRequestEncode(
      json_str, comfirm_upload_file_request);
  Cloud189::Apis::ComfirmUploadFileComplete::HttpResponseDecode(
      assistant_ptr->SyncHttpRequest(comfirm_upload_file_request),
      comfirm_upload_file_request, result);

  Cloud189::SessionHelper::Cloud189Logout();
  return 0;
}
