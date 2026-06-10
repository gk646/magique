#include <algorithm>

#include <magique/ui/controls/TabSwitcher.h>
#include <magique/util/RayUtils.h>
#include <magique/ui/UI.h>
#include <magique/core/Engine.h>
#include <magique/gamedev/TextDrawer.h>

namespace magique
{
    TabSwitcher::TabSwitcher(Rect bounds, Anchor anchor, Point inset, ScalingMode mode) :
        UIContainer(bounds, anchor, inset, mode)
    {
        setGamepadMapping(new GamepadMapping(*this,
                                             [&](GamepadMappingState& state)
                                             {
                                                 if (state.isLeftSwitch())
                                                     switchLeft();
                                                 else if (state.isRightSwitch())
                                                     switchRight();
                                                 return Point{-1};
                                             }));
    }

    void TabSwitcher::setAlignOffset(Point offset) { alignOffset = offset; }

    Point TabSwitcher::getAlignOffset() const { return alignOffset; }

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
        if (getChildren().empty())
            return nullptr;
        if (active >= 0 && active < (int)getChildren().size())
        {
            return getChild(active);
        }
        return nullptr;
    }

    bool TabSwitcher::isActive(std::string_view tab) const
    {
        for (int i = 0; i < (int)getChildren().size(); i++)
        {
            auto& [name, obj] = getChildren()[i];
            if (name == tab && active == i)
            {
                return true;
            }
        }
        return false;
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

    void TabSwitcher::onDraw(const Rect& bounds)
    {
        Point pos = bounds.pos();
        float biggestY = 0;
        for (int i = 0; i < (int)getChildren().size(); i++)
        {
            auto& child = getChildren()[i];
            auto dims = drawTab(pos, child, i == active);
            Rect tab = {pos, dims};
            if (tab.contains(GetMousePos()) && LayeredInput::IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                setActive(child.object);
            }
            pos.x += dims.x + 2;
            biggestY = std::max(biggestY, pos.y);
        }

        setBounds(Rect::FromSpanPoints(bounds.pos(), {pos.x, pos.y + biggestY}));
        auto activeObj = getActive();
        if (activeObj != nullptr)
        {
            activeObj->align(Anchor::TOP_CENTER, getBounds(), Point{0.0F, biggestY + 5} + alignOffset);
            activeObj->draw();
        }
    }

    Point TabSwitcher::drawTab(Point pos, const ContainerChild& child, bool isActive)
    {
        const auto& theme = EngineGetTheme();
        const auto& fnt = EngineGetFont();
        const Point dims = Point{8, 4} + Point{MeasureTextEx(fnt, child.name.data(), fnt.baseSize * 1.0F, 0.5F)};
        const Rect tab{pos, dims};
        const auto hovered = tab.contains(GetMousePos());
        DrawRectFrameFilled(tab, theme.getBodyColor(hovered, isActive), theme.getOutlineColor(hovered, isActive));
        TextDrawer{fnt, tab}.modCenterV().center(child.name, theme.textHighlight);
        return tab.size();
    }

    void TabSwitcher::updateInputs()
    {
        Point pos = getBounds().pos();
        for (int i = 0; i < (int)getChildren().size(); i++)
        {
            auto& child = getChildren()[i];
            auto dims = drawTab(pos, child, i == active);
            Rect tab = {pos, dims};
            if (tab.contains(GetMousePos()) && LayeredInput::IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                setActive(child.object);
            }
            pos.x += dims.x + 2;
        }
    }

} // namespace magique
