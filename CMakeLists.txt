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
    src/str.c
    src/dynamic.c
)
target_include_directories(sf-gfx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


# Fetch Dependencies
include(FetchContent)

FetchContent_Declare(
    sf-std
    GIT_REPOSITORY https://github.com/septumfunk/sf-std.git
    GIT_TAG master
)
FetchContent_MakeAvailable(sf-std)
add_custom_command(TARGET septek-core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sf-std>
        $<TARGET_FILE_DIR:septek-core>
)

target_link_libraries(sf-gfx PUBLIC
    sf-std
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