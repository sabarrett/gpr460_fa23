#include "System.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <crtdbg.h>


using namespace gpr460;

wchar_t*
gpr460::fromUTF8(
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

class System_Win : public System
{
    // Inherited via System
    void Initialize() override
    {
    }
    void PreGameStartup() override
    {
    }
    void PostGameShutdown() override
    {
    }
    void DisplayMessageBox(const string& msg) override
    {
    }
    void WriteToLogFile(const string& msg) override
    {
    }
};

System* System::Create()
{
    std::cout << "Creating Win32 version of System\n";
	return new System_Win();
}