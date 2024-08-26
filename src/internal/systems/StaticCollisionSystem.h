#ifndef STATICCOLLISIONSYSTEM_H
#define STATICCOLLISIONSYSTEM_H

//-----------------------------------------------
// Static Collision System
//-----------------------------------------------
// .....................................................................
// World bounds is given as white list area -> check against the outer rectangles
// Tiles are treated as squares and inserted into the grid
// .....................................................................

namespace magique
{
    void QueryHashGrid(vector<uint16_t>& vec, const ColliderHashGrid& grid, const PositionC& pos, const CollisionC& col);

    void CheckAgainstRect(const PositionC& pos, const CollisionC& col, const Rectangle& r, CollisionInfo& info);

    void CallEventFunc(EntityType type, entt::entity entity, const CollisionInfo& info, ColliderInfo cInfo);

    void CheckAgainstWorldBounds(entt::entity e, const PositionC& pos, CollisionC& col, const Rectangle& r);

    void CheckRange(int thread, int start, int end);

    inline void StaticCollisionSystem()
    {
        const auto& data = global::ENGINE_DATA;
        const int size = data.collisionVec.size(); // Multithread over certain amount
        if (size > 100)
        {
            std::array<jobHandle, WORK_PARTS> handles{};
            int end = 0;
            const int partSize = static_cast<int>((float)size * 0.81F) / WORK_PARTS; // Give main thread more work
            for (int j = 0; j < WORK_PARTS - 1; ++j)
            {
                const int start = end;
                end = start + partSize;
                handles[j] = AddJob(CreateExplicitJob(CheckRange, j, start, end));
            }
            CheckRange(WORK_PARTS - 1, end, size);
            AwaitJobs(handles);
        }
        else
        {
            CheckRange(0, 0, size);
        }
    }

    //----------------- IMPLEMENTATION -----------------//

    inline void QueryHashGrid(vector<uint16_t>& vec, const ColliderHashGrid& grid, const PositionC& pos,
                              const CollisionC& col)

    {
        switch (col.shape)
        {
        [[likely]] case Shape::RECT:
            {
                if (pos.rotation == 0) [[likely]]
                {
                    return grid.query(vec, pos.x, pos.y, col.p1, col.p2);
                }
                float pxs[4] = {0, col.p1, col.p1, 0};
                float pys[4] = {0, 0, col.p2, col.p2};
                RotatePoints4(pos.x, pos.y, pxs, pys, pos.rotation, col.anchorX, col.anchorY);
                const auto bb = GetBBQuadrilateral(pxs, pys);
                return grid.query(vec, bb.x, bb.y, bb.width, bb.height);
            }
        case Shape::CIRCLE:
            return grid.query(vec, pos.x, pos.y, col.p1 * 2.0F, col.p1 * 2.0F); // Top left and diameter as w and h
        case Shape::CAPSULE:
            // Top left and height as height / diameter as w
            return grid.query(vec, pos.x, pos.y, col.p1 * 2.0F, col.p2);
        case Shape::TRIANGLE:
            {
                if (pos.rotation == 0)
                {
                    const auto bb =
                        GetBBTriangle(pos.x, pos.y, pos.x + col.p1, pos.y + col.p2, pos.x + col.p3, pos.y + col.p4);
                    return grid.query(vec, bb.x, bb.y, bb.width, bb.height);
                }
                float txs[4] = {0, col.p1, col.p3, 0};
                float tys[4] = {0, col.p2, col.p4, 0};
                RotatePoints4(pos.x, pos.y, txs, tys, pos.rotation, col.anchorX, col.anchorY);
                const auto bb = GetBBTriangle(txs[0], tys[0], txs[1], tys[1], txs[2], tys[2]);
                return grid.query(vec, bb.x, bb.y, bb.width, bb.height);
            }
        }
    }

