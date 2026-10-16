# SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
# SPDX-License-Identifier: GPL-3.0-only
cmake_minimum_required(VERSION 3.20)
project(Grid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(ANDROID)
    set(CMAKE_CXX_FLAGS_DEBUG "-O2 -g")
    set(CMAKE_C_FLAGS_DEBUG "-O2 -g")
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)
    set(SDLTTF_PLUTOSVG OFF CACHE BOOL "" FORCE)
    set(SDLTTF_SAMPLES OFF CACHE BOOL "" FORCE)
    set(SDLTTF_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory(${SDL3_SOURCE_DIR} SDL3 EXCLUDE_FROM_ALL)
    add_subdirectory(${SDL3_TTF_SOURCE_DIR} SDL3_ttf EXCLUDE_FROM_ALL)
else()
    find_package(SDL3 REQUIRED CONFIG)
    find_package(SDL3_ttf REQUIRED CONFIG)
endif()

add_library(grid_logic STATIC
    src/piece.cpp
    src/board.cpp
    src/score.cpp
    src/deal.cpp
    src/game.cpp
    src/format.cpp
    src/drop.cpp
    src/serialize.cpp
)
target_include_directories(grid_logic PUBLIC src)

add_library(grid_render STATIC
    src/raster.cpp
    src/gfx.cpp
    src/text.cpp
    src/sprites.cpp
    src/widgets.cpp
    src/titlebar.cpp
    src/icon.cpp
    src/draw_menu.cpp
    src/draw_game.cpp
    src/draw_over.cpp
    src/draw_settings.cpp
    src/draw_pieces.cpp
    src/settings.cpp
)
target_link_libraries(grid_render PUBLIC grid_logic SDL3::SDL3 SDL3_ttf::SDL3_ttf)

set(GRID_APP_SOURCES
    src/main.cpp
    src/app.cpp
    src/storage.cpp
)

if(ANDROID)
    add_library(main SHARED ${GRID_APP_SOURCES})
    target_link_libraries(main PRIVATE grid_render)
    return()
endif()

add_executable(grid ${GRID_APP_SOURCES})
target_link_libraries(grid PRIVATE grid_render)

file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

enable_testing()

add_executable(test_logic tests/test_logic.cpp)
target_link_libraries(test_logic PRIVATE grid_logic)
add_test(NAME logic COMMAND test_logic)

add_executable(test_frames tests/test_frames.cpp tests/frames.cpp)
target_link_libraries(test_frames PRIVATE grid_render)
add_test(NAME frames COMMAND test_frames WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
