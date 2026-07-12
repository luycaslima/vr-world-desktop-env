cmake_minimum_required(VERSION 3.24)
project(game VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# 0. VR PLATFORM TOGGLE
# -----------------------------------------------------------------------------
option(BUILD_NATIVE_DRM "Build for headless device using DRM/KMS" OFF)

if(BUILD_NATIVE_DRM)
    set(PLATFORM "DRM" CACHE STRING "Target DRM/KMS without display server" FORCE)
else()
    set(PLATFORM "Desktop" CACHE STRING "Target Desktop (macOS/X11/Wayland)" FORCE)
endif()

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# -----------------------------------------------------------------------------
# 1. SYSTEM DEPENDENCIES (OpenXR & Vulkan)
# -----------------------------------------------------------------------------
find_package(OpenXR REQUIRED)
#find_package(Vulkan REQUIRED)

# -----------------------------------------------------------------------------
# 2. FETCH DEPENDENCIES (SDL3, Lua, Sol2, ImGui)
# -----------------------------------------------------------------------------
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE NEVER)
set(CMAKE_FETCHCONTENT_FULLY_DISCONNECTED OFF)
set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem") 
set(FetchContent_SYSTEM ON CACHE INTERNAL "Force dependencies to be system includes")

include(FetchContent)

# --- SDL3 ---
FetchContent_Declare(
    sdl3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        main # Correct stable tag RELEASE VERSION DOES NOT COME WITH OPENXR
)


set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_KMSDRM ${BUILD_NATIVE_DRM} CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(sdl3)

# --- Lua & Sol2 ---
FetchContent_Declare(lua GIT_REPOSITORY https://github.com/walterschell/Lua.git GIT_TAG master)
set(LUA_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(lua)

FetchContent_Declare(sol2 GIT_REPOSITORY https://github.com/ThePhd/sol2.git GIT_TAG main)
FetchContent_MakeAvailable(sol2)

# --- Dear ImGui ---
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        master
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
        #${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp # Switched to Vulkan backend
    )

    target_include_directories(imgui PUBLIC  
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui PRIVATE SDL3::SDL3 ) # Added Vulkan link
endif()


# -----------------------------------------------------------------------------
# 3. SOURCE FILES & TARGET
# -----------------------------------------------------------------------------
file(GLOB_RECURSE SOURCES 
    "${SRC_DIR}/*.c"
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.hpp"
    "${SRC_DIR}/*.h"
)

#Exclude the main example of OpenXr Support
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main_example_xr.c")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${SRC_DIR}
    ${lua_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    SDL3::SDL3
    OpenXR::openxr_loader
    #Vulkan::Vulkan # Added Vulkan link
    lua_static   
    sol2 
    imgui
)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    SOL_ALL_SAFETIES_ON=1
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -Wall 
        -Wextra 
        $<$<COMPILE_LANGUAGE:CXX>:-ftemplate-backtrace-limit=0>
        -Wno-unknown-pragmas
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES NO_SYSTEM_FROM_IMPORTED OFF)
endif()

# -----------------------------------------------------------------------------
# 4. ASSETS COPY
# -----------------------------------------------------------------------------
set(ASSETS_FOLDERS data assets)
foreach(FOLDER ${ASSETS_FOLDERS})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${FOLDER}")
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/${FOLDER}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${FOLDER}"
        )
    endif()
endforeach()