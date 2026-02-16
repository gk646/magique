// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_CONSOLE_DATA_H
#define MAGIQUE_CONSOLE_DATA_H

#include <deque>
#include <algorithm>
#include <raylib/config.h>

#include <magique/core/Debug.h>
#include <magique/ui/UI.h>
#include <magique/core/Game.h>
#include <magique/core/Core.h>
#include <magique/gamedev/Console.h>
#include <magique/util/RayUtils.h>
#include <magique/util/Math.h>
#include <magique/util/Datastructures.h>
#include <magique/util/Strings.h>

#include "internal/globals/EngineConfig.h"
#include "internal/globals/EngineData.h"
#include "external/raylib-compat/rcore_compat.h"

namespace magique
{
    using ParamList = std::vector<Param>;
    const char* GetTypeString(ParamType type);

    struct ConsoleHandler final
    {
        static constexpr float HEIGHT_P = 0.4F; // 40% of height is console - 100% of the width ...
        static constexpr float SPACING = 0.5F;
        static constexpr int CURSOR_BLINK_TICKS = 30;

        static void Draw(const ConsoleData& data);
        static void Update(ConsoleData& data);

    private:
        static void PollControls(ConsoleData& data);
    };

    struct ParamParser final
    {
        // Returns a ptr to the command to be executed - logs errors internally
        static const Command* ParseCommand(const std::vector<std::string>& chunks);
        static ParamList ParseParams(const Command& cmd, const std::vector<std::string>& chunks);
        static bool ValidateParams(const Command& cmd, ParamList& params);
    };

    struct ConsoleLine final
    {
        std::string text;
        Color color;

        ConsoleLine(const std::string& text, const Color& color) : text(text), color(color) {}
    };

    struct ConsoleData final
    {
        // Environment data
        StringHashMap<Param> envParams{25};
        std::function<void(const Param& param)> envChangeCallback{}; // Called every time any param changes

        // Console data
        std::deque<std::string> submitHistory; // Submitted text
        std::deque<ConsoleLine> consoleLines;  // Past lines
        std::string line;                      // Current input line
        const Command* parsedCommand = nullptr;
        std::string commandInfo;

        // Command data
        std::vector<Command> commands;
        std::vector<const Command*> suggestions;

        // State data
        int submitPos = 0;      // which entry of submitHistory to pick
        int cursorPos = 0;      // Cursor position
        int blinkCounter = 0;   //
        int suggestionPos = 0;  // which entry of suggestions to pick
        float scrollOffset = 0; // Render offset
        bool showConsole = false;
        bool showCursor = false;
        bool foundCommand = false; // Command is specified and user is typing parameters currently

        // Config data
        int openKey = KEY_PAGE_UP;
        int commandHistoryLen = 15;
        int consoleHistoryLen = 20;
        char envPrefix = '$';

