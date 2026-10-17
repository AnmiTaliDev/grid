# SPDX-FileCopyrightText: 2026 AnmiTaliDev <anmitalidev@nuros.org>
# SPDX-License-Identifier: GPL-3.0-only
cmake_minimum_required(VERSION 3.20)
set(GRID_VERSION "0.1.0" CACHE STRING "Package version")
project(Grid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    add_compile_options(/utf-8)
endif()

if(SDL3_SOURCE_DIR AND SDL3_TTF_SOURCE_DIR)
    if(ANDROID)
        set(CMAKE_CXX_FLAGS_DEBUG "-O2 -g")
        set(CMAKE_C_FLAGS_DEBUG "-O2 -g")
        set(SDL_SHARED ON CACHE BOOL "" FORCE)
        set(SDL_STATIC OFF CACHE BOOL "" FORCE)
        set(BUILD_SHARED_LIBS ON)
    else()
        set(SDL_SHARED OFF CACHE BOOL "" FORCE)
        set(SDL_STATIC ON CACHE BOOL "" FORCE)
        set(BUILD_SHARED_LIBS OFF)
    endif()
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
    src/bests.cpp
    src/kv.cpp
    src/settings_format.cpp
    src/sound_names.cpp
    src/pulses.cpp
    src/stats.cpp
    src/tutorial.cpp
    src/i18n.cpp
    src/mixer.cpp
)
target_include_directories(grid_logic PUBLIC src)

add_library(grid_synth STATIC
    src/synth.cpp
    src/wav.cpp
)
target_include_directories(grid_synth PUBLIC src)

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
    src/draw_stats.cpp
    src/draw_tutorial.cpp
    src/settings.cpp
    src/audio.cpp
    src/assets.cpp
    src/haptics.cpp
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

add_executable(make_sounds tools/make_sounds.cpp)
target_link_libraries(make_sounds PRIVATE grid_synth grid_logic)

add_executable(test_logic tests/test_logic.cpp)
target_link_libraries(test_logic PRIVATE grid_logic grid_synth)
target_compile_definitions(test_logic PRIVATE GRID_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets")
add_test(NAME logic COMMAND test_logic)

add_executable(test_sounds tests/test_sounds.cpp)
target_link_libraries(test_sounds PRIVATE grid_synth grid_logic SDL3::SDL3)
target_compile_definitions(test_sounds PRIVATE GRID_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets")
add_test(NAME sounds COMMAND test_sounds)

add_executable(test_frames tests/test_frames.cpp tests/frames.cpp)
target_link_libraries(test_frames PRIVATE grid_render)
add_test(NAME frames COMMAND test_frames WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

include(GNUInstallDirs)
install(TARGETS grid RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY assets/ DESTINATION ${CMAKE_INSTALL_DATADIR}/grid/assets)
install(FILES packaging/dev.anmitali.grid.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES assets/icon.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
        RENAME dev.anmitali.grid.svg)
install(FILES LICENSE assets/OFL.txt DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/grid)

set(CPACK_PACKAGE_NAME "grid")
set(CPACK_PACKAGE_VERSION "${GRID_VERSION}")
set(CPACK_PACKAGE_CONTACT "AnmiTaliDev <anmitalidev@nuros.org>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Block puzzle game")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_STRIP_FILES OFF)
set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-only")
set(CPACK_RPM_PACKAGE_GROUP "Amusements/Games")
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)
set(CPACK_RPM_PACKAGE_AUTOREQ ON)
set(CPACK_GENERATOR "DEB;RPM")
include(CPack)
