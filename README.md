# [11/52] OpenGL is Still Decent Actually

I know... I promised I'd be doing more engineering. We'll get there! But I was consuming some interesting digital creator / software engineering content the other day. There was one guy in particular (I won't say exactly who) who tends to do pretty interesting stuff, but who can be a little bit iconoclastic. He went on a wierd rant about how obscure, obtuse, and inaccesible OpenGL is. It was very strange because the critique was based on how difficult it was to get sprites or pixels up on the screen. But compared to pretty much anything else--Vulkan, Carbon, XTerminal--it's pretty straightforward and well-established how to get up and going.

This guy is pretty experienced, and he knows what he's doing, so it seemed strange and the comment came out of nowhere. So, naturally, it got me going and thinking, "okay, what is the simplest and shortest path to get up and going with OpenGL?" Assume you're trying to just put up a simple animation, create a simple game, put some sprites on the screen, etc. What is the shortest path to do that? And as it turns out, unless you're doing something platform-specific like Windows GDI, OpenGL is still a really good way to go.

There's a few other things you need in combination with it. SDL is an absolutely fantastic library--strongly recommended, check it out if you aren't familiar with it. There's a lot more to it, but for getting out of the box and going with a window and an event loop and a GL context and all that, it's fantastic. And of course GLEW is practically required for a lot of things. So today we're going to walk through, really quick, a brief demonstration of what the "shortest path" to a working "sprites on screen" is.

## Let's Get Started

Begin with a blank C++ project. We'll create the following files as placeholders for future content:

* `.gitignore`

* `basic.f.glsl`

* `basic.v.glsl`

* `CMakeLists.txt`

* `main.cpp`

After initializing our git repository, we'll also want to add some dependencies. We'll use git submodules to do this, and in most cases these will need to come from specific branches. So, run `git submodule add` for the following:

* https://github.com/libsdl-org/SDL.git (use branch `SDL2`)

* https://github.com/libsdl-org/SDL_image.git (use branch `SDL2`)

* https://github.com/Perlmint/glew-cmake.git0

You'll notice we're also adding SDL Image here, which is a fantastic extension to SDL that gives you out-of-the-box support for loading surfaces from a wide variety of image formats. We're also using a specific fork of GLEW that supports inclusion via CMake, to automate the dependency inclusion within our CMake project definition. Once those dependencies are cloned and submodules initialized (recursively!), we're ready to start populating our files.

You'll also notice we have some shaders. If you haven't messed with GLSL before, it's fascinating! We'll probably do another talk specifically about radiometry and applications to graphics programming, thermal, electro-optics, and other fields. We'll also want a test texture; you can use any .PNG you want, but I went with a nice picture of a seagull. Cheers.

Our goal--our mission, if we choose to accept it--is to put this image up in the window. If we do this well, it should be clear how we can extend this in the future to do more sophisticated sprite models and behaviors within the context of an app or game engine.

## The Main Thing

Let's start in `main.cpp` with some dependencies. We'll include the following, roughly broken into system includes and dependency includes:

```cpp
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_image.h>
```

## Vertex Formats

Next, we'll think about our data model. Let's stay away with classes and focus on how the state of our application will be packed into an aggregation of bytes (a struct).

```cpp
SDL_Surface* logo_rgba;
const GLfloat verts[4][4] = {
    { -1.0f, -1.0f, 0.0f, 1.0f },
    { -1.0f, 1.0f, 0.0f, 0.0f },
    { 1.0f, 1.0f, 1.0f, 0.0f },
    { 1.0f, -1.0f, 1.0f, 1.0f }
};
const GLint indices[6] = {
    0, 1, 2, 0, 2, 3
};
```

You do need to think about your vertex format! Briefly, this means thinking about what information is attached to, or defines, each vertex in the drawing sequence you will call. Since we're focusing on a textured 2d sprite, our `verts` array defines a set of 4 vertices, each of which defines 4 values:

* An `x` (position) coordiante
* An `y` (position) coordinate
* A `u` (texture) coordinate
* A `v` (texture) coordinate

We'll see how we "encode", or tell OpenGL about, the format of this vertex sequence in subsequent calls. And since we only want to define each vertex once, we also have an index buffer to define how the vertices are combined to form a shape (in this case, two triangles).

## Application State

We also need to think about what information defines the state our our application model. Let's use the following, which includes SDL references and a healthy mix of OpenGL unsigned integers (effectively used as handles to GPU data).

```cpp
struct App {
    SDL_Window* m_window = NULL;
    SDL_GLContext m_context = 0;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    GLuint m_tex = 0;
    GLuint m_vet_shader = 0;
    GLuint m_frag_shader = 0;
    GLuint m_shader_prog = 0;
};
```

## Behaviors

We want to define procedures by which we initialize and free specific models within this application. Let's define prototypes for the following:

```cpp
void initApplication(App* app);
void freeApplication(App* app);
void initShaders(App* app);
void initGeometries(App* app);
void initMaterials(App* app);
```

We'll also want some helper methods and a function to define specific loops. (In the long run, we'd want to split these loops across threads for different cadences like rendering, I/O handling, and internal updates.)

