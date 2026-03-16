# Eurovibes Sertest-NG Application
#
# Copyright (c) 2026 Benedikt Spranger
# SPDX-License-Identifier: Apache-2.0
#
# based on the Zephyr example application CMakeLists.txt

cmake_minimum_required(VERSION 3.13.1)
set(BOARD eurovibes_stm32g431_sertest-ng)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Configure cryptoauthlib
SET(ATCA_BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
SET(ATCA_PRINTF ON CACHE INTERNAL "")
SET(ATCA_HAL_I2C ON CACHE INTERNAL "")
SET(ATCA_NO_HEAP ON CACHE INTERNAL "")
add_subdirectory(../cryptoauthlib/lib cryptoauthlib)
include_directories(app ${CMAKE_CURRENT_BINARY_DIR}/cryptoauthlib 
                        ${CMAKE_CURRENT_SOURCE_DIR}/../cryptoauthlib
                        ${CMAKE_CURRENT_SOURCE_DIR}/../cryptoauthlib/lib
                        ${CMAKE_CURRENT_SOURCE_DIR}/../cryptoauthlib/third_party/hal/zephyr)
target_link_libraries(cryptoauth PUBLIC zephyr_interface)
target_link_libraries(app PUBLIC cryptoauth)

message("BOARD is ${BOARD}")

project(app LANGUAGES C)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

if(GIT_FOUND)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --absolute-git-dir
    WORKING_DIRECTORY .
    OUTPUT_VARIABLE application_git_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
    ERROR_VARIABLE stderr
    RESULT_VARIABLE return_code
  )

  if(NOT return_code)
    if(NOT "${stderr}" STREQUAL "")
      message(WARNING "Application build version git rev-parse warned: ${stderr}")
    endif()

    set_property(TARGET app_version_h PROPERTY APP_VERSION_DEPENDS ${application_git_dir}/index)
  endif()
endif()
