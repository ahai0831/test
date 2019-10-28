#include "session_helper.h"

#include <atomic>
#include <memory>

#include <DateHelper/DateHelper.h>
#include <HashAlgorithm/SHA1Digest.h>
#include <tools/string_format.hpp>

namespace {
// use to save and get session info.
class Cloud189SessionHelper {
 public:
  const std::string person_session_key_;
  const std::string person_session_secret_;
  const std::string family_session_key_;
  const std::string family_session_secret_;
  Cloud189SessionHelper(const std::string &person_session_key,
                        const std::string &persion_session_secret,
                        const std::string &family_session_key,
                        const std::string &family_session_secret)
      : person_session_key_(person_session_key),
        person_session_secret_(persion_session_secret),
        family_session_key_(family_session_key),
        family_session_secret_(family_session_secret) {}

 protected:
  Cloud189SessionHelper() = delete;
  Cloud189SessionHelper(const Cloud189SessionHelper &) = delete;
  Cloud189SessionHelper(Cloud189SessionHelper &&) = delete;
  Cloud189SessionHelper &operator=(const Cloud189SessionHelper &) = delete;
};

std::shared_ptr<Cloud189SessionHelper>
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

namespace Cloud189 {
namespace SessionHelper {

bool Cloud189Login(const std::string &person_session_key,
                   const std::string &persion_session_secret,
                   const std::string &family_session_key,
                   const std::string &family_session_secret) {
  decltype(session_helper_ptr) temp_ptr = nullptr;
  do {
    if (person_session_key.empty() || persion_session_secret.empty() ||
        family_session_key.empty() || family_session_secret.empty()) {
      break;
    }
    temp_ptr = std::make_shared<Cloud189SessionHelper>(
        person_session_key, persion_session_secret, family_session_key,
        family_session_secret);
    if (nullptr == temp_ptr) {
      break;
    }
    std::atomic_store(&session_helper_ptr, temp_ptr);
  } while (false);

  return nullptr != temp_ptr;
}

void Cloud189Logout() {
  decltype(session_helper_ptr) temp_ptr = nullptr;
  std::atomic_exchange(&session_helper_ptr, temp_ptr);
}

bool GetCloud189Session(std::string &person_session_key,
                        std::string &person_session_secret) {
  auto sh_ptr = std::atomic_load(&session_helper_ptr);
  do {
    if (nullptr == sh_ptr) {
      break;
    }
    person_session_key = sh_ptr->person_session_key_;
    person_session_secret = sh_ptr->person_session_secret_;
  } while (false);
  return nullptr != sh_ptr;
}

bool GetCloud189FamilySession(std::string &family_session_key,
                              std::string &family_session_secret) {
  auto sh_ptr = std::atomic_load(&session_helper_ptr);
  do {
    if (nullptr == sh_ptr) {
      break;
    }
    family_session_key = sh_ptr->family_session_key_;
    family_session_secret = sh_ptr->family_session_secret_;
  } while (false);
  return nullptr != sh_ptr;
}

bool GenerateCloud189Signature(const std::string &operate,
                               const std::string &request_uri,
                               const std::string &date,
                               std::string &signature) {
  bool is_success = false;
  std::string person_session_key, person_session_secret;
  do {
    if (!GetCloud189Session(person_session_key, person_session_secret)) {
      break;
    }
    // generate signature
    std::string signature_data = assistant::tools::string::StringFormat(
        "SessionKey=%s&Operate=%s&RequestURI=%s&Date=%s",
        person_session_key.c_str(), operate.c_str(), request_uri.c_str(),
        date.c_str());

    signature = cloud_base::hash_algorithm::GenerateSha1Digest(
        signature_data, person_session_secret);

    is_success = true;
  } while (false);

  return is_success;
}

bool GenerateCloud189FamilySignature(const std::string &operate,
                                     const std::string &request_uri,
                                     const std::string &date,
                                     std::string &signature) {
  bool is_success = false;
  std::string family_session_key, family_session_secret;
  do {
    if (!GetCloud189FamilySession(family_session_key, family_session_secret)) {
      break;
    }
    // generate signature
    std::string signature_data = assistant::tools::string::StringFormat(
        "SessionKey=%s&Operate=%s&RequestURI=%s&Date=%s",
        family_session_key.c_str(), operate.c_str(), request_uri.c_str(),
        date.c_str());

    signature = cloud_base::hash_algorithm::GenerateSha1Digest(
        signature_data, family_session_secret);

    is_success = true;
  } while (false);

  return is_success;
}

bool AddCloud189Signature(assistant::HttpRequest &request) {
  std::string person_session_key, person_session_secret;
  bool is_success = false;
  do {
    if (request.url.empty() || request.method.empty()) {
      break;
    }
    if (!GetCloud189Session(person_session_key, person_session_secret)) {
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
    if (!GenerateCloud189Signature(operate, uri, date, signature)) {
      break;
    }
    // set headers
    request.headers.Set("SessionKey", person_session_key);
    request.headers.Set("Signature", signature);
    request.headers.Set("Date", date);

    is_success = true;
  } while (false);

  return is_success;
}

bool AddCloud189FamilySignature(assistant::HttpRequest &request) {
  std::string family_session_key, family_session_secret;
  bool is_success = false;
  do {
    if (request.url.empty() || request.method.empty()) {
      break;
    }
    if (!GetCloud189FamilySession(family_session_key, family_session_secret)) {
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
    if (!GenerateCloud189FamilySignature(operate, uri, date, signature)) {
      break;
    }
    // set headers
    request.headers.Set("SessionKey", family_session_key);
    request.headers.Set("Signature", signature);
    request.headers.Set("Date", date);

    is_success = true;
  } while (false);

  return is_success;
}
}  // namespace SessionHelper
}  // namespace Cloud189
