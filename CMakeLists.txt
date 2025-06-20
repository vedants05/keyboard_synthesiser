cmake_minimum_required(VERSION 3.15)
project(synth C)

set(CMAKE_C_STANDARD 11)

# Prefer static libraries
# set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib" ".so" ".dylib")

set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(BUILD_SHARED_LIBS OFF) 

add_subdirectory(third_party/sdl3)
add_subdirectory(third_party/sdl_ttf-main)
# Source files
set(SHARED_SOURCES
        src/backend/audio.c
        src/backend/utils.c
        src/backend/waves.c
        src/backend/filters.c
        src/backend/notes.c
)

set(SHARED_SOURCES_2
        src/frontend/button.c
        src/frontend/keyboard.c
        src/frontend/slider.c
        src/frontend/utils.c
)

add_executable(synth 
    src/synth.c
    src/run.c
    ${SHARED_SOURCES}
    ${SHARED_SOURCES_2}
    )

target_link_libraries(synth PRIVATE SDL3-static SDL3_ttf-static)
target_compile_definitions(synth PRIVATE SDL_STATIC)

# Optional test
option(BUILD_KEYBOARD_TEST "Build keyboard test" OFF)
option(BUILD_SOUND_TEST "Build sound test" OFF)
option(BUILD_WAVE_TEST "Build wave test" OFF)


if(WIN32)
    target_link_libraries(synth PRIVATE
        winmm      # Windows Multimedia API
        imm32      # Input Method Manager
        setupapi   # Device driver setup
        version    # Version checking
        ole32      # OLE/COM
        advapi32   # Registry access
        user32     # User interface
        gdi32      # Graphics
        shell32    # Shell API
    )

endif()

if(APPLE)
    # Find required packages for text rendering on macOS
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(HARFBUZZ REQUIRED harfbuzz)
    pkg_check_modules(FREETYPE REQUIRED freetype2)

    target_link_libraries(synth PRIVATE
            ${HARFBUZZ_LIBRARIES}
            ${FREETYPE_LIBRARIES}
        )

    target_include_directories(synth PRIVATE
            ${HARFBUZZ_INCLUDE_DIRS}
            ${FREETYPE_INCLUDE_DIRS}
        )

    target_link_directories(synth PRIVATE
            ${HARFBUZZ_LIBRARY_DIRS}
            ${FREETYPE_LIBRARY_DIRS}
        )

else()
    # For Windows and other platforms
    target_link_libraries(synth PRIVATE
            harfbuzz
            freetype  
        )
endif()

if(BUILD_KEYBOARD_TEST)
    add_executable(keyboard_test test/test_keyboard.c)
    target_link_libraries(keyboard_test PRIVATE SDL3-static)
    target_compile_definitions(keyboard_test PRIVATE SDL_STATIC)
endif()


if(BUILD_SOUND_TEST)
    add_executable(sound_test test/test_sound.c)
    target_link_libraries(sound_test PRIVATE SDL3-static)
    target_compile_definitions(sound_test PRIVATE SDL_STATIC)
endif()

if(BUILD_WAVE_TEST)
    add_executable(wave_test 
        test/test_waves.c
        ${SHARED_SOURCES}
    )
    target_link_libraries(wave_test PRIVATE SDL3-static)
    target_compile_definitions(wave_test PRIVATE SDL_STATIC)
endif()


# A shared library for Python tests

add_library(synthcore SHARED
    src/backend/filters.c
    src/backend/waves.c
)

set_target_properties(synthcore PROPERTIES
    PREFIX ""
    OUTPUT_NAME "synthcore"
)

# Expose the headers for ctypes consumers
target_include_directories(synthcore PUBLIC
    ${CMAKE_SOURCE_DIR}/src/backend
)

# Link against SDL3 so we get the include paths (and definitions) too
target_link_libraries(synthcore PRIVATE SDL3-static)
target_compile_definitions(synthcore PRIVATE SDL_STATIC)
