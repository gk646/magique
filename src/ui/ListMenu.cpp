#include <cstring>
#include <raylib/raylib.h>
#include <magique/ui/controls/ListMenu.h>
#include "magique/ui/UI.h"
#include <magique/util/RayUtils.h>
#include <magique/core/Core.h>

namespace magique
{
    ListMenu::ListMenu(float x, float y, float w, float h) : UIObject(x, y, w, h) {}

    ListMenu::ListMenu(float w, float h, Anchor anchor, Point inset) : UIObject(w, h, anchor, inset) {}

    void ListMenu::onDraw(const Rectangle& bounds)
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
                entrySize = drawDefaultEntry(pos, entry.text.c_str(), hovered == i, selected == i);
            }
            pos.y += entrySize;
            entry.height = entrySize;
        }
    }

    void ListMenu::clear()
    {
        hovered = -1;
        selected = -1;
        entries.clear();
    }

    bool ListMenu::empty() const { return entries.empty(); }

    void ListMenu::add(const char* item, int index)
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
            entries.emplace_back(item);
        }
        else
        {
            entries.emplace(entries.begin() + index, item);
        }
    }

    bool ListMenu::remove(const char* item)
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

    bool ListMenu::remove(int index)
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

    int ListMenu::getHoveredIndex() const { return hovered; }

    const char* ListMenu::getHovered() const
    {
        if (hovered != -1)
        {
            return entries[hovered].text.c_str();
        }
        return nullptr;
    }

    int ListMenu::getSelectedIndex() const { return selected; }

    const char* ListMenu::getSelected() const
    {
        if (selected != -1)
        {
            return entries[selected].text.c_str();
        }
        return nullptr;
    }

    void ListMenu::setSelected(int index) { selected = index; }

    void ListMenu::setOnSelect(const SelectFunc& func) { selectFunc = func; }

    void ListMenu::setDrawEntryFunc(const DrawEntryFunc& func) { drawFunc = func; }

    float ListMenu::drawDefaultEntry(const Point& pos, const char* txt, bool hovered, bool selected) const
    {
        Color backGround = DARKGRAY;
        const Color textColor = LIGHTGRAY;
        if (hovered || selected)
        {
            backGround = GRAY;
        }
        const auto& fnt = GetEngineFont();
        const auto size = UIGetScaled(15);
        const auto bounds = getBounds();
        DrawRectangleRec({pos.x, pos.y, bounds.width, bounds.height}, backGround);
        DrawTextEx(fnt, txt, {pos.x + 2, pos.y + 1}, size, 1.0F, textColor);
        return size + 2;
    }

    void ListMenu::updateState()
    {
        const auto& bounds = getBounds();
        Point pos = {bounds.x, bounds.y};
        hovered = -1;
        for (int i = 0; i < (int)entries.size(); i++)
        {
            const auto& entry = entries[i];
            Rectangle lineRect = {pos.x, pos.y, bounds.width, entry.height};
            if (CheckCollisionMouseRect(lineRect) && !LayeredInput::GetIsMouseConsumed())
            {
                hovered = i;
                if (LayeredInput::IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    if (i != selected)
                    {
                        selected = i;
                        if (selectFunc)
                        {
                            selectFunc(entries[i].text);
                        }
                    }
                }
            }
            pos.y += entry.height;
        }
        setSize(bounds.width, pos.y - bounds.y);
    }

} // namespace magique
