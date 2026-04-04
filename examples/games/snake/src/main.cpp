#include <magique/magique.hpp>

enum class GameState : uint8_t
{
    None,
    Game,
    GameOver,
};

struct State
{
    int score = 0;
    int moves = 0;
    float cellSize = 24;
    magique::Point boardDims = {16};
    float foodDelaySec = 2.0F;
    float moveDelaySec = 0.5F;
    bool gameOver = false;
    magique::Direction direction = magique::Direction::UP;
    magique::Counter moveDelay{moveDelaySec / (MAGIQUE_TICK_TIME)};
    magique::Counter foodDelay{foodDelaySec / (MAGIQUE_TICK_TIME)};

    std::vector<magique::Point> snake;
    std::vector<magique::Point> food;
};

State STATE{};

struct InfoPanel : magique::UIObject
{
    InfoPanel() : UIObject{magique::Rect{300}, magique::Anchor::TOP_LEFT, magique::Point{25}} {}

    void onDraw(const magique::Rect& bounds) override
    {
        magique::TextDrawer drawer{magique::EngineGetFont(), bounds};

        drawer.modSize(3).left(GRAY, "Score: %d", STATE.score);
        drawer.linebreak(3);

        drawer.modSize(3).left(GRAY, "Moves: %d", STATE.moves);
        drawer.linebreak(3);

        drawer.modSize(3).left(GRAY, "Speed: %f s", STATE.moveDelaySec);
        drawer.linebreak(3);
    }
};

struct Snake final : magique::Game
{
    void onStartup(magique::AssetLoader& loader) override
    {
        auto& gameScene = magique::SceneGet(GameState::Game);
        gameScene.addObject(new InfoPanel());

        auto& slider = gameScene.addObject(new magique::Slider{{150, 50}, magique::Anchor::TOP_RIGHT, 50});
        slider.setScaleImblanced(2, 0.5, 0.05);
        slider.setOnChange(
            [&](float val, float perc)
            {
                STATE.moveDelaySec = val;
                STATE.moveDelay = {val / (MAGIQUE_TICK_TIME)};
            });

        auto& gameOverScene = magique::SceneGet(GameState::GameOver);
        gameOverScene.addObject(new magique::TextButton{"Restart", magique::Anchor::TOP_RIGHT, 50});

        SetWindowState(FLAG_WINDOW_RESIZABLE);
    }

    void onLoadingFinished() override
    {
        ResetGame();
        magique::EngineSetState(GameState::Game);
    }

    void onUpdateGame(GameState gameState) override
    {
        if (gameState != GameState::Game)
        {
            return;
        }

        if (magique::LayeredInput::IsKeyDown(KEY_W) && STATE.direction != magique::Direction::DOWN)
        {
            STATE.direction = magique::Direction::UP;
        }
        else if (magique::LayeredInput::IsKeyDown(KEY_A) && STATE.direction != magique::Direction::RIGHT)
        {
            STATE.direction = magique::Direction::LEFT;
        }
        else if (magique::LayeredInput::IsKeyDown(KEY_S) && STATE.direction != magique::Direction::UP)
        {
            STATE.direction = magique::Direction::DOWN;
        }
        else if (magique::LayeredInput::IsKeyDown(KEY_D) && STATE.direction != magique::Direction::LEFT)
        {
            STATE.direction = magique::Direction::RIGHT;
        }

        if (STATE.foodDelay.tick())
        {
            magique::Point pos = magique::Rect{STATE.boardDims}.random(1).floor();
            while (std::ranges::contains(STATE.food, pos))
            {
                pos = magique::Rect{STATE.boardDims}.random(1).floor();
            }
            STATE.food.push_back(pos);
        }

        if (STATE.moveDelay.tick())
        {
            STATE.moves++;
            magique::Point dx{};
            if (STATE.direction == magique::Direction::UP)
                dx = {0, -1};
            if (STATE.direction == magique::Direction::RIGHT)
                dx = {1, 0};
            if (STATE.direction == magique::Direction::DOWN)
                dx = {0, 1};
            if (STATE.direction == magique::Direction::LEFT)
                dx = {-1, 0};

            auto headPos = STATE.snake[0] + dx;
            headPos.x = (int)(headPos.x + STATE.boardDims.x) % (int)STATE.boardDims.x;
            headPos.y = (int)(headPos.y + STATE.boardDims.y) % (int)STATE.boardDims.y;

            STATE.snake.insert(STATE.snake.begin(), headPos);
            STATE.snake.pop_back();

            // Self collision
            if (STATE.snake.size() > 1)
            {
                for (auto& snake : STATE.snake)
                {
                    if (&snake != &STATE.snake.front() && snake == STATE.snake.front())
                    {
                        magique::EngineSetState(GameState::GameOver);
                    }
                }
            }
        }

        for (auto& food : STATE.food)
        {
            if (food == STATE.snake.front())
            {
                STATE.snake.push_back(food);
                std::erase(STATE.food, food);
                STATE.score++;
                break;
            }
        }
    }

    static void DrawBoard()
    {
        for (auto i = 0; i < (int)STATE.boardDims.y; i++)
        {
            for (int j = 0; j < (int)STATE.boardDims.x; j++)
            {
                DrawRectangleLinesEx(magique::Rect{magique::Point{(float)j, (float)i} * STATE.cellSize, STATE.cellSize},
                                     1, BLACK);
            }
        }
    }

    static void DrawObjects()
    {
        for (auto& snake : STATE.food)
        {
            DrawRectangleRec(magique::Rect{snake * STATE.cellSize, STATE.cellSize}, RED);
        }
        for (auto& snake : STATE.snake)
        {
            const auto color = &STATE.snake.front() == &snake ? BLUE : GREEN;
            DrawRectangleRec(magique::Rect{snake * STATE.cellSize, STATE.cellSize}, color);
        }
    }

    void ResetGame()
    {
        STATE.score = 0;
        STATE.moves = 0;
        STATE.gameOver = false;
        STATE.food.clear();
        STATE.snake.clear();
        STATE.snake.push_back(STATE.boardDims / 2);
    }

    void onDrawGame(GameState gameState, Camera2D& camera2D) override
    {
        camera2D.target = magique::Point{STATE.boardDims * STATE.cellSize} / 2.0F;
        BeginMode2D(camera2D);

        DrawObjects();
        DrawBoard();

        EndMode2D();
    }

    void onDrawUI(GameState gameState) override
    {
        auto& scene = magique::SceneGet(gameState);
        scene.draw();
    }
};

int main()
{
    Snake game{};
    return game.run();
}
