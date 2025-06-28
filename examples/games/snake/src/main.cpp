#include "magique/ui/controls/Slider.h"
#include <magique/magique.hpp>

enum EntityType : uint16_t
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
magique::Slider SPEED_SLIDER{150, 50, magique::Anchor::TOP_RIGHT, 50};

void resetGame()
{
    SCORE = 0;
    MOVES = 0;
    GAMEOVER = false;
    SPEED_SLIDER.setSliderPercent(0.5);
    magique::DestroyEntities({SNAKE_TAIL, SNAKE_HEAD, APPLE});
    magique::CreateEntity(SNAKE_HEAD, GetRandomValue(0, FIELD_DIMS - 1) * CELL_SIZE,
                          GetRandomValue(0, FIELD_DIMS - 1) * CELL_SIZE, {});
}

struct RestartButton : magique::Button
{
    RestartButton() : Button(100, 100, magique::Anchor::MID_CENTER) {}
    void onDraw(const Rectangle& bounds) override
    {
        drawDefault(bounds);
        magique::DrawCenteredText(magique::GetEngineFont(), "Restart", magique::GetRectCenter(bounds), 20);
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
        auto& head = magique::GetComponent<SnakeC>(self);
        if (magique::UIInput::IsKeyDown(KEY_W) && head.direction != DOWN)
        {
            head.direction = UP;
        }
        else if (magique::UIInput::IsKeyDown(KEY_A) && head.direction != RIGHT)
        {
            head.direction = LEFT;
        }
        else if (magique::UIInput::IsKeyDown(KEY_S) && head.direction != UP)
        {
            head.direction = DOWN;
        }
        else if (magique::UIInput::IsKeyDown(KEY_D) && head.direction != LEFT)
        {
            head.direction = RIGHT;
        }
    }

    void onDynamicCollision(entt::entity self, entt::entity other, magique::CollisionInfo& info) override
    {
        const auto& oPos = magique::GetComponent<magique::PositionC>(other);
        if (oPos.type == SNAKE_TAIL)
        {
            GAMEOVER = true;
        }
        else if (oPos.type == APPLE)
        {
            magique::DestroyEntity(other);
            auto& head = magique::GetComponent<SnakeC>(self);

            entt::entity tail = head.next;
            if (head.next == entt::null)
            {
                tail = self;
            }
            else
            {
                while (magique::GetComponent<SnakeC>(tail).next != entt::null)
                {
                    tail = magique::GetComponent<SnakeC>(tail).next;
                }
            }

            const auto& lastPos = magique::GetComponent<magique::PositionC>(tail);
            const auto& lastSnake = magique::GetComponent<SnakeC>(tail);
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
            const auto nextTail = magique::CreateEntity(SNAKE_TAIL, lastPos.x + xAdd, lastPos.y + yAdd, {});
            if (head.next == entt::null)
            {
                head.next = nextTail;
            }
            else
            {
                magique::GetComponent<SnakeC>(tail).next = nextTail;
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
    auto& pos = magique::GetComponent<magique::PositionC>(entity);
    auto& head = magique::GetComponent<SnakeC>(entity);
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
        magique::RegisterEntity(SNAKE_HEAD,
                                [](entt::entity entity, EntityType type)
                                {
                                    magique::GiveActor(entity);
                                    magique::GiveComponent<SnakeC>(entity);
                                    magique::GiveCollisionRect(entity, CELL_SIZE, CELL_SIZE);
                                });
        magique::RegisterEntity(SNAKE_TAIL,
                                [](entt::entity entity, EntityType type)
                                {
                                    magique::GiveComponent<SnakeC>(entity);
                                    magique::GiveCollisionRect(entity, CELL_SIZE, CELL_SIZE);
                                });
        magique::RegisterEntity(CAMERA, [](entt::entity entity, EntityType type) { magique::GiveCamera(entity); });

        magique::RegisterEntity(APPLE, [](entt::entity entity, EntityType type)
                                { magique::GiveCollisionRect(entity, CELL_SIZE, CELL_SIZE); });

        magique::SetEntityScript(SNAKE_HEAD, new SnakeScript());
        magique::SetEntityScript(CAMERA, new CameraScript());
        magique::SetEntityScript(SNAKE_TAIL, new SnakeTailScript());
        magique::SetEntityScript(APPLE, new AppleScript());

        const auto limit = FIELD_DIMS * CELL_SIZE;
        magique::CreateEntity(CAMERA, limit / 2, limit / 2, {});
        SPEED_SLIDER.setScaleImblanced(0, 1, 10);
    }

    void onLoadingFinished() override { resetGame(); }

    void updateGame(GameState gameState) override
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
            magique::CreateEntity(APPLE, x * CELL_SIZE, y * CELL_SIZE, {});
        }

        static int counter = 0;
        counter++;
        if (counter < DELAY * (1 / SPEED_SLIDER.getSliderValue()))
            return;
        counter = 0;

        auto head = magique::GetEntity(SNAKE_HEAD);
        if (head == entt::null)
            return;

        magique::PositionC prevPos = magique::GetComponent<magique::PositionC>(head);
        auto direction = magique::GetComponent<SnakeC>(head).direction;
        MoveSnakeHead(head);
        entt::entity iter = magique::GetComponent<SnakeC>(head).next;
        while (iter != entt::null)
        {
            auto& pos = magique::GetComponent<magique::PositionC>(iter);
            auto& snake = magique::GetComponent<SnakeC>(iter);
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

    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        auto text = std::string{std::string("Score: ") + std::to_string(SCORE)}.c_str();
        magique::DrawCenteredText(magique::GetEngineFont(), text, Vector2{GetScreenWidth() / 2.0F, 50}, 20);

        text = std::string{std::string("Moves: ") + std::to_string(MOVES)}.c_str();
        magique::DrawCenteredText(magique::GetEngineFont(), text, Vector2{GetScreenWidth() / 2.0F, 75}, 20);

        text = std::string{std::string("Speed: ") + std::to_string(SPEED_SLIDER.getSliderValue())}.c_str();
        const auto textPos = magique::GetUIAnchor(magique::Anchor::TOP_RIGHT, 0, 0, 100);
        magique::DrawCenteredText(magique::GetEngineFont(), text, Vector2{textPos.x, textPos.y}, 20);


        BeginMode2D(camera2D);
        const auto limit = FIELD_DIMS * CELL_SIZE;
        DrawRectangle(0, 0, limit, limit, GRAY);

        for (const auto e : magique::GetDrawEntities())
        {
            const auto& pos = magique::GetComponent<const magique::PositionC>(e);
            if (pos.type == SNAKE_HEAD)
            {
                DrawRectangle((int)pos.x, (int)pos.y, CELL_SIZE, CELL_SIZE, BLUE);
            }
            else if (pos.type == SNAKE_TAIL)
            {
                DrawRectangle((int)pos.x, (int)pos.y, CELL_SIZE, CELL_SIZE, GREEN);
                DrawRectangleLines((int)pos.x, (int)pos.y, CELL_SIZE, CELL_SIZE, GRAY);
            }
            else if (pos.type == APPLE)
            {
                DrawRectangle((int)pos.x, (int)pos.y, CELL_SIZE, CELL_SIZE, RED);
            }
        }
        EndMode2D();
    }

    void drawUI(GameState gameState) override
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