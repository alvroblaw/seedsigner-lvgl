#pragma once

#include <optional>
#include <string>
#include <vector>

#include "seedsigner_lvgl/contracts/RouteDescriptor.hpp"

namespace seedsigner::lvgl {

struct SeedWordsParams {
    std::vector<std::string> words; // 12, 18, or 24 words
    std::optional<std::string> fingerprint;
    std::optional<std::string> warning_text;
    std::optional<std::string> title;
    int words_per_page{6}; // default 6 words per page for 240x240
};

struct SeedWordsEvent {
    enum class Type {
        PageChanged,
        BackRequested,
        WordSelected,
    } type;
    // For PageChanged
    std::optional<int> page_index; // 0-based
    std::optional<int> total_pages;
    // For WordSelected
    std::optional<int> word_index; // 0-based
    std::optional<std::string> word;
};

PropertyMap make_seed_words_route_args(const SeedWordsParams& params);
SeedWordsParams parse_seed_words_params(const PropertyMap& args);
std::string encode_seed_words_event(const SeedWordsEvent& event);

}  // namespace seedsigner::lvgl