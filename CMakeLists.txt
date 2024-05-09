cmake_minimum_required(VERSION 3.14)
project(sdl-glew-cmake)
add_executable(${PROJECT_NAME}
    "main.cpp"
)

# assert depdendency options
set(SDL2IMAGE_VENDORED OFF)

# ensure dependencies are built
add_subdirectory("glew-cmake/")
add_subdirectory("SDL/")
add_subdirectory("SDL_image/")

# ensure dependency includes work
target_include_directories(${PROJECT_NAME} PRIVATE
    "glew-cmake/include/"
    "SDL/include/"
    "SDL_image/include/"
)

# ensure program links against dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    OpenGL32
    libglew_static
    SDL2_image
)

# define static runtime resources
set(OUTPUT_PATH "${CMAKE_BINARY_DIR}/Debug")
file(MAKE_DIRECTORY ${OUTPUT_PATH})
configure_file("basic.f.glsl" "${OUTPUT_PATH}/basic.f.glsl" COPYONLY)
configure_file("basic.v.glsl" "${OUTPUT_PATH}/basic.v.glsl" COPYONLY)
configure_file("logo.png" "${OUTPUT_PATH}/logo.png" COPYONLY)

# define dynamic runtime resources
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/SDL/Debug/SDL2d.dll
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2d.dll
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/SDL_image/Debug/SDL2_imaged.dll
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2_imaged.dll
)
