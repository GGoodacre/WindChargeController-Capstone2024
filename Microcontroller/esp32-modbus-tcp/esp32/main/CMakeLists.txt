idf_component_register(SRCS "esp32.cpp" "main.cpp"
                    INCLUDE_DIRS ".")
component_compile_options(-Wno-error=format= -Wno-format)