        // Register default commands
        void init()
        {
            Command print{"print", "Prints each given parameters in a new line to the console"};
            print.addVariadicParam({ParamType::BOOL, ParamType::STRING, ParamType::NUMBER})
                .setFunction(
                    [](const ParamList& params)
                    {
                        auto printParam = [](const Param& param)
                        {
                            switch (param.getType())
                            {
                            case ParamType::NUMBER:
                                if (IsWholeNumber(param.getFloat()))
                                {
                                    ConsoleAddStringF("%d", param.getInt());
                                }
                                else
                                {
                                    ConsoleAddStringF("%.3f", param.getFloat());
                                }
                                break;
                            case ParamType::BOOL:
                                if (param.getBool())
                                {
                                    ConsoleAddString("true");
                                }
                                else
                                {
                                    ConsoleAddString("false");
                                }
                                break;
                            case ParamType::STRING:
                                ConsoleAddStringF("%s", param.getString());
                                break;
                            }
                        };
                        for (const auto& param : params)
                        {
                            printParam(param);
                        }
                    });
            ConsoleRegisterCommand(print);

            Command clear{"clear", "Clears the console"};
            clear.setFunction([&](const ParamList&) { consoleLines.clear(); });
            ConsoleRegisterCommand(clear);

            Command help{"help", "Shows help text"};
            help.setFunction([&](const ParamList& /**/) { ConsoleAddString("\"all\" to show all available command"); });
            ConsoleRegisterCommand(help);

            ConsoleRegisterCommand(Command{"all", "Lists all registered commands"}.setFunction(
                [&](const ParamList& /**/)
                {
                    for (const auto& cmd : commands)
                    {
                        ConsoleAddStringF("%s - %s", cmd.getName().c_str(), cmd.getDescription().c_str());
                    }
                }));

            Command define{"define", "Creates/sets an environment param to the given value (and type)"};
            define.addParam("name", {ParamType::STRING})
                .addParam("value", {ParamType::STRING, ParamType::BOOL, ParamType::NUMBER})
                .setFunction(
                    [&](const ParamList& params)
                    {
                        const auto name = params.front().getString();
                        const auto typeStr = GetTypeString(params.back().getType());
                        switch (params.back().getType())
                        {
                        case ParamType::BOOL:
                            {
                                const auto val = params.back().getBool();
                                ConsoleSetEnvParam(name, val ? "true" : "false");
                                LOG_INFO("Set environmental param: %s to %s | %s", name, val ? "true" : "false",
                                         typeStr);
                            }
                            break;
                        case ParamType::NUMBER:
                            {
                                const auto val = params.back().getFloat();
                                ConsoleSetEnvParam(name, TextFormat("%f", val));
                                if (IsWholeNumber(val))
                                    LOG_INFO("Set environmental param: %s to %d | %s", name, (int)val, typeStr);
                                else
                                    LOG_INFO("Set environmental param: %s to %.3f | %s", name, val, typeStr);
                            }
                            break;
                        case ParamType::STRING:
                            {
                                const auto val = params.back().getString();
                                ConsoleSetEnvParam(name, val);
                                LOG_INFO("Set environmental param: %s to %s | %s", name, val, typeStr);
                            }
                            break;
                        }
                    });
            ConsoleRegisterCommand(define);

            Command undef{"undef", "Removes the environment param with the given name"};
            undef.addParam("name", {ParamType::STRING})
                .setFunction(
                    [&](const ParamList& params)
                    {
                        if (!ConsoleRemoveEnvParam(params.front().getString()))
                        {
                            LOG_WARNING("Given environment param does not exist");
                            return;
                        }
                        LOG_INFO("Removed environment param:%s", params.front().getString());
                    });
            ConsoleRegisterCommand(undef);

            Command shutdown{"shutdown", "Calls Game::shutdown() to close the game"};
            shutdown.setFunction([](const ParamList& /**/) { global::ENGINE_DATA.gameInstance->shutDown(); });
            ConsoleRegisterCommand(shutdown);

            Command showHitboxes{"mq.setHitboxesOverlay", "Turns entity hitboxes on/off"};
            showHitboxes.addParam("value", {ParamType::BOOL})
                .setFunction([](const ParamList& params) { SetShowHitboxes(params.back().getBool()); });
            ConsoleRegisterCommand(showHitboxes);

            Command showEntityGrid{"mq.setEntityOverlay", "Turns the debug entity overlay on/off"};
            showEntityGrid.addParam("value", {ParamType::BOOL})
                .setFunction([](const ParamList& params) { SetShowEntityGridOverlay(params.back().getBool()); });
            ConsoleRegisterCommand(showEntityGrid);

            Command showPathGrid{"mq.setPathfindingOverlay", "Turns the debug pathfinding overlay on/off"};
            showPathGrid.addParam("value", {ParamType::BOOL})
                .setFunction([](const ParamList& params) { SetShowPathFindingOverlay(params.back().getBool()); });
            ConsoleRegisterCommand(showPathGrid);

            Command setFps{"mq.setFps", "Sets the FPS limit - 0 for unlimited"};
            setFps.addParam("fpsLimit", {ParamType::NUMBER});
            setFps.setFunction([](const ParamList& params) { SetTargetFPS(params.front().getInt()); });
            ConsoleRegisterCommand(setFps);

            Command fullscreen{"mq.toggleFullscreen", "Toggles fullscreen on or off "};
            fullscreen.setFunction([](const ParamList& params) { ToggleFullscreen(); });
            ConsoleRegisterCommand(fullscreen);

            Command setWindowSize{"mq.setWindowSize", "Sets the window size of the window"};
            setWindowSize.addParam("x", {ParamType::NUMBER});
            setWindowSize.addParam("y", {ParamType::NUMBER});
            setWindowSize.setFunction([](const ParamList& params)
                                      { SetWindowSize(params.front().getInt(), params.back().getInt()); });
            ConsoleRegisterCommand(setWindowSize);

            Command version{"mq.version", "Prints the engine version"};
            version.setFunction([](const ParamList& params) { ConsoleAddString(MAGIQUE_VERSION); });
            ConsoleRegisterCommand(version);

            Command uptime{"mq.uptime", "Prints the current uptime"};
            uptime.setFunction([](const ParamList& params) { ConsoleAddStringF("Uptime: %.2f sec", GetEngineTime()); });
            ConsoleRegisterCommand(uptime);

            Command perfOverlay{"mq.setPerfOverlay", "Open the performance overlay"};
            perfOverlay.addParam("value", {ParamType::BOOL});
            perfOverlay.setFunction([](const ParamList& params) { SetShowPerformanceOverlay(params.back().getBool()); });
            ConsoleRegisterCommand(perfOverlay);

            Command showUIHitbox{"mq.setUIHitboxOverlay", "Turns the hitboxes for UI elements on/off"};
            showUIHitbox.addParam("value", {ParamType::BOOL});
            showUIHitbox.setFunction([](const ParamList& params) { UISetShowHitboxes(params.back().getBool()); });
            ConsoleRegisterCommand(showUIHitbox);

#ifndef MAGIQUE_TEST_MODE
            ConsoleSetEnvParam("GAME_NAME", global::ENGINE_DATA.gameInstance->getName());
#endif
        }

