# Host build and demo guide

This repo currently supports a **headless host build** for development and smoke validation. The runtime and tests are exercised on Linux in CI-style mode without SDL or a visible simulator window.

## What has actually been validated

- **Linux**: configured, built, tested, and ran `host_sim_demo`
- **macOS**: requirements and commands are documented below, but runtime execution has **not** been validated in this change set

## Requirements

### Linux

- CMake 3.20+
- A C++17-capable compiler (`g++` or `clang++`)
- Make or Ninja
- Git
- Network access during the first configure step so CMake can fetch LVGL 8.3.11 via `FetchContent`

Example Debian/Ubuntu packages:

```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build git
```

### macOS

- Xcode Command Line Tools
- CMake 3.20+
- Ninja (recommended) or Make
- Git
- Network access during the first configure step so CMake can fetch LVGL 8.3.11 via `FetchContent`

Example setup with Homebrew:

```bash
xcode-select --install
brew install cmake ninja git
```

## Configure and build

Using Ninja:

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

Using default generator:

```bash
cmake -S . -B build
cmake --build build
```

## Run tests

```bash
ctest --test-dir build --output-on-failure
```

## Run the host demo

```bash
./build/host_sim_demo
```

The current demo exercises a SeedSigner-style list/menu screen and prints emitted route/action/cancel events to stdout.

## Headless-mode caveats

The host path is intentionally minimal right now:

- no SDL window or desktop renderer yet
- no keyboard event loop wired to the executable
- no screenshot/golden-image testing
- visual work is validated indirectly through LVGL object creation plus headless flushes/events

That makes this mode good for:

- lifecycle testing
- navigation/event contract testing
- iterative screen composition work

It is **not** yet enough for:

- real desktop UX validation
- pixel-accurate parity checks
- interactive manual testing of focus/scroll behavior with live input devices

## Recommended workflow for contributors

```bash
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
./build/host_sim_demo
```

If you change screen composition or event behavior, keep the headless test suite green and update the demo payload to reflect the latest representative screen.
