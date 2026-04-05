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
