cmake_minimum_required(VERSION 3.13)

project(SDLiOSTest VERSION 0.1.0)

# Dependencies
set(INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/inc/sdl2")
set(LINK_LIBS
    "-framework AudioToolbox"
    "-framework AVFoundation"
    "-framework Foundation"
    "-framework CoreAudio"
    "-framework CoreGraphics"
    "-framework CoreHaptics"
    "-framework CoreMotion"
    "-framework GameController"
    "-framework Metal"
    "-framework OpenGLES"
    "-framework QuartzCore"
    "-framework UIKit")

# Target-specific dependencies
if(PLATFORM STREQUAL "OS64") # iOS
    list(APPEND LINK_LIBS "${PROJECT_SOURCE_DIR}/lib/libSDL2-ios.a")
elseif(PLATFORM STREQUAL "SIMULATOR64") # iOS Simulator
    list(APPEND LINK_LIBS "${PROJECT_SOURCE_DIR}/lib/libSDL2-ios-sim.a")
else()
    message(FATAL_ERROR "Could not detect platform!")
endif()

# Target
add_executable(${PROJECT_NAME} src/main.cpp)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LINK_LIBS}
)

# Compile options
if(WIN32)
set(COMPILE_OPTIONS "-DNOCOLOR" "-DNOMINMAX")
else()
set(COMPILE_OPTIONS "-Wall" "-Wextra" "-Wpedantic")
endif()
target_compile_options(${PROJECT_NAME} PUBLIC ${COMPILE_OPTIONS})

# Apple Bundle (iOS and macOS)
if(APPLE)
    # Info.plist template
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/res/Info.plist.in")

    # Bundle properties
    set(MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.test.${PROJECT_NAME}")

    # Xcode properties
    set(CMAKE_XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${MACOSX_BUNDLE_GUI_IDENTIFIER})
endif()
