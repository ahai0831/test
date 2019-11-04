#ifndef SESSION_HELPER_H
#define SESSION_HELPER_H

#include <string>

#include <http_primitives.h>

namespace EnterpriseCloud {
namespace SessionHelper {

// save session info only when log in,
// if success will return true else return false.
bool EnterpriseCloudLogin(const std::string &session_key,
                          const std::string &session_secret);

// destroy session info only when log out
void EnterpriseCloudLogout();

bool GetEnterpriseCloudSession(std::string &session_key,
                               std::string &session_secret);

// set SessionKey, Signature, Date to a request,
// if success will return true else return false.
bool GenerateEnterpriseCloudSignature(const std::string &http_mothod,
                                      const std::string &uri,
                                      const std::string &date,
                                      std::string &signature);

bool AddEnterpriseCloudSignature(assistant::HttpRequest &request);
}  // namespace SessionHelper
}  // namespace EnterpriseCloud
#endif  // SESSION_HELPER_H
