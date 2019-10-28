#ifndef SESSION_HELPER_H
#define SESSION_HELPER_H

#include <string>

#include <http_primitives.h>

namespace Cloud189 {
namespace SessionHelper {

// save session info only when log in,
// if success will return true else return false.
bool Cloud189Login(const std::string &person_session_key,
                   const std::string &person_session_secret,
                   const std::string &family_session_key,
                   const std::string &family_session_secret);

// destroy session info only when log out
void Cloud189Logout();

bool GetCloud189Session(std::string &person_session_key,
                        std::string &person_session_secret);

bool GetCloud189FamilySession(std::string &family_session_key,
                              std::string &family_session_secret);

// set SessionKey, Signature, Date to a request,
// if success will return true else return false.
bool GenerateCloud189Signature(const std::string &http_mothod,
                               const std::string &uri, const std::string &date,
                               std::string &signature);

// set SessionKey, Signature, Date to a request,
// if success will return true else return false.
bool GenerateCloud189FamilySignature(const std::string &http_mothod,
                                     const std::string &uri,
                                     const std::string &date,
                                     std::string &signature);

bool AddCloud189Signature(assistant::HttpRequest &request);
bool AddCloud189FamilySignature(assistant::HttpRequest &request);

}  // namespace SessionHelper
}  // namespace Cloud189
#endif  // SESSION_HELPER_H
