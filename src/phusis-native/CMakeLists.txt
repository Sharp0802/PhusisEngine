cmake_minimum_required(VERSION 3.25)
project(phusis LANGUAGES C CXX)

set(SRCDIR src)
set(INCDIR inc)
set(BINDIR bin)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan COMPONENTS dxc REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

file(GLOB_RECURSE SRCs *.cxx)
file(GLOB_RECURSE INCs *.hxx)

add_executable(phusis ${SRCs} ${INCs})

target_compile_definitions(phusis PRIVATE SRCDIR="${CMAKE_SOURCE_DIR}")
target_include_directories(phusis PRIVATE ${INCDIR})
target_link_libraries(phusis
    Vulkan::Vulkan
    Vulkan::Headers
    Vulkan::dxc_lib
    glfw
    glm
)