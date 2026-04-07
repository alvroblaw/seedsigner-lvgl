#include "seedsigner_lvgl/contracts/PSBTDetailContract.hpp"

#include <charconv>
#include <optional>
#include <string>
#include <string_view>

namespace seedsigner::lvgl {
namespace {

constexpr const char* kTypeArg = "type";
constexpr const char* kIndexArg = "index";
constexpr const char* kAddressArg = "address";
constexpr const char* kAmountArg = "amount";
constexpr const char* kDerivationPathArg = "derivation_path";
constexpr const char* kPubkeyArg = "pubkey";
constexpr const char* kNetworkArg = "network";
constexpr const char* kLabelArg = "label";
constexpr const char* kMemoArg = "memo";

constexpr const char* kEventTypeArg = "event";
constexpr const char* kQRTargetArg = "qr_target";

std::optional<int> parse_int(std::string_view str) {
    int value;
    auto result = std::from_chars(str.data(), str.data() + str.size(), value);
    if (result.ec != std::errc{} || result.ptr != str.data() + str.size()) {
        return std::nullopt;
    }
    return value;
}

std::string value_or(const PropertyMap& values, const char* key, const char* fallback = "") {
    const auto it = values.find(key);
    return it == values.end() ? std::string{fallback} : it->second;
}

PSBTDetailType parse_type(std::string_view str) {
    if (str == "output") {
        return PSBTDetailType::Output;
    }
    // default to input
    return PSBTDetailType::Input;
}

const char* type_to_string(PSBTDetailType type) {
    switch (type) {
        case PSBTDetailType::Output: return "output";
        case PSBTDetailType::Input:  // fallthrough
        default: return "input";
    }
}

}  // namespace

PropertyMap make_psbt_detail_route_args(const PSBTDetailParams& params) {
    PropertyMap args;
    args[kTypeArg] = type_to_string(params.type);
    args[kIndexArg] = std::to_string(params.index);
    args[kAddressArg] = params.address;
    args[kAmountArg] = params.amount;
    if (params.derivation_path) {
        args[kDerivationPathArg] = *params.derivation_path;
    }
    if (params.pubkey) {
        args[kPubkeyArg] = *params.pubkey;
    }
    args[kNetworkArg] = params.network;
    if (params.label) {
        args[kLabelArg] = *params.label;
    }
    if (params.memo) {
        args[kMemoArg] = *params.memo;
    }
    return args;
}

PSBTDetailParams parse_psbt_detail_params(const PropertyMap& args) {
    PSBTDetailParams params;
    const std::string type_str = value_or(args, kTypeArg, "input");
    params.type = parse_type(type_str);
    const std::string index_str = value_or(args, kIndexArg, "0");
    const auto index = parse_int(index_str);
    params.index = index.value_or(0);
    params.address = value_or(args, kAddressArg, "");
    params.amount = value_or(args, kAmountArg, "");
    const auto derivation_it = args.find(kDerivationPathArg);
    if (derivation_it != args.end() && !derivation_it->second.empty()) {
        params.derivation_path = derivation_it->second;
    }
    const auto pubkey_it = args.find(kPubkeyArg);
    if (pubkey_it != args.end() && !pubkey_it->second.empty()) {
        params.pubkey = pubkey_it->second;
    }
    params.network = value_or(args, kNetworkArg, "mainnet");
    const auto label_it = args.find(kLabelArg);
    if (label_it != args.end() && !label_it->second.empty()) {
        params.label = label_it->second;
    }
    const auto memo_it = args.find(kMemoArg);
    if (memo_it != args.end() && !memo_it->second.empty()) {
        params.memo = memo_it->second;
    }
    return params;
}

std::string encode_psbt_detail_event(const PSBTDetailEvent& event) {
    std::string encoded;
    switch (event.type) {
        case PSBTDetailEvent::Type::BackRequested:
            encoded += kEventTypeArg;
            encoded += "=back_requested";
            break;
        case PSBTDetailEvent::Type::ViewQRRequested:
            encoded += kEventTypeArg;
            encoded += "=view_qr_requested";
            if (event.qr_target) {
                encoded += ";";
                encoded += kQRTargetArg;
                encoded += "=";
                encoded += *event.qr_target;
            }
            break;
    }
    return encoded;
}

}  // namespace seedsigner::lvgl