# ==================================================================================================
# Android vendor library wiring
#
# The desktop builds (Windows/Linux/macOS) get SDL2/OpenAL/mpg123 through Conan
# or system packages, which populate the normal find_package() search paths
# automatically. Android has no Conan profile here, so this file points the
# very same find_package() calls used by src/CMakeLists.txt at the prebuilt,
# per-ABI binaries already vendored under vendor/<lib>/.../Android/${ANDROID_ABI}/.
#
# Included from android/launcher/app/CMakeLists.txt, before add_subdirectory()
# pulls in the game module.
# ==================================================================================================

if(NOT ANDROID)
    return()
endif()

if(NOT DEFINED ANDROID_ABI OR ANDROID_ABI STREQUAL "")
    message(FATAL_ERROR "AndroidConfig.cmake: ANDROID_ABI is not set")
endif()

get_filename_component(RE3_VENDOR_DIR "${CMAKE_CURRENT_LIST_DIR}/../../vendor" ABSOLUTE)

message(STATUS "AndroidConfig: wiring vendor libraries for ABI ${ANDROID_ABI} from ${RE3_VENDOR_DIR}")

# librw defaults to GLFW (vendor/librw/CMakeLists.txt) -- there's no GLFW on
# Android, so it has to be forced to its SDL2 backend instead, before
# add_subdirectory(vendor/librw) (pulled in from the root CMakeLists.txt via
# add_subdirectory(${RE3_GAME_DIR}) right after this file is included)
# evaluates that default. Without this, librw builds against GLFW headers
# that don't exist here, and separately -- since src/CMakeLists.txt's own
# LIBRW_SDL2 define (added for the game target only) makes librw's rwgl3.h
# take its "#include <SDL.h>" branch regardless -- the game target would
# still be missing SDL2's include dir, which only librw's own SDL2::SDL2
# link (PUBLIC, transitive) actually provides.
#
# LIBRW_PLATFORM itself also needs forcing: librw's own default (first entry
# of its platform list for a generic/"else" OS, which Android falls into) is
# "NULL", not "GL3" -- the game target defines RW_GL3 unconditionally for
# Android (src/CMakeLists.txt) regardless of what librw was actually built
# as, so a mismatch here would build librw's .o files against one set of
# types/API and re3's against another.
set(LIBRW_PLATFORM "GL3" CACHE STRING "" FORCE)
set(LIBRW_GL3_GFXLIB "SDL2" CACHE STRING "" FORCE)

# --- SDL2 (consumed by vendor/librw's own FindSDL2.cmake) -----------------------------------
set(SDL2_INCLUDE_DIR "${RE3_VENDOR_DIR}/sdl2/include" CACHE PATH "" FORCE)
set(SDL2_LIBRARY "${RE3_VENDOR_DIR}/sdl2/libs/Android/${ANDROID_ABI}/libSDL2.so" CACHE FILEPATH "" FORCE)

# --- OpenAL (consumed by CMake's builtin FindOpenAL.cmake) ----------------------------------
set(OPENAL_INCLUDE_DIR "${RE3_VENDOR_DIR}/openal-soft/include" CACHE PATH "" FORCE)
set(OPENAL_LIBRARY "${RE3_VENDOR_DIR}/openal-soft/libs/Android/${ANDROID_ABI}/libopenal.so" CACHE FILEPATH "" FORCE)

# --- mpg123 (consumed by cmake/Findmpg123.cmake) ---------------------------------------------
set(mpg123_INCLUDE_DIR "${RE3_VENDOR_DIR}/mpg123/include" CACHE PATH "" FORCE)
set(mpg123_LIBRARIES "${RE3_VENDOR_DIR}/mpg123/lib/Android/${ANDROID_ABI}/libmpg123.so" CACHE FILEPATH "" FORCE)

foreach(_re3_check IN ITEMS SDL2_LIBRARY OPENAL_LIBRARY mpg123_LIBRARIES)
    if(NOT EXISTS "${${_re3_check}}")
        message(FATAL_ERROR "AndroidConfig: expected prebuilt library not found: ${${_re3_check}}")
    endif()
endforeach()
