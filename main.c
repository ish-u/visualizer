#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <glad/gl.h>

#define WINDOW_HEIGHT 480
#define WINDOW_WIDTH 640

char *readShaderFile(const char *fileName)
{
    // Reading  Shader File in Binary Mode
    FILE *shaderFile = fopen(fileName, "rb");
    if (!shaderFile)
    {
        printf("Failed to read file: %s", fileName);
        return NULL;
    }

    // Getting file size
    fseek(shaderFile, 0, SEEK_END);
    long fileSize = ftell(shaderFile);
    rewind(shaderFile);

    // Allocate Buffer (fileSize + 1 -> for null terminator)
    char *buffer = malloc(fileSize + 1);
    if (!buffer)
    {
        printf("Failed to malloc");
        fclose(shaderFile);
        return NULL;
    }

    // Read file into buffer
    fread(buffer, 1, fileSize, shaderFile);
    buffer[fileSize] = '\0';

    fclose(shaderFile);
    return buffer;
}

int main(int argc, char *argv[])
{
    // Pointer to SDL Window Struct
    SDL_Window *window = NULL;
    SDL_GLContext openGLContext = NULL;

    // Initializing SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("Error initialzing SDL: %s", SDL_GetError());
        return 1;
    }

    // Setting OpenGL Attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Creating Window
    window = SDL_CreateWindow("Visualizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window)
    {
        printf("Error creating window: %s", SDL_GetError());
        return 1;
    }

    // Creating OpenGL Context
    openGLContext = SDL_GL_CreateContext(window);
    if (!openGLContext)
    {
        printf("Error creating OpenGL Context: %s", SDL_GetError());
        return 1;
    }

    // Initialize GLAD
    if (!gladLoaderLoadGL())
    {
        printf("Failed to initialize GLAD: %s", SDL_GetError());
        return 1;
    }

    // OpenGL Info
    printf("Vendor: %s\n", glGetString(GL_VENDOR));
    printf("Renderer %s\n", glGetString(GL_RENDERER));
    printf("Version: %s\n", glGetString(GL_VERSION));
    printf("Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Vertex
    GLfloat vertexPosition[9] = {
        -0.3f,
        -0.3f,
        0.0f,
        0.3f,
        -0.3f,
        0.0f,
        0.0f,
        0.3f,
        0.0f,
    };

    // VAO
    GLuint vertexArrayObject = 0;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    // VBO
    GLuint vertexBufferObject = 0;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPosition), &vertexPosition, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    // Clean up
    glBindVertexArray(0);
    glDisableVertexAttribArray(0);

    // Graphics Pipeline
    const char *vertexShaderSource = readShaderFile("./shaders/vert.glsl");
    const char *fragmentShaderSource = readShaderFile("./shaders/frag.glsl");

    GLuint graphicsPipelineShaderProgram = glCreateProgram();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLint vertex_compiled;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertex_compiled);
    if (vertex_compiled != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(vertexShader, 1024, &log_length, message);
        printf("Failed to compile vertex shader: %s", message);
        return 1;
    }

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    GLint fragment_compiled;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragment_compiled);
    if (fragment_compiled != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetShaderInfoLog(fragmentShader, 1024, &log_length, message);
        printf("Failed to compile fragment shader: %s", message);
        return 1;
    }

    glAttachShader(graphicsPipelineShaderProgram, vertexShader);
    glAttachShader(graphicsPipelineShaderProgram, fragmentShader);
    glLinkProgram(graphicsPipelineShaderProgram);
    GLint program_linked;
    glGetProgramiv(graphicsPipelineShaderProgram, GL_LINK_STATUS, &program_linked);
    if (program_linked != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        glGetProgramInfoLog(graphicsPipelineShaderProgram, 1024, &log_length, message);
        printf("Failed to link shader program : %s", message);
        return 1;
    }

    glValidateProgram(graphicsPipelineShaderProgram);

    glDetachShader(graphicsPipelineShaderProgram, vertexShader);
    glDetachShader(graphicsPipelineShaderProgram, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    free((void *)vertexShaderSource);
    free((void *)fragmentShaderSource);

    // Setup
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(0.f, 0.f, 0.f, 0.f);

    // Uniforms
    GLuint scaleLocation = glGetUniformLocation(graphicsPipelineShaderProgram, "scale");
    if (scaleLocation == -1)
    {
        printf("Failed to get uniform 'scale'");
        return 1;
    }

    glUseProgram(graphicsPipelineShaderProgram);

    // Loop
    int quit = 0;
    SDL_Event event;

    while (!quit)
    {
        SDL_Delay(1);
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = 1;
            }
            if (event.type == SDL_KEYDOWN)
            {
                quit = 1;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);

        // Render
        glBindVertexArray(vertexArrayObject);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        static float scale = 0.0f;
        static float delta = 0.005f;
        scale += delta;
        if ((scale >= 1.0f) || scale <= -1.0f)
        {
            delta *= -1.0f;
        }
        glUniform1f(scaleLocation, scale);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Update window with OpenGL
        SDL_GL_SwapWindow(window);
    }

    // Destroy window
    SDL_DestroyWindow(window);

    // Quit SDL
    SDL_Quit();

    return 0;
}
