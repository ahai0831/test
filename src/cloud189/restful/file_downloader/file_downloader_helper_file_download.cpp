#include "file_downloader_helper.h"

using Cloud189::Restful::details::downloader_thread_data;
using httpbusiness::downloader::proof::downloader_proof;
using httpbusiness::downloader::proof::downloader_stage;
using httpbusiness::downloader::proof::ProofObsCallback;
using httpbusiness::downloader::proof::stage_result;

namespace Cloud189 {
namespace Restful {
namespace file_downloader_helper {
namespace details {
ProofObsCallback file_download(
    const std::weak_ptr<downloader_thread_data>& thread_data_weak) {
  return [](downloader_proof) -> rxcpp::observable<downloader_proof> {
    /// TODO: 必须完善
    return rxcpp::observable<>::just(downloader_proof{
        downloader_stage::FileDownload, stage_result::Succeeded});
  };
}
}  // namespace details
}  // namespace file_downloader_helper
}  // namespace Restful
}  // namespace Cloud189