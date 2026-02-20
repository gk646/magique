#include <magique/magique.hpp>

enum class EntityType : uint16_t
{
    SNAKE_HEAD,
    SNAKE_TAIL,
    APPLE,
    CAMERA,
};

enum Direction : uint8_t
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

int SCORE = 0;
int MOVES = 0;
int FIELD_DIMS = 30;
int CELL_SIZE = 15;
int DELAY = 30;
int APPLE_DELAY = 90;
bool GAMEOVER = false;
magique::Slider SPEED_SLIDER{{150, 50}, magique::Anchor::TOP_RIGHT, 50};

void resetGame()
{
    SCORE = 0;
    MOVES = 0;
    GAMEOVER = false;
    SPEED_SLIDER.setSliderPercent(0.5);
    magique::EntityDestroy({EntityType::SNAKE_TAIL, EntityType::SNAKE_HEAD, EntityType::APPLE});
    magique::EntityCreate(EntityType::SNAKE_HEAD, magique::Point ::Random(0, FIELD_DIMS - 1) * CELL_SIZE, {});
}

struct RestartButton : magique::Button
{
    RestartButton() : Button({100, 100}, magique::Anchor::MID_CENTER) {}
    void onDraw(const magique::Rect& bounds) override
    {
        drawDefault(bounds);
        magique::DrawTextCentered(magique::EngineGetFont(), "Restart", bounds.mid(), 20);
    }
    void onClick(const Rectangle& bounds, int button) override { resetGame(); }
};

RestartButton RESTART_BUTTON{};

struct SnakeC final
{
    Direction direction = UP;
    entt::entity next = entt::null;
};


struct SnakeScript : magique::EntityScript
{
    void onTick(entt::entity self, bool updated) override
    {
        auto& head = magique::ComponentGet<SnakeC>(self);
        if (magique::LayeredInput::IsKeyDown(KEY_W) && head.direction != DOWN)
        {
            head.direction = UP;
        }
        else if (magique::LayeredInput::IsKeyDown(KEY_A) && head.direction != RIGHT)
        {
            head.direction = LEFT;
        }
        else if (magique::LayeredInput::IsKeyDown(KEY_S) && head.direction != UP)
        {
            head.direction = DOWN;
        }
        else if (magique::LayeredInput::IsKeyDown(KEY_D) && head.direction != LEFT)
        {
            head.direction = RIGHT;
        }
    }

    void onDynamicCollision(entt::entity self, entt::entity other, magique::CollisionInfo& info) override
    {
        const auto& oPos = magique::ComponentGet<magique::PositionC>(other);
        if (oPos.type == EntityType::SNAKE_TAIL)
        {
            GAMEOVER = true;
        }
        else if (oPos.type == EntityType::APPLE)
        {
            magique::EntityDestroy(other);
            auto& head = magique::ComponentGet<SnakeC>(self);

            entt::entity tail = head.next;
            if (head.next == entt::null)
            {
                tail = self;
            }
            else
            {
                while (magique::ComponentGet<SnakeC>(tail).next != entt::null)
                {
                    tail = magique::ComponentGet<SnakeC>(tail).next;
                }
            }

            const auto& lastPos = magique::ComponentGet<magique::PositionC>(tail);
            const auto& lastSnake = magique::ComponentGet<SnakeC>(tail);
            float xAdd = 0;
            float yAdd = 0;
            if (lastSnake.direction == UP)
            {
                yAdd = CELL_SIZE;
            }
            else if (lastSnake.direction == LEFT)
            {
                xAdd = CELL_SIZE;
            }
            else if (lastSnake.direction == DOWN)
            {
                yAdd = -CELL_SIZE;
            }
            else if (lastSnake.direction == RIGHT)
            {
                xAdd = -CELL_SIZE;
            }
            const auto nextTail = magique::EntityCreate(EntityType::SNAKE_TAIL, lastPos.x + xAdd, lastPos.y + yAdd, {});
            if (head.next == entt::null)
            {
                head.next = nextTail;
            }
            else
            {
                magique::ComponentGet<SnakeC>(tail).next = nextTail;
            }
            SCORE++;
        }
    }
};

struct SnakeTailScript final : magique::EntityScript
{
    void onDynamicCollision(entt::entity self, entt::entity other, magique::CollisionInfo& info) override {}
};

void MoveSnakeHead(entt::entity entity)
{
    auto& pos = magique::ComponentGet<magique::PositionC>(entity);
    auto& head = magique::ComponentGet<SnakeC>(entity);
    const auto limit = CELL_SIZE * FIELD_DIMS;
    if (head.direction == UP)
    {
        pos.y -= CELL_SIZE;
        if (pos.y < 0)
        {
            pos.y = limit - CELL_SIZE;
        }
    }
    else if (head.direction == LEFT)
    {
        pos.x -= CELL_SIZE;
        if (pos.x < 0)
        {
            pos.x = limit - CELL_SIZE;
        }
    }
    else if (head.direction == DOWN)
    {
        pos.y += CELL_SIZE;
        if (pos.y >= limit)
        {
            pos.y = 0;
        }
    }
    else if (head.direction == RIGHT)
    {
        pos.x += CELL_SIZE;
        if (pos.x >= limit)
        {
            pos.x = 0;
        }
    }
}

struct CameraScript : magique::EntityScript
{
    void onDynamicCollision(entt::entity self, entt::entity other, magique::CollisionInfo& info) override {}
};

struct AppleScript : magique::EntityScript
{
    void onDynamicCollision(entt::entity self, entt::entity other, magique::CollisionInfo& info) override {}
};

