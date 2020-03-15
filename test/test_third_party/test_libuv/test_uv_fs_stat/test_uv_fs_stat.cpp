#include <cstdio>
#include <cstdlib>

#include <uv.h>

#include <rx_uv_fs.hpp>

static void stat_callback(uv_fs_t* req) {
  const auto& mode = req->statbuf.st_mode;
  if (mode & S_IFREG) {
    printf("isfile\n");
  }
  if (mode & S_IFDIR) {
    printf("isdir\n");
  }
}

int main(void) {
  /// 便于跨平台通过编译，以当前文件夹的相对路径为目标
  const char* test_path = ".";

  /// An example for rx_uv_fs::rx_uv_fs_factory::Stat
  auto thread = std::make_shared<rx_uv_fs::uv_loop_with_thread>();
  auto obs_1 = rx_uv_fs::rx_uv_fs_factory::Stat(thread, test_path);
  obs_1.as_blocking().subscribe([](int32_t result) { printf("%d\n", result); });

  /// An example for uv_fs_stat
  auto uvdd = uv_default_loop();
  uv_loop_init(uvdd);
  uv_fs_t* req_1 = (uv_fs_t*)calloc(1, sizeof(uv_fs_t));
  uv_fs_stat(uvdd, req_1, test_path, stat_callback);

  uv_run(uvdd, UV_RUN_DEFAULT);

  uv_fs_req_cleanup(req_1);
  if (nullptr != req_1) {
    free(req_1);
    req_1 = nullptr;
  }

  return 0;
}
