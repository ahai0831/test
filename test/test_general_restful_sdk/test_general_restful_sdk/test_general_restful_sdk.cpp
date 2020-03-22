#include "test_headers.h"
int main(void) {
  /// AstConfig调用传入Session信息，因此必须最先调用此case
  test_ast_config();

  test_ast_process_cloud189_file_upload();

  test_ast_process_cloud189_folder_upload();

  test_ast_process_cloud189_file_download();

  test_ast_process_cloud189_folder_download();

  test_ast_process_cloud189_file_download_resume_from_breakpoint();

  return 0;
}
