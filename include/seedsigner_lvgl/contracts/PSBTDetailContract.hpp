#pragma once

#include <optional>
#include <string>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"

namespace seedsigner::lvgl {

enum class PSBTDetailType {
    Input,
    Output,
};

struct PSBTDetailParams {
    PSBTDetailType type{PSBTDetailType::Input};
    int index{0};                        // 0‑based index within inputs/outputs
    std::string address;                 // human‑readable address (if known)
    std::string amount;                  // e.g., "0.001 BTC"
    std::optional<std::string> derivation_path; // BIP32 path (if known)
    std::optional<std::string> pubkey;   // hex‑encoded public key (if known)
    std::string network;                 // "mainnet", "testnet", "signet"
    // Additional fields for future extensibility
    std::optional<std::string> label;
    std::optional<std::string> memo;
};

struct PSBTDetailEvent {
    enum class Type {
        BackRequested,
        ViewQRRequested,  // placeholder for future "view QR" action
    } type;
    // For ViewQRRequested, optionally indicate which data to encode
    std::optional<std::string> qr_target; // "address", "pubkey", "derivation"
};

PropertyMap make_psbt_detail_route_args(const PSBTDetailParams& params);
PSBTDetailParams parse_psbt_detail_params(const PropertyMap& args);
std::string encode_psbt_detail_event(const PSBTDetailEvent& event);

}  // namespace seedsigner::lvgl