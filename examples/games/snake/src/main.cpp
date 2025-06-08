#include <magique/magique.hpp>


enum EntityType : uint16_t
{
    SNAKE_HEAD,
    SNAKE_TAIL,
    APPLE,
};

float MOVE_SPEED = 25;
float MOVE_DELAY = 1.0F;

enum Direction : uint8_t
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct SnakeHeadC final
{
    Direction direction = UP;
};

struct SnakeTailC final
{
    entt::entity next = entt::null;
};

struct SnakeScript : magique::EntityScript
{
    void onTick(entt::entity self, bool updated) override
    {
        auto& head = magique::GetComponent<SnakeHeadC>(self);
        if (magique::UIInput::IsKeyDown(KEY_W))
        {
            head.direction = Direction::UP;
        }
        if (magique::UIInput::IsKeyDown(KEY_A))
        {
            head.direction = Direction::LEFT;
        }
        if (magique::UIInput::IsKeyDown(KEY_S))
        {
            head.direction = Direction::DOWN;
        }
        if (magique::UIInput::IsKeyDown(KEY_D))
        {
            head.direction = Direction::RIGHT;
        }
    }
};


struct SnakeTailScript final : magique::EntityScript
{
};

struct Snake final : magique::Game
{
    void onStartup(magique::AssetLoader& loader) override
    {
        magique::RegisterEntity(SNAKE_HEAD,
                                [](entt::entity entity, EntityType type)
                                {
                                    magique::GiveActor(entity);
                                    magique::GiveComponent<SnakeHeadC>(entity);
                                    magique::GiveCollisionRect(entity, MOVE_SPEED, MOVE_SPEED);
                                });
        magique::SetEntityScript(SNAKE_HEAD, new SnakeScript());
        magique::RegisterEntity(SNAKE_TAIL,
                                [](entt::entity entity, EntityType type)
                                {
                                    magique::GiveComponent<SnakeTailC>(entity);
                                    magique::GiveCollisionRect(entity, MOVE_SPEED, MOVE_SPEED);
                                });
        magique::SetEntityScript(SNAKE_HEAD, new SnakeTailScript());
    }

    void onLoadingFinished() override
    {
        magique::CreateEntity(SNAKE_HEAD, 0, 0, {});

    }

    void updateGame(GameState gameState) override {}

    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        magique::GetCamera().target.x = GetScreenWidth() / 2;
        magique::GetCamera().target.y = GetScreenHeight() / 2;
        BeginMode2D(camera2D);
        for (int i = 0; i < GetScreenWidth() / (int)MOVE_SPEED +1; ++i)
        {
            auto gap = 0 + i * (int)MOVE_SPEED;
            DrawLine(gap, 0, gap, GetScreenHeight(), BLACK);
            DrawLine(0, gap, GetScreenWidth(), gap, BLACK);
        }



        EndMode2D();
    }

    void drawUI(GameState gameState) override {}
};


int main()
{
    Snake game{};
    return game.run();
}