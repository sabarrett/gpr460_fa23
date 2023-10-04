
#include <stdlib.h>
#include <iostream>
#include <SDL2/SDL.h>
#include "System.h"

class Engine
{
public:
    static Engine* Create()
    {
        if (instance == nullptr)
        {
            instance = new Engine();
            return instance;
        }

        // Only the main loop should be able to make
        // and get instances!
        return nullptr;
    }

    Engine()
    {
        const Uint8* currentKeys = SDL_GetKeyboardState(&numkeys);

        keysLastFrame = new Uint8[numkeys];
        keysThisFrame = new Uint8[numkeys];

        memcpy(keysLastFrame, currentKeys, numkeys);
        memcpy(keysThisFrame, currentKeys, numkeys);

        // Same as this loop:
        /*
        for (int i = 0; i < numkeys; i++)
        {
            keysLastFrame[i] = currentKeys[i];
            keysThisFrame[i] = currentKeys[i];
        }
        */
    }

    static int GetCurrentFrame();
    static float Dt();
    static int GetKey(int keycode);
    static int GetKeyDown(int keycode);

    void RunGameLoop(SDL_Renderer* renderer);

private:
    static Engine* instance;

    Uint8* keysLastFrame;
    Uint8* keysThisFrame;
    int numkeys;
    int frame;
    float dt;
};

Engine* Engine::instance;

int Engine::GetCurrentFrame() { return instance->frame; }

float Engine::Dt() { return instance->dt; }

int Engine::GetKey(int keycode)
{
    if (keycode < instance->numkeys)
    {
        return instance->keysThisFrame[keycode];
    }

    // Really, this is an error...
    return -1;
}

/*
       Current Frame
  A   B   C   D   E   F   G
-------------------------------------
| 1 | 0 | 1 | 1 | 0 | 0 | 0 | ....
------------------------------------

       Previous Frame
  A   B   C   D   E   F   G
-------------------------------------
| 1 | 0 | 1 | 0 | 0 | 0 | 0 | ....
------------------------------------

=> D WAS PRESSED THIS FRAME!
*/

int Engine::GetKeyDown(int keycode)
{
    if (keycode < instance->numkeys)
    {
        return !instance->keysLastFrame[keycode] && instance->keysThisFrame[keycode];
    }

    // Really, this is an error...
    return -1;
}

class PlayerComponent;
class RectangleRenderComponent;
class SinMovement;

class GameObject
{
public:
    void Update();

    PlayerComponent* playerComponent = nullptr;
    RectangleRenderComponent* renderComponent = nullptr;
    SinMovement* sinMovement = nullptr;

    float x, y;
};

class PlayerComponent
{
public:

    PlayerComponent(GameObject* owner) : gameObject(owner) {}

    void Update()
    {
        float movement = Engine::GetKey(SDL_SCANCODE_DOWN) - Engine::GetKey(SDL_SCANCODE_UP);
        movement *= 25;

        gameObject->y += movement * Engine::Dt();

        if (Engine::GetKeyDown(SDL_SCANCODE_T))
        {
            std::cout << "T pressed this frame!\n";
            gameObject->y = 50;
            gameObject->x = 50;
        }
    }

    GameObject* gameObject = nullptr;
};

class RectangleRenderComponent
{
public:
    RectangleRenderComponent(GameObject* owner) : gameObject(owner), height(50), width(50) {}

    void Render(SDL_Renderer* renderer)
    {
        SDL_Rect r = {
            gameObject->x,
            gameObject->y,
            width,
            height
        };

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &r);
    }

    GameObject* gameObject = nullptr;
    float width, height;
};

class SinMovement
{
public:
    SinMovement(GameObject* owner) : gameObject(owner) {}

    void Update()
    {
        gameObject->x = (SDL_sinf(Engine::GetCurrentFrame() / 10.0f) * 100) + 200;
    }

    GameObject* gameObject = nullptr;

};

void GameObject::Update()
{
    if (playerComponent != nullptr) playerComponent->Update();
    if (sinMovement != nullptr) sinMovement->Update();
}


void Engine::RunGameLoop(SDL_Renderer* renderer)
{
    SDL_Event event;
    bool quit = false;
    Uint32 frameStart = SDL_GetTicks64();

    const int NUM_GAMEOBJECTS = 2;
    GameObject gameObjects[NUM_GAMEOBJECTS];

    // Initialize player object
    {
        GameObject& player = gameObjects[0];
        // Heap allocations! Have to clean these up sometime :/
        player.playerComponent = new PlayerComponent(&player);
        player.renderComponent = new RectangleRenderComponent(&player);
        player.x = 100;
        player.y = 100;
    }

    // Initialize moving rectangle
    {
        GameObject& movingRectangle = gameObjects[1];
        movingRectangle.sinMovement = new SinMovement(&movingRectangle);
        movingRectangle.renderComponent = new RectangleRenderComponent(&movingRectangle);
        movingRectangle.x = 200;
        movingRectangle.y = 10;
    }

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
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    quit = true;
                }
            }
        }

        if (SDL_GetTicks64() - frameStart >= 16)
        {
            memcpy(keysThisFrame, SDL_GetKeyboardState(NULL), numkeys);

            frame++;

            dt = (SDL_GetTicks64() - frameStart) / 1000.0f;

            frameStart = SDL_GetTicks64();

            for (int i = 0; i < NUM_GAMEOBJECTS; i++)
            {
                gameObjects[i].Update();
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
            SDL_RenderClear(renderer);

            for (int i = 0; i < NUM_GAMEOBJECTS; i++)
            {
                if (gameObjects[i].renderComponent != nullptr)
                {
                    gameObjects[i].renderComponent->Render(renderer);
                }
            }

            SDL_RenderPresent(renderer);

            // Switch input buffers
            {
                Uint8* tmp = keysThisFrame;
                keysThisFrame = keysLastFrame;
                keysLastFrame = tmp;
            }
        }
    }
}

int main(int argc, char* argv[])
{
    const int WIDTH = 640;
    const int HEIGHT = 480;
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    gpr460::System* system = gpr460::System::Create();

    // Class note: You can use this to report memory leaks on exit:
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);

    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("SDL2 Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    int x = 0;
    int frame = 0;

    Engine* instance = Engine::Create();

    instance->RunGameLoop(renderer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}