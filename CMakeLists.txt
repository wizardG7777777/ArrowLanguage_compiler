cmake_minimum_required(VERSION 3.16)

# Project configuration
project(ArrowLanguage_compiler 
    VERSION 1.0.0
    DESCRIPTION "Compiler for Arrow mini-language"
    LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Platform-specific configurations for Windows
if(WIN32)
    # Ensure we're using the correct runtime library
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    
    # Add Windows-specific definitions
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

# Compiler-specific options
if(MSVC)
    # Microsoft Visual C++ compiler options
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Od /Zi")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2")
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    # GCC or Clang compiler options (for MinGW on Windows)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -DNDEBUG")
endif()

# Create the main executable
add_executable(runml runml.c)

# Set output directory
set_target_properties(runml PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    OUTPUT_NAME "runml"
)

# Installation rules
install(TARGETS runml
    RUNTIME DESTINATION bin
    COMPONENT Runtime
)

# Optional: Add custom target for testing
if(EXISTS "${CMAKE_SOURCE_DIR}/testExamples.zip")
    # Extract test files if zip exists
    find_program(POWERSHELL_PATH powershell.exe)
    if(POWERSHELL_PATH)
        add_custom_target(extract_tests
            COMMAND ${POWERSHELL_PATH} -Command 
                "Add-Type -AssemblyName System.IO.Compression.FileSystem; [System.IO.Compression.ZipFile]::ExtractToDirectory('${CMAKE_SOURCE_DIR}/testExamples.zip', '${CMAKE_BINARY_DIR}/tests')"
            COMMENT "Extracting test files..."
            VERBATIM
        )
        
        # Add dependency so tests are extracted when building
        add_dependencies(runml extract_tests)
    endif()
endif()

# Custom target for running a test
add_custom_target(run_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/runml test.ml
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    DEPENDS runml
    COMMENT "Running Arrow language compiler test"
)

# Print configuration information
message(STATUS "Arrow Language Compiler Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "  C Standard: C${CMAKE_C_STANDARD}")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
if(WIN32)
    message(STATUS "  Windows version detected")
endif()
