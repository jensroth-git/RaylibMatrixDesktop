cmake_minimum_required(VERSION 3.16)
project(RaylibMatrixDesktop VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection
set(PLATFORM_WINDOWS FALSE)
set(PLATFORM_MACOS FALSE)
set(PLATFORM_LINUX FALSE)

if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
    enable_language(OBJC)
    enable_language(OBJCXX)
elseif(UNIX)
    set(PLATFORM_LINUX TRUE)
endif()

# Find or build raylib
find_package(raylib 5.0 QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
    )

    FetchContent_MakeAvailable(raylib)
endif()

# Create the executable
set(SOURCES
    src/main.cpp
    src/matrix_rain.cpp
    src/matrix_rain.h
    src/hsl_rgb.h
    src/asset_loader.h
    src/asset_loader.cpp
)

# Platform-specific sources
if(PLATFORM_WINDOWS)
    list(APPEND SOURCES
        src/platform/windows/desktop_integration_windows.cpp
        src/platform/desktop_integration.h
    )
elseif(PLATFORM_MACOS)
    list(APPEND SOURCES
        src/platform/macos/desktop_integration_macos.mm
        src/platform/desktop_integration.h
    )
elseif(PLATFORM_LINUX)
    list(APPEND SOURCES
        src/platform/linux/desktop_integration_linux.cpp
        src/platform/desktop_integration.h
    )
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} raylib)

# Platform-specific linking
if(PLATFORM_WINDOWS)
    target_link_libraries(${PROJECT_NAME} 
        dwmapi 
        shcore
        user32
        gdi32
    )
elseif(PLATFORM_MACOS)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    find_library(OPENGL_LIBRARY OpenGL)
    find_library(CARBON_LIBRARY Carbon)
    
    target_link_libraries(${PROJECT_NAME}
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
        ${OPENGL_LIBRARY}
        ${CARBON_LIBRARY}
    )
elseif(PLATFORM_LINUX)
    find_package(X11 REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(XRANDR REQUIRED xrandr)

    target_link_libraries(${PROJECT_NAME}
        ${X11_LIBRARIES}
        ${XRANDR_LIBRARIES}
    )

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${XRANDR_INCLUDE_DIRS}
        ${X11_INCLUDE_DIR}
    )
endif()

# Compiler-specific options
if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
    )
    target_compile_options(${PROJECT_NAME} PRIVATE /W3)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
    
    # Add debug flags for better debugging experience
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${PROJECT_NAME} PRIVATE -g -O0)
    endif()
    
    # Special flags for Objective-C++ on macOS
    if(PLATFORM_MACOS)
        target_compile_options(${PROJECT_NAME} PRIVATE 
            $<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>
            $<$<COMPILE_LANGUAGE:OBJCXX>:-fno-objc-exceptions>
        )
        
        # Ensure debug symbols for .mm files
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(${PROJECT_NAME} PRIVATE 
                $<$<COMPILE_LANGUAGE:OBJCXX>:-gdwarf-2>
            )
        endif()
    endif()
endif()

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy assets to the correct location for each platform
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/bin/assets
)

if(PLATFORM_MACOS)
    # For macOS app bundles, also copy assets into the app bundle Resources directory
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME}.app/Contents/Resources
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME}.app/Contents/Resources
    )
endif()

# Platform-specific properties
if(PLATFORM_MACOS)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.raylibmatrixdesktop"
        MACOSX_BUNDLE_BUNDLE_NAME "Matrix Desktop"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    )
endif() 