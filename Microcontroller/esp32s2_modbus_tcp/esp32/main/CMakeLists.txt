idf_component_register(SRCS "modbus_tcp_master.cpp" "main.cpp"
                    INCLUDE_DIRS ".")
component_compile_options(-Wno-error=format= -Wno-format)
