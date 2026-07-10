cmake_minimum_required(VERSION 3.24) # 3.24+ adds stable FetchContent optimization
project(game VERSION 1.0.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

#For the ARM Compilation error glDrawBuffersEXT
add_compile_definitions(glDrawBuffersEXT=glDrawBuffers)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 1. Define a toggleable option (Defaults to ON)
option(BUILD_DESKTOP "Build for Desktop PCVR (WiVRn/OpenXR)" ON)

# TODO change raylib to SDL3

# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # 1. Fetch SDL3 directly from the main branch (required for the new XR features)
# include(FetchContent)
# FetchContent_Declare(
#     SDL3
#     GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
#     GIT_TAG        main
# )
# FetchContent_MakeAvailable(SDL3)

# # 2. Find the system OpenXR loader (Install via: sudo apt install libopenxr-loader-dev)
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(OPENXR REQUIRED openxr)

# # 3. Define the executable
# add_executable(vr_launcher src/main.cpp)

# # 4. Include directories and link libraries
# target_include_directories(vr_launcher PRIVATE ${OPENXR_INCLUDE_DIRS})
# target_link_directories(vr_launcher PRIVATE ${OPENXR_LIBRARY_DIRS})

# # Link SDL3 statically or dynamically (SDL3::SDL3) and the OpenXR loader
# target_link_libraries(vr_launcher PRIVATE 
#     SDL3::SDL3
# )


# -----------------------------------------------------------------------------
# 0. VR PLATFORM TOGGLE
# -----------------------------------------------------------------------------
option(BUILD_NATIVE_DRM "Build for headless device using DRM/KMS" OFF)

if(BUILD_NATIVE_DRM)
    set(PLATFORM "DRM" CACHE STRING "Target DRM/KMS without display server" FORCE)
    # The Pi 4 and Radxa Q6A support GLES 3.2. 
    # ES 3.0+ is highly recommended for complex custom barrel distortion GLSL shaders.
    set(OPENGL_VERSION "ES 3.0" CACHE STRING "Target OpenGL ES" FORCE) 
else()
    set(PLATFORM "Desktop" CACHE STRING "Target Desktop (macOS/X11/Wayland)" FORCE)
endif()

# Define project structures
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)


# -----------------------------------------------------------------------------
# 1. FETCH DEPENDENCIES (Raylib, Lua, Sol2, ImGui) WITH STABLE VERSION TAGS
# -----------------------------------------------------------------------------
#SYSTEM is added on the fetchcontent to be tretaed as EXTERNAL libraries so it wont polute the warnings , remove if breaks
# Force all FetchContent dependencies to be treated as SYSTEM includes to hide warnings
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE NEVER)
set(CMAKE_FETCHCONTENT_FULLY_DISCONNECTED OFF)

# Add this line right before you declare or populate your dependencies:
set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem") 
set(FetchContent_SYSTEM ON CACHE INTERNAL "Force dependencies to be system includes")

include(FetchContent)

# --- Raylib (Pinned to v5.5) ---
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
)
set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

# --- Lua (Pinned to v5.4.7 commit/stable branch wrapper) ---
FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/walterschell/Lua.git
    GIT_TAG        master  # Note: This specific repository uses the master branch as its 5.4 stable version wrapper.
)
set(LUA_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(lua)

# --- Sol2 (Pinned to main for clang compatibility issues) ---
FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhd/sol2.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(sol2)


# --- Dear ImGui Core (Required by rlImGui) ---
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.5 # Excellent stable companion for Raylib 5.5
)
FetchContent_MakeAvailable(imgui)

# --- rlImGui Integration ---
FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG        Raylib_5_5
)
FetchContent_GetProperties(rlimgui)
if(NOT rlimgui_POPULATED)
    FetchContent_Populate(rlimgui)
    
    add_library(rlImGui STATIC
        ${rlimgui_SOURCE_DIR}/rlImGui.cpp
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
    )

    # Changed from PRIVATE to PUBLIC so your game files can see these headers!
    target_include_directories(rlImGui PUBLIC  
        ${rlimgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}
    )
    
    target_link_libraries(rlImGui PRIVATE raylib)
endif()


# -----------------------------------------------------------------------------
# 2. SOURCE FILES
# -----------------------------------------------------------------------------
# Recursively find all source files inside src/
file(GLOB_RECURSE SOURCES 
    "${SRC_DIR}/*.c"
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.hpp"
    "${SRC_DIR}/*.h"
)

# -----------------------------------------------------------------------------
# 3. TARGET & COMPILER FLAGS
# -----------------------------------------------------------------------------
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${SRC_DIR}
    ${lua_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    raylib 
    lua_static   
    sol2 
    rlImGui
)

if(BUILD_DESKTOP)
    message(STATUS "Configuring for Desktop OpenXR Mode...")

    # 2. Add the rlxr subdirectory
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/include/rlxr)
    
    # 3. Find the system OpenXR loader (Required for WiVRn communication)
    find_package(OpenXR REQUIRED)
    
    # 4. Link rlxr and the OpenXR loader to your executable
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        rlxr 
        OpenXR::openxr_loader
    )
    
    # 5. Pass a macro to your C/C++ code so it knows we are in VR Desktop mode
    #target_compile_definitions(${PROJECT_NAME} PRIVATE VR_DESKTOP_MODE)

else()
    message(STATUS "Configuring for Bare-Metal DRM Mode...")
    
    # Your future Raspberry Pi / Radxa native Wayland/DRM configuration will go here
    # target_compile_definitions(${PROJECT_NAME} PRIVATE NATIVE_DRM_MODE)

endif()

#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/include/rlxr)
#target_link_libraries(${PROJECT_NAME} PRIVATE rlxr)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    SOL_ALL_SAFETIES_ON=1
)

# Enable compiler warnings, but cap template backtraces so we can see the real errors
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    #target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -ftemplate-backtrace-limit=0)
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -Wall 
        -Wextra 
        $<$<COMPILE_LANGUAGE:CXX>:-ftemplate-backtrace-limit=0>
        -Wno-unknown-pragmas      # Suppresses noise from external headers macro warnings
    )

    # This magic target property forces CMake to honor the SYSTEM status of dependencies 
    # even when they leak into your local source files via template expansions.
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        NO_SYSTEM_FROM_IMPORTED OFF
    )
endif()

# -----------------------------------------------------------------------------
# 4. ASSETS & DATA COPY (macOS Bundling safe)
# -----------------------------------------------------------------------------
# This copies your data and assets to the build folder whenever they change
set(ASSETS_FOLDERS data assets)

foreach(FOLDER ${ASSETS_FOLDERS})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${FOLDER}")
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/${FOLDER}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${FOLDER}"
            COMMENT "Syncing ${FOLDER} directory to build output..."
        )
    endif()
endforeach()