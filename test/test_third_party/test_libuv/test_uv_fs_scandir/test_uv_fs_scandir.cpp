#include <cstdlib>

#include <uv.h>

static void scandir_callback(uv_fs_t* req) {
  for (uv_dirent_t dirent{nullptr};
       UV_EOF != uv_fs_scandir_next(req, &dirent);) {
    printf("%s", dirent.name);
    if (dirent.type == UV_DIRENT_DIR) {
      printf(" is a dir.\n");
    } else if (dirent.type == UV_DIRENT_FILE) {
      printf(" is a file.\n");
    } else {
      printf("\n");
    }
  }
}

int main(void) {
  /// An example for uv_fs_scandir.
  auto loop = uv_default_loop();
  auto fs_req = (uv_fs_t*)calloc(1, sizeof(uv_fs_t));
  /// 便于跨平台通过编译，以当前文件夹的相对路径为目标
  const char* test_path = "./";

  uv_fs_scandir(loop, fs_req, test_path, 0, scandir_callback);

  uv_run(loop, UV_RUN_DEFAULT);
  if (nullptr != fs_req) {
    uv_fs_req_cleanup(fs_req);
    free(fs_req);
    fs_req = nullptr;
  }

  return 0;
}
