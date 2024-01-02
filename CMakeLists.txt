cmake_minimum_required(VERSION 3.26)
project(playground)
#project(output_an_image)
project(ray_tracing)
project(experiments)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(E:/ComputerGraphics/libraries/Utilities/includes)
link_directories(E:/ComputerGraphics/libraries/Utilities/lib)

add_executable(playground playground.cpp)
add_executable(experiments experiments.cpp)
#add_executable(output_an_image output_an_image/output_an_image.cpp)
add_executable(ray_tracing main.cpp
        includes/lambertian.h)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(ray_tracing PUBLIC OpenMP::OpenMP_CXX)
endif()