        void refreshSuggestions()
        {
            suggestionPos = 0;
            suggestions.clear();

            auto itFunc = [&](const auto& cmd)
            {
                if (strncmpnc(cmd.getName().c_str(), line.c_str(), line.size()))
                {
                    suggestions.push_back(&cmd);
                }
            };
            std::ranges::for_each(commands, itFunc);

            auto pred = [&](const Command* c1, const Command* c2)
            {
                auto first = StringDistancePhysical(line, c1->getName());
                auto second = StringDistancePhysical(line, c2->getName());
                return first < second;
            };
            std::ranges::sort(suggestions, pred);

            auto chunks = SplitString(line, ' ');
            parsedCommand = ParamParser::ParseCommand(chunks);
        }

        void draw() const
        {
            if (showConsole)
            {
                ConsoleHandler::Draw(*this);
            }
        }

        void update()
        {
            if (IsKeyPressed(openKey)) [[unlikely]]
                showConsole = !showConsole;

            if (showConsole)
            {
                ConsoleHandler::Update(*this);
            }
        }

        void submit() const
        {
            // Must be separated by spaces
            auto chunks = SplitString(line, ' ');
            const Command* cmd = ParamParser::ParseCommand(chunks);
            if (cmd == nullptr)
            {
                LOG_WARNING("No such command: %s", chunks.front().c_str());
                return;
            }

            auto params = ParamParser::ParseParams(*cmd, chunks);
            if (params.empty())
            {
                return;
            }
            if (!ParamParser::ValidateParams(*cmd, params))
            {
                return;
            }
            cmd->cmdFunc(params);
        }

        void addString(const char* string, Color color = global::ENGINE_CONFIG.theme.textPassive)
        {
            consoleLines.emplace_front(string, color);
        }

        void addStringF(const char* fmt, va_list va_args, Color color = global::ENGINE_CONFIG.theme.textPassive)
        {
            constexpr int MAX_LEN = 128;
            char buf[MAX_LEN]{};
            vsnprintf(buf, MAX_LEN, fmt, va_args);
            addString(buf, color);
        }

        const Command* getCommand(std::string_view name)
        {
            auto it = std::ranges::find_if(commands, [&](const auto& cmd) { return cmd.getName() == name; });
            if (it != commands.end())
            {
                return &*it;
            }
            return nullptr;
        }
    };

    namespace global
    {
        inline ConsoleData CONSOLE_DATA{};
    }

    //================= HANDLER =================//

