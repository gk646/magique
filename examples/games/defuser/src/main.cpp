#include <magique/ecs/ECS.h>
#include <magique/magique.hpp>

using namespace magique;

enum EntityType : uint16_t
{
    ROBOT,
    MINE,
    TARGET,
    PING,
};

struct PingInfoC final
{
    float radius = 0;
};

struct MineInfoC final
{
    int visibleCounter = 0;
};

enum class MapID : uint8_t
{

    LEVEL_1
};

struct Stats
{
    int defuses = 4;
    int energy = 10;
    bool successful = false;
    uint64_t finalTime = 0;
    bool automatic = false;
};

enum class GameState : int
{
    PLAY,
    GAME_OVER,
};

static Stats STATS{};

struct RobotScript final : EntityScript
{
    void onKeyEvent(entt::entity self) override
    {
        auto& pos = GetComponent<PositionC>(self);
        auto& col = GetComponent<CollisionC>(self);
        if (UIInput::IsKeyDown(KEY_W))
        {
            pos.y -= 1;
        }
        if (UIInput::IsKeyDown(KEY_A))
        {
            pos.x -= 1;
        }
        if (UIInput::IsKeyDown(KEY_S))
        {
            pos.y += 1;
        }
        if (UIInput::IsKeyDown(KEY_D))
        {
            pos.x += 1;
        }

        if (UIInput::IsKeyPressed(KEY_E) && STATS.defuses > 0)
        {
            entt::entity closest = entt::null;
            float minDist = FLT_MAX;
            auto mid = pos.getMiddle(col);
            for (auto entity : GetNearbyEntities(MapID::LEVEL_1, pos.getMiddle(col), 50))
            {
                if (entity == self)
                    continue;

                auto& oPos = GetComponent<PositionC>(entity);
                auto& oCol = GetComponent<CollisionC>(entity);

                if (oPos.getMiddle(oCol).euclidean(mid) < minDist && oPos.type == MINE)
                {
                    minDist = oPos.getMiddle(oCol).euclidean(mid);
                    closest = entity;
                }
            }

            if (closest != entt::null)
            {
                auto& oPos = GetComponent<PositionC>(closest);

                if (oPos.type == TARGET)
                {
                    SetGameState(GameState::GAME_OVER);
                    STATS.successful = true;
                    if (STATS.finalTime == 0)
                        STATS.finalTime = StopTimer(0);
                }

                DestroyEntity(closest);
                STATS.defuses -= 1;
            }
        }
    }

    void onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info) override
    {
        auto& oPos = GetComponent<PositionC>(other);
        if (oPos.type == MINE)
        {
            SetGameState(GameState::GAME_OVER);
            if (STATS.finalTime == 0)
                STATS.finalTime = StopTimer(0);
        }
    }
};

struct PingScript final : EntityScript
{
    void onTick(entt::entity self, bool updated) override
    {
        auto& info = GetComponent<PingInfoC>(self);
        auto& col = GetComponent<CollisionC>(self);
        auto& pos = GetComponent<PositionC>(self);
        info.radius += 2.5;
        col.p1 = info.radius;
        pos.x -= 2.5;
        pos.y -= 2.5;
        if (info.radius > 200)
        {
            DestroyEntity(self);
        }
    }

    void onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info) override {}

    void onStaticCollision(entt::entity self, ColliderInfo collider, CollisionInfo& info) override {}
};

struct MineScript final : EntityScript
{
    void onTick(entt::entity self, bool updated) override
    {
        auto& mineInfo = GetComponent<MineInfoC>(self);
        if (mineInfo.visibleCounter > 0)
        {
            mineInfo.visibleCounter--;
        }
        else
        {
            SetEntityPathSolid(self, false);
        }
    }

    void onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo& info) override
    {
        auto& oPos = GetComponent<PositionC>(other);
        if (oPos.type == TARGET)
        {
            DestroyEntity(self);
        }

        if (oPos.type != PING)
            return;
        auto& mineInfo = GetComponent<MineInfoC>(self);
        mineInfo.visibleCounter = 100;
        SetEntityPathSolid(self, true);
    }

    void onStaticCollision(entt::entity self, ColliderInfo collider, CollisionInfo& info) override
    {
        DestroyEntity(self);
    }
};

