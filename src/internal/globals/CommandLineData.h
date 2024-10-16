#ifndef COMMANDLINEDATA_H
#define COMMANDLINEDATA_H

#include <functional>
#include <string>

#include <magique/util/RayUtils.h>
#include <magique/core/Draw.h>

#include "internal/datastructures/VectorType.h"
#include "internal/globals/EngineConfig.h"
#include "external/raylib/src/coredata.h"

namespace magique
{
    struct CommandInfo final
    {
        std::string name;
        std::string description;
        std::function<void()> func;
    };

    struct CommandLineData final
    {
        vector<CommandInfo> commands;
        vector<std::string> history;
        vector<std::string> terminalLines;
        vector<const CommandInfo*> suggestions;
        std::string input;
        int openKey = KEY_PAGE_UP;
        int historyPos = 0;
        int maxHistoryLen = 10;
        int cursorPos = 0;
        int blinkCounter = 0;
        bool showConsole = false;
        bool showCursor = false;

        void draw()
        {
            if (showConsole) [[unlikely]]
            {
                const auto& font = global::ENGINE_CONFIG.font;
                const auto scw = static_cast<float>(GetScreenWidth());  // screen height
                const auto sch = static_cast<float>(GetScreenHeight()); // screen width
                const auto width = scw * 0.7F;
                const auto height = sch * 0.04F;
                const auto x = (scw - width) / 2.0F;
                const auto y = sch - height * 3.0F;
                const auto fsize = height / 1.3F;            // fontsize
                const auto tx = x + width * 0.01F;           // text-x
                const auto ty = y + (height - fsize) / 2.0F; // text-y
                const Rectangle rect = {x, y, width, height};

                DrawRectangleRec(rect, ColorAlpha(DARKGRAY, 0.75F));
                DrawRectangleLinesEx(rect, 2.0F, ColorAlpha(GRAY, 0.75F));
                DrawTextEx(font, input.c_str(), {tx, ty}, fsize, 0.5F, LIGHTGRAY);
                if (!suggestions.empty())
                {
                    for (int i = 0; i < suggestions.size(); ++i)
                    {
                        const auto sy = y - fsize * static_cast<float>(i + 1); // suggestion-y
                        const char* txt = suggestions[i]->name.c_str();
                        DrawTextEx(font, txt, {tx, sy}, fsize, 1.0F, DARKGRAY);
                        txt = suggestions[i]->description.c_str();
                        DrawRightBoundText(font, txt, {x + width, sy}, fsize, 1.0F, DARKGRAY);
                    }
                }
                if (showCursor)
                {
                    const float offset = MeasureTextUpTo(input.data(), cursorPos, font, fsize, 0.5F);
                    DrawTextEx(font, "|", {tx + offset, ty}, fsize, 0.5F, DARKGRAY);
                }
            }
        }

        void update()
        {
            if (IsKeyPressed(openKey)) [[unlikely]]
                showConsole = !showConsole;

            if (showConsole) [[unlikely]]
            {
                const int pos = cursorPos;
                const int inputSize = static_cast<int>(input.size());
                if (blinkCounter > 30)
                {
                    showCursor = !showCursor;
                    blinkCounter = 0;
                }
                if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && cursorPos > 0)
                {
                    input.erase(cursorPos - 1, 1);
                    cursorPos--;
                }
                else if ((IsKeyPressed(KEY_DELETE) || IsKeyPressedRepeat(KEY_DELETE)) && cursorPos < inputSize)
                {
                    input.erase(cursorPos, 1);
                    refreshSuggestions();
                }
                else if ((IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) && cursorPos > 0)
                {
                    --cursorPos;
                }

                else if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) && cursorPos < inputSize)
                {
                    ++cursorPos;
                }
                else if (IsKeyPressed(KEY_ENTER))
                {
                    submit();
                }
                else if (IsKeyPressed(KEY_TAB) && suggestions.size() == 1)
                {
                    input = suggestions[0]->name;
                    cursorPos =inputSize;
                }
                else if ((IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) &&
                         historyPos < history.size() - 1)
                {
                    ++historyPos;
                    input = history[history.size() - 1 - historyPos];
                    cursorPos =inputSize;
                }
                else if ((IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)) && historyPos > -1)
                {
                    --historyPos;
                    if (historyPos == -1)
                    {
                        input.clear();
                    }
                    else
                    {
                        input = history[history.size() - 1 - historyPos];
                    }
                    cursorPos = static_cast<int>(inputSize);
                }

                char c;
                while ((c = static_cast<char>(GetCharPressed())) != 0)
                {
                    if (c != openKey && GetKeyPressed() != openKey)
                    {
                        input.insert(cursorPos, 1, c);
                        cursorPos++;
                    }
                }

                if (pos != cursorPos)
                    refreshSuggestions();

                // Consume key state
                std::memset(CORE.Input.Keyboard.currentKeyState, 0, MAX_KEYBOARD_KEYS);
                blinkCounter++;
            }
        }

        void refreshSuggestions()
        {
            suggestions.clear();
            if (input.empty())
                return;
            const size_t inputLen = input.size();
            for (const auto& cmd : commands)
            {
                if (strncmp(cmd.name.c_str(), input.c_str(), inputLen) == 0 && cmd.name.size() != inputLen)
                {
                    suggestions.push_back(&cmd);
                }
            }
        }

        void submit()
        {
            if (history.size() > maxHistoryLen)
            {
                history.erase(history.begin());
            }
            if (terminalLines.size() > maxHistoryLen)
            {
                terminalLines.erase(terminalLines.begin());
            }
            terminalLines.push_back(input);
            history.push_back(input);
            historyPos = -1;
            cursorPos = 0;
            for (const auto& cmd : commands)
            {
                if (cmd.name == input)
                {
                    cmd.func();
                    break;
                }
            }
            input.clear();
        }
    };

    namespace global
    {
        inline CommandLineData CMD_DATA{};
    }
} // namespace magique

#endif //COMMANDLINEDATA_H