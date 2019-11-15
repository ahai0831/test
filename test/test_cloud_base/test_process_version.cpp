#include <gtest/gtest.h>

#include <process_version\process_version.h>

TEST(ProcessVersion, ProcessVersionTest)

{
  std::string proVersion =
      cloud_base::process_version::GetCurrentProcessVersion();
  std::cout << "process version:" << proVersion << std::endl;
}