entt::entity TARGET_ENT = entt::null;

void resetGame()
{
    STATS.defuses = 4;
    STATS.energy = 10;
    STATS.successful = false;
    STATS.finalTime = 0;
    STATS.automatic = false;

    DestroyEntities({});
    CreateEntity(ROBOT, 25 * 24, 37 * 24, MapID::LEVEL_1);
    TARGET_ENT = CreateEntity(TARGET, 34 * 24, 14 * 24, MapID::LEVEL_1);
    for (int i = 0; i < 35; ++i)
    {
        for (int j = 0; j < 50; ++j)
        {
            if (GetRandomFloat(0, 1) > 0.4)
                continue;
            CreateEntity(MINE, j * 24, i * 24, MapID::LEVEL_1);
        }
    }
    SetGameState(GameState::PLAY);
    StopTimer(0);
    StartTimer(0);
}

struct Robo final : Game
{
    Robo() : Game("Robo") {}

    void onStartup(AssetLoader& loader) override
    {
        SetWindowState(FLAG_WINDOW_RESIZABLE);
        loader.registerTask(
            [](AssetContainer& assets)
            {
                RegisterHandle(RegisterTileMap(assets.getAsset("city.tmx")), "tilemap");
                RegisterHandle(RegisterTileSheet(assets.getAsset("topdown.png"), 8, 3), "tilesheet");
                RegisterHandle(RegisterTileSet(assets.getAsset("topdown.tsx")), "tileset");
                RegisterHandle(RegisterTexture(assets.getAsset("robot.png")), "robot");
            },
            THREAD_MAIN);

        RegisterEntity(ROBOT,
                       [](entt::entity entity, EntityType type)
                       {
                           GiveActor(entity);
                           GiveCamera(entity);
                           GiveCollisionRect(entity, 20, 20);
                       });
        SetEntityScript(ROBOT, new RobotScript());
        RegisterEntity(PING,
                       [](entt::entity entity, EntityType type)
                       {
                           GiveComponent<PingInfoC>(entity);
                           GiveCollisionCircle(entity, 10);
                       });
        SetEntityScript(PING, new PingScript());
        RegisterEntity(MINE,
                       [](entt::entity entity, EntityType type)
                       {
                           GiveComponent<MineInfoC>(entity);
                           GiveCollisionCircle(entity, 10);
                       });
        SetEntityScript(MINE, new MineScript());

        RegisterEntity(TARGET,
                       [](entt::entity entity, EntityType type)
                       {
                           GiveComponent<MineInfoC>(entity);
                           GiveCollisionCircle(entity, 15);
                       });
        SetEntityScript(TARGET, new MineScript());
    }

    void onLoadingFinished() override
    {
        LoadGlobalTileSet(GetTileSet(GetHandle(GetHash("tileset"))), {1}, 3);
        auto mapHandle = GetHandle(GetHash("tilemap"));
        AddTileCollisions(MapID::LEVEL_1, GetTileMap(mapHandle), {0, 1});

        resetGame();
        // SetShowHitboxes(true);
        // SetShowPathFindingOverlay(true);
    }

    void updateGame(GameState gameState) override
    {
        if (UIInput::IsKeyPressed(KEY_ESCAPE))
        {
            resetGame();
        }

        if (UIInput::IsKeyPressed(KEY_ENTER))
        {
            STATS.automatic = !STATS.automatic;
        }

        if (UIInput::IsKeyPressed(KEY_SPACE) & STATS.energy > 0)
        {
            auto camPos = GetCameraPosition();
            CreateEntity(PING, camPos.x, camPos.y, MapID::LEVEL_1);
            STATS.energy -= 1;
        }
        static int counter = 0;
        if (counter > 120 && STATS.energy < 10)
        {
            STATS.energy += 1;
            counter = 0;
        }
        ++counter;


        if (STATS.automatic)
        {
            auto& pos = GetComponent<PositionC>(GetCameraEntity());
            auto& col = GetComponent<CollisionC>(GetCameraEntity());
            auto& oPos = GetComponent<PositionC>(TARGET_ENT);
            auto& oCol = GetComponent<CollisionC>(TARGET_ENT);

            auto current = pos.getMiddle(col);
            auto next = GetNextOnPath(current, oPos.getMiddle(oCol), MapID::LEVEL_1, 25000);
            if (next == Point{0, 0})
                return;
            auto dir = GetDirectionVector(current, next);
            pos.x += dir.x;
            pos.y += dir.y;
        }
    }