```cpp
const char* getSource(const char* path);
void renderLoop(App* app);
```

And now we have enough defined to think about how we use these behaviors in the context of a program. So let's write our `main()` entry point!

## The Main Main

First, let's start up the application by allocating, loading resources, and calling our initializers.

```cpp
int main(int nArgs, char** vArgs) {
    // startup 
    std::cout << "Initialzing..." << std::endl;
    std::string filename = "logo.png";
    logo_rgba = IMG_Load(filename.c_str());
    App* app = new App();
    initApplication(app);
    initShaders(app);
    initGeometries(app);
    initMaterials(app);0
    
    // ...
}
```

Even though we've consolidated all of our state within a specific structure, you'll notice we've broken out initialization into specific steps. If you've used THREE.js before, this model may loop familiar. In the long run, this will make it easy to extract and organize specific models within our application--like individual shader programs, complex geometry data that may be reused or even animated, and material resources that need internally-organized bindings to things like multiple texture uniforms.

(We might look at a "part two" in which we see how these models can evolve into something more... interesting, if not entirely professional yet.0)

Next we can think about our "core" loop. This is pretty straightforward:

```cpp
int main(int nArgs, char** vArgs) {
    // ...

    // main loop
    std::cout << "Running" << std::endl;
    bool is_running = true;
    while (is_running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
                is_running = false;
                break;
            }
        }
        renderLoop(app);
    }
    // ...
}
```

Finally, we clear up our resources:

```cpp
int main(int nArgs, char** vArgs) {
    // ...

    // cleanup
    std::cout << "Exiting..." << std::endl;
    freeApplication(app);
    delete app;
    SDL_FreeSurface(logo_rgba);
    logo_regba =nNULL;
    return 0;
}
```

## Initialization

When we initialize the application, what are we talking about? Since we have separate initialization for our different groups of GL data, this is largely SDL-specific. Let's write our `initApplication()` to handle this top-level logic.

```cpp
void initApplication(App* app) {
    if (SDL_init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Initializing SDL video failed!" << std::endl;
        throw std::exception();
    }

    // create window
    app->m_window = SDL_CreateWindow("App", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
    if (app->m_window == NULL) {
        std::cerr << "Creating main window failed!" << std::endl;
        SDL_Quit();
        throw std::exception();
    }

    // initialize GL context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    app->m_context = SDL_GL_CreateContext(app->m_window);
    if (app->m_context== NULL) {
        std::cerr << "Creating GL context failed!" << std::endl;
        SDL_DestroyWindow(app->m_window);
        SDL_Quit();
        throw std::exception();
    }

    // initialize glew
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Initializing GLEW failed!" << std::endl;
        SDL_GL_DeleteContext(app->m_context);
        SDL_DestroyWindow(app->m_window);
        SDL_Quit();
        throw std::exception();
    }
}
```

