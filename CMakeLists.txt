cmake_minimum_required(VERSION 3.27)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(SF_LIBRARY_TYPE STATIC)
if (BUILD_SHARED_LIBS)
    set(SF_LIBRARY_TYPE SHARED)
endif()

project(sf-gfx C)

add_library(sf-gfx ${SF_LIBRARY_TYPE}
    src/camera.c
    src/window.c
    src/shaders.c
    src/meshes.c
    src/textures.c
)
target_include_directories(sf-gfx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(stb ${SF_LIBRARY_TYPE}
    src/stb_image.c
)
target_include_directories(stb PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)


# Fetch Dependencies
include(FetchContent)
if (NOT SF_DEVENV)
    FetchContent_Declare(
            sf-std
            GIT_REPOSITORY https://github.com/septek/sf-std.git
            GIT_TAG master
    )
    FetchContent_MakeAvailable(sf-std)
endif()

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(cglm CONFIG REQUIRED)

target_link_libraries(sf-gfx PUBLIC
    sf-std
    glfw
    cglm
    glad
    stb
)
target_compile_options(sf-gfx PRIVATE
    -Wall -Werror -Wextra -pedantic -Wconversion
    -Wsign-conversion -Wformat=2 -Wundef
    -Wdouble-promotion -Wnull-dereference -Wstrict-overflow
)

if (WIN32)
    if (BUILD_SHARED_LIBS)
        set(CMAKE_SHARED_LIBRARY_PREFIX "")
        target_compile_definitions(sf-gfx PUBLIC SF_DYNAMIC)
        target_compile_definitions(sf-gfx PRIVATE SF_EXPORTS)
    endif()
endif()