    inline void CheckAgainstRect(const PositionC& pos, const CollisionC& col, const Rectangle& r, CollisionInfo& info)
    {
        switch (col.shape)
        {
        case Shape::RECT:
            {
                if (pos.rotation == 0) [[likely]]
                {
                    return RectToRect(pos.x, pos.y, col.p1, col.p2, r.x, r.y, r.width, r.height, info);
                }
                // Entity
                float pxs[4] = {0, col.p1, col.p1, 0};
                float pys[4] = {0, 0, col.p2, col.p2};
                RotatePoints4(pos.x, pos.y, pxs, pys, pos.rotation, col.anchorX, col.anchorY);

                // World rect
                const float p1xs[4] = {r.x, r.x + r.width, r.x + r.width, r.x};
                const float p1ys[4] = {r.y, r.y, r.y + r.height, r.y + r.height};
                return SAT(pxs, pys, p1xs, p1ys, info);
            }
        case Shape::CIRCLE:
            {
                return RectToCircle(r.x, r.y, r.width, r.height, pos.x + col.p1, pos.y + col.p1, col.p1, info);
            }
        case Shape::CAPSULE:
            {
                return RectToCapsule(r.x, r.y, r.width, r.height, pos.x, pos.y, col.p1, col.p2, info);
            }
        case Shape::TRIANGLE:
            const float rectX[4] = {r.x, r.x + r.width, r.x + r.width, r.x}; // World rect
            const float rectY[4] = {r.y, r.y, r.y + r.height, r.y + r.height};
            if (pos.rotation == 0) [[likely]]
            {
                const float triX[4] = {pos.x, pos.x + col.p1, pos.x + col.p3, pos.x};
                const float triY[4] = {pos.y, pos.y + col.p2, pos.y + col.p4, pos.y};
                return SAT(rectX, rectY, triX, triY, info);
            }
            float triX[4] = {0, col.p1, col.p3, 0};
            float triY[4] = {0, col.p2, col.p4, 0};
            RotatePoints4(pos.x, pos.y, triX, triY, pos.rotation, col.anchorX, col.anchorY);
            return SAT(rectX, rectY, triX, triY, info);
        }
    }

    inline void CallEventFunc(const EntityType type, const entt::entity entity, const CollisionInfo& info,
                              const ColliderInfo cInfo)

    {
        InvokeEventDirect<onStaticCollision>(GetScript(type), entity, cInfo, info);
    }

    inline void CheckAgainstWorldBounds(vector<StaticPair>& collector, const entt::entity e, const PositionC& pos,
                                        CollisionC& col, const Rectangle& r)
    {
        CollisionInfo info{};
        CheckAgainstRect(pos, col, r, info);
        if (info.isColliding())
        {
            collector.push_back({info, &col, e, 0, ColliderType::WORLD_BOUNDS});
        }
    }

    inline void CheckRange(const int thread, const int start, const int end)
    {
        const auto& data = global::ENGINE_DATA;
        const auto& config = global::ENGINE_CONFIG;
        auto& group = internal::POSITION_GROUP;
        auto& stCollData = global::STATIC_COLL_DATA;

        const auto& collisionVec = data.collisionVec;
        const auto& grid = stCollData.objectGrid;
        auto& collCollector = stCollData.colliderCollector[thread].vec; // non const
        auto& pairCollector = stCollData.pairCollector[thread].vec;     // non const

        const auto startIt = collisionVec.begin() + start;
        const auto endIt = collisionVec.begin() + end;

        // Cache
        constexpr float depth = 250.0F;
        const auto wBounds = config.worldBounds;
        const Rectangle r1 = {wBounds.x - depth, wBounds.y - depth, depth, wBounds.height + depth};
        const Rectangle r2 = {wBounds.x, wBounds.y - depth, wBounds.width, depth};
        const Rectangle r3 = {wBounds.x + wBounds.width, wBounds.y - depth, depth, wBounds.height + depth};
        const Rectangle r4 = {wBounds.x, wBounds.y + wBounds.height, wBounds.width, depth};
        const auto checkWorld = config.getIsWorldBoundSet();

        for (auto it = startIt; it != endIt; ++it)
        {
            const auto e = *it;
            const auto& pos = group.get<const PositionC>(e);
            auto& col = group.get<CollisionC>(e); // Non cost for saving reference

            if (checkWorld) // Check if worldbounds active
            {
                CheckAgainstWorldBounds(pairCollector, e, pos, col, r1);
                CheckAgainstWorldBounds(pairCollector, e, pos, col, r2);
                CheckAgainstWorldBounds(pairCollector, e, pos, col, r3);
                CheckAgainstWorldBounds(pairCollector, e, pos, col, r4);
            }

            QueryHashGrid(collCollector, grid, pos, col); // Tilemap objects
            for (const auto num : collCollector)
            {
                const auto collider = stCollData.getCollider(num); // O(1) direct lookup
                CollisionInfo info{};
                CheckAgainstRect(pos, col, {collider.x, collider.y, collider.p1, collider.p2}, info);
                if (info.isColliding())
                {
                    pairCollector.push_back({info, &col, e, 0, ColliderType::WORLD_BOUNDS});
                }
            }
            collCollector.clear();
        }
    }
} // namespace magique

#endif //STATICCOLLISIONSYSTEM_H