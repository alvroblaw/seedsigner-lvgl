#pragma once

#include <lvgl.h>
#include <optional>
#include <string>
#include <vector>

#include "seedsigner_lvgl/runtime/Event.hpp"
#include "seedsigner_lvgl/screen/Screen.hpp"

namespace seedsigner::lvgl {

struct TopNavBarConfig {
    std::string title;
    bool show_back{true};
    bool show_home{false};
    bool show_cancel{false};
    // Custom action buttons on the right side
    struct ActionButton {
        std::string id;
        std::string label; // text or symbol
        bool enabled{true};
        bool operator==(const ActionButton& other) const {
            return id == other.id && label == other.label && enabled == other.enabled;
        }
    };
    std::vector<ActionButton> actions;
    bool operator==(const TopNavBarConfig& other) const {
        return title == other.title &&
               show_back == other.show_back &&
               show_home == other.show_home &&
               show_cancel == other.show_cancel &&
               actions == other.actions;
    }
};

class TopNavBar {
public:
    explicit TopNavBar(ScreenContext context);
    ~TopNavBar();

    /// Attach the navigation bar to a parent container.
    /// The parent must be a valid LVGL object (typically the screen's root).
    /// The bar will be placed at the top of the parent, spanning full width.
    void attach(lv_obj_t* parent);

    /// Detach and destroy the internal LVGL objects.
    void detach();

    /// Update configuration (recreates internal UI if needed).
    void set_config(const TopNavBarConfig& config);

    /// Get the current configuration.
    const TopNavBarConfig& get_config() const { return config_; }

    /// Get the height of the bar (in pixels).
    lv_coord_t get_height() const { return height_; }

    /// Get the internal container (for custom styling or layout adjustments).
    lv_obj_t* get_container() const { return container_; }

    /// Handle hardware input (optional).
    /// Returns true if the input was consumed by the bar.
    bool handle_input(const InputEvent& input);

    /// Emit back event programmatically (e.g., from hardware back button).
    void emit_back() const;

    /// Emit home event programmatically.
    void emit_home() const;

    /// Emit cancel event programmatically.
    void emit_cancel() const;

    /// Emit custom action event.
    void emit_action(const std::string& action_id) const;

private:
    void create_widgets();
    void destroy_widgets();
    void update_layout();

    static void on_back_clicked(lv_event_t* e);
    static void on_home_clicked(lv_event_t* e);
    static void on_cancel_clicked(lv_event_t* e);
    static void on_action_clicked(lv_event_t* e);

    ScreenContext context_;
    TopNavBarConfig config_;
    lv_obj_t* container_{nullptr};
    lv_obj_t* back_btn_{nullptr};
    lv_obj_t* home_btn_{nullptr};
    lv_obj_t* cancel_btn_{nullptr};
    lv_obj_t* title_label_{nullptr};
    std::vector<lv_obj_t*> action_buttons_;
    lv_coord_t height_{48}; // default height, adjust based on design
};

} // namespace seedsigner::lvgl