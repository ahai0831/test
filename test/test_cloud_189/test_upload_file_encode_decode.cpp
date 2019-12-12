#include <fstream>
#include <iostream>
#include <memory>

#include <gtest/gtest.h>
#include <json/json.h>

#include <Assistant_v3.hpp>

#include "cloud189/apis/comfirm_upload_file_complete.h"
#include "cloud189/apis/create_upload_file.h"
#include "cloud189/apis/get_upload_status.h"
#include "cloud189/apis/upload_file_data.h"
#include "cloud189/session_helper/session_helper.h"
#include "restful_common/jsoncpp_helper/jsoncpp_helper.hpp"

TEST(UploadFile, EncodeDecode) {
  auto assistant_ptr = std::make_unique<assistant::Assistant_v3>();
  // ��Ҫ���ú���Ӧ�Ĳ���������ȷ�ϴ�
  std::string access_token = "";

  std::string psk = "6ca144ee-5ba5-4d28-973b-f4d79ed9ccec";
  std::string pss = "2F64B5DB552D6DD215053382ED52A4D1";
  std::string fsk = "c4726d6e-5c66-4dca-a9b5-33597121a4b1_family";
  std::string fss = "2F64B5DB552D6DD215053382ED52A4D1";
  Cloud189::SessionHelper::Cloud189Login(psk, pss, fsk, fss);  // ��¼

  std::string json_str;
  std::string parent_folder_id = "-11";
  std::string file_local_path =
      "E:\\\\Desktop\\\\Snipaste_2019-10-25_09-47-36.png";
  std::string file_md5 = "11459464ef87eebab3c93dd749f20581";
  std::string x_request_id = "E18980D8-CA36-4158-8380-412AEC11AB5F";

  // test create upload file
  // ���ñ�Ҫ����
  assistant::HttpRequest create_upload_file_request("");
  json_str = Cloud189::Apis::CreateUploadFile::JsonStringHelper(
      parent_folder_id, file_local_path, file_md5, x_request_id, 1, 0);
  // encode
  Cloud189::Apis::CreateUploadFile::HttpRequestEncode(
      json_str, create_upload_file_request);
  // decode
  std::string result;
  Cloud189::Apis::CreateUploadFile::HttpResponseDecode(
      assistant_ptr->SyncHttpRequest(create_upload_file_request),
      create_upload_file_request, result);

  Json::Value result_json;
  if (!restful_common::jsoncpp_helper::ReaderHelper(result, result_json)) {
    return;
  }
  // test get upload file status
  // ���ñ�Ҫ����
  assistant::HttpRequest get_upload_file_request("");
  json_str = Cloud189::Apis::GetUploadFileStatus::JsonStringHelper(
      restful_common::jsoncpp_helper::GetString(result_json["uploadFileId"]),
      x_request_id);
  // encode
  Cloud189::Apis::GetUploadFileStatus::HttpRequestEncode(
      json_str, get_upload_file_request);
  // decode
  Cloud189::Apis::GetUploadFileStatus::HttpResponseDecode(
      assistant_ptr->SyncHttpRequest(get_upload_file_request),
      get_upload_file_request, result);

  // test upload file data
  // ���ñ�Ҫ����
  assistant::HttpRequest upload_file_data_request("");
  json_str = Cloud189::Apis::UploadFileData::JsonStringHelper(
      restful_common::jsoncpp_helper::GetString(result_json["fileUploadUrl"]),
      file_local_path,
      restful_common::jsoncpp_helper::GetString(result_json["uploadFileId"]),
      x_request_id, 0, 84853);
  // encode
  Cloud189::Apis::UploadFileData::HttpRequestEncode(json_str,
                                                    upload_file_data_request);
  // decode
  Cloud189::Apis::UploadFileData::HttpResponseDecode(
      assistant_ptr->SyncHttpRequest(upload_file_data_request),
      upload_file_data_request, result);

  // test comfirm upload file
  // ���ñ�Ҫ����
  assistant::HttpRequest comfirm_upload_file_request("");
  json_str = Cloud189::Apis::ComfirmUploadFileComplete::JsonStringHelper(
      restful_common::jsoncpp_helper::GetString(result_json["fileCommitUrl"]),
      restful_common::jsoncpp_helper::GetString(result_json["uploadFileId"]),
      x_request_id, 1, 0);
  // encode
  Cloud189::Apis::ComfirmUploadFileComplete::HttpRequestEncode(
      json_str, comfirm_upload_file_request);
  // decode
  Cloud189::Apis::ComfirmUploadFileComplete::HttpResponseDecode(
      assistant_ptr->SyncHttpRequest(comfirm_upload_file_request),
      comfirm_upload_file_request, result);

  Cloud189::SessionHelper::Cloud189Logout();  // �ǳ�
}
