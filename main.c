#include <stdio.h>
#include <SDL.h>

#define WINDOW_HEIGHT 400
#define WINDOW_WIDTH 400

int main(int argc, char *argv[])
{
    // Pointer to SDL Window Struct
    SDL_Window *window = NULL;

    // Initializing SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("Error initialzing SDL: %s", SDL_GetError());
        return 1;
    }

    // Creating Window
    window = SDL_CreateWindow("Visualizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        printf("Error creating window: %s", SDL_GetError());
        return 1;
    }

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
    }

    // Destroy window
    SDL_DestroyWindow(window);

    // Quit SDL
    SDL_Quit();

    return 0;
}
