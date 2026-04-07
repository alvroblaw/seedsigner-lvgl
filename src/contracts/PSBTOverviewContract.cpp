#include "seedsigner_lvgl/contracts/PSBTOverviewContract.hpp"

#include <charconv>
#include <optional>
#include <string>
#include <string_view>

namespace seedsigner::lvgl {
namespace {

constexpr const char* kTotalAmountArg = "total_amount";
constexpr const char* kFeeAmountArg = "fee_amount";
constexpr const char* kChangeAmountArg = "change_amount";
constexpr const char* kInputsCountArg = "inputs_count";
constexpr const char* kOutputsCountArg = "outputs_count";
constexpr const char* kNetworkArg = "network";
constexpr const char* kHasOpReturnArg = "has_op_return";
constexpr const char* kSelfTransferArg = "self_transfer";
constexpr const char* kMemoArg = "memo";
constexpr const char* kRecipientLabelArg = "recipient_label";

constexpr const char* kEventTypeArg = "event";
constexpr const char* kDetailTargetArg = "detail_target";

std::optional<int> parse_int(std::string_view str) {
    int value;
    auto result = std::from_chars(str.data(), str.data() + str.size(), value);
    if (result.ec != std::errc{} || result.ptr != str.data() + str.size()) {
        return std::nullopt;
    }
    return value;
}

std::optional<bool> parse_bool(std::string_view str) {
    if (str == "1" || str == "true" || str == "yes") {
        return true;
    }
    if (str == "0" || str == "false" || str == "no") {
        return false;
    }
    return std::nullopt;
}

std::string value_or(const PropertyMap& values, const char* key, const char* fallback = "") {
    const auto it = values.find(key);
    return it == values.end() ? std::string{fallback} : it->second;
}

}  // namespace

PropertyMap make_psbt_overview_route_args(const PSBTOverviewParams& params) {
    PropertyMap args;
    args[kTotalAmountArg] = params.total_amount;
    args[kFeeAmountArg] = params.fee_amount;
    if (params.change_amount) {
        args[kChangeAmountArg] = *params.change_amount;
    }
    args[kInputsCountArg] = std::to_string(params.inputs_count);
    args[kOutputsCountArg] = std::to_string(params.outputs_count);
    args[kNetworkArg] = params.network;
    args[kHasOpReturnArg] = params.has_op_return ? "true" : "false";
    args[kSelfTransferArg] = params.self_transfer ? "true" : "false";
    if (params.memo) {
        args[kMemoArg] = *params.memo;
    }
    if (params.recipient_label) {
        args[kRecipientLabelArg] = *params.recipient_label;
    }
    return args;
}

PSBTOverviewParams parse_psbt_overview_params(const PropertyMap& args) {
    PSBTOverviewParams params;
    params.total_amount = value_or(args, kTotalAmountArg, "");
    params.fee_amount = value_or(args, kFeeAmountArg, "");
    const auto change_it = args.find(kChangeAmountArg);
    if (change_it != args.end() && !change_it->second.empty()) {
        params.change_amount = change_it->second;
    }
    const std::string inputs_str = value_or(args, kInputsCountArg, "0");
    const auto inputs_count = parse_int(inputs_str);
    params.inputs_count = inputs_count.value_or(0);
    const std::string outputs_str = value_or(args, kOutputsCountArg, "0");
    const auto outputs_count = parse_int(outputs_str);
    params.outputs_count = outputs_count.value_or(0);
    params.network = value_or(args, kNetworkArg, "mainnet");
    const std::string has_op_return_str = value_or(args, kHasOpReturnArg, "false");
    const auto has_op_return = parse_bool(has_op_return_str);
    params.has_op_return = has_op_return.value_or(false);
    const std::string self_transfer_str = value_or(args, kSelfTransferArg, "false");
    const auto self_transfer = parse_bool(self_transfer_str);
    params.self_transfer = self_transfer.value_or(false);
    const auto memo_it = args.find(kMemoArg);
    if (memo_it != args.end() && !memo_it->second.empty()) {
        params.memo = memo_it->second;
    }
    const auto recipient_it = args.find(kRecipientLabelArg);
    if (recipient_it != args.end() && !recipient_it->second.empty()) {
        params.recipient_label = recipient_it->second;
    }
    return params;
}

std::string encode_psbt_overview_event(const PSBTOverviewEvent& event) {
    std::string encoded;
    switch (event.type) {
        case PSBTOverviewEvent::Type::NextRequested:
            encoded += kEventTypeArg;
            encoded += "=next_requested";
            break;
        case PSBTOverviewEvent::Type::BackRequested:
            encoded += kEventTypeArg;
            encoded += "=back_requested";
            break;
        case PSBTOverviewEvent::Type::DetailRequested:
            encoded += kEventTypeArg;
            encoded += "=detail_requested";
            if (event.detail_target) {
                encoded += ";";
                encoded += kDetailTargetArg;
                encoded += "=";
                encoded += *event.detail_target;
            }
            break;
    }
    return encoded;
}

}  // namespace seedsigner::lvgl