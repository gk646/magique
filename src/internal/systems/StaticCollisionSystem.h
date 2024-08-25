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
    // Give each thread a piece and the main thread aswell - so it doesnt wait while doing nothing

    // checks dynamic objects against all static colliders
    inline void Check(const PositionC& pos, const CollisionC& col, float x, float y, float w, float h,
                      CollisionInfo& info)
    {
    }

    inline void QueryHashGrid(vector<uint16_t>& collector, const ColliderHashGrid& grid, const PositionC& pos,
                              const CollisionC& col)
    {
        switch (col.shape)
        {
        [[likely]] case Shape::RECT:
            {
                if (pos.rotation == 0) [[likely]]
                {
                    return grid.query(collector, pos.x, pos.y, col.p1, col.p2);
                }
                float pxs[4] = {0, col.p1, col.p1, 0};
                float pys[4] = {0, 0, col.p2, col.p2};
                RotatePoints4(pos.x, pos.y, pxs, pys, pos.rotation, col.anchorX, col.anchorY);
                const auto bb = GetBBQuadrilateral(pxs, pys);
                return grid.query(collector, bb.x, bb.y, bb.width, bb.height);
            }
        case Shape::CIRCLE:
            return grid.query(collector, pos.x, pos.y, col.p1 * 2.0F, col.p1 * 2.0F); // Top left and diameter as w and h
        case Shape::CAPSULE:
            // Top left and height as height / diameter as w
            return grid.query(collector, pos.x, pos.y, col.p1 * 2.0F, col.p2);
        case Shape::TRIANGLE:
            {
                if (pos.rotation == 0)
                {
                    const auto bb = GetBBTriangle(pos.x, pos.y, col.p1, col.p2, col.p3, col.p4);
                    return grid.query(collector, bb.x, bb.y, bb.width, bb.height);
                }
                float txs[4] = {0, col.p1, col.p3, 0};
                float tys[4] = {0, col.p2, col.p4, 0};
                RotatePoints4(pos.x, pos.y, txs, tys, pos.rotation, col.anchorX, col.anchorY);
                const auto bb = GetBBTriangle(txs[0], tys[0], txs[1], tys[1], txs[2], tys[2]);
                return grid.query(collector, bb.x, bb.y, bb.width, bb.height);
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
    inline void CallEventFunc(const EntityType id, const entt::entity e, const CollisionInfo& i, const ColliderInfo cI)
    {
        InvokeEventDirect<onStaticCollision>(GetScript(id), e,  cI);
    }

    inline void StaticCollisionSystem()
    {
        return;
        constexpr float depth = 250.0F;

        const auto& data = global::ENGINE_DATA;
        const auto& config = global::ENGINE_CONFIG;
        auto& stCollData = global::STATIC_COLL_DATA;

        const auto& collisionVec = data.collisionVec;
        const auto& grid = stCollData.objectGrid;
        const auto wBounds = config.worldBounds;
        auto& collector = stCollData.colliderCollector;

        // Left Side - Upper side - Right side - Lower side
        const Rectangle r1 = {wBounds.x - depth, wBounds.y - depth, depth, wBounds.height + depth};
        const Rectangle r2 = {wBounds.x, wBounds.y - depth, wBounds.width, depth};
        const Rectangle r3 = {wBounds.x + wBounds.width, wBounds.y - depth, depth, wBounds.height + depth};
        const Rectangle r4 = {wBounds.x, wBounds.y + wBounds.height, wBounds.width, depth};

        const auto checkWorld = config.getIsWorldBoundSet();
        for (const auto e : collisionVec)
        {
            const auto& pos = internal::POSITION_GROUP.get<const PositionC>(e);
            const auto& col = internal::POSITION_GROUP.get<const CollisionC>(e);

            CollisionInfo info = CollisionInfo::NoCollision();

            if (checkWorld) // Check if worldbounds active
            {
                CheckAgainstRect(pos, col, r1, info);
                if (info.isColliding())
                {
                    CallEventFunc(pos.type, e, info, ColliderInfo{0, ColliderType::WORLD_BOUNDS});
                    continue;
                }
                CheckAgainstRect(pos, col, r2, info);
                if (info.isColliding())
                {
                    CallEventFunc(pos.type, e, info, ColliderInfo{0, ColliderType::WORLD_BOUNDS});
                    continue;
                }
                CheckAgainstRect(pos, col, r3, info);
                if (info.isColliding())
                {
                    CallEventFunc(pos.type, e, info, ColliderInfo{0, ColliderType::WORLD_BOUNDS});
                    continue;
                }
                CheckAgainstRect(pos, col, r4, info);
                if (info.isColliding())
                {
                    CallEventFunc(pos.type, e, info, ColliderInfo{0, ColliderType::WORLD_BOUNDS});
                    continue;
                }
            }

            QueryHashGrid(collector, grid, pos, col);
            for (const auto num : collector)
            {
                const auto collider = stCollData.getCollider(num);
                const Rectangle rect = {collider.x, collider.y, collider.p1, collider.p2};
                CheckAgainstRect(pos, col, rect, info);
                if (info.isColliding())
                {
                    CallEventFunc(pos.type, e, info, ColliderInfo{0, ColliderType::TILEMAP_OBJECT});
                    break;
                }
            }
        }
    }

} // namespace magique

#endif //STATICCOLLISIONSYSTEM_H