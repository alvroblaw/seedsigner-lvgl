cmake_minimum_required(VERSION 3.20)
project(seedsigner_lvgl VERSION 0.1.0 LANGUAGES C CXX)

include(FetchContent)
include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  lvgl
  URL https://github.com/lvgl/lvgl/archive/refs/tags/v8.3.11.tar.gz
)
FetchContent_MakeAvailable(lvgl)
set_target_properties(lvgl_examples lvgl_demos PROPERTIES EXCLUDE_FROM_ALL TRUE)

set(SEEDSIGNER_LV_CONF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/config)
target_include_directories(lvgl PUBLIC ${SEEDSIGNER_LV_CONF_DIR})
target_compile_definitions(lvgl PUBLIC LV_CONF_INCLUDE_SIMPLE)

add_library(seedsigner_lvgl
  src/contracts/SettingsContract.cpp
  src/contracts/QRDisplayContract.cpp
  src/contracts/KeyboardContract.cpp
  src/contracts/CameraContract.cpp
  src/navigation/NavigationController.cpp
  src/platform/HeadlessDisplay.cpp
  src/runtime/EventQueue.cpp
  src/runtime/UiRuntime.cpp
  src/screen/ScreenRegistry.cpp
  src/components/TopNavBar.cpp
  src/screens/CameraPreviewScreen.cpp
  src/screens/MenuListScreen.cpp
  src/screens/PlaceholderScreen.cpp
  src/screens/ResultScreen.cpp
  src/screens/SettingsSelectionScreen.cpp
  src/screens/SettingsMenuScreen.cpp
  src/screens/WarningScreen.cpp
  src/screens/ErrorScreen.cpp
  src/screens/DireWarningScreen.cpp
  src/screens/QRDisplayScreen.cpp
  src/screens/KeyboardScreen.cpp
  src/screens/ScanScreen.cpp
  src/contracts/SeedWordsContract.cpp
  src/screens/SeedWordsScreen.cpp
  src/contracts/PSBTOverviewContract.cpp
  src/screens/PSBTOverviewScreen.cpp
  src/contracts/PSBTMathContract.cpp
  src/contracts/PSBTDetailContract.cpp
  src/screens/PSBTMathScreen.cpp
  src/screens/PSBTDetailScreen.cpp
  src/contracts/StartupSplashContract.cpp
  src/contracts/ScreensaverContract.cpp
  src/screens/StartupSplashScreen.cpp
  src/screens/ScreensaverScreen.cpp
  src/platform/FramebufferCapture.cpp
  src/visual/SeedSignerTheme.cpp
  src/visual/DisplayProfile.cpp
  src/input/InputProfile.cpp
  src/assets/icons.c
)

target_include_directories(seedsigner_lvgl
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/seedsigner_lvgl
    ${CMAKE_CURRENT_SOURCE_DIR}/src/assets
    ${SEEDSIGNER_LV_CONF_DIR}
    ${lvgl_SOURCE_DIR}
    ${lvgl_SOURCE_DIR}/src/extra/libs/qrcode
)

target_compile_definitions(seedsigner_lvgl PUBLIC LV_CONF_INCLUDE_SIMPLE)

target_link_libraries(seedsigner_lvgl PUBLIC lvgl)

target_compile_features(seedsigner_lvgl PUBLIC cxx_std_17)

add_executable(host_sim_demo examples/host_sim_demo.cpp)
target_link_libraries(host_sim_demo PRIVATE seedsigner_lvgl)

# --- Host desktop interactive demo (SDL2) ---
option(BUILD_HOST_DESKTOP "Build the interactive SDL2 desktop demo" OFF)
if(BUILD_HOST_DESKTOP)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(SDL2 REQUIRED sdl2)

  # nlohmann/json for scenario loader
  FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  )
  FetchContent_MakeAvailable(nlohmann_json)

  add_executable(host_desktop_demo
    examples/host_desktop_demo.cpp
    src/platform/SdlDisplay.cpp
    src/desktop/ScenarioRunner.cpp
  )
  target_include_directories(host_desktop_demo PRIVATE
    ${SDL2_INCLUDE_DIRS}
  )
  target_link_libraries(host_desktop_demo PRIVATE seedsigner_lvgl ${SDL2_LIBRARIES} nlohmann_json::nlohmann_json)
  target_compile_options(host_desktop_demo PRIVATE ${SDL2_CFLAGS_OTHER})

  # --- Scenario validation suite (headless) ---
  add_executable(scenario_suite_runner
    tests/scenario_suite_runner.cpp
    src/desktop/ScenarioRunner.cpp
  )
  target_link_libraries(scenario_suite_runner PRIVATE seedsigner_lvgl nlohmann_json::nlohmann_json)
  target_compile_definitions(scenario_suite_runner PRIVATE SOURCE_ROOT="${CMAKE_SOURCE_DIR}")
endif()

if(BUILD_TESTING)
  add_executable(seedsigner_lvgl_tests
    tests/navigation_runtime_tests.cpp
    tests/ui_runtime_smoke_test.cpp
  )

  target_link_libraries(seedsigner_lvgl_tests PRIVATE seedsigner_lvgl)

  add_test(NAME seedsigner_lvgl_tests COMMAND seedsigner_lvgl_tests)

  add_executable(screenshot_tests
    tests/screenshot_tests.cpp
  )
  target_compile_definitions(screenshot_tests PRIVATE SOURCE_ROOT="${CMAKE_SOURCE_DIR}")
  target_link_libraries(screenshot_tests PRIVATE seedsigner_lvgl)

  add_executable(input_profile_tests
    tests/input_profile_tests.cpp
  )
  target_link_libraries(input_profile_tests PRIVATE seedsigner_lvgl)
  add_test(NAME input_profile_tests COMMAND input_profile_tests)

  # Scenario suite as a test target (requires BUILD_HOST_DESKTOP for nlohmann_json)
  if(BUILD_HOST_DESKTOP)
    add_test(NAME scenario_suite
      COMMAND scenario_suite_runner
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set_tests_properties(scenario_suite PROPERTIES
      ENVIRONMENT "SCENARIO_DIR=${CMAKE_SOURCE_DIR}/scenarios;SCENARIO_OUT_DIR=${CMAKE_BINARY_DIR}/scenario_out")
  endif()
endif()
