idf_component_register(
    SRCS
        "main.cpp"
        "../../../src/components/TopNavBar.cpp"
        "../../../src/screens/MenuListScreen.cpp"
        "../../../src/visual/SeedSignerTheme.cpp"
        "../../../src/assets/icons.c"
    INCLUDE_DIRS
        "."
        "../../../include"
        "../../../src/assets"
    REQUIRES
        esp_lvgl_port
)

target_compile_features(${COMPONENT_LIB} PRIVATE cxx_std_17)
