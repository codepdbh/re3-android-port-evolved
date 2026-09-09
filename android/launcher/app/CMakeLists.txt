# ==================================================================================================
# 📱 re3 – Mobile Edition (Android) – Root CMake Configuration
#
# Description:
# This file defines the root CMake configuration for the re3 – Mobile Edition,
# targeting the Android platform via the Android NDK. It initializes the build system
# for the mobile game, integrates shared cross-platform sources, configures compiler settings,
# and handles dependency resolution.
# ==================================================================================================

cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

message(" ")
message("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(" *  🌐  re3 – Build Configuration Initialized")
message(" *  🔧  Preparing Android platform modules and environment configuration...")
message("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(" ")

# ────────────────────────────────────────────────────────────────────────────────
# 🔖 Project Metadata Setup
# ────────────────────────────────────────────────────────────────────────────────
set(RE3_PROJECT_NAME "re3")
set(RE3_VERSION 0.0.1)
set(RE3_DESCRIPTION "reverted ${RE3_PROJECT_NAME}")
set(RE3_LIB_NAME re3)

project(${RE3_PROJECT_NAME}
        VERSION ${RE3_VERSION}
        LANGUAGES C CXX
        DESCRIPTION ${RE3_DESCRIPTION}
)

# ────────────────────────────────────────────────────────────────────────────────
# 📂 Define Important Directory Paths
# ────────────────────────────────────────────────────────────────────────────────
get_filename_component(RE3_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
set(RE3_GAME_DIR "${RE3_ROOT_DIR}/../../../")
message(" *  📂 Game source directory set to: ${RE3_GAME_DIR}")

# ────────────────────────────────────────────────────────────────────────────────
# ➕ Add Game Subdirectory If Available
# ────────────────────────────────────────────────────────────────────────────────
if(IS_DIRECTORY "${RE3_GAME_DIR}" AND EXISTS "${RE3_GAME_DIR}/CMakeLists.txt")
    message(" *  ➕ Including Game module from: ${RE3_GAME_DIR}")
    message(" ")
    message("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    message(" *  🚀 Initiating Game module build configuration...")
    message("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    message(" ")

    # Wire SDL2/OpenAL/mpg123 to the prebuilt per-ABI vendor binaries before
    # the game module's find_package() calls run.
    list(APPEND CMAKE_MODULE_PATH "${RE3_GAME_DIR}/cmake/android")
    include(AndroidConfig)

    add_subdirectory(${RE3_GAME_DIR} ${CMAKE_BINARY_DIR}/game_build)
else()
    message(" *  ⚠️ Game source directory or CMakeLists.txt not found at: ${RE3_GAME_DIR}")
    message(FATAL_ERROR " *  ❗ Please ensure the game module sources are available for the build.")
endif()
