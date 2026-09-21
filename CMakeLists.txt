cmake_minimum_required(VERSION 3.24)
project(game VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# 0. PLATFORM & VR TOGGLES
# -----------------------------------------------------------------------------
option(BUILD_NATIVE_DRM "Build for headless device using DRM/KMS" OFF)
option(ENABLE_OPENXR "Enable OpenXR / VR support" OFF)

if(BUILD_NATIVE_DRM)
    set(PLATFORM "DRM" CACHE STRING "Target DRM/KMS without display server" FORCE)
else()
    set(PLATFORM "Desktop" CACHE STRING "Target Desktop (macOS/X11/Wayland)" FORCE)
endif()

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# -----------------------------------------------------------------------------
# 1. SYSTEM DEPENDENCIES (OpenXR & Vulkan)
# -----------------------------------------------------------------------------
if(ENABLE_OPENXR)
    find_package(OpenXR REQUIRED)
endif()

#GTK CONFIG
# 2. Find the pkg-config tool
# find_package(PkgConfig REQUIRED)
# # 3. Use pkg-config to locate GTK4 and create an imported target
# pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

# -----------------------------------------------------------------------------
# 2. FETCH DEPENDENCIES (SDL3, Lua, Sol2, ImGui)
# -----------------------------------------------------------------------------
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE NEVER)
set(CMAKE_FETCHCONTENT_FULLY_DISCONNECTED OFF)
set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem") 
set(FetchContent_SYSTEM OFF CACHE INTERNAL "Force dependencies to be normal includes")

include(FetchContent)

# --- SDL3 ---
FetchContent_Declare(
    sdl3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        cc2713c0477118d3074b625abe8df7de7118fe2e
)

set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_KMSDRM ${BUILD_NATIVE_DRM} CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(sdl3)

# --- Lua & Sol2 ---
FetchContent_Declare(
    lua 
    GIT_REPOSITORY https://github.com/walterschell/Lua.git 
    GIT_TAG     504ef66d500fa1fb4f1684b6617b01342eee704a
)
set(LUA_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(lua)

FetchContent_Declare(
    sol2 
    GIT_REPOSITORY https://github.com/ThePhd/sol2.git 
    GIT_TAG c1f95a773c6f8f4fde8ca3efe872e7286afe4444)
FetchContent_MakeAvailable(sol2)

# --- Dear ImGui ---
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        b335bf4328054498219afb4188ec39912f2a95b1
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
    )

    target_include_directories(imgui PUBLIC  
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui PRIVATE SDL3::SDL3)
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

# Exclude standalone VR example if OpenXR is disabled
if(NOT ENABLE_OPENXR)
    list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main_example_xr.c")
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${SRC_DIR})

# Conditionally link OpenXR using modern CMake generator expressions
target_link_libraries(${PROJECT_NAME} PRIVATE 
    SDL3::SDL3
    #PkgConfig::GTK4
    $<$<BOOL:${ENABLE_OPENXR}>:OpenXR::openxr_loader>
    lua_static   
    sol2 
    imgui
)

# Pass a preprocessor definition so your C++ code knows if VR is available
if(ENABLE_OPENXR)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_OPENXR=1)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE SOL_ALL_SAFETIES_ON=1)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -Wall 
        -Wextra 
        $<$<COMPILE_LANGUAGE:CXX>:-ftemplate-backtrace-limit=0>
        -Wno-unknown-pragmas
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES NO_SYSTEM_FROM_IMPORTED OFF)
endif()

#commands glslc circle.frag -o circle.frag.spv 
#spirv-cross --msl ./shaders/glsl450/sdf/circle.frag.spv --output ./shaders/glsl450/sdf/circle.frag.msl

# find_program(GLSLC glslc)
# find_program(SPIRV_CROSS spirv-cross)

# function(compile_shader SHADER_NAME STAGE)
#     set(GLSL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/glsl450/${SHADER_NAME}")
#     set(SPV_OUT "${CMAKE_CURRENT_BINARY_DIR}/assets/shaders/compiled/SPIRV/${SHADER_NAME}.spv")
#     set(MSL_OUT "${CMAKE_CURRENT_BINARY_DIR}/asset/shaders/compiled/MSL/${SHADER_NAME}.msl")

#     # Compile GLSL to SPIR-V
#     add_custom_command(
#         OUTPUT ${SPV_OUT}
#         COMMAND ${GLSLC} ${GLSL_SRC} -o ${SPV_OUT}
#         DEPENDS ${GLSL_SRC}
#     )

#     # Transpile SPIR-V to MSL on Apple platforms
#     if(APPLE)
#         add_custom_command(
#             OUTPUT ${MSL_OUT}
#             COMMAND ${SPIRV_CROSS} --msl ${SPV_OUT} --output ${MSL_OUT}
#             DEPENDS ${SPV_OUT}
#         )
#         list(APPEND SHADER_OUTPUTS ${MSL_OUT})
#     else()
#         list(APPEND SHADER_OUTPUTS ${SPV_OUT})
#     endif()

#     add_custom_target(${SHADER_NAME}_target ALL DEPENDS ${SHADER_OUTPUTS})
# endfunction()

# compile_shader("sdf_circle.vert" "vert")
# compile_shader("sdf_circle.frag" "frag")


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