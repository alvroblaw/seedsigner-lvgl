#pragma once

#include <optional>
#include <string>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"

namespace seedsigner::lvgl {

/// Supported camera frame formats.
///
/// - RGB565: 16‑bit color (5‑6‑5) packed in little‑endian order; two bytes per pixel.
///   Suitable for direct rendering to RGB565 displays.
/// - Grayscale: 8‑bit luminance, one byte per pixel. Recommended for QR scanning.
/// - JPEG: Compressed JPEG data; pixels are not directly accessible without decoding.
///
/// Hardware constraints:
/// - ESP32‑S3 and P4 have limited RAM; large RGB565 frames may exhaust heap.
/// - Grayscale is the most memory‑efficient for processing.
/// - JPEG reduces bandwidth but requires decompression (CPU cost).
enum class CameraFormat {
    RGB565,
    Grayscale,
    JPEG,
};

/// Parameters to configure a camera source for a camera‑backed screen.
///
/// Passed as route arguments when navigating to a screen that requires camera input.
/// The screen may ignore unsupported parameters; defaults should be safe.
///
/// Ownership and lifetime:
/// - Camera frames are delivered via `push_frame()` with a `CameraFrame` struct.
/// - The producer (camera driver) lends the pixel buffer only for the duration of
///   the `push_frame()` call. The screen must copy the data if it needs to retain it.
/// - For zero‑copy rendering, the screen may hold references only while the producer
///   guarantees the buffer stays alive (e.g., double‑buffering with ownership hand‑off).
///   This contract does not enforce a particular ownership model; it is the responsibility
///   of the camera driver and screen to agree on a buffer‑pooling strategy.
///
/// Thread safety:
/// - Frames may arrive from a different thread (e.g., camera ISR). The `push_frame()`
///   call must be synchronised externally; the contract does not provide locking.
/// - Screen implementations should assume `push_frame()` can be called at any time.
struct CameraParams {
    /// Desired pixel format. Defaults to Grayscale for compatibility.
    CameraFormat desired_format{CameraFormat::Grayscale};

    /// Desired frame width in pixels. 0 means “use the camera's default”.
    int desired_width{0};

    /// Desired frame height in pixels. 0 means “use the camera's default”.
    int desired_height{0};

    /// Maximum frames per second the screen can consume. 0 means “unlimited”.
    /// The camera driver may lower the actual FPS to match hardware limits.
    int max_fps{0};

    /// Number of buffers the camera driver should allocate for double/triple buffering.
    /// A value greater than 1 can reduce frame drops when the screen takes longer to process.
    /// The screen does not allocate buffers; this is a hint to the producer.
    int buffer_count{1};
};

/// Create a PropertyMap suitable for `RouteDescriptor::args` from CameraParams.
PropertyMap make_camera_route_args(const CameraParams& params);

/// Parse CameraParams from a PropertyMap (typically route arguments).
CameraParams parse_camera_params(const PropertyMap& args);

/// Convert CameraFormat to a string for serialisation.
std::string to_string(CameraFormat format);

/// Parse a CameraFormat from a string (case‑insensitive).
/// Returns Grayscale if the string is not recognised.
CameraFormat parse_camera_format(std::string_view raw);

}  // namespace seedsigner::lvgl