    void drawGame(GameState gameState, Camera2D& camera) override
    {
        if (GetGameState() == GameState::GAME_OVER)
            return;

        BeginMode2D(camera);

        auto& tileMap = GetTileMap(GetHandle(GetHash("tilemap")));
        auto& tileSheet = GetTileSheet(GetHandle(GetHash("tilesheet")));

        DrawTileMap(tileMap, tileSheet, 0);
        DrawTileMap(tileMap, tileSheet, 1);

        for (const auto entity : GetDrawEntities())
        {
            const auto& pos = GetComponent<PositionC>(entity);
            const auto& col = GetComponent<CollisionC>(entity);

            if (entity == GetCameraEntity())
            {
                DrawRegion(GetTexture(GetHash("robot")), pos.x, pos.y);
                continue;
            }

            if (EntityHasComponents<PingInfoC>(entity))
            {
                const auto& info = GetComponent<PingInfoC>(entity);
                DrawCircleLinesV({pos.x + col.p1, pos.y + col.p1}, info.radius, GRAY);
            }
            else if (pos.type == TARGET)
            {
                DrawCircleV({pos.x + col.p1, pos.y + col.p1}, 15, GREEN);
            }
            else if (EntityHasComponents<MineInfoC>(entity))
            {
                if (GetComponent<MineInfoC>(entity).visibleCounter == 0)
                    continue;
                DrawCircleV({pos.x + col.p1, pos.y + col.p1}, col.p1, GRAY);
            }
        }

        EndMode2D();
    }

    void drawUI(GameState gameState) override
    {
        if (GetGameState() == GameState::PLAY)
        {
            DrawTextEx(GetFont(), "Defuses:", {50, 25}, 25, 1, BLACK);
            DrawRectangleLines(50, 50, 200, 50, BLACK);
            for (int i = 0; i < STATS.defuses; ++i)
            {
                DrawRectangle(50 + 50 * i, 50, 50, 50, GREEN);
            }
            DrawTextEx(GetFont(), "Energy:", {50, 125}, 25, 1, BLACK);
            DrawRectangleLines(50, 150, 200, 50, BLACK);
            for (int i = 0; i < STATS.energy; ++i)
            {
                DrawRectangle(50 + 20 * i, 150, 20, 50, BLUE);
            }
            auto time = GetTimerTime(0) / 1'000'000'000;
            DrawTextEx(GetFont(), std::string{std::string{"Timer:"} + std::to_string(time)}.c_str(),
                       {GetScreenWidth() / 2.0F - 50, 25}, 30, 1, BLACK);
        }
        else
        {
            const char* text = nullptr;
            if (STATS.successful)
            {
                text = "You defused the big bomb!";
            }
            else
            {
                text = "You exploded!";
            }
            auto pos = Vector2{GetScreenWidth() / 2.0F - 50, 25};
            DrawTextEx(GetFont(), text, pos, 25, 1, BLACK);

            pos.y += 30;
            DrawTextEx(GetFont(), "Press ESC to restart", pos, 25, 1, BLACK);

            pos.y += 30;
            auto time = STATS.finalTime / 1'000'000'000;
            DrawTextEx(GetFont(), std::string{std::string{"Time taken:"} + std::to_string(time)}.c_str(), pos, 30, 1,
                       BLACK);
        }
    }
};

int main()
{
    AssetPackCompile("../res");
    Robo game{};
    return game.run();
}