    inline const char* GetTypeString(const ParamType type)
    {
        switch (type)
        {
        case ParamType::BOOL:
            return "BOOL";
        case ParamType::NUMBER:
            return "NUMBER";
        case ParamType::STRING:
            return "STRING";
        }
        return "(unknown)";
    }

    inline const char* GetTypesString(const std::array<ParamType, 3>& types)
    {
        bool hasBool = false;
        bool hasNumber = false;
        bool hasString = false;

        for (const auto type : types)
        {
            if (type == ParamType::BOOL)
                hasBool = true;
            else if (type == ParamType::NUMBER)
                hasNumber = true;
            else if (type == ParamType::STRING)
                hasString = true;
        }

        if (hasBool && hasNumber && hasString)
            return "(BOOL | NUMBER | STRING)";
        if (hasBool && hasNumber)
            return "(BOOL | NUMBER)";
        if (hasBool && hasString)
            return "(BOOL | STRING)";
        if (hasNumber && hasString)
            return "(NUMBER | STRING)";
        if (hasBool)
            return "BOOL";
        if (hasNumber)
            return "NUMBER";
        if (hasString)
            return "STRING";

        return "(unknown)";
    }

    inline const char* GetCommandParamInfo(const Command& cmd)
    {
        static std::string cache;
        cache.clear();
        for (const auto& param : cmd.getParamInfo())
        {
            if (param.isOptional)
            {
                cache += TextFormat("[%s %s], ", param.name.c_str(), GetTypesString(param.allowedTypes));
            }
            else if (param.isVariadic)
            {
                cache += TextFormat("[%s %s, ...]", param.name.c_str(), GetTypesString(param.allowedTypes));
            }
            else
            {
                cache += TextFormat("%s %s, ", param.name.c_str(), GetTypesString(param.allowedTypes));
            }
        }
        return cache.c_str();
    }

    inline void ConsoleHandler::Draw(const ConsoleData& data)
    {
        const auto& theme = global::ENGINE_CONFIG.theme;
        const auto& font = global::ENGINE_CONFIG.font;
        const auto fsize = global::ENGINE_CONFIG.fontSize;
        const auto lineHeight = fsize * 1.3F;
        const auto inputOff = MeasureTextEx(font, "> ", fsize, SPACING).x;

        const auto cWidth = static_cast<float>(GetScreenWidth());
        const auto cHeight = HEIGHT_P * static_cast<float>(GetScreenHeight());
        const Rectangle cRect = {0, 0, cWidth, cHeight};
        const Rectangle inputBox = {0, cHeight - lineHeight, cWidth, lineHeight};

        // Draw shape
        DrawRectangleRec(cRect, ColorAlpha(theme.backDark, 0.85F));
        DrawRectangleLinesEx(cRect, 2.0F, ColorAlpha(theme.backDark, 0.85F));
        DrawRectangleRec(inputBox, ColorAlpha(theme.backLight, 0.35F));

        // Draw input texty
        Vector2 textPos = {5, cHeight - lineHeight};

        DrawTextEx(font, "> ", textPos, fsize, SPACING, theme.textPassive);
        DrawTextEx(font, data.line.c_str(), {textPos.x + inputOff, textPos.y}, fsize, SPACING, theme.textActive);

        float offset = MeasureTextUpTo(data.line.c_str(), data.cursorPos, font, fsize, SPACING) - fsize * 0.2F;
        if (data.showCursor)
        {
            DrawTextEx(font, "|", {textPos.x + offset + inputOff, textPos.y}, fsize, SPACING, theme.textPassive);
        }

        if (data.parsedCommand != nullptr)
        {
            offset += 20;
            auto info = GetCommandParamInfo(*data.parsedCommand);
            DrawTextEx(font, info, {textPos.x + offset + inputOff, textPos.y}, fsize, SPACING, theme.textPassive);
        }

        BeginScissorMode(0, 0, (int)cWidth, (int)cHeight - (int)lineHeight);
        {
            // Draw console text
            textPos.y -= lineHeight - data.scrollOffset; // Start one line up for console strings
            for (const auto& line : data.consoleLines)
            {
                DrawTextEx(font, line.text.c_str(), textPos, fsize, SPACING, line.color);
                textPos.y -= lineHeight;
            }

            float maxWidth = 100;
            auto getSuggestionText = [](const Command& cmd)
            {
                return TextFormat("%s / %s", cmd.getName().c_str(), cmd.getDescription().c_str());
            };

            // Measure widest suggestion
            auto measureFunc = [&](const auto* cmd)
            {
                maxWidth = std::max(maxWidth, MeasureTextEx(font, getSuggestionText(*cmd), fsize, SPACING).x);
            };
            std::ranges::for_each(data.suggestions, measureFunc);

            textPos = {textPos.x, cHeight - 2 * lineHeight}; // Reset text pos
            for (int i = 0; i < (int)data.suggestions.size(); i++)
            {
                const Rectangle sBox = {textPos.x, textPos.y, maxWidth, lineHeight};
                const auto isSelected = i == data.suggestionPos;
                const Color& text = isSelected ? theme.textActive : theme.textPassive;
                const Color& back = isSelected ? theme.backLight : theme.backDark;

                DrawRectangleRec(sBox, back);
                DrawTextEx(font, getSuggestionText(*data.suggestions[i]), textPos, fsize, SPACING, text);
                textPos.y -= lineHeight;
            }
        }
        EndScissorMode();
    }

