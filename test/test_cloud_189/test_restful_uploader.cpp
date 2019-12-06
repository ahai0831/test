#include <gtest/gtest.h>

#include "cloud189/restful/cloud189_slice_uploader.h"
#include "cloud189/restful/cloud189_uploader.h"

TEST(cloud189_uploader, init) {
  Cloud189::Restful::Uploader up("", [](const std::string&) {});

  Cloud189::Restful::SliceUploader slice_up("", [](const std::string&) {});

  return;
}
