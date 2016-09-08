cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
set(PROJECT_NAME arduino)
project(${PROJECT_NAME})

set(${CMAKE_PROJECT_NAME}_SKETCH arduino.ino)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/libraries)

generate_arduino_library(Adafruit_PWMServoDriver
        SRCS libraries/Adafruit_PWMServoDriver/Adafruit_PWMServoDriver.cpp
        HDRS libraries/Adafruit_PWMServoDriver/Adafruit_PWMServoDriver.h
        BOARD uno)

generate_arduino_firmware(${CMAKE_PROJECT_NAME}
        SRCS arduino.ino
        LIBS Adafruit_PWMServoDriver
        PORT /dev/ttyACM0
        SERIAL picocom @SERIAL_PORT@ 
        BOARD uno
        )

