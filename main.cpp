/**
 * main.cpp
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_image.h>

SDL_Surface* logo_rgba;

const GLfloat verts[6][4] = {
    { -1.0f, -1.0f, 0.0f, 1.0f },
    { -1.0f, 1.0f, 0.0f, 0.0f },
    { 1.0f, 1.0f, 1.0f, 0.0f },
    { 1.0f, -1.0f, 1.0f, 1.0f }
};

const GLint indicies[] = {
    0, 1, 2, 0, 2, 3
};

struct App {
    SDL_Window *m_window = NULL;
    SDL_GLContext m_context = 0;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    GLuint m_tex = 0;
    GLuint m_vert_shader = 0;
    GLuint m_frag_shader = 0;
    GLuint m_shader_prog = 0;
};

void initApplication(App* app) {
    // initialize SDL w/ video
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
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
    if (app->m_context == NULL) {
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
    glDeleteVertexArrays(1, &app->m_vao);

    // invoke delete/destroy methods for SDL state
    SDL_GL_DeleteContext(app->m_context);
    SDL_DestroyWindow(app->m_window);
    SDL_Quit();
}

const char* getSource(const char* path) {
    // reads contents of file and returns the allocated character buffer
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << path << std::endl;
        return nullptr;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    char* charBuffer = new char[content.size() + 1];
    std::copy(content.begin(), content.end(), charBuffer);
    charBuffer[content.size()] = '\0';
    return charBuffer;
}

void initShaders(App* app) {
    GLint status;
    char err_buf[512];
    glGenVertexArrays(1, &(app->m_vao));
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
    delete[] vertexSource;

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
    delete[] fragmentSource;

    // link shader program
    app->m_shader_prog = glCreateProgram();
    glAttachShader(app->m_shader_prog, app->m_vert_shader);
    glAttachShader(app->m_shader_prog, app->m_frag_shader);
    glBindFragDataLocation(app->m_shader_prog, 0, "oRGBA");
    glLinkProgram(app->m_shader_prog);
    glUseProgram(app->m_shader_prog);
    return;
}

int initGeometries(App* app) {
    // Populate vertex and element buffers
    glGenBuffers(1, &app->m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, app->m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glGenBuffers(1, &app->m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);

    // bind vertex position and texture coordinate attributes
    GLint pos_attr_loc = glGetAttribLocation(app->m_shader_prog, "aXY");
    glVertexAttribPointer(pos_attr_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0); // 2 values; stride 4, offset by 0
    glEnableVertexAttribArray(pos_attr_loc);
    GLint tex_attr_loc = glGetAttribLocation(app->m_shader_prog, "aUV");
    glVertexAttribPointer(tex_attr_loc, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat))); // 2 values; stride 4, offset by 2
    glEnableVertexAttribArray(tex_attr_loc);
    return 0;
}

int initMaterials(App* app) {
    // results in the successful transcription of raw image bytes into a uniform texture buffer in the GPU program
    glGenTextures(1, &app->m_tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app->m_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glUniform1i(glGetUniformLocation(app->m_shader_prog, "uTexture"), 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // define texture sampling parameters and map raw image data
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, logo_rgba->pixels);
    return 0;
}

void renderLoop(App* app) {
    // to render: clear color buffer; draw elements (constant program references); and swap buffers
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    SDL_GL_SwapWindow(app->m_window);
}

int main(int nArgs, char** vArgs) {
    // startup
    std::cout << "Initializing..." << std::endl;
    std::string filename = "logo.png";
    logo_rgba = IMG_Load(filename.c_str());
    App* app = new App();
    initApplication(app);
    initShaders(app);
    initGeometries(app);
    initMaterials(app);

    // main loop
    std::cout << "Running..." << std::endl;
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

    // cleanup
    std::cout << "Exiting..." << std::endl;
    freeApplication(app);
    SDL_FreeSurface(logo_rgba);
    logo_rgba = NULL;
    return 0;
}
