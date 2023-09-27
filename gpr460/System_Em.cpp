#include "System.h"

using namespace gpr460;

class System_Em : public System
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
    void Shutdown() override
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
    std::cout << "Creating emscripten version of System\n";
	return new System_Em();
}