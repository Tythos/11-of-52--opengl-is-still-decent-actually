cmake_minimum_required(VERSION 3.14)
project(sdl-glew-cmake)
add_executable(${PROJECT_NAME}
    "main.cpp"
)

# glew dependency
add_subdirectory("glew-cmake/")

# sdl dependency
add_subdirectory("SDL/")
