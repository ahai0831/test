#include <iostream>
#include <string>

#include <http_primitives.h>

#include <Cloud189/apis/create_upload_file.h>

int main() {
  assistant::HttpRequest http_request("");
  auto json_str = Cloud189::Apis::CreateUploadFile::JsonStringHelper(
      -11, "E:\\\\Desktop\\\\StudyNotes.docx",
      "5429B146F082A63F19CA5670C61FD7D9", 1, 0);
  Cloud189::Apis::CreateUploadFile::HttpRequestEncode(json_str, http_request);
  return 0;
}
