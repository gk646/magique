#include <magique/ecs/ECS.h>
#include <magique/ui/TextFormat.h>
#include <magique/core/Core.h>

#include "ui/UIControls.h"
#include "ecs/Components.h"

void PlayerHUD::draw(const Rectangle& bounds)
{
    const auto& stats = GetComponent<EntityStatsC>(GetCameraEntity());

    const Rectangle health = {bounds.x, bounds.y, bounds.width * stats.getHealthPercent(), bounds.height / 2.0F};
    DrawRectangleRec(health, RED);

    const Rectangle mana = {bounds.x, bounds.y + bounds.height / 2.0F, bounds.width * stats.getManaPercent(),
                            bounds.height / 2.0F};
    DrawRectangleRec(mana, BLUE);

    const auto& font = GetFontDefault();
    Vector2 p = {bounds.x, bounds.y};
    DrawTextFmt(font, "${P_HEALTH}/${P_MAX_HEALTH}", p, 18, 1.0F);
    p.y += bounds.height / 2.0F;
    DrawTextFmt(font, "${P_MANA}/${P_MAX_MANA}", p, 18, 1.0F);
}

void PlayerHUD::update(const Rectangle& bounds, const bool isDrawn)
{
    if (!isDrawn) // No need for update
        return;
    const auto& stats = GetComponent<EntityStatsC>(GetCameraEntity());
    SetFormatValue("P_HEALTH", (int)stats.health);
    SetFormatValue("P_MAX_HEALTH", (int)stats.maxHealth);
    SetFormatValue("P_MANA", (int)stats.mana);
    SetFormatValue("P_MAX_MANA", (int)stats.maxMana);
}

void HotbarSlot::draw(const Rectangle& bounds)
{
    DrawRectangleLinesEx(bounds, 1, DARKGRAY);
}

PlayerHotbar::PlayerHotbar() : UIContainer(480, 900, 960, 180)
{
    for (int i = 0; i < slots; ++i)
    {
        addChild(nullptr, new HotbarSlot());
        getChild(i)->setDimensions(480 + i * HotbarSlot::size, 900);
    }
}

void PlayerHotbar::draw(const Rectangle& bounds) { DrawRectangleLinesEx(bounds, 2, DARKGRAY); }

void PlayerHotbar::update(const Rectangle& bounds, bool isDrawn) {}