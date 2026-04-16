#include "seedsigner_lvgl/scan/QrDecoder.hpp"

namespace seedsigner::lvgl::scan {

// NullQrDecoder is defined inline in the header.

std::unique_ptr<QrDecoder> create_decoder() {
    // Default: no-op decoder. The host or embedded application injects
    // a real implementation via set_decoder() or directly on ScanScreen.
    return std::make_unique<NullQrDecoder>();
}

}  // namespace seedsigner::lvgl::scan
