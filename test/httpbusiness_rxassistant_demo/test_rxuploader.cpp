#include <gtest/gtest.h>

#include <thread>

#include <rx_uploader.hpp>
TEST(rxuploader_test, normal_succeeded) {
  httpbusiness::uploader::proof::ProofObsCallback calculate_md5;
  httpbusiness::uploader::proof::ProofObsCallback create_upload;
  httpbusiness::uploader::proof::ProofObsCallback check_upload;
  httpbusiness::uploader::proof::ProofObsCallback file_uplaod;
  httpbusiness::uploader::proof::ProofObsCallback file_commit;

  calculate_md5 = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do calculate_md5\n");
    const auto calculate_md5_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::CalculateMd5,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::CreateUpload, 0, 0};
    return rxcpp::observable<>::just(calculate_md5_result);
  };
  create_upload = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do create_upload\n");
    const auto create_upload_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::CreateUpload,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::FileUplaod, 0, 0};
    return rxcpp::observable<>::just(create_upload_result);
  };
  check_upload = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do check_upload\n");
    const auto check_upload_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::CheckUpload,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::FileUplaod, 0, 0};
    return rxcpp::observable<>::just(check_upload_result);
  };
  file_uplaod = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do file_uplaod\n");
    const auto file_uplaod_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::FileUplaod,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::FileCommit, 0, 0};
    return rxcpp::observable<>::just(file_uplaod_result);
  };
  file_commit = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do file_commit\n");
    const auto file_commit_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::FileCommit,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::UploadFinal, 0, 0};
    return rxcpp::observable<>::just(file_commit_result);
  };

  httpbusiness::uploader::proof::proof_obs_packages orders{
      calculate_md5, create_upload, check_upload, file_uplaod, file_commit};
  httpbusiness::uploader::rx_uploader test_upload(
      orders, [](const httpbusiness::uploader::rx_uploader&) {
        printf("on httpbusiness::rx_uploader complete.\n");
      });

  test_upload.AsyncStart();
  test_upload.SyncWait();
}

TEST(rxuploader_test, MD5_fail) {
  httpbusiness::uploader::proof::ProofObsCallback calculate_md5;
  httpbusiness::uploader::proof::ProofObsCallback create_upload;
  httpbusiness::uploader::proof::ProofObsCallback check_upload;
  httpbusiness::uploader::proof::ProofObsCallback file_uplaod;
  httpbusiness::uploader::proof::ProofObsCallback file_commit;

  calculate_md5 = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do calculate_md5\n");
    /// ע��˴�����MD5ʧ��
    const auto calculate_md5_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::CalculateMd5,
            httpbusiness::uploader::proof::stage_result::GiveupRetry,
            httpbusiness::uploader::proof::uploader_stage::CreateUpload, 0, 0};
    printf("Calculate md5 fails\n");
    return rxcpp::observable<>::just(calculate_md5_result);
  };
  create_upload = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do create_upload\n");
    const auto create_upload_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::CreateUpload,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::FileUplaod, 0, 0};
    return rxcpp::observable<>::just(create_upload_result);
  };
  check_upload = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do check_upload\n");
    const auto check_upload_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::CheckUpload,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::FileUplaod, 0, 0};
    return rxcpp::observable<>::just(check_upload_result);
  };
  file_uplaod = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do file_uplaod\n");
    const auto file_uplaod_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::FileUplaod,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::FileCommit, 0, 0};
    return rxcpp::observable<>::just(file_uplaod_result);
  };
  file_commit = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do file_commit\n");
    const auto file_commit_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::FileCommit,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::UploadFinal, 0, 0};
    return rxcpp::observable<>::just(file_commit_result);
  };

  httpbusiness::uploader::proof::proof_obs_packages orders{
      calculate_md5, create_upload, check_upload, file_uplaod, file_commit};
  httpbusiness::uploader::rx_uploader test_upload(
      orders, [](const httpbusiness::uploader::rx_uploader&) {
        printf("on httpbusiness::rx_uploader complete.\n");
      });

  test_upload.AsyncStart();
  test_upload.SyncWait();
}

TEST(rxuploader_test, Invalid_Login_Info) {
  httpbusiness::uploader::proof::ProofObsCallback calculate_md5;
  httpbusiness::uploader::proof::ProofObsCallback create_upload;
  httpbusiness::uploader::proof::ProofObsCallback check_upload;
  httpbusiness::uploader::proof::ProofObsCallback file_uplaod;
  httpbusiness::uploader::proof::ProofObsCallback file_commit;

  calculate_md5 = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do calculate_md5\n");
    const auto calculate_md5_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::CalculateMd5,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::CreateUpload, 0, 0};
    return rxcpp::observable<>::just(calculate_md5_result);
  };
  create_upload = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do create_upload\n");
    /// ע��˴�ģ���¼��ϢʧЧ����ΪCreateUpload��Ȼʧ�ܵ�һ�ֳ���
    const auto create_upload_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::CreateUpload,
            httpbusiness::uploader::proof::stage_result::GiveupRetry,
            httpbusiness::uploader::proof::uploader_stage::FileUplaod, 0, 0};
    printf("create_upload: Invalid_Login_Info\n");
    return rxcpp::observable<>::just(create_upload_result);
  };
  check_upload = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do check_upload\n");
    const auto check_upload_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::CheckUpload,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::FileUplaod, 0, 0};
    return rxcpp::observable<>::just(check_upload_result);
  };
  file_uplaod = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do file_uplaod\n");
    const auto file_uplaod_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::FileUplaod,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::FileCommit, 0, 0};
    return rxcpp::observable<>::just(file_uplaod_result);
  };
  file_commit = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do file_commit\n");
    const auto file_commit_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::FileCommit,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::UploadFinal, 0, 0};
    return rxcpp::observable<>::just(file_commit_result);
  };

  httpbusiness::uploader::proof::proof_obs_packages orders{
      calculate_md5, create_upload, check_upload, file_uplaod, file_commit};
  httpbusiness::uploader::rx_uploader test_upload(
      orders, [](const httpbusiness::uploader::rx_uploader&) {
        printf("on httpbusiness::rx_uploader complete.\n");
      });

  test_upload.AsyncStart();
  test_upload.SyncWait();
}

