# Scenario-Driven Validation Suite

Headless JSON scenarios that exercise every screen family through the `ScenarioRunner`.

## What It Does

Each scenario is a JSON file that drives `activate` / `input` / `screenshot` / `wait` steps against a headless `UiRuntime`. The suite runner forks per scenario to keep LVGL state clean.

## Screen Families Covered

| # | Scenario | Screen |
|---|----------|--------|
| 01 | `01_menu_navigation.json` | MenuListScreen |
| 02 | `02_settings_selection.json` | SettingsSelectionScreen |
| 03 | `03_warning.json` | WarningScreen |
| 04 | `04_error.json` | ErrorScreen |
| 05 | `05_dire_warning.json` | DireWarningScreen |
| 06 | `06_result.json` | ResultScreen |
| 07 | `07_qr_display.json` | QRDisplayScreen |
| 08 | `08_keyboard.json` | KeyboardScreen |
| 09 | `09_seed_words.json` | SeedWordsScreen |
| 10 | `10_psbt_overview.json` | PSBTOverviewScreen |
| 11 | `11_startup_splash.json` | StartupSplashScreen |

## Build & Run

```bash
cmake -S . -B build-suite -DBUILD_HOST_DESKTOP=ON
cmake --build build-suite --target scenario_suite_runner

# Run all scenarios (from repo root)
./build-suite/scenario_suite_runner

# Or via CTest
cd build-suite && ctest -R scenario_suite
```

Screenshots land in `scenarios/out/` (or `SCENARIO_OUT_DIR` if set).

## Scenario JSON Schema

```json
{
  "default_delay_ms": 50,
  "steps": [
    { "action": "activate", "route": "suite.menu", "args": { ... } },
    { "action": "input", "key": "Down" },
    { "action": "screenshot", "path": "out/menu.png" },
    { "action": "wait", "ms": 100 }
  ]
}
```

Actions: `activate`, `input`, `screenshot`, `wait`.  
Keys: `Up`, `Down`, `Left`, `Right`, `Press`, `Back`.

## Adding a Scenario

1. Create `scenarios/NN_name.json` following the schema above
2. Use `suite.*` route prefix (registered in `tests/scenario_suite_runner.cpp`)
3. Add any new routes to `register_suite_routes()` in the runner
4. Run the suite — your scenario is picked up automatically
