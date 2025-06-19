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
    ${SHARED_SOURCES}
    )

target_link_libraries(synth PRIVATE SDL3-static)
target_compile_definitions(synth PRIVATE SDL_STATIC)

add_executable(gui_test 
    src/run.c
    ${SHARED_SOURCES_2}
)
target_link_libraries(gui_test PRIVATE SDL3-static SDL3_ttf-static)
target_compile_definitions(gui_test PRIVATE SDL_STATIC)

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
    target_link_libraries(gui_test PRIVATE
        # SDL3 and SDL3_ttf will pull in most of these automatically via their own
        # CMake config files (if properly set up).
        # However, for tricky cases like HarfBuzz's C++ dependency,
        # it's often necessary to explicitly add them here, AND crucially,
        # ensure stdc++ is LAST.

        # Libraries that *might* be explicitly needed or improve linker order:
        # These are commonly needed by SDL and its extensions on Windows/MinGW
        # And ensure HarfBuzz/Freetype are listed BEFORE stdc++
        harfbuzz     # Explicitly list harfbuzz
        freetype     # Explicitly list freetype

        # Other transitive dependencies from pacman installation of SDL/TTF deps
        # (These are the ones you see in your large linker command that CMake auto-discovered)
        # It's better to let SDL3_ttf's CMake handle these if it passes them along.
        # If the problem persists, you might need to manually list them here,
        # ensuring they come BEFORE stdc++.
        usp10 rpcrt4 bz2 png16 z brotlidec brotlicommon glib-2.0 intl ws2_32 shlwapi pcre2-8 graphite2

        # Core Windows system libraries
        kernel32 user32 gdi32 winmm imm32 ole32 oleaut32 version uuid advapi32 setupapi shell32 dinput8 winspool comdlg32

        # C++ Standard Library and Math Library - THESE MUST BE LAST
        stdc++ # C++ standard library
        m      # Math library (usually just '-lm' on Unix, but useful to keep in mind)
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
