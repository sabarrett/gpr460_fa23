
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <SDL2/SDL.h>
#include "System.h"
#include "StackAllocator.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cassert>

#include "lua.hpp"


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

    void InitLua();
    void CallLuaUpdate();
    void RunGameLoop(SDL_Renderer* renderer);

private:
    static Engine* instance;

    lua_State* L;

    Uint8* keysLastFrame;
    Uint8* keysThisFrame;
    int numkeys;
    int frame;
    float dt;
};

Engine* Engine::instance;



int LuaC_GetCurrentFrame(lua_State* L)
{
    // To return something to lua, push it onto the stack
    lua_pushinteger(L, Engine::GetCurrentFrame());

    // Then, return the number of items that you've returned.
    // In this case, we returned a single integer, so we
    // return 1.

    return 1;
}

// Needed to remove "luac.c" and "lua.c"
void Engine::InitLua()
{
    // First, we create the Lua interpreter
    L = luaL_newstate();

    // Make sure our interpreter could be created successfully
    if (L == nullptr)
    {
        std::cerr << "Could not create lua interpreter\n";
        return;
    }

    std::cout << "Successfully created lua interpreter\n";

    // Set up standard libraries
    luaL_openlibs(L);

    // Now, we have a lua state and can ask it to do things!
    if (luaL_dostring(L, "print(10 + 5)") == 1)
    {
        std::cerr << "Error calling luaL_dostring\n";
    }

    // Lua reads a string like "print(10 + 5)" and parses it.
    // It turns it into a tree like:
    // 
    //        call_fn
    //        /     \
    //   "print"     +
    //              / \
    //            10   5

    // Then, it executes the tree:
    //    Add 10 and 5. Store that somewhere.
    //    Call print. Pass it the stored value as an argument.

    // When we invoke Lua, we'd like to skip the parsing step and go
    // straight to the instructions. We want to write C(++) code that
    // tells Lua to, e.g., add 10 and 5 and then call print.


    lua_getglobal(L, "print");
    // -> [ fn_print ]

    lua_pushnumber(L, 10);
    // -> [ 10, fn_print ]

    lua_pushnumber(L, 5);
    // -> [ 5, 10, fn_print ]

    lua_arith(L, LUA_OPADD);
    // Pop the top two values off the stack and adds them together.
    // Pushes the result back onto the stack.
    // -> [ 15, fn_print ]

    // args:
    //    Lua State
    //    Number of parameters to pass to function (consumes parameters from the top)
    //    Number of return values expected by function
    //    Where in the stack is the function to call

    // -> [ 15, fn_print ]
    //      -1    -2
    // Want to call function at location -2
    // Want to pass 1 paramter (the thing on the top of the stack
    // Print doesn't return a value so we pass 0 for the third argument
    lua_pcall(L, 1, 0, -2);

    std::fstream lua_file("init.lua");
    std::string line;
    std::getline(lua_file, line);
    std::cout << "Read " << line << " from init.lua\n";

    lua_CFunction getCurrentFrame = LuaC_GetCurrentFrame;

    lua_register(L, "get_current_frame", getCurrentFrame);

    luaL_dofile(L, "init.lua");
}

void Engine::CallLuaUpdate()
{
    lua_getglobal(L, "Update");
    lua_pcall(L, 0, 0, -1);
}

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

    PlayerComponent() : gameObject(nullptr), speed(25), xTeleportDestination(50), yTeleportDestination(50) {}

    PlayerComponent(GameObject* owner) : gameObject(owner), speed(25), xTeleportDestination(50), yTeleportDestination(50) {}

    void Update()
    {
        float movement = Engine::GetKey(SDL_SCANCODE_DOWN) - Engine::GetKey(SDL_SCANCODE_UP);
        movement *= speed;

        gameObject->y += movement * Engine::Dt();

        if (Engine::GetKeyDown(SDL_SCANCODE_T))
        {
            std::cout << "T pressed this frame!\n";
            gameObject->x = xTeleportDestination;
            gameObject->y = yTeleportDestination;
        }
    }

    struct Data
    {
        float speed;
        float xTeleportDestination;
        float yTeleportDestination;
    };

    float speed;
    float xTeleportDestination;
    float yTeleportDestination;
    GameObject* gameObject = nullptr;
};

