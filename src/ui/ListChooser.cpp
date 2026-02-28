#include <cstring>
#include <raylib/raylib.h>

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
            pos.y += entrySize;
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

    void ListChooser::add(std::string_view item, int index)
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
    }

    bool ListChooser::remove(const char* item)
    {
        for (int i = 0; i < (int)entries.size(); i++)
        {
            auto& entry = entries[i];
            if (strcmp(item, entry.text.c_str()) == 0)
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

    const char* ListChooser::getHovered() const
    {
        if (hovered != -1)
        {
            return entries[hovered].text.c_str();
        }
        return nullptr;
    }

    int ListChooser::getSelectedIndex() const { return selected; }

    const char* ListChooser::getSelected() const
    {
        if (selected != -1)
        {
            return entries[selected].text.c_str();
        }
        return nullptr;
    }

    void ListChooser::setSelected(int index) { selected = index; }

    void ListChooser::setSelected(std::string_view item)
    {
        for (int i = 0; i < (int)entries.size(); i++)
        {
            auto& entry = entries[i];
            if (entry.text == item)
            {
                selected = i;
                return;
            }
        }
    }

    void ListChooser::setOnSelect(const SelectFunc<std::string>& func) { selectFunc = func; }

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
        for (int i = 0; i < (int)entries.size(); i++)
        {
            const auto& entry = entries[i];
            const Rect lineRect = {pos, {bounds.width, entry.height}};
            if (CheckCollisionMouseRect(lineRect) && !LayeredInput::GetIsMouseConsumed())
            {
                hovered = i;
                if (LayeredInput::IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    if (i != selected)
                    {
                        selected = i;
                        if (selectFunc)
                            selectFunc(entries[i].text);
                    }
                    LayeredInput::ConsumeMouse();
                }
            }
            pos.y += entry.height;
        }
        setSize({bounds.width, std::max(pos.y - bounds.y, getStartBounds().height)});
    }

} // namespace magique
