#include <glad/gl.h>
#include <stdio.h>
#include <SDL.h>

#define WINDOW_HEIGHT 400
#define WINDOW_WIDTH 400

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

        // Update window with OpenGL
        SDL_GL_SwapWindow(window);
    }

    // Destroy window
    SDL_DestroyWindow(window);

    // Quit SDL
    SDL_Quit();

    return 0;
}
