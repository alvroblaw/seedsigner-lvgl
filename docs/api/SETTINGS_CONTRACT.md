# Settings Route Contract

`SettingsSelectionScreen` now has a small host-side contract layer in `include/seedsigner_lvgl/contracts/SettingsContract.hpp`.

## Host-facing types

- `SettingDefinition`
  - screen framing: `title`, `subtitle`, `section_title`, `help_text`, `footer_text`
  - stable setting identity: `id`
  - value semantics: `value_type` (`single` or `multi`)
  - defaults: `default_values`
  - current values: `current_values`
  - option rows: `items`
- `SettingItemDefinition`
  - `id`, `label`, `secondary_text`
  - `item_type` (`choice`, `toggle`, `action`)
  - optional `accessory`

## Route-arg bridge

Use `make_settings_route_args(definition)` to translate a typed setting definition into the string-keyed `RouteDescriptor.args` map expected by the runtime.

The helper emits a stable string contract for the active settings slice:

- `setting_id`
- `setting_label`
- `setting_type`
- `default_value` / `default_values`
- `current_value` / `current_values`
- `items`

`items` remains line-oriented for MicroPython/host friendliness, but now uses an explicit row schema:

```text
item_id|label|secondary_text|item_type|accessory
```

Legacy 2-4 column rows still parse for compatibility.

## Event payload bridge

`focus_changed` and `setting_selected` continue to emit compact string payloads, but the payload is now derived from the same typed setting definition used for route args.

Current payload fields:

```text
event=<focus|select>;
setting_id=<id>;
setting_label=<label>;
setting_type=<single|multi>;
index=<row_index>;
item_id=<option_id>;
item_label=<option_label>;
item_type=<choice|toggle|action>;
is_current=<true|false>;
default_value=<value>;
default_values=<csv>;
current_value=<value>;
current_values=<csv>
```

This keeps the external-control architecture intact:
- host owns the definition/default/current state
- runtime owns rendering/focus/transient selection visuals
- outbound events reflect the host-facing definition without requiring LVGL knowledge

## Grouped settings menu

To present multiple settings in a grouped menu, use `SettingsMenuScreen`. The host can pass a list of `SettingDefinition` objects via `make_settings_menu_route_args(definitions)`. This function returns a `PropertyMap` suitable for `RouteDescriptor.args` with the following structure:

- `setting_count`: number of definitions
- `setting_0_id`, `setting_0_title`, `setting_0_subtitle`, … (full flattened args per definition)
- `title`, `help_text`, `footer_text`: optional global header/footer strings
- `selected_index`: optional initial selection (default 0)

The screen will display each setting as a row using its `section_title` (or `subtitle` or `title` as fallback) as the primary label, and `help_text` as secondary label. A chevron accessory indicates navigation.

Events emitted:
- `focus_changed`: when focus moves between rows, includes setting id and label.
- `setting_selected`: when a row is pressed, includes setting id and label.

The host should listen for `setting_selected` and respond by activating a `SettingsSelectionScreen` with the corresponding `SettingDefinition`. This can be done by reusing the same definition passed to the menu, or by looking up the definition by id.

Example host usage (C++):

```cpp
std::vector<SettingDefinition> definitions = {
    {
        .id = "locale",
        .title = "Settings",
        .subtitle = "Language",
        .section_title = "Display language",
        .help_text = "Choose one language for the active UI session.",
        .value_type = SettingValueType::SingleChoice,
        .items = {...}
    },
    {
        .id = "features",
        .title = "Settings",
        .subtitle = "Advanced features",
        .section_title = "Enabled features",
        .value_type = SettingValueType::MultiChoice,
        .items = {...}
    }
};

PropertyMap args = make_settings_menu_route_args(definitions);
args["title"] = "Settings";
args["help_text"] = "Select a setting to adjust.";
runtime.activate(RouteId{"settings.menu"}, args);
```

When the user selects a setting, the host receives an `action_invoked` event with `action_id` = `"setting_selected"` and `meta.key` = setting id. The host can then activate the individual setting screen using `make_settings_route_args` for that definition.

This pattern keeps the host in control of the definition lifecycle and allows dynamic updates to settings (e.g., current values) without rebuilding the entire menu.