class RectangleRenderComponent
{
public:
    RectangleRenderComponent() : gameObject(nullptr), height(50), width(50) {}

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
    SinMovement() : gameObject(nullptr) {}

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

/*
// Comparison of AoS vs. SoA ------------------
struct GameObjectStruct {
    RectangleRenderComponent c;
    SinMovement s;
};

// Array-of-structures
GameObjectStruct objects[200];

// Structure-of-Arrays
struct GameObjectArray
{
    RectangleRenderComponent cs[200];
    SinMovement s[200];
};

GameObjectArray gameObjects;
// -----------------------------------------------
*/

const size_t MAX_GAMEOBJECTS = 200;

// Untested but plausible free list implementation
class PlayerPool
{
    PlayerPool()
    {
        for (int i = 0; i < MAX_GAMEOBJECTS - 1; i++)
        {
            components[i].next = &components[i + 1];
        }
        components[MAX_GAMEOBJECTS - 1].next = nullptr;
        first = &components[0];
    }

    PlayerComponent::Data* New(GameObject* go)
    {
        PlayerComponent::Data* toReturn = &first->component;
        first = first->next;
        return toReturn;
    }

    void Delete(PlayerComponent::Data* toDelete)
    {
        Node* tmp = first;
        first = (Node*)toDelete;
        first->next = tmp;
    }

private:
    union Node
    {
        PlayerComponent::Data component;
        Node* next;
    };
    Node components[MAX_GAMEOBJECTS];

    Node* first;
};

template<typename T>
class ComponentPool
{
public:
    ComponentPool()
    {
        for (int i = 0; i < MAX_GAMEOBJECTS; i++)
        {
            isActive[i] = false;
        }
    }

    void Update()
    {
        for (int i = 0; i < MAX_GAMEOBJECTS; i++)
        {
            if (isActive[i])
            {
                components[i].Update();
            }
        }
    }

    void Render(SDL_Renderer* renderer)
    {

        for (int i = 0; i < MAX_GAMEOBJECTS; i++)
        {
            if (isActive[i])
            {
                components[i].Render(renderer);
            }
        }
    }

    T* New(GameObject* go)
    {
        for (int i = 0; i < MAX_GAMEOBJECTS; i++)
        {
            if (!isActive[i])
            {
                T* next = &components[i];
                next->gameObject = go;
                isActive[i] = true;
                return next;
            }
        }

        // Ran out of components to return :(
        assert(false);
    }

    // You implement Delete ;)

public:
    T components[MAX_GAMEOBJECTS];
    bool isActive[MAX_GAMEOBJECTS];
};

class ToBeAllocated
{
public:
    ToBeAllocated()
    {
        std::cout << "I've been allocated at address " << this << std::endl;
    }

    virtual void Print()
    {
        std::cout << "ToBeAllocated::Print()\n";
    }
};

class ToBeAllocatedChild : public ToBeAllocated
{
public:
    ToBeAllocatedChild() {}

