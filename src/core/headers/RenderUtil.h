#ifndef RENDERUTIL_H
#define RENDERUTIL_H


namespace magique
{
    inline void HandleLoadingScreen(Game& game)
    {
        auto& loader = global::LOADER;
        if (loader != nullptr) [[likely]]
        {
            auto& lScreen = *global::ENGINE_CONFIG.loadingScreen;
            const bool isStartup = loader->isStartup();
            const auto drawRes = lScreen.draw(isStartup, loader->getProgressPercent());
            const auto res = loader->step();
            if (res && drawRes)
            {
                delete loader;
                loader = nullptr;
                game.isLoading = false;
                if (isStartup)
                {
                    game.onLoadingFinished();
                    ResetPerformanceData();
                }
            }
        }
        else
        {
            game.isLoading = false;
        }
    }

    inline void AssignCameraPosition()
    {
        auto& data = global::ENGINE_DATA;
        auto& config = global::ENGINE_CONFIG;
        const auto smoothing = 1.0F - config.cameraSmoothing; // The higher the value the smoother
        const auto cameraEntity = GetCameraEntity();

        Point targetPosition{0, 0};
        if (cameraEntity != entt::entity{UINT32_MAX}) [[unlikely]] // No camera assigned
        {
            targetPosition = internal::REGISTRY.get<PositionC>(cameraEntity).getPosition();
        }

        // Center the camera on the collision shape if provided
        if (config.cameraPositionOff.x != 0.0F || config.cameraPositionOff.y != 0.0F)
        {
            targetPosition.x += config.cameraPositionOff.x;
            targetPosition.y += config.cameraPositionOff.y;
        }
        else
        {
            CollisionC* coll = internal::REGISTRY.try_get<CollisionC>(cameraEntity);
            if (coll != nullptr)
            {
                switch (coll->shape)
                {
                case Shape::RECT:
                    targetPosition.x += coll->p1 / 2.0F;
                    targetPosition.y += coll->p2 / 2.0F;
                    break;
                case Shape::CIRCLE:
                    targetPosition.x += coll->p1;
                    targetPosition.y += coll->p1;
                    break;
                case Shape::CAPSULE:
                    targetPosition.x += coll->p1;
                    targetPosition.y += coll->p2 / 2.0F;
                    break;
                case Shape::TRIANGLE:
                    break;
                }
            }
        }

        data.camera.target.x += (targetPosition.x - data.camera.target.x) * smoothing;
        data.camera.target.y += (targetPosition.y - data.camera.target.y) * smoothing;
        data.camera.target.x = std::floor(data.camera.target.x);
        data.camera.target.y = std::floor(data.camera.target.y);
    }

