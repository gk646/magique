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
    Point boardDims = {16};
    float foodDelaySec = 2.0F;
    float moveDelaySec = 0.5F;
    bool gameOver = false;
    Direction direction = Direction::UP;
    Counter moveDelay{moveDelaySec / (MAGIQUE_TICK_TIME)};
    Counter foodDelay{foodDelaySec / (MAGIQUE_TICK_TIME)};

    std::vector<Point> snake;
    std::vector<Point> food;
};

State STATE{};

struct InfoPanel : UIObject
{
    InfoPanel() : UIObject{Rect{300}, Anchor::TOP_LEFT, Point{25}} {}

    void onDraw(const Rect& bounds) override
    {
        TextDrawer drawer{EngineGetFont(), bounds};

        drawer.modSize(3).left(GRAY, "Score: %d", STATE.score);
        drawer.linebreak(3);

        drawer.modSize(3).left(GRAY, "Moves: %d", STATE.moves);
        drawer.linebreak(3);

        drawer.modSize(3).left(GRAY, "Speed: %f s", STATE.moveDelaySec);
        drawer.linebreak(3);
    }
};

struct Snake final : Game
{
    void onStartup(AssetLoader& loader) override
    {
        auto& gameScene = SceneGet(GameState::Game);
        gameScene.addObject(new InfoPanel());

        auto& slider = gameScene.addObject(new Slider{{150, 50}, "Speed", Direction::LEFT, Anchor::TOP_RIGHT, 50});
        slider.setScaleZones(1, 0.3, 0.05);
        slider.setOnChange(
            [&](float val, float perc)
            {
                STATE.moveDelaySec = val;
                STATE.moveDelay = {val / (MAGIQUE_TICK_TIME)};
            });

        auto& sizeSlider =
            gameScene.addObject(new Slider{{150, 50}, "Board Size", Direction::LEFT, Anchor::TOP_RIGHT, {50, 100}});
        sizeSlider.setScaleZones(8, 16, 64);
        sizeSlider.setOnChange([&](float val, float perc) { STATE.boardDims = {val}; });

        auto& gameOverScene = SceneGet(GameState::GameOver);
        auto& restart = gameOverScene.addObject(new TextButton{"Restart", Anchor::TOP_RIGHT, 50});
        restart.setOnClick(
            [&](MouseButton button)
            {
                ResetGame();
                EngineSetState(GameState::Game);
            });

        SetWindowState(FLAG_WINDOW_RESIZABLE);
    }

    void onLoadingFinished() override
    {
        ResetGame();
        EngineSetState(GameState::Game);
    }

    void onUpdateGame(GameState gameState) override
    {
        STATE.cellSize = std::round((GetScreenDims().y * 0.8F) / STATE.boardDims.y);
        if (gameState != GameState::Game)
        {
            return;
        }

        if (LayeredInput::IsKeyDown(KEY_W) && STATE.direction != Direction::DOWN)
        {
            STATE.direction = Direction::UP;
        }
        else if (LayeredInput::IsKeyDown(KEY_A) && STATE.direction != Direction::RIGHT)
        {
            STATE.direction = Direction::LEFT;
        }
        else if (LayeredInput::IsKeyDown(KEY_S) && STATE.direction != Direction::UP)
        {
            STATE.direction = Direction::DOWN;
        }
        else if (LayeredInput::IsKeyDown(KEY_D) && STATE.direction != Direction::LEFT)
        {
            STATE.direction = Direction::RIGHT;
        }

        if (STATE.foodDelay.tick())
        {
            Point pos = Rect{STATE.boardDims}.random(1).floor();
            while (std::ranges::contains(STATE.food, pos))
            {
                pos = Rect{STATE.boardDims}.random(1).floor();
            }
            STATE.food.push_back(pos);
        }

        if (STATE.moveDelay.tick())
        {
            STATE.moves++;
            Point dx{};
            if (STATE.direction == Direction::UP)
                dx = {0, -1};
            if (STATE.direction == Direction::RIGHT)
                dx = {1, 0};
            if (STATE.direction == Direction::DOWN)
                dx = {0, 1};
            if (STATE.direction == Direction::LEFT)
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
                        EngineSetState(GameState::GameOver);
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
                DrawRectangleLinesEx(Rect{Point{(float)j, (float)i} * STATE.cellSize, STATE.cellSize}, 1, BLACK);
            }
        }
    }

    static void DrawObjects()
    {
        for (auto& snake : STATE.food)
        {
            DrawRectangleRec(Rect{snake * STATE.cellSize, STATE.cellSize}, RED);
        }
        for (auto& snake : STATE.snake)
        {
            const auto color = &STATE.snake.front() == &snake ? BLUE : GREEN;
            DrawRectangleRec(Rect{snake * STATE.cellSize, STATE.cellSize}, color);
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
        STATE.direction = Direction::UP;
    }

    void onDrawGame(GameState state, Camera2D& camera) override
    {
        camera.target = Point{STATE.boardDims * STATE.cellSize} / 2.0F;
        BeginMode2D(camera);

        DrawObjects();
        DrawBoard();

        EndMode2D();
    }

    void onDrawUI(GameState gameState) override
    {
        auto& scene = SceneGet(gameState);
        scene.draw();
    }
};

int main()
{
    Snake game{};
    return game.run();
}