The big "lift" here is window management, of course, and that's the important part SDL automates for us. Once we have an agnostic window generated, getting a GL context is straightforward. These things would be 80% of the effort (a nightmare) if we didn't have SDL or something like it. Once you have your GL context, you're home free and almost everything else is platform-neutral.

## A Brief Break for CMake

Let's jump over to our `CMakeLists.txt` for a moment to make sure we'll be able to build this mess once we've finished coding. We'll start with the standard three CMake commeands: defining the version, defining the project, and defining the main build artifact (executable, in this case).

```CMake
cmake_minimum_required(VERSION 3.14)
project(11-of-52--opengl-is-still-decent-actually)
add_executable(${PROJECT_NAME}
    "main.cpp"
)
```

Next, we'll assert specific options for our dependencies.

```CMake
# assert dependency options
set(SDL2IMAGE_VENDORED OFF)
```

Now we can recursively include our submodules:

```CMake
# ensure dependencies are built
add_subdirectory("glew-cmake/")
add_subdirectory("SDL/")
add_subdirectory("SDL_image/)
```

Now we'll want to make sure our main build target can resolve the appropriate `#include` paths.

```CMake
target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    OpenGL32
    libglew_static
    SDL2_image
)
```

When in doubt, these are basically the library names. Some CMake projects will have their own unique library names defined (the `::` is a big clue); you can always check their `CMakeLists.txt` for an `add_library()` directive. There's also some useful logic/automation build into the `find_package()` directive within CMake--that might be worth going over in its own video at some point.

Finally, we'll want to set specific runtime resources to copy into the binary folder. We'll do this for static resources (like our image), as well as dynamic resources (like dependency DLLs). At some point, you can automate a degree of this with something like `CPack`, which is also probably worth its own video.

```CMake
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
```