    inline void RenderHitboxes()
    {
        BeginMode2D(GetCamera());
        auto& group = internal::POSITION_GROUP;
        const auto& staticData = global::STATIC_COLL_DATA;

        const auto bounds = GetCameraBounds();
        const auto map = GetCameraMap();

        // Dynamic entities
        for (const auto e : GetDrawEntities())
        {
            if (!group.contains(e))
                continue;

            const auto& pos = group.get<const PositionC>(e);
            const auto& col = group.get<const CollisionC>(e);

            if (!PointToRect(pos.x, pos.y, bounds.x, bounds.y, bounds.width, bounds.height))
                continue;

            switch (col.shape)
            {
            [[likely]] case Shape::RECT:
                DrawRectangleLinesRot({pos.x, pos.y, col.p1, col.p2}, pos.rotation, col.anchorX, col.anchorY, RED);
                break;
            case Shape::CIRCLE:
                DrawCircleLinesV({pos.x + col.p1, pos.y + col.p1}, col.p1, RED);
                break;
            case Shape::CAPSULE:
                DrawCapsule2DLines(pos.x, pos.y, col.p1, col.p2, RED);
                break;
            case Shape::TRIANGLE:
                DrawTriangleLinesRot({pos.x, pos.y}, {pos.x + col.p1, pos.y + col.p2}, {pos.x + col.p3, pos.y + col.p4},
                                     pos.rotation, col.anchorX, col.anchorY, RED);
                break;
            }
        }

        auto drawStaticObjectVectorHitboxes = [&](const vector<uint32_t>& objectIds)
        {
            for (const auto idx : objectIds)
            {
                const auto& [x, y, p1, p2] = staticData.colliderStorage.get(idx);
                if (p1 == 0 || !PointToRect(x, y, bounds.x, bounds.y, bounds.width, bounds.height))
                    continue;

                if (p2 != 0)
                {
                    DrawRectangleLinesEx({x, y, p1, p2}, 2, RED);
                }
                else
                {
                    DrawCircleLinesV({x + p1, y + p1}, p1, RED);
                }
            }
        };

        // Static Colliders
        if (staticData.colliderReferences.tileObjectMap.contains(map))
        {
            for (const auto& info : staticData.colliderReferences.tileObjectMap.at(map))
            {
                drawStaticObjectVectorHitboxes(info.objectIds);
            }
        }

        // Static tiles
        if (staticData.colliderReferences.tilesCollisionMap.contains(map))
        {
            drawStaticObjectVectorHitboxes(staticData.colliderReferences.tilesCollisionMap.at(map));
        }

        // Collision Groups
        if (staticData.colliderReferences.groupMap.contains(map))
        {
            for (const auto& info : staticData.colliderReferences.groupMap.at(map))
            {
                drawStaticObjectVectorHitboxes(info.objectIds);
            }
        }

        // WorldBounds
        if (staticData.getIsWorldBoundSet()) // enabled
        {
            constexpr float depth = MAGIQUE_WORLD_BOUND_DEPTH;
            const auto wBounds = staticData.worldBounds;
            const Rectangle r1 = {wBounds.x - depth, wBounds.y - depth, depth, wBounds.height + depth};
            const Rectangle r2 = {wBounds.x, wBounds.y - depth, wBounds.width, depth};
            const Rectangle r3 = {wBounds.x + wBounds.width, wBounds.y - depth, depth, wBounds.height + depth};
            const Rectangle r4 = {wBounds.x, wBounds.y + wBounds.height, wBounds.width, depth};
            DrawRectangleLinesEx(r1, 2, RED);
            DrawRectangleLinesEx(r2, 2, RED);
            DrawRectangleLinesEx(r3, 2, RED);
            DrawRectangleLinesEx(r4, 2, RED);
        }
        EndMode2D();
    }

