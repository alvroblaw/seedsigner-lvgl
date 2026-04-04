cmake_minimum_required(VERSION 3.20)
project(seedsigner_lvgl VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SEEDSIGNER_BUILD_EXAMPLES "Build example executables" ON)
option(SEEDSIGNER_BUILD_TESTS "Build tests" ON)
option(SEEDSIGNER_ENABLE_LVGL "Fetch and link LVGL" OFF)

add_library(seedsigner_lvgl
  src/runtime/EventQueue.cpp
  src/runtime/ScreenRegistry.cpp
  src/runtime/UiRuntime.cpp
  src/platform/HeadlessDisplay.cpp
  src/screens/PlaceholderScreen.cpp
)

target_include_directories(seedsigner_lvgl
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(seedsigner_lvgl PUBLIC cxx_std_17)

if(SEEDSIGNER_BUILD_EXAMPLES)
  add_executable(host_sim_demo examples/host_sim_demo.cpp)
  target_link_libraries(host_sim_demo PRIVATE seedsigner_lvgl)
endif()

if(SEEDSIGNER_BUILD_TESTS)
  include(CTest)
  enable_testing()

  add_executable(ui_runtime_smoke_test tests/ui_runtime_smoke_test.cpp)
  target_link_libraries(ui_runtime_smoke_test PRIVATE seedsigner_lvgl)

  add_test(NAME ui_runtime_smoke_test COMMAND ui_runtime_smoke_test)
endif()
