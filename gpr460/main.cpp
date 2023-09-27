
#include <stdlib.h>
#include <crtdbg.h>
#include <iostream>
#include <SDL2/SDL.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

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

// fromUTF8 taken from https://gist.github.com/xebecnan/6d070c93fb69f40c3673.
wchar_t*
fromUTF8(
    const char* src,
    size_t src_length,  /* = 0 */
    size_t* out_length  /* = NULL */
)
{
    if (!src)
    {
        return NULL;
    }

    if (src_length == 0) { src_length = strlen(src); }
    int length = MultiByteToWideChar(CP_UTF8, 0, src, src_length, 0, 0);
    wchar_t* output_buffer = (wchar_t*)malloc((length + 1) * sizeof(wchar_t));
    if (output_buffer) {
        MultiByteToWideChar(CP_UTF8, 0, src, src_length, output_buffer, length);
        output_buffer[length] = L'\0';
    }
    if (out_length) { *out_length = length; }
    return output_buffer;
}

// This should be moved to System.h.
namespace gpr460
{
    typedef std::string string;

    class System
    {
    public:
        // This call is already added to main().
        void Initialize() {
            _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
            _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
        }

        // TODO: Call this before the game loop starts.
        void PreGameStartup() {}

        // TODO: Call this after the game loop exits.
        void PostGameShutdown() {}

        // TODO: Call this just before or after the
        //      SDL_Destroy*() group of calls.
        void Shutdown() {}

        void DisplayMessageBox(const string& msg)
        {
            wchar_t* winString = fromUTF8(msg.c_str(), msg.size(), NULL);
            MessageBox(NULL, winString, L"Message Caption", MB_OK);
            free(winString);
        }

        // This is already called in main()
        void WriteToLogFile(const string& msg)
        {
            HANDLE logFile = CreateFileW(L"Log.txt", GENERIC_WRITE,
                                FILE_SHARE_READ, NULL,
                                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

            std::cout << (SUCCEEDED(logFile) ? "OPENED" : "FAILED :(") << std::endl;

            WriteFile(logFile, msg.c_str(), msg.length(), NULL, NULL);
        }
    };
}

int main(int argc, char* argv[])
{
    const int WIDTH = 640;
    const int HEIGHT = 480;
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    gpr460::System system;
    system.Initialize();

    // Get cout working.
    if (!AllocConsole())
        std::cout << "Alloc failed\n";
    std::cout << "Hi there!\n";

    system.WriteToLogFile("MyFile.txt");

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

    // Set up our initial checkpoint
    //    TODO: Make sure this is done in the System class instead
    //    of happening here!
    _CrtMemState checkPoint;
    _CrtMemCheckpoint(&checkPoint);

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
                    system.DisplayMessageBox("The 'K' key was pressed!");
                    system.WriteToLogFile("The 'K' key was pressed!");
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

    // TODO: Replace these windows calls with
    //    an equivalent call to your system class.
    _CrtMemDumpAllObjectsSince(&checkPoint);

    _CrtMemState checkPoint2;
    _CrtMemState difference;
    _CrtMemCheckpoint(&checkPoint2);
    
    // TODO(Optional): Display leaks in console instead of
    //                  just in Debug window.
    if (_CrtMemDifference(&difference, &checkPoint, &checkPoint2))
    {
        std::cout << "You leaked memory!!!! Check logs for more details >:(\n";
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}