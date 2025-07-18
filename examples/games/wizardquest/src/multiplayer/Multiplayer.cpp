#include "Multiplayer.h"

#include <cassert>
#include <raylib/raylib.h>

#include <WizardQuest.h>
#include <ankerl/unordered_dense.h>
#include <magique/core/Core.h>
#include <magique/ecs/ECS.h>
#include <magique/multiplayer/LocalSockets.h>
#include <magique/multiplayer/Multiplayer.h>
#include <magique/util/Logging.h>

using namespace magique;

// This implementation reuses code from example/headers/LocalNetworking.h
// Check it out for more info

// Network updates
struct PositionUpdate final // Position update from ane entity in the host world
{
    entt::entity entity;
    float x, y;
    MapID map;
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

HashMap<Connection, entt::entity> networkPlayerMap{}; // Maps outgoing connections to a player in our world (for host)

void Multiplayer::init()
{
    InitLocalMultiplayer();

    // Setup event callback so we can react to multiplayer events
    SetMultiplayerCallback(
        [&](MultiplayerEvent event, Connection conn)
        {
            if (event == MultiplayerEvent::HOST_NEW_CONNECTION)
            {
                const auto id = CreateEntity(NET_PLAYER, 24 * 24, 24 * 24, MapID(0)); // Create a new netplayer
                const auto lastConnection = GetCurrentConnections().back();
                networkPlayerMap[lastConnection] = id; // Save the mapping

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
                        spawnUpdate.type = PLAYER;
                    else if (pos.type == PLAYER) // Filter out the host - the host is a network player on the client
                        spawnUpdate.type = NET_PLAYER;
                    else
                        spawnUpdate.type = pos.type;

                    const auto payload = CreatePayload(&spawnUpdate, sizeof(SpawnUpdate), MessageType::SPAWN_UPDATE);

                    BatchMessage(lastConnection, payload); // Batch the message - its sent with the next flush
                }
            }
            else if (event == MultiplayerEvent::CLIENT_CONNECTION_ACCEPTED)
            {
                EnterClientMode();
                DestroyEntities({}); // Destroy all entities in our world as we enter the hosts world
            }
            else if (event == MultiplayerEvent::CLIENT_CONNECTION_CLOSED)
            {
                ExitClientMode();
            }
        });
}

void Multiplayer::checkBeginSession()
{
    const int port = 35000;
    if (IsKeyPressed(KEY_H))
    {
        if (CreateLocalSocket(port))
            LOG_INFO("Opened server");
        return;
    }

    if (IsKeyPressed(KEY_J))
    {
        LOG_INFO("Trying to connect to local server...");
        Connection conn = ConnectToLocalSocket(GetLocalIP(), port);
        if (conn == Connection::INVALID_CONNECTION)
        {
            LOG_WARNING("Error trying to connect to local server");
        }
    }
}

void Multiplayer::update()
{
    if (!GetInMultiplayerSession()) // Only check if NOT in a session
    {
        checkBeginSession();
    }

    // The host gets the client inputs
    if (GetIsHost())
    {
        const std::vector<Message>& msgs = ReceiveIncomingMessages();
        for (const auto& msg : msgs)
        {
            if (msg.payload.type != MessageType::INPUT_UPDATE)
            {
                LOG_WARNING("Received wrong message"); // Client only sends inputs
            }

            const auto inputUpdate = msg.payload.getDataAs<InputUpdate>();
            assert(networkPlayerMap.contains(msg.connection)); // Can only get updates from connected clients
            const auto entity = networkPlayerMap[msg.connection];

            // Move the entity in our world
            // Note: Normally you would look up their movement speed or invoke a method to move it
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
                    PositionUpdate positionUpdate = msg.payload.getDataAs<PositionUpdate>();
                    auto& pos = GetComponent<PositionC>(positionUpdate.entity);
                    pos.x = positionUpdate.x;
                    pos.y = positionUpdate.y;
                    pos.map = positionUpdate.map;
                }
                break;
            case MessageType::INPUT_UPDATE:
                LOG_WARNING("Received wrong message"); // Only host gets inputs
                break;
            case MessageType::SPAWN_UPDATE:
                {
                    auto [x, y, entity, type, map] = msg.payload.getDataAs<SpawnUpdate>();
                    assert(!EntityExists(entity));                    // Entity MUST not exist already!
                    CreateEntityEx(entity, type, x, y, map, 0, true); // Create a new entity
                }
                break;
            }
        }
    }
}

void Multiplayer::postUpdate()
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
                posUpdate.map = pos.map;

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
    }
}