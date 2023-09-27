
#include <stdlib.h>
#include <iostream>
#include <SDL2/SDL.h>
#include "System.h"

// Add your System.h include file here
struct String
{
    char*  buf;
    size_t len;
    size_t cap;

    void push(char item);
};

void String::push(char item)
{
    buf = new char[1024];
    len++;
    cap = 1024;
}

int main(int argc, char* argv[])
{
    const int WIDTH = 640;
    const int HEIGHT = 480;
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    gpr460::System* system = gpr460::System::Create();
    system->Initialize();

    std::cout << "Hi there!\n";

    system->WriteToLogFile("MyFile.txt");

    // Class note: You can use this to report memory leaks on exit:
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);

    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("SDL2 Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    int x = 0;
    int frame = 0;

    SDL_Event event;
    bool quit = false;
    Uint32 frameStart = SDL_GetTicks64();

#ifdef __EMSCRIPTEN__
    // do the loop one kind of way
    std::cout << "I'm the emscripten loop! Wee!!\n";

#else
    // do it our cool way

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = true;
            }

            if (event.type == SDL_KEYDOWN)
            {
                // Don't delete this leak -- it will be used
                //    to check your 
                int* intentionalLeak = DBG_NEW int[32];
                if (event.key.keysym.sym == SDLK_k)
                {
                    system->DisplayMessageBox("The 'K' key was pressed!");
                    system->WriteToLogFile("The 'K' key was pressed!");
                }
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    quit = true;
                }
            }
        }

        if (SDL_GetTicks64() - frameStart >= 16)
        {
            frame++;
            frameStart = SDL_GetTicks64();
            x = (SDL_sinf(frame / 10.0f) * 100) + 200;

            SDL_Rect r = {
                x,
                100,
                50,
                50
            };

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
            SDL_RenderFillRect(renderer, &r);
            SDL_RenderPresent(renderer);
        }
    }
#endif

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}