#include "WizardQuest.h"
#include "Multiplayer.h"

// This implementation reuses code from example/headers/LocalNetworking.h
// Check it out for more info

// Network updates
struct PositionUpdate final // Position update from ane entity in the host world
{
    entt::entity entity;
    Point pos;
    MapID map;
};

struct InputUpdate final // Inputs from the client to the host
{
    int key;
};

struct SpawnUpdate final // Entity spawned in the host world
{
    Point pos;
    entt::entity entity;
    EntityType type;
    MapID map;
};

HashMap<Connection, entt::entity> networkPlayerMap{}; // Maps outgoing connections to a player in our world (for host)

void Multiplayer::init()
{
    LocalSocketInit();

    // Setup event callback so we can react to multiplayer events
    NetworkSetCallback(
        [&](NetworkEvent event, Connection conn, const NetworkEventData& data)
        {
            if (event == NetworkEvent::HOST_NEW_CONNECTION)
            {
                // Create a new netplayer
                const auto id = EntityCreate(EntityType::NET_PLAYER, {24 * 24}, MapID(0));
                const auto lastConnection = NetworkGetConnections().back();
                networkPlayerMap[lastConnection] = id; // Save the mapping

                // Send the new client the current world state - iterate all entities
                for (const auto e : ComponentGetView<PositionC>())
                {
                    SpawnUpdate spawnUpdate{};
                    spawnUpdate.entity = e;
                    spawnUpdate.map = MapID(0);
                    const auto& pos = ComponentGet<PositionC>(e);
                    spawnUpdate.pos = pos;
                    // If it's the network player itself send the player type (for the camera)
                    if (id == e)
                        spawnUpdate.type = EntityType::PLAYER;
                    // Filter out the host - the host is a network player on the client
                    else if (pos.type == EntityType::PLAYER)
                        spawnUpdate.type = EntityType::NET_PLAYER;
                    else
                        spawnUpdate.type = pos.type;

                    const auto payload = Payload(&spawnUpdate, sizeof(SpawnUpdate), MessageType::SPAWN_UPDATE);

                    NetworkSend(lastConnection, payload);
                }
            }
            else if (event == NetworkEvent::CLIENT_CONNECTION_ACCEPTED)
            {
                NetworkEnterClientMode();
                EntityDestroy({}); // Destroy all entities in our world as we enter the hosts world
            }
            else if (event == NetworkEvent::CLIENT_CONNECTION_CLOSED)
            {
                NetworkExitClientMode();
            }
        });
}

void Multiplayer::update()
{
    if (!NetworkInSession()) // Only check if NOT in a session
    {
        checkBeginSession();
    }

    // The host gets the client inputs
    if (NetworkIsHost())
    {
        auto msgs = NetworkReceive();
        for (const auto& msg : msgs)
        {
            if (msg.payload.getType() != MessageType::INPUT_UPDATE)
            {
                LOG_WARNING("Received wrong message"); // Client only sends inputs
                continue;
            }

            const auto inputUpdate = msg.payload.getDataAs<InputUpdate>();
            assert(networkPlayerMap.contains(msg.connection)); // Can only get updates from connected clients
            const auto entity = networkPlayerMap[msg.connection];

            // Move the entity in our world
            // Note: Normally you would look up their movement speed or invoke a method to move it
            auto& pos = ComponentGet<PositionC>(entity).pos;
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
    if (NetworkIsClient())
    {
        auto msgs = NetworkReceive();
        for (const auto& msg : msgs)
        {
            switch (msg.payload.getType())
            {
            case MessageType::POSITION_UPDATE:
                {
                    // Get the data
                    PositionUpdate positionUpdate = msg.payload.getDataAs<PositionUpdate>();
                    auto& pos = ComponentGet<PositionC>(positionUpdate.entity);
                    pos.pos = positionUpdate.pos;
                    pos.map = positionUpdate.map;
                }
                break;
            case MessageType::INPUT_UPDATE:
                LOG_WARNING("Received wrong message"); // Only host gets inputs
                break;
            case MessageType::SPAWN_UPDATE:
                {
                    auto [pos, entity, type, map] = msg.payload.getDataAs<SpawnUpdate>();
                    assert(!EntityExists(entity));                   // Entity MUST not exist already!
                    EntityCreateEx(entity, type, pos, map, 0, true); // Create a new entity
                }
                break;
            }
        }
    }
}

void Multiplayer::postUpdate()
{
    // Here we send out the data for this tick
    if (NetworkInSession())
    {
        // The host sends out the current gamestate to all clients
        // Note: Usually you want to optimize this to send as little as possible
        //      -> instead of position send the position delta - pack multiple single updates together...
        if (NetworkIsHost())
        {
            for (const auto e : EngineGetUpdateEntities())
            {
                const auto& pos = ComponentGet<const PositionC>(e);

                // Create the data
                PositionUpdate posUpdate{};
                posUpdate.pos = pos;
                posUpdate.entity = e;
                posUpdate.map = pos.map;

                // Create the payload
                const auto payload = Payload(&posUpdate, sizeof(PositionUpdate), MessageType::POSITION_UPDATE);

                // Use batching to avoid the overhead of sending multiple times - send to all connected clients
                NetworkSendAll(payload);
            }
        }

        // Send inputs - in client mode all script event methods are skipped!
        if (NetworkIsClient())
        {
            const auto host = NetworkGetConnections()[0];
            constexpr KeyboardKey keyArr[] = {KEY_W, KEY_A, KEY_S, KEY_D};
            for (const auto key : keyArr)
            {
                if (IsKeyDown(key))
                {
                    InputUpdate inputUpdate{};
                    inputUpdate.key = key;
                    NetworkSend(host, Payload(&inputUpdate, sizeof(InputUpdate), MessageType::INPUT_UPDATE));
                }
            }
        }

        // Send the accumulated message for this tick
        NetworkFlush();
    }
}

void Multiplayer::checkBeginSession()
{
    const int port = 35000;
    if (IsKeyPressed(KEY_H))
    {
        if (LocalSocketCreate(port))
            LOG_INFO("Opened server");
        return;
    }

    if (IsKeyPressed(KEY_J))
    {
        LOG_INFO("Trying to connect to local server...");
        Connection conn = LocalSocketConnect(LocalSocketGetIP(), port);
        if (conn == Connection::INVALID)
            LOG_WARNING("Error trying to connect to local server");
    }
}
