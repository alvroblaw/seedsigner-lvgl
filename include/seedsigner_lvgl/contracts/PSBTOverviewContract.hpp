#pragma once

#include <optional>
#include <string>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"

namespace seedsigner::lvgl {

struct PSBTOverviewParams {
    std::string total_amount;           // e.g., "0.005 BTC"
    std::string fee_amount;             // e.g., "0.0001 BTC"
    std::optional<std::string> change_amount; // optional if no change
    int inputs_count{0};
    int outputs_count{0};
    std::string network;                // "mainnet", "testnet", "signet"
    bool has_op_return{false};
    bool self_transfer{false};          // true if all outputs are change (self‑transfer)
    // Additional fields for future extensibility
    std::optional<std::string> memo;
    std::optional<std::string> recipient_label;
};

struct PSBTOverviewEvent {
    enum class Type {
        NextRequested,
        BackRequested,
        DetailRequested,
    } type;
    // For DetailRequested, optionally indicate which element
    std::optional<std::string> detail_target; // "inputs", "outputs", "fee", "change", "op_return"
};

PropertyMap make_psbt_overview_route_args(const PSBTOverviewParams& params);
PSBTOverviewParams parse_psbt_overview_params(const PropertyMap& args);
std::string encode_psbt_overview_event(const PSBTOverviewEvent& event);

}  // namespace seedsigner::lvgl