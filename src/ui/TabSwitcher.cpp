#include <algorithm>

#include <magique/ui/controls/TabSwitcher.h>

#include "magique/util/RayUtils.h"

#include <magique/ui/UI.h>
#include <magique/core/Engine.h>
#include <magique/gamedev/TextDrawer.h>

namespace magique
{
    TabSwitcher::TabSwitcher(Rect bounds, Anchor anchor, Point inset, ScalingMode mode) :
        UIContainer(bounds, anchor, inset, mode)
    {
    }

    void TabSwitcher::setActive(UIObject* tab)
    {
        for (int i = 0; i < (int)getChildren().size(); i++)
        {
            auto& [name, obj] = getChildren()[i];
            if (obj == tab)
            {
                active = i;
                break;
            }
        }
    }

    void TabSwitcher::setActive(std::string_view tab)
    {
        for (int i = 0; i < (int)getChildren().size(); i++)
        {
            auto& [name, obj] = getChildren()[i];
            if (name == tab)
            {
                active = i;
                break;
            }
        }
    }

    void TabSwitcher::setActive(int index)
    {
        active = std::clamp(index, 0, (int)getChildren().size() - 1);
        if (getChildren().empty())
        {
            active = -1;
        }
    }

    UIObject* TabSwitcher::getActive() const
    {
        if (active >= 0 && active < (int)getChildren().size())
        {
            return getChild(active);
        }
        return nullptr;
    }

    void TabSwitcher::switchLeft()
    {
        active--;
        active = (active + getChildren().size()) % getChildren().size();
    }

    void TabSwitcher::switchRight()
    {
        active++;
        active = (active + getChildren().size()) % getChildren().size();
    }

    constexpr float GAP = 3;

    void TabSwitcher::updateInputs()
    {
        if (LayeredInput::IsKeyPressed(KEY_LEFT) || LayeredInput::IsKeyPressed(KEY_Q))
        {
            switchLeft();
        }
        else if (LayeredInput::IsKeyPressed(KEY_RIGHT) || LayeredInput::IsKeyPressed(KEY_E))
        {
            switchRight();
        }
    }

    void TabSwitcher::drawDefault(const Rect& bounds)
    {
        const auto& theme = EngineGetTheme();
        const auto& fnt = EngineGetFont();
        Point pos = bounds.pos();
        auto drawSwitchButton = [&](const Rect& area, bool left)
        {
            auto hov = area.contains(GetMousePos());
            auto pres = hov && LayeredInput::IsMouseButtonDown(MOUSE_BUTTON_LEFT);
            DrawRectFrameFilled(area, theme.getBodyColor(hov, pres), theme.getOutlineColor(hov, pres));
            TextDrawer drawer{fnt, area};
            drawer.modCenterV().center(left ? "Q" : "E", theme.textPassive);
        };

        drawSwitchButton({pos, 8}, true);
        pos += Point{8 + GAP, 0};

        for (auto& [name, obj] : getChildren())
        {
            Rect tab{pos, {MeasureTextEx(fnt, name.data(), fnt.baseSize, 1.0F).x + 4, fnt.baseSize + 4.0F}};
            const auto hovered = tab.contains(GetMousePos());
            const auto hoverFill = hovered ? theme.text : theme.textPassive;
            DrawRectFrameFilled(tab, hoverFill, theme.getOutlineColor(hovered, false));
            TextDrawer{fnt, tab}.modCenterV().center(name, theme.textHighlight);
            pos += Point{tab.width + GAP, 0};
        }
        drawSwitchButton({pos, 8}, false);
        setBounds(Rect::FromSpanPoints(bounds.pos(), pos + Point{0, fnt.baseSize + 2.0F}));

        auto activeObj = getActive();
        if (activeObj != nullptr)
        {
            activeObj->align(Anchor::TOP_CENTER, getBounds(), {0, 20});
            activeObj->draw();
        }
    }

} // namespace magique
