#ifndef PARAMS_HELPER_H
#define PARAMS_HELPER_H

#include <string>
namespace Cloud189 {
namespace ParamsHelper {
inline std::string GetHost() { return "https://api.cloud.189.cn"; }
inline std::string GetClientType() {
#ifdef _WIN32
  return "TELEPC";
#elif __linux__
  return "";
#else
  return "TELEMAC";
#endif
}
inline std::string GetChannelId() { return "web_cloud.189.cn"; }

}  // namespace ParamsHelper
}  // namespace Cloud189
#endif  // PARAMS_HELPER_H