    inline void ConsoleHandler::Update(ConsoleData& data)
    {
        data.blinkCounter++;
        if (data.blinkCounter > CURSOR_BLINK_TICKS)
        {
            data.showCursor = !data.showCursor;
            data.blinkCounter = 0;
        }

        PollControls(data);

        // Handle input
        char c;
        while ((c = static_cast<char>(GetCharPressed())) != 0)
        {
            if (c != data.openKey)
            {
                data.line.insert(data.cursorPos, 1, c);
                data.cursorPos++;
                data.refreshSuggestions();
            }
        }

        // Consume key state
        std::memset(GetCurrentKeyState(), 0, MAX_KEYBOARD_KEYS);
    }

    inline void ConsoleHandler::PollControls(ConsoleData& data)
    {
        auto& cursorPos = data.cursorPos;
        auto& historyPos = data.submitPos;
        auto& suggestionPos = data.suggestionPos;
        const auto inputSize = static_cast<int>(data.line.size());
        const auto historySize = static_cast<int>(data.submitHistory.size());
        const int suggestionSize = data.suggestions.size();

        if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && cursorPos > 0)
        {
            data.line.erase(cursorPos - 1, 1);
            cursorPos--;
            data.refreshSuggestions();
        }
        else if ((IsKeyPressed(KEY_DELETE) || IsKeyPressedRepeat(KEY_DELETE)) && cursorPos < inputSize)
        {
            data.line.erase(cursorPos, 1);
            data.refreshSuggestions();
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
            if (data.line.empty())
            {
                return;
            }

            if (historyPos > data.commandHistoryLen)
            {
                data.submitHistory.pop_back();
            }
            data.submitHistory.push_front(data.line);

            suggestionPos = 0;
            historyPos = -1;
            cursorPos = 0;
            data.submit();
            data.parsedCommand = nullptr;
            data.line.clear();
            data.scrollOffset = 0;
            data.suggestions.clear();
        }
        else if (IsKeyPressed(KEY_TAB) && suggestionPos >= 0 && suggestionSize > 0)
        {
            MAGIQUE_ASSERT(suggestionPos < suggestionSize, "Out of bounds");
            data.line = data.suggestions[suggestionPos]->getName();
            cursorPos = static_cast<int>(data.line.size());
            data.refreshSuggestions();
        }
        else if ((IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)))
        {
            // If we have suggestions scroll them else insert previous submits
            if (suggestionSize > 0) // Have one
            {
                if (suggestionPos < (suggestionSize - 1))
                    suggestionPos++;
            }
            else if (historyPos < (historySize - 1))
            {
                ++historyPos;
                data.line = data.submitHistory[historyPos];
                cursorPos = static_cast<int>(data.line.size());
            }
        }
        else if ((IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN)))
        {
            if (suggestionSize > 0)
            {
                if (suggestionPos > 0)
                {
                    suggestionPos--;
                }
            }
            else
            {
                if (historyPos >= 0)
                    --historyPos;
                if (historyPos == -1)
                {
                    data.line.clear();
                }
                else
                {
                    data.line = data.submitHistory[historyPos];
                }
                cursorPos = static_cast<int>(data.line.size());
            }
        }
        const auto cWidth = static_cast<float>(GetScreenWidth());
        const auto cHeight = HEIGHT_P * static_cast<float>(GetScreenHeight());
        const Rectangle cRect = {0, 0, cWidth, cHeight};
        const auto wheelMove = GetMouseWheelMove() * 10.0F;
        if (wheelMove != 0 && CheckCollisionPointRec(GetMousePos(), cRect))
        {
            data.scrollOffset = std::max(0.0F, data.scrollOffset + wheelMove);
            LayeredInput::ConsumeMouse();
        }
    }

    //================= PARSER =================//

    inline const Command* ParamParser::ParseCommand(const std::vector<std::string>& chunks)
    {
        auto& data = global::CONSOLE_DATA;
        if (chunks.empty())
        {
            return nullptr;
        }

        auto* cmd = data.getCommand(chunks.front());
        if (cmd == nullptr)
        {
            return nullptr;
        }
        return cmd;
    }

    inline ParamList ParamParser::ParseParams(const Command& cmd, const std::vector<std::string>& chunks)
    {
        auto getParamName = [&](int i) // Direct index - until for variadics
        {
            const auto size = (int)cmd.getParamInfo().size();
            if (size == 0)
            {
                return std::string{};
            }
            return cmd.getParamInfo()[std::min(size - 1, i)].name;
        };

        ParamList params;
        for (int i = 1; i < (int)chunks.size(); i++)
        {
            // Env param
            if (chunks.size() > 1 && chunks[i][0] == '$' && chunks[i][2] != '$')
            {
                auto it = global::CONSOLE_DATA.envParams.find(chunks[i].c_str() + 1);
                if (it != global::CONSOLE_DATA.envParams.end())
                {
                    params.emplace_back(it->second);
                }
                else
                {
                    LOG_WARNING("No such environment parameter: %s", chunks[i].c_str() + 1);
                    return {};
                }
            }
            else
            {
                params.emplace_back(getParamName(i).c_str(), chunks[i].c_str());
            }
        }
        return params;
    }

    inline bool ParamParser::ValidateParams(const Command& cmd, ParamList& params)
    {
        int min = 0;
        int max = 0;
        const int parsed = (int)params.size();
        for (const auto& info : cmd.getParamInfo())
        {
            if (info.isOptional)
            {
                max++;
            }
            else if (info.isVariadic)
            {
                max = -1;
            }
            else
            {
                min++;
                max++;
            }
        }

        // Size check
        if (parsed < min)
        {
            if (min == max)
            {
                LOG_WARNING("Not enough arguments! Expected: %d Got: %d", min, parsed);
                return false;
            }
            else
            {
                LOG_WARNING("Not enough arguments! Expected at least: %d Got: %d", min, parsed);
                return false;
            }
        }

        if (max != -1 && parsed > max)
        {
            if (min == max)
            {
                LOG_WARNING("Too many arguments! Expected: %d Got: %d", max, parsed);
                return false;
            }
            else
            {
                LOG_WARNING("Too many arguments! Expected at most: %d Got: %d", max, parsed);
                return false;
            }
        }

        auto getParamTypes = [&](int i) // Direct index - until for variadics
        {
            const auto size = (int)cmd.getParamInfo().size();
            return cmd.getParamInfo()[std::min(size - 1, i)].allowedTypes;
        };

        // Type check
        for (int i = 0; i < (int)params.size(); i++)
        {
            auto& param = params[i];
            if (!std::ranges::contains(getParamTypes(i), param.type))
            {
                const auto* expected = GetTypesString(getParamTypes(i));
                const auto* actual = GetTypeString(param.type);
                LOG_WARNING("Type mismatch at \"%s\"! Expected: %s Have: %s", param.name.c_str(), expected, actual);
                return false;
            }
        }

        // Need optionals
        if (max != -1 && parsed != max)
        {
            for (int i = parsed; i < (int)cmd.getParamInfo().size(); i++)
            {
                params.push_back(cmd.getParamInfo()[i]);
            }
        }

        return true;
    }


} // namespace magique

#endif // MAGIQUE_CONSOLE_DATA_H
