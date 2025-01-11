#include <magique/ecs/ECS.h>
#include <magique/ui/TextFormat.h>
#include <magique/core/Core.h>
#include <magique/core/Camera.h>

#include "ui/UIControls.h"
#include "ecs/Components.h"

void PlayerHUD::onDraw(const Rectangle& bounds)
{
    if (GetCameraEntity() == entt::entity(UINT32_MAX))
        return;
    const auto& stats = GetComponent<EntityStatsC>(GetCameraEntity()); // Player is always the camera in this example

    const auto healthWidth = bounds.width * stats.getHealthPercent();
    const Rectangle health = {bounds.x, bounds.y, healthWidth, bounds.height / 2.0F};
    DrawRectangleRec(health, RED);

    const auto manaWidth = bounds.width * stats.getManaPercent();
    const Rectangle mana = {bounds.x, bounds.y + bounds.height / 2.0F, manaWidth, bounds.height / 2.0F};
    DrawRectangleRec(mana, BLUE);

    const auto& font = GetFontDefault(); // Use a custom font here
    Vector2 p = {bounds.x, bounds.y};
    DrawTextFmt(font, "${P_HEALTH}/${P_MAX_HEALTH}", p, 18, 1.0F);
    p.y += bounds.height / 2.0F;
    DrawTextFmt(font, "${P_MANA}/${P_MAX_MANA}", p, 18, 1.0F);
}

void PlayerHUD::onUpdate(const Rectangle& bounds, const bool isDrawn)
{
    if (!isDrawn || GetCameraEntity() == entt::entity(UINT32_MAX)) // No need for update or no camera
        return;
    const auto& stats = GetComponent<EntityStatsC>(GetCameraEntity());
    SetFormatValue("P_HEALTH", (int)stats.health);
    SetFormatValue("P_MAX_HEALTH", (int)stats.maxHealth);
    SetFormatValue("P_MANA", (int)stats.mana);
    SetFormatValue("P_MAX_MANA", (int)stats.maxMana);
}

void HotbarSlot::onDraw(const Rectangle& bounds) { DrawRectangleLinesEx(bounds, 1, DARKGRAY); }

PlayerHotbar::PlayerHotbar() : UIContainer(slots * HotbarSlot::size, 50, Anchor::BOTTOM_CENTER, ScalingMode::KEEP_RATIO)
{
    for (int i = 0; i < slots; ++i)
    {
        addChild(new HotbarSlot());
    }
}

void PlayerHotbar::onDraw(const Rectangle& bounds)
{
    DrawRectangleLinesEx(bounds, 2, DARKGRAY);

    // Align the slots next to each other
    const UIObject* prevChild = nullptr;
    for (const auto child : getChildren())
    {
        if (prevChild == nullptr)
        {
            child->align(Anchor::TOP_LEFT, *this);
        }
        else
        {
            child->align(Direction::RIGHT, *prevChild);
        }
        prevChild = child;
        child->draw();
    }
}