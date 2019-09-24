
#include <gtest/gtest.h>

#include "slicedownload_test.h"
TEST(slicedownload_test, default_gtest) { ASSERT_TRUE(true); }

TEST(slicedownload_test, sync_wait) {
	slicedownload_sync_test(
      "http://download.cloud.189.cn/download/client/android/cloud189_v8.2.0_1568206533185.apk",
      "cloud189_v8.2.0_1568206533185.apk");
  ASSERT_TRUE(true);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();

  return 0;
}