    inline void RenderOverlays()
    {
        const auto& dynamic = global::DY_COLL_DATA;
        const auto& path = global::PATH_DATA;
        const auto& config = global::ENGINE_CONFIG;
        const auto currentMap = GetCameraMap();

        auto drawEntityGrid = [&]()
        {
            if (!dynamic.mapEntityGrids.contains(currentMap))
                return;
            const auto& grid = dynamic.mapEntityGrids[currentMap];
            const auto bounds = GetCameraBounds();
            constexpr int cellSize = MAGIQUE_COLLISION_CELL_SIZE;
            const int fontSize = config.fontSize;
            const int startX = static_cast<int>(bounds.x) / cellSize;
            const int startY = static_cast<int>(bounds.y) / cellSize;
            const int width = static_cast<int>(bounds.width) / cellSize;
            const int height = static_cast<int>(bounds.height) / cellSize;
            const int textOff = MAGIQUE_COLLISION_CELL_SIZE / 2 - fontSize / 2;

            for (int i = 0; i < height; ++i)
            {
                const int currY = startY + i;
                for (int j = 0; j < width; ++j)
                {
                    const int currX = startX + j;
                    const int x = currX * MAGIQUE_COLLISION_CELL_SIZE;
                    const int y = currY * MAGIQUE_COLLISION_CELL_SIZE;

                    const auto id = GetCellID(currX, currY);
                    const auto it = grid.cellMap.find(id);
                    if (it != grid.cellMap.end())
                    {
                        const auto count = static_cast<int>(grid.dataBlocks[it->second].count);
                        const auto color = count > grid.getBlockSize() ? RED : GREEN; // Over the limit
                        const Vector2 pos = {static_cast<float>(x) + textOff, static_cast<float>(y) + textOff};
                        DrawTextEx(config.font, std::to_string(count).c_str(), pos, fontSize, 1, color);
                    }
                    DrawRectangleLines(x, y, MAGIQUE_COLLISION_CELL_SIZE, MAGIQUE_COLLISION_CELL_SIZE, BLACK);
                }
            }
        };

        auto drawCompass = [&](Color color)
        {
            constexpr float DISTANCE = 10'000;
            constexpr int MARKER_GAP = 500; // Pixels between each marker
            constexpr int MARKER_SIZE = 30; // Pixels between each marker

            const auto bounds = GetCameraBounds();
            const int startX = static_cast<int>(bounds.x) / MARKER_GAP;
            const int startY = static_cast<int>(bounds.y) / MARKER_GAP;
            const int width = static_cast<int>(bounds.width) / MARKER_GAP;
            const int height = static_cast<int>(bounds.height) / MARKER_GAP;

            DrawLineEx({-DISTANCE, 0}, {DISTANCE, 0}, 2, color);
            DrawLineEx({0, -DISTANCE}, {0, DISTANCE}, 2, color);

            for (int currX = startX; currX < startX + width; ++currX)
            {
                const int x = currX * MARKER_GAP;
                DrawLine(x, -MARKER_SIZE, x, MARKER_SIZE, color);
            }
            for (int currY = startY; currY < startY + height; ++currY)
            {
                const int y = currY * MARKER_GAP;
                DrawLine(-MARKER_SIZE, y, MARKER_SIZE, y, color);
            }
        };


        auto drawPathFindingGrid = [&]()
        {
            if (!path.mapsDynamicGrids.contains(currentMap)) // Could be called before any entity is created
                return;
            const auto& staticGrid = path.mapsStaticGrids[currentMap];
            const auto& dynamicGrid = path.mapsDynamicGrids[currentMap];

            const auto bounds = GetCameraBounds();
            constexpr int cellSize = MAGIQUE_PATHFINDING_CELL_SIZE;
            const int startX = static_cast<int>(bounds.x) / cellSize;
            const int startY = static_cast<int>(bounds.y) / cellSize;
            const int width = static_cast<int>(bounds.width) / cellSize;
            const int height = static_cast<int>(bounds.height) / cellSize;

            for (int currY = startY; currY < startY + height; ++currY)
            {
                for (int currX = startX; currX < startX + width; ++currX)
                {
                    const float cellX = static_cast<float>(currX) * cellSize;
                    const float cellY = static_cast<float>(currY) * cellSize;
                    const Rectangle rect = {cellX, cellY, (float)cellSize, (float)cellSize};
                    const bool isSolid = PathFindingData::IsCellSolid(cellX, cellY, staticGrid, dynamicGrid);
                    if (isSolid)
                        DrawRectangleRec(rect, ColorAlpha(RED, 0.4F));
                    DrawRectangleLinesEx(rect, 1, BLACK);
                }
            }
        };

        BeginMode2D(GetCamera());
        {
            if (config.showCompassOverlay)
                drawCompass(BLACK);

            if (config.showPathFindingOverlay)
                drawPathFindingGrid();

            if (config.showEntityOverlay)
                drawEntityGrid();
        }
        EndMode2D();
    }

    inline void InternalRenderPost()
    {
        RenderLighting(internal::REGISTRY);
        RenderOverlays();
        if (global::ENGINE_CONFIG.showHitboxes) [[unlikely]]
            RenderHitboxes();
    }

} // namespace magique

#endif //RENDERUTIL_H