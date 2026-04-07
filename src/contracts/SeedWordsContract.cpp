#include "seedsigner_lvgl/contracts/SeedWordsContract.hpp"

#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace seedsigner::lvgl {
namespace {

constexpr const char* kWordsArg = "words";
constexpr const char* kFingerprintArg = "fingerprint";
constexpr const char* kWarningTextArg = "warning_text";
constexpr const char* kTitleArg = "title";
constexpr const char* kWordsPerPageArg = "words_per_page";

constexpr const char* kEventTypeArg = "event";
constexpr const char* kPageIndexArg = "page_index";
constexpr const char* kTotalPagesArg = "total_pages";
constexpr const char* kWordIndexArg = "word_index";
constexpr const char* kWordValueArg = "word";

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

// Split a string by spaces (BIP39 words are separated by spaces)
std::vector<std::string> split_words(std::string_view words_str) {
    std::vector<std::string> words;
    size_t start = 0;
    size_t end = words_str.find(' ');
    while (end != std::string_view::npos) {
        if (end > start) {
            words.emplace_back(words_str.substr(start, end - start));
        }
        start = end + 1;
        end = words_str.find(' ', start);
    }
    if (start < words_str.size()) {
        words.emplace_back(words_str.substr(start));
    }
    return words;
}

// Join words with spaces
std::string join_words(const std::vector<std::string>& words) {
    if (words.empty()) {
        return "";
    }
    std::string result = words[0];
    for (size_t i = 1; i < words.size(); ++i) {
        result += ' ';
        result += words[i];
    }
    return result;
}

}  // namespace

PropertyMap make_seed_words_route_args(const SeedWordsParams& params) {
    PropertyMap args;
    args[kWordsArg] = join_words(params.words);
    if (params.fingerprint) {
        args[kFingerprintArg] = *params.fingerprint;
    }
    if (params.warning_text) {
        args[kWarningTextArg] = *params.warning_text;
    }
    if (params.title) {
        args[kTitleArg] = *params.title;
    }
    args[kWordsPerPageArg] = std::to_string(params.words_per_page);
    return args;
}

SeedWordsParams parse_seed_words_params(const PropertyMap& args) {
    SeedWordsParams params;
    const std::string words_str = value_or(args, kWordsArg, "");
    params.words = split_words(words_str);
    const auto fingerprint_it = args.find(kFingerprintArg);
    if (fingerprint_it != args.end() && !fingerprint_it->second.empty()) {
        params.fingerprint = fingerprint_it->second;
    }
    const auto warning_it = args.find(kWarningTextArg);
    if (warning_it != args.end() && !warning_it->second.empty()) {
        params.warning_text = warning_it->second;
    }
    const auto title_it = args.find(kTitleArg);
    if (title_it != args.end() && !title_it->second.empty()) {
        params.title = title_it->second;
    }
    const std::string per_page_str = value_or(args, kWordsPerPageArg, "6");
    const auto per_page = parse_int(per_page_str);
    params.words_per_page = per_page.value_or(6);
    // clamp to reasonable range
    if (params.words_per_page < 1) params.words_per_page = 1;
    if (params.words_per_page > 24) params.words_per_page = 24;
    return params;
}

std::string encode_seed_words_event(const SeedWordsEvent& event) {
    std::string encoded;
    switch (event.type) {
        case SeedWordsEvent::Type::PageChanged:
            encoded += kEventTypeArg;
            encoded += "=page_changed;";
            if (event.page_index) {
                encoded += kPageIndexArg;
                encoded += "=";
                encoded += std::to_string(*event.page_index);
                encoded += ";";
            }
            if (event.total_pages) {
                encoded += kTotalPagesArg;
                encoded += "=";
                encoded += std::to_string(*event.total_pages);
            }
            break;
        case SeedWordsEvent::Type::BackRequested:
            encoded += kEventTypeArg;
            encoded += "=back_requested";
            break;
        case SeedWordsEvent::Type::WordSelected:
            encoded += kEventTypeArg;
            encoded += "=word_selected;";
            if (event.word_index) {
                encoded += kWordIndexArg;
                encoded += "=";
                encoded += std::to_string(*event.word_index);
                encoded += ";";
            }
            if (event.word) {
                encoded += kWordValueArg;
                encoded += "=";
                encoded += *event.word;
            }
            break;
    }
    return encoded;
}

}  // namespace seedsigner::lvgl