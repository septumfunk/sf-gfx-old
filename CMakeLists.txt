cmake_minimum_required(VERSION 3.27)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SF_LIBRARY_TYPE STATIC)
if (BUILD_SHARED_LIBS)
    set(SF_LIBRARY_TYPE SHARED)
endif()

project(sf-gfx C)

add_library(sf-gfx ${SF_LIBRARY_TYPE}
    src/graphics.c
)
target_include_directories(sf-gfx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(glad ${SF_LIBRARY_TYPE}
    src/glad.c
)
target_include_directories(glad PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)


# Fetch Dependencies
include(FetchContent)

FetchContent_Declare(
    sf-std
    GIT_REPOSITORY https://github.com/septek/sf-std.git
    GIT_TAG master
)
FetchContent_MakeAvailable(sf-std)
add_custom_command(TARGET sf-gfx POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sf-std>
        $<TARGET_FILE_DIR:sf-gfx>
)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG master
)
FetchContent_MakeAvailable(glfw)
add_custom_command(TARGET sf-gfx POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:glfw>
        $<TARGET_FILE_DIR:sf-gfx>
)

FetchContent_Declare(
    cglm
    GIT_REPOSITORY https://github.com/recp/cglm.git
    GIT_TAG master
)
FetchContent_MakeAvailable(cglm)
add_custom_command(TARGET sf-gfx POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:cglm>
        $<TARGET_FILE_DIR:sf-gfx>
)


target_link_libraries(sf-gfx PUBLIC
    sf-std
    glfw
    cglm
    glad
)
target_compile_options(sf-gfx PRIVATE
    -Wall -Werror -Wextra -pedantic -Wconversion
    -Wsign-conversion -Wformat=2 -Wundef
    -Wold-style-cast -Woverloaded-virtual
    -Wdouble-promotion -Wnon-virtual-dtor
    -Wnull-dereference -Wstrict-overflow
)

if (WIN32)
    if (BUILD_SHARED_LIBS)
        set(CMAKE_SHARED_LIBRARY_PREFIX "")
        target_compile_definitions(sf-gfx PUBLIC SF_DYNAMIC)
        target_compile_definitions(sf-gfx PRIVATE SF_EXPORTS)
    endif()
endif()