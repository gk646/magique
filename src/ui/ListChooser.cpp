#include <cstring>
#include <raylib/raylib.h>
#include <algorithm>

#include <magique/ui/controls/ListChooser.h>
#include <magique/util/RayUtils.h>
#include <magique/core/Engine.h>
#include <magique/ui/UI.h>

#include "internal/globals/EngineConfig.h"

namespace magique
{
    ListChooser::ListChooser(Rect bounds, Anchor anchor, Point inset, ScalingMode mode) :
        UIObject(bounds, anchor, inset, mode)
    {
        setGamepadMapping(new GamepadMapping(*this,
                                             [&](GamepadMappingState& state, GamepadButton button)
                                             {
                                                 if (state.isUpOrDown())
                                                 {
                                                     int direction = state.isUp() ? -1 : 1;
                                                     state.row =
                                                         std::clamp(state.row + direction, 0, (int)entries.size() - 1);

                                                     const auto bounds = getBounds();
                                                     Point pos = {bounds.mid().x, bounds.y};
                                                     for (int i = 0; i < state.row; i++)
                                                     {
                                                         pos.y += entries[i].height + spacing;
                                                     }
                                                     return pos;
                                                 }

                                                 if (state.event == GamepadMappingEvent::Back)
                                                     LayeredInput::ConsumeKey();

                                                 return Point{-1};
                                             }));
    }

    void ListChooser::onDraw(const Rect& bounds)
    {
        Point pos = {bounds.x, bounds.y};
        for (int i = 0; i < (int)entries.size(); i++)
        {
            auto& entry = entries[i];
            float entrySize = 0;
            if (drawFunc)
            {
                entrySize = drawFunc(pos, entry.text.c_str(), i, hovered == i, selected == i);
            }
            else
            {
                entrySize = drawDefaultEntry(pos, entry.text, hovered == i, selected == i);
            }
            pos.y += entrySize + spacing;
            entry.height = entrySize;
        }
    }

    void ListChooser::clear()
    {
        hovered = -1;
        selected = -1;
        entries.clear();
    }

    bool ListChooser::empty() const { return entries.empty(); }

    int ListChooser::size() const { return (int)entries.size(); }

    bool ListChooser::contains(std::string_view item) const
    {
        return std::ranges::any_of(entries, [&](const auto& entry) { return entry.text == item; });
    }

    ListChooser& ListChooser::add(std::string_view item, int index)
    {
        if (index < hovered)
        {
            hovered++;
        }
        if (index < selected)
        {
            selected++;
        }

        if (index == -1)
        {
            entries.emplace_back(std::string{item});
        }
        else
        {
            entries.emplace(entries.begin() + index, std::string{item});
        }
        return *this;
    }

    ListChooser& ListChooser::add(std::initializer_list<std::string_view> items)
    {
        for (auto& item : items)
        {
            add(item);
        }
        return *this;
    }

    bool ListChooser::remove(std::string_view item)
    {
        for (int i = 0; i < (int)entries.size(); i++)
        {
            auto& entry = entries[i];
            if (item == entry.text)
            {
                return remove(i);
            }
        }
        return false;
    }

    bool ListChooser::remove(int index)
    {
        if (index >= 0 && index < (int)entries.size())
        {
            if (index < hovered)
            {
                hovered--;
            }
            if (index < selected)
            {
                selected--;
            }
            entries.erase(entries.begin() + index);
            return true;
        }
        return false;
    }

    int ListChooser::getHoveredIndex() const { return hovered; }

    std::string_view ListChooser::getHovered() const
    {
        if (selected >= 0 && selected < (int)entries.size())
        {
            return entries[hovered].text.c_str();
        }
        return {};
    }

    int ListChooser::getSelectedIndex() const { return selected; }

    std::string_view ListChooser::getSelected() const
    {
        if (selected >= 0 && selected < (int)entries.size())
        {
            return entries[selected].text;
        }
        return {};
    }

    void ListChooser::setSelected(int index, bool triggerCallback)
    {
        selected = index;
        if (triggerCallback && selectFunc && selected >= 0 && selected < (int)entries.size())
            selectFunc(getSelected());
    }

    void ListChooser::setSelected(std::string_view item, bool triggerCallback)
    {
        for (int i = 0; i < (int)entries.size(); i++)
        {
            auto& entry = entries[i];
            if (entry.text == item)
            {
                selected = i;
                if (triggerCallback && selectFunc)
                    selectFunc(entry.text);
                return;
            }
        }
    }

    float ListChooser::getSpacing() const { return spacing; }

    void ListChooser::setSpacing(float newSpacing) { spacing = newSpacing; }

    void ListChooser::setOnSelect(const SelectFunc<std::string_view>& func) { selectFunc = func; }

    void ListChooser::setDrawEntryFunc(const DrawItemFunc& func) { drawFunc = func; }

    float ListChooser::drawDefaultEntry(const Point& pos, std::string_view txt, bool isHovered, bool isSelected) const
    {
        const auto& theme = global::ENGINE_CONFIG.theme;
        const Color backGround = theme.getBodyColor(isHovered, isSelected);
        const Color outline = theme.getOutlineColor(isHovered, isSelected);
        const Color textColor = theme.getTextColor(isHovered, isSelected);
        const auto& fnt = EngineGetFont();
        const float fsize = UIGetScaled(fnt.baseSize);
        const auto bounds = getBounds();
        DrawRectFrameFilled({pos.x, pos.y, bounds.width, fsize + 2.0F}, backGround, outline);
        DrawTextEx(fnt, txt.data(), {pos.x + 2, pos.y + 1}, fsize, 1.0F, textColor);
        return fsize + 2.0F;
    }

    void ListChooser::updateState()
    {
        const auto& bounds = getBounds();
        Point pos = bounds.pos();
        hovered = -1;
        const auto pressed = LayeredInput::IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        for (int i = 0; i < (int)entries.size(); i++)
        {
            const auto& entry = entries[i];
            const Rect lineRect = {pos, {bounds.width, entry.height}};
            if (CheckCollisionMouseRect(lineRect) && !LayeredInput::GetIsMouseConsumed())
            {
                hovered = i;
                if (pressed)
                {
                    if (i != selected)
                    {
                        selected = i;
                        if (selectFunc)
                            selectFunc(entries[i].text);
                    }
                    UISetPreviousGamepadMap();
                    LayeredInput::ConsumeMouse();
                }
            }
            pos.y += entry.height + spacing;
        }

        if (pressed && bounds.contains(GetMousePos()))
            LayeredInput::ConsumeMouse();

        setSize({bounds.width, std::max(pos.y - bounds.y, getStartBounds().height)});
    }

} // namespace magique
