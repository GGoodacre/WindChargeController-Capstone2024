idf_component_register(SRCS "HardwareControl.cpp" "PowerSupplyControl.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES "arduino" "PowerElectronics" "SPImaster" "QuickPID")
