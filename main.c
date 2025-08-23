#include <stdio.h>
#include <SDL.h>
#include <glad/gl.h>

#define WINDOW_HEIGHT 480
#define WINDOW_WIDTH 640

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
    const char *vertexShaderSource =
        "#version 410 core\n"
        "in vec4 position;\n"
        "void main() {\n"
        "    gl_Position = position;\n"
        "}\n";
    const char *fragmentShaderSource =
        "#version 410 core\n"
        "out vec4 color;\n"
        "void main(){\n"
        "   color = vec4(0.5f, 0.75f, 1.0f, 1.0f);\n"
        "}\n";

    GLuint graphicsPipelineShaderProgram = glCreateProgram();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glAttachShader(graphicsPipelineShaderProgram, vertexShader);
    glAttachShader(graphicsPipelineShaderProgram, fragmentShader);
    glLinkProgram(graphicsPipelineShaderProgram);

    glValidateProgram(graphicsPipelineShaderProgram);

    // Setup
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    glClearColor(1.f, 1.f, 0.f, 1.f);

    glUseProgram(graphicsPipelineShaderProgram);

    // Loop
    int quit = 0;
    SDL_Event event;
    while (!quit)
    {
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

        // Render
        glBindVertexArray(vertexArrayObject);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);

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
