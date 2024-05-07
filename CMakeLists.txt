cmake_minimum_required(VERSION 3.14)
project(sdl-glew-cmake)
add_executable(${PROJECT_NAME}
    "main.cpp"
)

# glew dependency
add_subdirectory("glew-cmake/")
target_include_directories(${PROJECT_NAME} PRIVATE
    "glew-cmake/include/"
)

# sdl dependency
add_subdirectory("SDL/")
target_include_directories(${PROJECT_NAME} PRIVATE
    "SDL/include/"
)

# finally, list all link libaries
target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    OpenGL32
    libglew_static
)
