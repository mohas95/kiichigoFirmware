cmake_minimum_required(VERSION 3.13)

# initialize pico-sdk from GIT
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_FETCH_FROM_GIT on)

# pico_sdk_import.cmake is a single file copied from this SDK
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(kiichigoFirmware)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

# generate an executable from our source file
add_executable(${PROJECT_NAME} main.cpp)

# link our executable with pico libraries
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib 
    hardware_pwm
)

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${PROJECT_NAME})
