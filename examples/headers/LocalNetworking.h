#ifndef MAGIQUE_LOCAL_MULTIPLAYER_EXAMPLE_H
#define MAGIQUE_LOCAL_MULTIPLAYER_EXAMPLE_H

#include <cassert> // For asserts

#include <magique/core/Game.h>
#include <magique/ecs/ECS.h>
#include <magique/ecs/Scripting.h>
#include <magique/core/Core.h>
#include <magique/core/Debug.h>
#include <magique/core/Draw.h>
#include <magique/multiplayer/LocalSockets.h>
#include <magique/multiplayer/Multiplayer.h>
#include <magique/util/Logging.h>

//-----------------------------------------------
// Local Multiplayer Test
//-----------------------------------------------
// .....................................................................
// This is a simple multiplayer test.
// It's not optimized and uses the easiest implementation possible to showcase the functionality
// magique only gives you the facilities to send and receive data, what and how this is done is up to you.
// For a smooth experience you should look into:
//     - delta compression
//     - message interpolation
//     - game state buffering
//     - ...
// .....................................................................

using namespace magique;

enum EntityType : uint16_t
{
    PLAYER,
    NET_PLAYER,
    OBJECT,
};

enum class MessageType : uint8_t
{
    POSITION_UPDATE, // Host sends the clients a entity position update
    SPAWN_UPDATE,    // Host sends the clients data of a entity spawn
    INPUT_UPDATE,    // Client sends the host its current input(s)
};

struct TestCompC final
{
    int counter = 0;
    bool isColliding = false;
};

// Network updates

struct PositionUpdate final // Position update from ane entity in the host world
{
    entt::entity entity;
    float x, y;
};

struct InputUpdate final // Inputs from the client to the host
{
    int key;
};

struct SpawnUpdate final // Entity spawned in the host world
{
    float x, y;
    entt::entity entity;
    EntityType type;
    MapID map;
};

// Scripts

struct PlayerScript final : EntityScript
{
    void onKeyEvent(entt::entity self) override
    {
        auto& pos = GetComponent<PositionC>(self);
        if (IsKeyDown(KEY_W))
            pos.y -= 2.5F;
        if (IsKeyDown(KEY_S))
            pos.y += 2.5F;
        if (IsKeyDown(KEY_A))
            pos.x -= 2.5F;
        if (IsKeyDown(KEY_D))
            pos.x += 2.5F;
    }
};

struct NetPlayerScript final : EntityScript
{
};

struct ObjectScript final : EntityScript // Moving platform
{
    void onTick(entt::entity self, bool updated) override
    {
        auto& myComp = GetComponent<TestCompC>(self);
        myComp.isColliding = false;
        auto& test = GetComponent<TestCompC>(self);
        auto& pos = GetComponent<PositionC>(self);
        if (test.counter > 100)
        {
            pos.x++;
            if (test.counter >= 200)
            {
                test.counter = 0;
            }
        }
        else if (test.counter < 100)
        {
            pos.x--;
        }
        test.counter++;
    }

    void onDynamicCollision(entt::entity self, entt::entity other, CollisionInfo&) override
    {
        auto& myComp = GetComponent<TestCompC>(self);
        myComp.isColliding = true;
    }
};

// Game class

struct Example final : Game
{
    Example() : Game("magique - Example: Local Multiplayer") {}