(We're cheating a little bit here, because we know what DLLs will be generated and where they need to go.)

And that just about does it for our CMake. This is enough for us to do a basic configure test from the command line:

```sh
cmake -S . -B build
```

## Back to the Source

Let's finish our initialization. We've initialized the application. How are we going to initialize our shaders? There's a basic three-step process: first, we compile the vertex shader from source; second, we compile the fragment shader from source; third, we link these two shaders into a fully-defined graphics program.

```cpp
void initShaders(App* app) {
    GLint status;
    char err_buf[512];
    glGenVertexArays(1, &(app->m_vao));
    glBindVertexArray(app->m_vao);

    // compile vertex shader
    app->m_vert_shader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexSource = getSource("basic.v.glsl");
    glShaderSource(app->m_vert_shader, 1, &vertexSource, NULL);
    glCompileShader(app->m_vert_shader);
    glGetShaderiv(app->m_vert_shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        glGetShaderInfoLog(app->m_vert_shader, sizeof(err_buf), NULL, err_buf);
        err_buf[sizeof(err_buf)-1] = '\0';
        std::cerr << "Compiling vertex shader failed!" << std::endl;
        std::cerr << err_buf << std::endl;
        return;
    }

    // compile fragment shader
    app->m_frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentSource = getSource("basic.f.glsl");
    glShaderSource(app->m_frag_shader, 1, &fragmentSource, NULL);
    glCompileShader(app->m_frag_shader);
    glGetShaderiv(app->m_frag_shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        glGetShaderInfoLog(app->m_frag_shader, sizeof(err_buf), NULL, err_buf);
        err_buf[sizeof(err_buf)-1] = '\0';
        std::cerr << "Compiling fragment shader failed!" << std::endl;
        std::cerr << err_buf << std::endl;
        return;
    }

    // link shader program
    app->m_shader_prog = glCreateProgram();
    glAttachShader(app->m_shader_prog, app->m_vert_shader);
    glAttachShader(app->m_shader_prog, app->m_frag_shader);
    glBindFragDataLocation(app->m_shader_prog, 0, "uRGBA");
    glLinkProgram(app->m_shader_prog);
    glUseProgram(app->m_shader_prog);
    return;
}
```

(You'll notice we're null-terminating our string copy from the error buffer, which isn't a great idea in general. Don't try this at home, kids!)

In modern graphics programming, you would not be necessarily doing this full build from source at runtime like this. Instead, you'd have an intermediate format (like SPIR-V, with Vulkan) that you would use to do a lot of the preliminary compilation. For our purposes, though, this is enough (and interesting, and useful; it also gives a transparent view into our application state and graphics pipeline.)

Note that we "know" special things about our shader program, in this case. For example, we "know" that there is a uniform variable we'll need to bind to our texture data. We'll look at how we set this up in the material initialization.

## Geometries

Now let's think about our geometry data. We've defined a set of vertices with a specific format, and some indices that define how those are mapped to specific shapes for drawing. We need to tell OpenGL how these vertices are structured. We also need to hand off (copy) the data buffers themselves. These are mostly done with buffer commands, using the "handles" (unsigned integers) we've defined as part of our application state to share persistent references.

```cpp
void initGeometries(App* app) {
    // populate vertex and element buffers
    glGenBuffers(1, &app->m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, app->m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glGenBuffers(1, &app->m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // bind vertex position and texture coordinate attributes
    GLint pos_attr_loc = glGetAttribLocation(app->m_shader_prog, "aXY");
    glVertexAttribPointer(pos_attr_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(pos_attr_loc);
    GLint tex_attr_loc = glGetAttribLocation(app->m_shader_prog, "aUV");
    glVertexAttribPointer(tex_attr_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(Glfloat)));
    glEnableVertexAttribArray(tex_attr_loc);
}
```

That middle clause is probably the most interesting, because this is where we tell OpenGL how the vertex attributes are structured. Given a sequence of vertex attributes, each segment defines a vertex--but how is that information "packed"? There are a total of four values (or "stride") between each segment (that is, the segment length).

* The first pair of values define the "x-y" pair, or `vec2`, vertex attribute; these are floats and offset from the beginning of the segment by zero values

* The second pair of values define the "u-v" pair, or `vec`, vertex attribute; these are floats and offset from the beginning of the segment by two values

## Materials

With our geometry data and shader program defined, we need to pass in material data. In this case, we have a single diffuse texture that will be sampled to define the pixel (or fragment) color within our "sprite". We do this by loading the image data from an SDL surface for OpenGL to reference as a "uniform" input to our shader program.

```cpp
void initMaterials(App* app) {
    // results in the successful transcription of raw image bytes into a uniform texture buffer
    glGenTextures(1, &app->m_tex);
    glActiveTexture(GL_TEXUTRE0);
    glBindTexture(GL_TEXTURE_2D, app->m_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glUniform1i(glGetUniformLocation(app->m_shader_prog, "uTexture"), 0);
    glEnable(GL_BLEND);
    glBendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // define texture sampling parameters and map raw image data
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, logo_rgba->pixels);
}
```

Most of the second block is just defining the sampling parameters for OpenGL. The most interesting call is the last line, where we pass off the actual pixel data from the SDL surface to the GPU.

## Loops

We're just about done! Let's define our rendering pass, which is pretty straightforward because we have only one draw call.

```cpp
void renderLoop(App* app) {
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    SDL_GL_SwapWindow(app->m_window);
}
```

Everything is already loaded (buffers, objects, program, etc.) so we just need a draw call. In this case, we tell OpenGL to draw six elements from our buffer (vertices), and to treat them as triangles (that is two triangles with three vertices each). Finally, we swap our buffer (I can't tell you how much time I've wasted on other projects before I realized nothing was showing because I never swapped the window buffers...).

## Helpers

There are a few "helper" functions we need to define, as well as freeing our application state.

```cpp
const char* getSource(const char* path) {
    // reads contents of file and returns the allocated character buffer
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Opening file failed!" << std::endl;
        return NULL;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    char* charBuffer = new char[content.size() + 1];
    std::copy(content.begin(), content.end(), charBuffer);
    charBuffer[content.size()] = '\0';
    return charBuffer;
}
```

Lastly, let's define our our application state is cleaned up. This is basically in reverse order from our initialization.

```cpp
void freeApplication(App* app) {
    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glDetachShader(app->m_shader_prog, app->m_vert_shader);
    glDetachShader(app->m_shader_prog, app->m_frag_shader);
    glDeleteProgram(app->m_shader_prog);
    glDeleteShader(app->m_vert_shader);
    glDeleteShader(app->m_frag_shader);
    glDeleteTextures(1, &app->m_tex);
    glDeleteBuffers(1, &app->m_ebo);
    glDeleteBuffers(1, &app->m_vbo);
    glDeleteVertexArrays(1, &app->m_vbo);

    // invoke delete/destory methods for SDL state
    SDL_GL_DeleteContext(app->m_context);
    SDL_DestoryWindow(app->m_window);
    SDL_Quit();
}
```

## Shader

We're done! With the C++. We still need to define a *very* basic graphics pipeline. Let's start with the vertex shader, which is simply forwarding the texture coordinates as a `varying` parameter for the fragment shader, and defining the basic position transform from our 2d space into the 4d position OpenGL expects.

```glsl
/**
 * basic.v.glsl
 */

in vec2 aXY;
in vec2 aUV;
varying vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = vec4(aXY, 0.0, 1.0);
}
```

Then, our fragment shader uses those texture coordinates (interpolated for each pixel) to look up the appropriate fragment color from our texture data.

```glsl
/**
 * basic.f.glsl
 */

varying vec2 vUV;
out vec4 oRGBA;
uniform sampler2D uTexture;

void main() {
    oRGBA = texture(uTexture, vUV);
}
```

## Building

We have enough! Let's compile our project using the CMake configuration we already set up.

```sh
cmake --build build
```

If successful, you should see an executable show up in `build/Debug/`. And when you run it, you should see your sprite appear!

## Stepping Back

We started this conversation off by saying "it's actually really easy to get started with OpenGL!"... but this took a little bit of time, didn't it?

If you think about what we were doing, none of these things were really optional--whether we're using OpenGL or anything else. Most importantly, we've put ourselves in a position where it's fairly extensible to more sophisticated to other things we might want to do. (We have image loading support, we have customizable shaders, we have structured state models, we have an extensible/threadable event loop...) Some of these came with optional dependencies (like SDL_Image) but this gave us a pretty well-organized "starter" project.

It will be very easy in our next iteration to break parts of this application structure apart into reusable models for shader programs, individual sprites, scene graph nodes with their own transforms, etc. This is the first of two big takeaways: With a little bit of help, you can get started with a sprite-based application very easily, and you can do it in a way where you make it possible to do a lot more in the future with that.

Secondly, believe it or not... well, there's a saying attributed to Winston Churchill, roughly along the lines of "democracy is the worst form of government... except for all the others that have been tried." OpenGL is a lot like this--trying to get started this quickly with any other approach is an absolute nightmare. OpenGL is the worst way to get started... except for all the others that have been tried. (Vulkan, Wayland, you name it.)

So, this is a little involved. But (maybe because I've just started at this too much over the years) everything here still makes sense. Compared to some of the more obscure setups, you're not trying to abstract away too much of what's going on with the GPU, you have a nicely customized graphics pipeline that you have a lot of control over but it's still straightfowrard to setup and get something going.

This is part one of two. In part two, I'm thinking of looking at a basic 2d engine that you might put together based off of this. But this is a good way to get going, and a good way to start doing quick 2d cross-platform applications, especially if you're new to it or just want to draw some sprites.
