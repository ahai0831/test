
#include <gtest/gtest.h>

#include "slicedownload_test.h"
TEST(slicedownload_test, default_gtest) { ASSERT_TRUE(true); }

TEST(slicedownload_test, sync_wait) {
	slicedownload_sync_test(
      "http://www.w3school.com.cn/i/movie.mp4",
      "movie.mp4");
  ASSERT_TRUE(true);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();

  return 0;
}