    void onStartup(AssetLoader& loader) override
    {
        SetGameState({}); // Set empty gamestate - needs to be set in a real game
        SetShowHitboxes(true);
        // Player
        const auto playerFunc = [](entt::entity e, EntityType type)
        {
            GiveActor(e);
            GiveCamera(e);
            GiveCollisionRect(e, 25, 25);
            GiveComponent<TestCompC>(e);
        };
        RegisterEntity(Player, playerFunc);

        // Other players
        const auto netPlayerFunc = [](entt::entity e, EntityType type)
        {
            GiveActor(e);
            GiveCollisionRect(e, 25, 25);
            GiveComponent<TestCompC>(e);
        };
        RegisterEntity(NET_PLAYER, netPlayerFunc);

        // Objects
        const auto objectFunc = [](entt::entity e, EntityType type)
        {
            GiveCollisionRect(e, 25, 25);
            GiveComponent<TestCompC>(e);
        };
        RegisterEntity(OBJECT, objectFunc);

        SetEntityScript(Player, new PlayerScript());
        SetEntityScript(NET_PLAYER, new NetPlayerScript());
        SetEntityScript(OBJECT, new ObjectScript());

        // Create some entities
        CreateEntity(Player, 0, 0, MapID(0));
        for (int i = 0; i < 25; ++i)
        {
            CreateEntity(OBJECT, GetRandomValue(1, 1000), GetRandomValue(1, 1000), MapID(0));
        }

        InitLocalMultiplayer();

        // Setup event callback so we can react to multiplayer events
        SetMultiplayerCallback(
            [&](MultiplayerEvent event, Connection conn)
            {
                if (event == MultiplayerEvent::HOST_NEW_CONNECTION)
                {
                    const auto id = CreateEntity(NET_PLAYER, 0, 0, MapID(0)); // Create a new netplayer
                    SetConnectionEntityMapping(conn, id);

                    // Send the new client the current world state - iterate all entities
                    for (const auto e : GetRegistry().view<PositionC>())
                    {
                        SpawnUpdate spawnUpdate{};
                        spawnUpdate.entity = e;
                        spawnUpdate.map = MapID(0);
                        const auto& pos = GetComponent<PositionC>(e);
                        spawnUpdate.x = pos.x;
                        spawnUpdate.y = pos.y;
                        if (id == e) // If it's the network player itself send the player type (for the camera)
                            spawnUpdate.type = Player;
                        else if (pos.type == Player) // Filter out the host - the host is a network player on the client
                            spawnUpdate.type = NET_PLAYER;
                        else
                            spawnUpdate.type = pos.type;

                        const auto payload = CreatePayload(&spawnUpdate, sizeof(SpawnUpdate), MessageType::SPAWN_UPDATE);

                        BatchMessage(conn, payload);
                    }
                }
                else if (event == MultiplayerEvent::CLIENT_CONNECTION_ACCEPTED)
                {
                    EnterClientMode();
                    DestroyEntities({}); // Pass an empty list - destroys all entities as we enter the hosts world now
                }
            });
    }

    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        if (GetIsClient())
            DrawText("You are a client", 50, 100, 25, BLACK);
        else if (GetIsHost())
            DrawText("You are the host", 50, 100, 25, BLACK);
        else
        {
            DrawText("Press H to host", 50, 50, 25, BLACK);
            DrawText("Press J to join a locally hosted game", 50, 100, 25, BLACK);
        }

