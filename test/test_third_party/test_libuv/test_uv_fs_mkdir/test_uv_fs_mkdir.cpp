#include <cstdio>
#include <cstdlib>

#include <uv.h>

#include <rx_uv_fs.hpp>

static void mkdir_callback(uv_fs_t* req) {
  printf("mkdir: %s, result: %d\n", req->path,
         static_cast<int32_t>(req->result));
}

int main(void) {
  /// 便于跨平台通过编译，以当前文件夹的相对路径为目标
  //	const char* test_path = "./$1/$test/$tmp/$test_uv_fs_mkdir/";

  const char* test_path = "./$1/";
  const char* test_path_2 = "./$12/";

  /// An example for rx_uv_fs::rx_uv_fs_factory::Mkdir
  auto thread = std::make_shared<rx_uv_fs::uv_loop_with_thread>();
  auto obs_1 = rx_uv_fs::rx_uv_fs_factory::Mkdir(thread, test_path);
  obs_1.as_blocking().subscribe([](int32_t result) { printf("%d\n", result); });

  /// An example for uv_fs_mkdir
  auto uvdd = uv_default_loop();
  uv_loop_init(uvdd);
  uv_fs_t* req_1 = (uv_fs_t*)calloc(1, sizeof(uv_fs_t));
  uv_fs_mkdir(uvdd, req_1, test_path_2, 0, mkdir_callback);

  uv_run(uvdd, UV_RUN_DEFAULT);

  uv_fs_req_cleanup(req_1);
  if (nullptr != req_1) {
    free(req_1);
    req_1 = nullptr;
  }

  return 0;
}
