cmake_minimum_required(VERSION 3.15)
project(synth C)

set(CMAKE_C_STANDARD 11)

# Prefer static libraries
# set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib" ".so" ".dylib")

set(CMAKE_FIND_LIBRARY_SUFFIXES .a .lib ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(BUILD_SHARED_LIBS OFF)

add_subdirectory(third_party/sdl3)

# Source files
set(SHARED_SOURCES
        src/backend/audio.c
        src/backend/utils.c
        src/backend/waves.c
        src/backend/filters.c
        src/backend/notes.c
)

add_executable(synth 
    src/synth.c
    ${SHARED_SOURCES}
    )

target_link_libraries(synth PRIVATE SDL3-static)
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
)

set_target_properties(synthcore PROPERTIES
    PREFIX ""
    OUTPUT_NAME "synthcore"
)

target_include_directories(synthcore PUBLIC
    ${CMAKE_SOURCE_DIR}/src/backend
)

# On Windows you may need to export symbols; if so, define an export macro:
# target_compile_definitions(synthcore PRIVATE EXPORT_FOR_PYTHON)

# find the Python interpreter
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# make sure Python can find synthcore in the build dir
set(ENV{PYTHONPATH} "${CMAKE_BINARY_DIR}")

add_custom_target(python-test
  COMMAND ${Python3_EXECUTABLE}
          ${CMAKE_SOURCE_DIR}/test/test_filters.py
  DEPENDS synthcore
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Running Python filter unit tests…"
)