        BeginMode2D(camera2D);
        DrawRectangle(250, 250, 50, 50, RED);
        for (const auto e : GetDrawEntities())
        {
            const auto& pos = GetComponent<const PositionC>(e);
            const auto& col = GetComponent<const CollisionC>(e);
            const auto& test = GetComponent<const TestCompC>(e);
            const auto color = test.isColliding ? PURPLE : BLUE;
            switch (col.shape)
            {
            [[likely]] case Shape::RECT: // Missing rotation anchor
                DrawRectanglePro({pos.x, pos.y, col.p1, col.p2}, {0, 0}, pos.rotation, color);
                break;
            case Shape::CIRCLE:
                DrawCircleV({pos.x + col.p1, pos.y + col.p1}, col.p1, color);
                break;
            case Shape::CAPSULE:
                DrawCapsule2D(pos.x, pos.y, col.p1, col.p2, color);
                break;
            case Shape::TRIANGLE:
                DrawTriangleRot({pos.x, pos.y}, {pos.x + col.p1, pos.y + col.p2}, {pos.x + col.p3, pos.y + col.p4},
                                pos.rotation, col.anchorX, col.anchorY, color);
                break;
            }
        }
        EndMode2D();
    }

    void updateGame(GameState gameState) override
    {
        // Enter a session
        if (!GetInMultiplayerSession())
        {
            const int port = 15000;
            if (IsKeyPressed(KEY_H))
            {
                CreateLocalSocket(port);
            }
            if (IsKeyPressed(KEY_J))
            {
                ConnectToLocalSocket(GetLocalIP(), port);
            }
        }

        // Here we receive incoming messages and update the gamestate
        if (GetInMultiplayerSession())
        {
            // The host gets the client inputs
            if (GetIsHost())
            {
                const auto& msgs = ReceiveIncomingMessages();
                for (const auto& msg : msgs)
                {
                    if (msg.payload.type != MessageType::INPUT_UPDATE)
                    {
                        LOG_WARNING("Received wrong message"); // Client only sends inputs
                    }

                    const auto inputUpdate = msg.payload.getDataAs<InputUpdate>();
                    const auto entity = GetConnectionEntityMapping(msg.connection);
                    MAGIQUE_ASSERT(entity != entt::null, "Entity must be registered");

                    auto& pos = GetComponent<PositionC>(entity);
                    if (inputUpdate.key == KEY_W)
                    {
                        pos.y -= 2.5F;
                    }
                    else if (inputUpdate.key == KEY_S)
                    {
                        pos.y += 2.5F;
                    }
                    else if (inputUpdate.key == KEY_A)
                    {
                        pos.x -= 2.5F;
                    }
                    else if (inputUpdate.key == KEY_D)
                    {
                        pos.x += 2.5F;
                    }
                }
            }

            // The client gets the gamestate updates
            if (GetIsClient())
            {
                const auto& msgs = ReceiveIncomingMessages();
                for (const auto& msg : msgs)
                {
                    switch (msg.payload.type)
                    {
                    case MessageType::POSITION_UPDATE:
                        {
                            // Get the data
                            auto positionUpdate = msg.payload.getDataAs<PositionUpdate>();
                            auto& pos = GetComponent<PositionC>(positionUpdate.entity);
                            pos.x = positionUpdate.x;
                            pos.y = positionUpdate.y;
                        }
                        break;
                    case MessageType::INPUT_UPDATE:
                        LOG_WARNING("Received wrong message"); // Only host gets inputs
                        break;
                    case MessageType::SPAWN_UPDATE:
                        {
                            auto [x, y, entity, type, map] = msg.payload.getDataAs<SpawnUpdate>();
                            // Entity MUST not exist already!
                            MAGIQUE_ASSERT(!EntityExists(entity), "Entity exists");
                            // Create a new entity with a preset entity id
                            CreateEntityEx(entity, type, x, y, map, 0, true);
                        }
                        break;
                    default:
                        LOG_WARNING("Received wrong message");
                        break;
                    }
                }
            }
        }
    }

    void postTickUpdate(GameState gameState) override
    {
        // Here we send out the data for this tick
        if (GetInMultiplayerSession())
        {
            // The host sends out the current gamestate to all clients
            // Note: Usually you want to optimize this to send as little as possible
            //      -> instead of position send the position delta - pack multiple single updates together...
            if (GetIsHost())
            {
                for (const auto e : GetUpdateEntities())
                {
                    const auto& pos = GetComponent<const PositionC>(e);

                    // Create the data
                    PositionUpdate posUpdate{};
                    posUpdate.x = pos.x;
                    posUpdate.y = pos.y;
                    posUpdate.entity = e;

                    // Create the payload
                    const auto payload = CreatePayload(&posUpdate, sizeof(PositionUpdate), MessageType::POSITION_UPDATE);

                    // Use batching to avoid the overhead of sending multiple times - send to all connected clients
                    BatchMessageToAll(payload);
                }
            }

            // Send inputs - in client mode all script event methods are skipped!
            if (GetIsClient())
            {
                const auto host = GetCurrentConnections()[0];
                constexpr KeyboardKey keyArr[] = {KEY_W, KEY_A, KEY_S, KEY_D};
                for (const auto key : keyArr)
                {
                    if (IsKeyDown(key))
                    {
                        InputUpdate inputUpdate{};
                        inputUpdate.key = key;
                        BatchMessage(host, CreatePayload(&inputUpdate, sizeof(InputUpdate), MessageType::INPUT_UPDATE));
                    }
                }
            }

            // Send the accumulated message for this tick
            SendBatch();
            FlushMessages();
        }
    }
};

#endif //MAGIQUE_LOCAL_MULTIPLAYER_EXAMPLE_H