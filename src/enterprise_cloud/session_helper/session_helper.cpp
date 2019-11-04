#include "session_helper.h"

#include <atomic>
#include <memory>

#include <DateHelper/DateHelper.h>
#include <HashAlgorithm/SHA1Digest.h>
#include <tools/string_format.hpp>

namespace {
// use to save and get session info.
class EnterpriseCloudSessionHelper {
 public:
  const std::string session_key_;
  const std::string session_secret_;
  EnterpriseCloudSessionHelper(const std::string &session_key,
                               const std::string &session_secret)
      : session_key_(session_key), session_secret_(session_secret) {}

 protected:
  EnterpriseCloudSessionHelper() = delete;
  EnterpriseCloudSessionHelper(const EnterpriseCloudSessionHelper &) = delete;
  EnterpriseCloudSessionHelper(EnterpriseCloudSessionHelper &&) = delete;
  EnterpriseCloudSessionHelper &operator=(
      const EnterpriseCloudSessionHelper &) = delete;
};

std::shared_ptr<EnterpriseCloudSessionHelper>
    session_helper_ptr;  // a SessionHelper object pointer for manager session
                         // info.

// use to get uri from url.
bool GetUri(const std::string &url, std::string &uri) {
  uri.clear();
  do {
    auto http_pos = url.find("http://");
    auto https_pos = url.find("https://");
    size_t temp_pos;
    if (http_pos == 0) {
      temp_pos = http_pos + 7;
    } else if (https_pos == 0) {
      temp_pos = https_pos + 8;
    } else {
      break;
    }

    auto begin_pos = url.find('/', temp_pos);  // find fist '/'
    auto end_pos = url.find('?');              // find fist '?'

    if (std::string::npos == begin_pos) {
      break;
    }
    // find first '?', and get the uri between '/' and '?'
    if (std::string::npos == end_pos) {
      uri = url.substr(begin_pos);
    } else if (begin_pos < end_pos) {
      uri = url.substr(begin_pos, end_pos - begin_pos);
    }
  } while (false);

  return !uri.empty();
}
}  // namespace

namespace EnterpriseCloud {
namespace SessionHelper {

bool EnterpriseCloudLogin(const std::string &session_key,
                          const std::string &session_secret) {
  decltype(session_helper_ptr) temp_ptr = nullptr;
  do {
    if (session_key.empty() || session_secret.empty()) {
      break;
    }
    temp_ptr = std::make_shared<EnterpriseCloudSessionHelper>(session_key,
                                                              session_secret);
    if (nullptr == temp_ptr) {
      break;
    }
    std::atomic_store(&session_helper_ptr, temp_ptr);
  } while (false);

  return nullptr != temp_ptr;
}

void EnterpriseCloudLogout() {
  decltype(session_helper_ptr) temp_ptr = nullptr;
  std::atomic_exchange(&session_helper_ptr, temp_ptr);
}

bool GetEnterpriseCloudSession(std::string &session_key,
                               std::string &session_secret) {
  auto sh_ptr = std::atomic_load(&session_helper_ptr);
  do {
    if (nullptr == sh_ptr) {
      break;
    }
    session_key = sh_ptr->session_key_;
    session_secret = sh_ptr->session_secret_;
  } while (false);
  return nullptr != sh_ptr;
}

bool GenerateEnterpriseCloudSignature(const std::string &operate,
                                      const std::string &request_uri,
                                      const std::string &date,
                                      std::string &signature) {
  bool is_success = false;
  std::string session_key, session_secret;
  do {
    if (!GetEnterpriseCloudSession(session_key, session_secret)) {
      break;
    }
    // generate signature
    std::string signature_data = assistant::tools::string::StringFormat(
        "SessionKey=%s&Operate=%s&RequestURI=%s&Date=%s", session_key.c_str(),
        operate.c_str(), request_uri.c_str(), date.c_str());

    signature = cloud_base::hash_algorithm::GenerateSha1Digest(session_secret,
                                                               signature_data);

    is_success = true;
  } while (false);

  return is_success;
}

bool AddEnterpriseCloudSignature(assistant::HttpRequest &request) {
  std::string session_key, session_secret;
  bool is_success = false;
  do {
    if (request.url.empty() || request.method.empty()) {
      break;
    }
    if (!GetEnterpriseCloudSession(session_key, session_secret)) {
      break;
    }
    std::string url = request.url;
    std::string operate = request.method;
    std::string date = cloud_base::date_helper::get_gmt_time_stamp();
    // get uri
    std::string uri;
    if (!GetUri(url, uri)) {
      break;
    }
    // get signature
    std::string signature;
    if (!GenerateEnterpriseCloudSignature(operate, uri, date, signature)) {
      break;
    }
    // set headers
    request.headers.Set("SessionKey", session_key);
    request.headers.Set("Signature", signature);
    request.headers.Set("Date", date);

    is_success = true;
  } while (false);

  return is_success;
}

}  // namespace SessionHelper
}  // namespace EnterpriseCloud
