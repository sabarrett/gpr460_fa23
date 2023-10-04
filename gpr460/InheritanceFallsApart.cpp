// Throwing this code out >:(

#if 0

class MovingRectangle
{
public:
    float x, y, width, height;
    bool colorSwitch;

    MovingRectangle() :x(100), y(100), width(50), height(50), colorSwitch(true)
    {
    }

    virtual void Update()
    {
        // Shared between player and other rectangles
        x = (SDL_sinf(Engine::GetCurrentFrame() / 10.0f) * 100) + 200;
    }

    virtual void Render(SDL_Renderer* renderer)
    {
        SDL_Rect r = {
            x,
            y,
            width * 1.1,
            height * 1.1
        };

        if (colorSwitch)
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
        }
        SDL_RenderFillRect(renderer, &r);
    }
};

class Player
{
public:
    void Update() override
    {
        //MovingRectangle::Update();

        // Player-Specific Code
        float movement = Engine::GetKey(SDL_SCANCODE_DOWN) - Engine::GetKey(SDL_SCANCODE_UP);
        movement *= 25;

        y += movement * Engine::Dt();

        if (Engine::GetKeyDown(SDL_SCANCODE_T))
        {
            std::cout << "T pressed this frame!\n";
            y = 50;
            x = 50;
        }
    }

    void Render(SDL_Renderer* renderer) override
    {
        SDL_Rect r = {
            x,
            y,
            width,
            height
        };

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &r);
    }
};

#endif