struct Snake final : magique::Game
{
    void onStartup(magique::AssetLoader& loader) override
    {
        SetWindowState(FLAG_WINDOW_RESIZABLE);
        magique::EntityRegister(EntityType::SNAKE_HEAD,
                                [](entt::entity entity, EntityType type)
                                {
                                    magique::ComponentGiveActor(entity);
                                    magique::ComponentGive<SnakeC>(entity);
                                    magique::GiveCollisionRect(entity, CELL_SIZE, CELL_SIZE);
                                });
        magique::EntityRegister(EntityType::SNAKE_TAIL,
                                [](entt::entity entity, EntityType type)
                                {
                                    magique::ComponentGive<SnakeC>(entity);
                                    magique::GiveCollisionRect(entity, CELL_SIZE, CELL_SIZE);
                                });
        magique::EntityRegister(EntityType::CAMERA,
                                [](entt::entity entity, EntityType type) { magique::ComponentGiveCamera(entity); });

        magique::EntityRegister(EntityType::APPLE, [](entt::entity entity, EntityType type)
                                { magique::GiveCollisionRect(entity, CELL_SIZE, CELL_SIZE); });

        magique::ScriptingSetScript(EntityType::SNAKE_HEAD, new SnakeScript());
        magique::ScriptingSetScript(EntityType::CAMERA, new CameraScript());
        magique::ScriptingSetScript(EntityType::SNAKE_TAIL, new SnakeTailScript());
        magique::ScriptingSetScript(EntityType::APPLE, new AppleScript());

        const auto limit = FIELD_DIMS * CELL_SIZE;
        magique::EntityCreate(EntityType::CAMERA, limit / 2, limit / 2, {});
        SPEED_SLIDER.setScaleImblanced(0, 1, 10);
    }

    void onLoadingFinished() override { resetGame(); }

    void onUpdateGame(GameState gameState) override
    {
        if (GAMEOVER)
        {
            return;
        }

        static int appleCounter = 0;
        appleCounter++;
        if (appleCounter >= APPLE_DELAY)
        {
            appleCounter = 0;
            auto x = GetRandomValue(0, FIELD_DIMS - 1);
            auto y = GetRandomValue(0, FIELD_DIMS - 1);
            magique::EntityCreate(EntityType::APPLE, x * CELL_SIZE, y * CELL_SIZE, {});
        }

        static int counter = 0;
        counter++;
        if (counter < DELAY * (1 / SPEED_SLIDER.getSliderValue()))
            return;
        counter = 0;

        auto head = magique::GetEntity(EntityType::SNAKE_HEAD);
        if (head == entt::null)
            return;

        magique::PositionC prevPos = magique::ComponentGet<magique::PositionC>(head);
        auto direction = magique::ComponentGet<SnakeC>(head).direction;
        MoveSnakeHead(head);
        entt::entity iter = magique::ComponentGet<SnakeC>(head).next;
        while (iter != entt::null)
        {
            auto& pos = magique::ComponentGet<magique::PositionC>(iter);
            auto& snake = magique::ComponentGet<SnakeC>(iter);
            auto tempDirection = snake.direction;
            snake.direction = direction;
            direction = tempDirection;
            auto tempPos = pos;
            pos.x = prevPos.x;
            pos.y = prevPos.y;
            prevPos = tempPos;
            iter = snake.next;
        }
        MOVES++;
    }

    void onDrawGame(GameState gameState, Camera2D& camera2D) override
    {
        auto text = std::string{std::string("Score: ") + std::to_string(SCORE)}.c_str();
        magique::DrawTextCentered(magique::EngineGetFont(), text, Vector2{GetScreenWidth() / 2.0F, 50}, 20);

        text = std::string{std::string("Moves: ") + std::to_string(MOVES)}.c_str();
        magique::DrawTextCentered(magique::EngineGetFont(), text, Vector2{GetScreenWidth() / 2.0F, 75}, 20);

        text = std::string{std::string("Speed: ") + std::to_string(SPEED_SLIDER.getSliderValue())}.c_str();
        const auto textPos = magique::GetUIAnchor(magique::Anchor::TOP_RIGHT, 0, 0, 100);
        magique::DrawTextCentered(magique::EngineGetFont(), text, Vector2{textPos.x, textPos.y}, 20);


        BeginMode2D(camera2D);
        const auto limit = FIELD_DIMS * CELL_SIZE;
        DrawRectangle(0, 0, limit, limit, GRAY);

        for (const auto e : magique::EngineGetDrawEntities())
        {
            const auto& pos = magique::ComponentGet<const magique::PositionC>(e);
            if (pos.type == EntityType::SNAKE_HEAD)
            {
                DrawRectangle((int)pos.x, (int)pos.y, CELL_SIZE, CELL_SIZE, BLUE);
            }
            else if (pos.type == EntityType::SNAKE_TAIL)
            {
                DrawRectangle((int)pos.x, (int)pos.y, CELL_SIZE, CELL_SIZE, GREEN);
                DrawRectangleLines((int)pos.x, (int)pos.y, CELL_SIZE, CELL_SIZE, GRAY);
            }
            else if (pos.type == EntityType::APPLE)
            {
                DrawRectangle((int)pos.x, (int)pos.y, CELL_SIZE, CELL_SIZE, RED);
            }
        }
        EndMode2D();
    }

    void onDrawUI(GameState gameState) override
    {
        SPEED_SLIDER.draw();
        if (GAMEOVER)
        {
            RESTART_BUTTON.draw();
        }
    }
};


int main()
{
    Snake game{};
    return game.run();
}
