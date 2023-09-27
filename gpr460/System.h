#pragma once

#include <cstdlib>
#include <string>
#include <iostream>


#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

namespace gpr460
{

    typedef std::string string;

    // fromUTF8 taken from https://gist.github.com/xebecnan/6d070c93fb69f40c3673.
    wchar_t*
        fromUTF8(
            const char* src,
            size_t src_length,  /* = 0 */
            size_t* out_length  /* = NULL */
        );

    class System
    {
    public:

        static System* Create();

        static void Destroy(System* sys) { delete sys; }

        virtual ~System() {}

        // This call is already added to main().
        virtual void Initialize() = 0;

        // TODO: Call this before the game loop starts.
        virtual void PreGameStartup() = 0;

        // TODO: Call this after the game loop exits.
        virtual void PostGameShutdown() = 0;

        // TODO: Call this just before or after the
        //      SDL_Destroy*() group of calls.
        virtual void Shutdown() = 0;

        virtual void DisplayMessageBox(const string& msg) = 0;

        // This is already called in main()
        virtual void WriteToLogFile(const string& msg) = 0;
    };
}