    void Print() override
    {
        std::cout << "Child::Print() -- Polymorphism works!\n";
    }
};

void StackAllocatorExample()
{
    StackAllocator levelAllocator(5);

    int* int1 = levelAllocator.New<int>();
    *int1 = 100;
    int* int2 = levelAllocator.New<int>();
    if (int2 == nullptr)
    {
        std::cout << "Allocation failed (as expected)\n";
    }
    char* letter = levelAllocator.New<char>();
    if (letter == nullptr)
    {
        std::cout << "Letter allocation failed unexpectedly\n";
    }
    else
    {
        *letter = 'a';
    }

    if (int1 != nullptr && letter != nullptr)
    {
        std::cout << "int1: " << *int1 << "  letter:  " << *letter << std::endl;
    }

    levelAllocator.Clear();

    ToBeAllocated* dummy = levelAllocator.New<ToBeAllocatedChild>();
    std::cout << "levelAllocator starts at " << levelAllocator.Data() << std::endl;
    dummy->Print();
}

void StackPolymorphismExample()
{
    StackAllocator levelAllocator;

    ToBeAllocated* dummy = levelAllocator.New<ToBeAllocatedChild>();
    std::cout << "levelAllocator starts at " << levelAllocator.Data() << std::endl;
    dummy->Print();
}

void ArrayAllocationWithStack()
{
    StackAllocator frameAllocator;

    int* array = frameAllocator.StartArray<int>();
    size_t arrayLen = 0;
    srand(time(NULL));

    for (int i = 0; i < 100; i++)
    {
        int dieRoll = rand() % 100;
        if (dieRoll < 50)
        {
            int* arraySlot = frameAllocator.PushArray<int>();
            *arraySlot = dieRoll;
            arrayLen++;
        }
    }

    std::cout << "Allocated " << arrayLen << " integers:\n\n";
    for (int i = 0; i < arrayLen; i++)
    {
        std::cout << array[i] << std::endl;
    }
}

void VirtualAllocExamples()
{
    // https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    size_t page_size = info.dwPageSize;


    // Can use VirtualAlloc to allocate a specific pointer, if we want.
    char* myPointer = (char*)0x000001CDEF6C00ab;

    // Can allocate Virtual Memory and bind it to Physical Memory all at once
    //    with MEM_RESERVE | MEM_COMMIT, or can just reserve memory (which is very cheap!)
    //    and then commit pages individually later.
    void* page = VirtualAlloc(myPointer, page_size * 20, MEM_RESERVE, PAGE_NOACCESS);
    assert(page != nullptr);

    // Commit two pages of the 20 pages we reserved.
    VirtualAlloc(page, page_size, MEM_COMMIT, PAGE_READWRITE);
    VirtualAlloc((char*)page + page_size, page_size, MEM_COMMIT, PAGE_READWRITE);

    // Can decommit a page, unbinding it from physical memory but keeping the virtual
    // page still allocated (and thus, committable in the future).
    VirtualFree((char*)page + page_size, page_size, MEM_DECOMMIT);

    // If we instead wanted to decommit the page and release the virtual memory (so it
    // can be reallocated by another call to VirtualAlloc), we would instead do:
    // VirtualFree((char*)page + page_size, 0, MEM_RELEASE);

    VirtualAlloc((char*)page + page_size, page_size, MEM_COMMIT, PAGE_READWRITE);

    // Read into the third page -- this causes a crash (read access violation)
    for (int i = 0; i < 2 * (page_size / sizeof(int)); i++)
    {
        int* pageAsInts = (int*)page;
        std::cout << pageAsInts[i] << std::endl;
    }

    std::cout << "page: " << page << std::endl;
    std::cout << "myPointer: " << (void*)myPointer << std::endl;
    //myPointer = (char*)page;
    *myPointer = 'h';
    *(myPointer + 1) = 'i';
    std::cout << *myPointer << *(myPointer + 1) << std::endl;
}

void Engine::RunGameLoop(SDL_Renderer* renderer)
{
    InitLua();

    SDL_Event event;
    bool quit = false;
    Uint32 frameStart = SDL_GetTicks64();

    GameObject gameObjects[MAX_GAMEOBJECTS];
    ComponentPool<RectangleRenderComponent> renderers;
    ComponentPool<PlayerComponent> playerComponents;
    ComponentPool<SinMovement> sinComponents;

    //StackPolymorphismExample();

    //GameObject* gameObjects = levelAllocator.New()

    // Initialize player object
    {
        GameObject& player = gameObjects[0];
        player.playerComponent = playerComponents.New(&player);
        player.renderComponent = renderers.New(&player);
        player.x = 100;
        player.y = 100;
    }

    // Initialize moving rectangle
    {
        GameObject& movingRectangle = gameObjects[1];
        movingRectangle.sinMovement = sinComponents.New(&movingRectangle);
        movingRectangle.renderComponent = renderers.New(&movingRectangle);
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

            // Update each component type
            playerComponents.Update();

            sinComponents.Update();

            CallLuaUpdate();

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
            SDL_RenderClear(renderer);

            renderers.Render(renderer);

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