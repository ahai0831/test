
#include <gtest/gtest.h>

#include "slicedownload_test.h"
TEST(slicedownload_test, default_gtest) { ASSERT_TRUE(true); }

TEST(slicedownload_test, sync_wait_complete) {
  slicedownload_sync_test(
      "http://download.cloud.189.cn/file/downloadFile.action?dlt=4&dt=1&expired=1720752344199&sk=297953828&ufi=4152752762109060&zyc=60&token=cloud4&sig=d2rdJKa9Xud1%2FbcnP%2Fn0Ly7ftpI%3D",
      "hi.exe");
  ASSERT_TRUE(true);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();

  return 0;
}
