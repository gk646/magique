#ifndef COMMANDLINEDATA_H
#define COMMANDLINEDATA_H

#include <vector>
#include <string>
#include <raylib/raylib.h>

#include "internal/globals/Configuration.h"
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
        std::vector<CommandInfo> commands;
        std::vector<std::string> history;
        std::string input;
        int openKey = KEY_GRAVE;
        int historyPos = 0;
        int maxHistoryLen = 10;
        bool isVisible = false;

        // Input handling and drawing done in 1
        // Doesnt need to be efficient as its not open a lot
        void draw()
        {
            if (IsKeyPressed(openKey)) [[unlikely]]
                isVisible = !isVisible;

            if (isVisible) [[unlikely]] // mostly not visible
            {

                // Input
                {
                    char c;
                    while ((c = GetCharPressed()) != 0)
                    {
                        if (c != openKey)
                            input.push_back(c);
                    }
                }
                // Draw
                {
                    const auto scw = static_cast<float>(GetScreenWidth());
                    const auto sch = static_cast<float>(GetScreenHeight());
                    const auto width = scw * 0.7F;
                    const auto height = sch * 0.1F;
                    const auto x = (scw - width) / 2.0F;
                    const auto y = sch - height * 3.0F;
                    const Rectangle rect = {x, y, width, height};
                    DrawRectangleRec(rect, ColorAlpha(GRAY, 0.75F));
                    DrawRectangleLinesEx(rect, 2.0F, ColorAlpha(DARKGRAY, 0.75F));
                    DrawTextEx(global::CONFIGURATION.font, input.c_str(), {x, y}, height / 5, 1.0F, DARKGRAY);
                }

                // Consume key state
                std::memset(CORE.Input.Keyboard.currentKeyState, 0, MAX_KEYBOARD_KEYS);
            }
        }
    };

    namespace global
    {
        inline CommandLineData COMMAND_LINE{};
    }
} // namespace magique

#endif //COMMANDLINEDATA_H