/// Also, 602
TEST(rxuploader_test, have_previousupload_but_invalid) {
  httpbusiness::uploader::proof::ProofObsCallback calculate_md5;
  httpbusiness::uploader::proof::ProofObsCallback create_upload;
  httpbusiness::uploader::proof::ProofObsCallback check_upload;
  httpbusiness::uploader::proof::ProofObsCallback file_uplaod;
  httpbusiness::uploader::proof::ProofObsCallback file_commit;

  /// ģ��֮ǰ��������¼����������¼ʧЧ�����´���������¼��ɹ�����ϴ�
  calculate_md5 = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do calculate_md5\n");
    const auto calculate_md5_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::CalculateMd5,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::CheckUpload, 0, 0};
    return rxcpp::observable<>::just(calculate_md5_result);
  };
  create_upload = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do create_upload\n");
    const auto create_upload_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::CreateUpload,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::FileUplaod, 0, 0};
    return rxcpp::observable<>::just(create_upload_result);
  };
  check_upload = [](httpbusiness::uploader::proof::uploader_proof proof) {
    /// �˴�ģ��������¼ʧЧ
    printf("Do check_upload\n");
    const auto check_upload_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::CheckUpload,
            httpbusiness::uploader::proof::stage_result::RetryTargetStage,
            httpbusiness::uploader::proof::uploader_stage::CreateUpload, 0, 0};
    printf("Previous upload invalid, should CreateUpload!\n");
    return rxcpp::observable<>::just(check_upload_result);
  };
  file_uplaod = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do file_uplaod\n");
    const auto file_uplaod_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::FileUplaod,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::FileCommit, 0, 0};
    return rxcpp::observable<>::just(file_uplaod_result);
  };
  file_commit = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do file_commit\n");
    const auto file_commit_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::FileCommit,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::UploadFinal, 0, 0};
    return rxcpp::observable<>::just(file_commit_result);
  };

  httpbusiness::uploader::proof::proof_obs_packages orders{
      calculate_md5, create_upload, check_upload, file_uplaod, file_commit};
  httpbusiness::uploader::rx_uploader test_upload(
      orders, [](const httpbusiness::uploader::rx_uploader&) {
        printf("on httpbusiness::rx_uploader complete.\n");
      });

  test_upload.AsyncStart();
  test_upload.SyncWait();
}

TEST(rxuploader_test, test_601_fails) {
  httpbusiness::uploader::proof::ProofObsCallback calculate_md5;
  httpbusiness::uploader::proof::ProofObsCallback create_upload;
  httpbusiness::uploader::proof::ProofObsCallback check_upload;
  httpbusiness::uploader::proof::ProofObsCallback file_uplaod;
  httpbusiness::uploader::proof::ProofObsCallback file_commit;
  /// �� �ϴ�������һ��ʧ��
  /// ��checkʱ�����޷���602��ģ��������¼�����ĳ��������յ����������������Դ����ﵽ���޶�ʧ��
  calculate_md5 = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do calculate_md5\n");
    const auto calculate_md5_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::CalculateMd5,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::CreateUpload, 0, 0};
    return rxcpp::observable<>::just(calculate_md5_result);
  };
  create_upload = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do create_upload\n");
    const auto create_upload_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::CreateUpload,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::FileUplaod, 0, 0};
    return rxcpp::observable<>::just(create_upload_result);
  };
  check_upload = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do check_upload\n");
    /// ÿ�ζ�ʧ��
    const auto check_upload_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::CheckUpload,
            httpbusiness::uploader::proof::stage_result::RetrySelf,
            httpbusiness::uploader::proof::uploader_stage::CheckUpload, 0, 0};
    return rxcpp::observable<>::just(check_upload_result);
  };
  file_uplaod = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do file_uplaod\n");
    /// ���ϴ�ʧ��
    const auto file_uplaod_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::FileUplaod,
            httpbusiness::uploader::proof::stage_result::RetryTargetStage,
            httpbusiness::uploader::proof::uploader_stage::CheckUpload, 0, 0};
    printf("File uplaod fails\n");
    return rxcpp::observable<>::just(file_uplaod_result);
  };
  file_commit = [](httpbusiness::uploader::proof::uploader_proof proof) {
    printf("Do file_commit\n");
    const auto file_commit_result =
        httpbusiness::uploader::proof::uploader_proof{
            httpbusiness::uploader::proof::uploader_stage::FileCommit,
            httpbusiness::uploader::proof::stage_result::Succeeded,
            httpbusiness::uploader::proof::uploader_stage::UploadFinal, 0, 0};
    return rxcpp::observable<>::just(file_commit_result);
  };

  httpbusiness::uploader::proof::proof_obs_packages orders{
      calculate_md5, create_upload, check_upload, file_uplaod, file_commit};
  httpbusiness::uploader::rx_uploader test_upload(
      orders, [](const httpbusiness::uploader::rx_uploader&) {
        printf("on httpbusiness::rx_uploader complete.\n");
      });

  test_upload.AsyncStart();
  test_upload.SyncWait();
}
