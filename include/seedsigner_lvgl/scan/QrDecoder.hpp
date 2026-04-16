#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace seedsigner::lvgl::scan {

/// Result of a QR decode attempt.
struct QrDecodeResult {
    std::string payload;     ///< Decoded QR data (UTF-8)
    std::string format;      ///< "qr" (extensible for future formats)
    int version;             ///< QR version (1-40), 0 if unknown
    int ecc_level;           ///< Error correction level (quirc enum)
};

/// Abstract QR decoder interface.
class QrDecoder {
public:
    virtual ~QrDecoder() = default;

    /// Attempt to decode QR codes from a grayscale image buffer.
    /// @param data   Grayscale pixel data (1 byte per pixel)
    /// @param width  Image width in pixels
    /// @param height Image height in pixels
    /// @param stride Row stride in bytes (0 = width)
    /// @return First decoded result, or std::nullopt if no QR found.
    virtual std::optional<QrDecodeResult> decode(
        const uint8_t* data, uint32_t width, uint32_t height, uint32_t stride = 0) = 0;

    /// Decode all QR codes found in the image.
    virtual std::vector<QrDecodeResult> decode_all(
        const uint8_t* data, uint32_t width, uint32_t height, uint32_t stride = 0) = 0;
};

/// No-op decoder for builds without QR support. Always returns nullopt.
class NullQrDecoder : public QrDecoder {
public:
    std::optional<QrDecodeResult> decode(
        const uint8_t*, uint32_t, uint32_t, uint32_t = 0) override { return std::nullopt; }
    std::vector<QrDecodeResult> decode_all(
        const uint8_t*, uint32_t, uint32_t, uint32_t = 0) override { return {}; }
};

/// Factory: create the best available decoder for this build.
std::unique_ptr<QrDecoder> create_decoder();

}  // namespace seedsigner::lvgl::scan
