# Scenario-Driven Validation Suite

Headless JSON scenarios that exercise every screen family through the `ScenarioRunner`.
Supports running scenarios across multiple display profiles (resolution × layout).

## Display Profiles

The profile matrix validates that all screens render correctly across different
hardware resolutions:

| Profile | Resolution | Layout | Target Hardware |
|---------|-----------|--------|-----------------|
| `square_240x240` | 240×240 | Square | Original SeedSigner |
| `portrait_240x320` | 240×320 | Portrait | Waveshare-style hat |

Profiles are defined in `src/visual/DisplayProfile.cpp` and control layout constants
(words per page, preview size, etc.) that vary by resolution.

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

### Profile Matrix (multi-profile validation)

Run every scenario across **all** display profiles:

```bash
cmake -S . -B build-matrix -DBUILD_HOST_DESKTOP=ON
cmake --build build-matrix --target profile_matrix_runner

# Run the full profile × scenario matrix
./build-matrix/profile_matrix_runner

# Or via CTest
cd build-matrix && ctest -R profile_matrix
```

Output structure:
```
matrix_out/
├── square_240x240/       # screenshots at 240×240
│   ├── 01_menu_navigation.png
│   └── ...
├── portrait_240x320/     # screenshots at 240×320
│   ├── 01_menu_navigation.png
│   └── ...
└── profile_matrix_report.json   # pass/fail matrix
```

The JSON report contains per-profile, per-scenario results:
```json
{
  "profiles": [
    {
      "name": "square_240x240",
      "width": 240, "height": 240,
      "results": { "01_menu_navigation.json": true, ... }
    }, ...
  ],
  "summary": { "total": 22, "passed": 22, "failed": 0 }
}
```

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
