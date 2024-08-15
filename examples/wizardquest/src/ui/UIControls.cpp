#include <magique/ecs/ECS.h>
#include <magique/ui/TextFormat.h>
#include <magique/core/Core.h>

#include "UIControls.h"
#include "Components.h"

void PlayerHUD::draw(const Rectangle& bounds)
{
    auto& pState = GetComponent<PlayerStateC>(GetCameraEntity());

    auto healthP = pState.health / PlayerStateC::MAX_HEALTH;
    const Rectangle health = {bounds.x, bounds.y, bounds.width * healthP, bounds.height / 2.0F};
    DrawRectangleRec(health, RED);

    auto manaP = (pState.mana / PlayerStateC::MAX_MANA) * 0.5F;
    const Rectangle mana = {bounds.x, bounds.y + bounds.height / 2.0F, bounds.width * manaP, bounds.height / 2.0F};
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
    auto& pState = GetComponent<PlayerStateC>(GetCameraEntity());
    SetFormatValue("P_HEALTH", (int)pState.health);
    SetFormatValue("P_MAX_HEALTH", (int)PlayerStateC::MAX_HEALTH);
    SetFormatValue("P_MANA", (int)pState.mana);
    SetFormatValue("P_MAX_MANA", (int)PlayerStateC::MAX_MANA);
}