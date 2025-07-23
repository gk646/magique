// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_CONSOLE_DATA_H
#define MAGIQUE_CONSOLE_DATA_H

#include <deque>
#include <raylib/config.h>

#include <magique/core/Debug.h>
#include <magique/core/Game.h>
#include <magique/core/Core.h>
#include <magique/gamedev/Console.h>
#include <magique/util/RayUtils.h>
#include <magique/util/Math.h>
#include <magique/util/Strings.h>

#include "internal/datastructures/VectorType.h"
#include "internal/datastructures/StringHashMap.h"
#include "internal/globals/EngineConfig.h"
#include "internal/globals/EngineData.h"
#include "internal/utils/STLUtil.h"
#include "external/raylib-compat/rcore_compat.h"

namespace magique
{
    using ParamList = std::vector<Param>;
    static const char* GetTypeString(ParamType type);
    inline const char* GetTypesString(const ParamType (&types)[3]);
    inline bool GetTypeMatches(ParamType actual, const ParamType (&types)[3]);

    struct ConsoleHandler final
    {
        static constexpr float HEIGHT_P = 0.4F; // 40% of height is console - 100% of the width ...
        static constexpr int CURSOR_BLINK_TICKS = 30;
        static constexpr float SPACING = 0.5F;

        static void draw(const ConsoleData& data);
        static void update(ConsoleData& data);


    private:
        static void PollControls(ConsoleData& data);
    };

    struct ParamParser final
    {
        // Returns a ptr to the command to be executed - logs errors internally
        static const Command* parse(ConsoleData& data);

    private:
        static bool ParseNextParam(ConsoleData& data, int& off, int& paramCount, Param& param);

        // Errors
        static Command* TypeMismatchError(ConsoleData& d, Param& p, const internal::ParamData& e, int idx);
        static Command* CountMismatchError(const ConsoleData& data, const Command& cmd);
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
        StringHashMap<Param> environParams{25};                          // Environment parameters
        std::function<void(const Param& param)> environChangeCallback{}; // Called every time any param changes

        // Console data
        std::deque<std::string> submitHistory; // Submitted text
        std::deque<ConsoleLine> consoleLines;  // Past lines
        std::string line;                      // Current input line

        // Command data
        std::vector<Param> parameters;        // Command parameters
        vector<Command> commands;             // All registered commands
        vector<const Command*> suggestions;   // Command suggestions
        vector<std::string> stringParameters; // Storage for the string parameters

        // State data
        int submitPos = 0;          // which entry of submitHistory to pick
        int cursorPos = 0;          // Cursor position
        int blinkCounter = 0;       //
        int suggestionPos = 0;      // which entry of suggestions to pick
        float maxSuggestionLen = 0; // Maximum length of the longest suggestion text
        float renderOffset = 0;     // Render offset
        bool showConsole = false;
        bool showCursor = false;
        bool foundCommand = false; // Command is specified and user is typing parameters currently

        // Config data
        float fontSize = 15.0F;
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
                                    AddConsoleStringF("%d", param.getInt());
                                }
                                else
                                {
                                    AddConsoleStringF("%.3f", param.getFloat());
                                }
                                break;
                            case ParamType::BOOL:
                                if (param.getBool())
                                {
                                    AddConsoleString("true");
                                }
                                else
                                {
                                    AddConsoleString("false");
                                }
                                break;
                            case ParamType::STRING:
                                AddConsoleStringF("%s", param.getString());
                                break;
                            }
                        };
                        for (const auto& param : params)
                        {
                            printParam(param);
                        }
                    });
            RegisterConsoleCommand(print);

            Command clear{"clear", "Clears the console"};
            clear.setFunction([&](const ParamList&) { consoleLines.clear(); });
            RegisterConsoleCommand(clear);

            Command help{"help", "Shows help text"};
            help.setFunction(
                [&](const ParamList& /**/)
                { AddConsoleString("Type in a command and press ENTER. See gamedev/Console.h for more info"); });
            RegisterConsoleCommand(help);

            RegisterConsoleCommand(Command{"all", "Lists all registered commands"}.setFunction(
                [&](const ParamList& /**/)
                {
                    for (const auto& cmd : commands)
                    {
                        const char* desc = cmd.description.c_str() ? cmd.description.c_str() : "...";
                        AddConsoleStringF("%s - %s", cmd.name.c_str(), desc);
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
                                SetEnvironmentParam(name, val);
                                LOG_INFO("Set environmental param:%s to %s | %s", name, val ? "true" : "false", typeStr);
                            }
                            break;
                        case ParamType::NUMBER:
                            {
                                const auto val = params.back().getFloat();
                                SetEnvironmentParam(name, val);
                                if (IsWholeNumber(val))
                                    LOG_INFO("Set environmental param:%s to %df | %s", name, (int)val, typeStr);
                                else
                                    LOG_INFO("Set environmental param:%s to %.3f | %s", name, val, typeStr);
                            }
                            break;
                        case ParamType::STRING:
                            {
                                const auto val = params.back().getString();
                                SetEnvironmentParam(name, val);
                                LOG_INFO("Set environmental param:%s to %s | %s", name, val, typeStr);
                            }
                            break;
                        }
                    });
            RegisterConsoleCommand(define);

            Command undef{"undef", "Removes the environment param with the given name"};
            undef.addParam("name", {ParamType::STRING})
                .setFunction(
                    [&](const ParamList& params)
                    {
                        if (!RemoveEnvironmentParam(params.front().getString()))
                        {
                            LOG_WARNING("Given environment param does not exist");
                            return;
                        }
                        LOG_INFO("Removed environment param:%s", params.front().getString());
                    });
            RegisterConsoleCommand(undef);

            Command shutdown{"shutdown", "Calls Game::shutdown() to close the game"};
            shutdown.setFunction([](const ParamList& /**/) { global::ENGINE_DATA.gameInstance->shutDown(); });
            RegisterConsoleCommand(shutdown);

            Command showHitboxes{"m.setHitboxesOverlay", "Turns entity hitboxes on/off"};
            showHitboxes.addParam("value", {ParamType::BOOL})
                .setFunction([](const ParamList& params) { SetShowHitboxes(params.back().getBool()); });
            RegisterConsoleCommand(showHitboxes);

            Command showEntityGrid{"m.setEntityOverlay", "Turns the debug entity overlay on/off"};
            showEntityGrid.addParam("value", {ParamType::BOOL})
                .setFunction([](const ParamList& params) { SetShowEntityGridOverlay(params.back().getBool()); });
            RegisterConsoleCommand(showEntityGrid);

            Command showPathGrid{"m.setPathfindingOverlay", "Turns the debug pathfinding overlay on/off"};
            showPathGrid.addParam("value", {ParamType::BOOL})
                .setFunction([](const ParamList& params) { SetShowPathFindingOverlay(params.back().getBool()); });
            RegisterConsoleCommand(showPathGrid);

            Command setFps{"m.setFps", "Sets the FPS limit - 0 for unlimited"};
            setFps.addParam("fpsLimit", {ParamType::NUMBER});
            setFps.setFunction([](const ParamList& params) { SetTargetFPS(params.front().getInt()); });
            RegisterConsoleCommand(setFps);

            Command fullscreen{"m.toggleFullscreen", "Toggles fullscreen on or off "};
            fullscreen.setFunction([](const ParamList& params) { ToggleFullscreen(); });
            RegisterConsoleCommand(fullscreen);

            Command setWindowSize{"m.setWindowSize", "Sets the window size of the window"};
            setWindowSize.addParam("x", {ParamType::NUMBER});
            setWindowSize.addParam("y", {ParamType::NUMBER});
            setWindowSize.setFunction([](const ParamList& params)
                                      { SetWindowSize(params.front().getInt(), params.back().getInt()); });
            RegisterConsoleCommand(setWindowSize);

            Command version{"m.version", "Prints the engine version"};
            version.setFunction([](const ParamList& params) { AddConsoleString(MAGIQUE_VERSION); });
            RegisterConsoleCommand(version);

            Command uptime{"m.uptime", "Prints the current uptime"};
            uptime.setFunction([](const ParamList& params) { AddConsoleStringF("Uptime: %.2f sec", GetEngineTime()); });
            RegisterConsoleCommand(uptime);

#ifndef MAGIQUE_TEST_MODE
            SetEnvironmentParam("GAME_NAME", global::ENGINE_DATA.gameInstance->getName());
#endif
        }

        ~ConsoleData()
        {
            for (auto& cmd : commands)
            {
                for (auto& param : cmd.parameters)
                {
                    free(param.name);
                    if (param.types[0] == ParamType::STRING)
                    {
                        // free(param.string); error?
                    }
                }
            }
        }

        void refreshSuggestions()
        {
            suggestionPos = 0;
            maxSuggestionLen = 0;
            suggestions.clear();
            if (line.empty())
                return;
            const char* lineStr = line.c_str();
            const int inputLen = static_cast<int>(line.size());
            for (const auto& cmd : commands)
            {
                const char* cmdStr = cmd.name.c_str();
                if (strncmpnc(cmdStr, lineStr, inputLen))
                {
                    suggestions.push_back(&cmd);
                    const auto& font = global::ENGINE_CONFIG.font;
                    float len = MeasureTextEx(font, cmdStr, fontSize, ConsoleHandler::SPACING).x;
                    len += MeasureTextEx(font, cmd.description.c_str(), fontSize, ConsoleHandler::SPACING).x;
                    if (len > maxSuggestionLen)
                        maxSuggestionLen = len + fontSize * 4;
                }
            }

            auto pred = [&](const Command* c1, const Command* c2)
            {
                auto first = StringDistance(lineStr, c1->name.c_str());
                auto second = StringDistance(lineStr, c2->name.c_str());
                return first < second;
            };
            QuickSort(suggestions.data(), suggestions.size(), pred);
        }

        void draw() const { ConsoleHandler::draw(*this); }

        void update()
        {
            fontSize = global::ENGINE_CONFIG.fontSize;
            ConsoleHandler::update(*this);
        }

        void submit()
        {
            const auto* command = ParamParser::parse(*this);
            if (command != nullptr)
            {
                command->cmdFunc(parameters);
            }
            line.clear();
            suggestions.clear();
            renderOffset = 0.0F;
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

        // Saves allocations by reusing the string
        std::string& getParsedParamString(const int i)
        {
            // Reuse existing string memory
            if (stringParameters.size() > i)
            {
                return stringParameters[i];
            }
            stringParameters.emplace_back();
            // Resize above small buffer optimization so move doesnt break c_str() pointer - yes...
            stringParameters.back().resize(16, '\0');
            return stringParameters.back();
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

    inline const char* GetTypesString(const ParamType (&types)[3])
    {
        if (types[0] == ParamType::BOOL && types[1] == ParamType::NUMBER && types[2] == ParamType::STRING)
            return "(BOOL | NUMBER | STRING)";
        if (types[0] == ParamType::BOOL && types[1] == ParamType::NUMBER)
            return "(BOOL | NUMBER)";
        if (types[0] == ParamType::BOOL && types[1] == ParamType::STRING)
            return "(BOOL | STRING)";
        if (types[0] == ParamType::NUMBER && types[1] == ParamType::STRING)
            return "(NUMBER | STRING)";
        if (types[0] == ParamType::BOOL)
            return "BOOL";
        if (types[0] == ParamType::NUMBER)
            return "NUMBER";
        if (types[0] == ParamType::STRING)
            return "STRING";
        return "(unknown)";
    }

    inline bool GetTypeMatches(const ParamType actual, const ParamType (&types)[3])
    {
        for (const auto type : types)
        {
            if (actual == type)
            {
                return true;
            }
        }
        return false;
    }

    inline void ConsoleHandler::draw(const ConsoleData& data)
    {
        if (data.showConsole) [[unlikely]]
        {
            const auto& theme = global::ENGINE_CONFIG.theme;
            const auto& font = global::ENGINE_CONFIG.font;
            const auto fsize = data.fontSize;
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

            // Draw input text
            Vector2 textPos = {5, cHeight - lineHeight};

            DrawTextEx(font, "> ", textPos, fsize, SPACING, theme.textPassive);
            DrawTextEx(font, data.line.c_str(), {textPos.x + inputOff, textPos.y}, fsize, SPACING, theme.textActive);
            if (data.showCursor)
            {
                float offset = MeasureTextUpTo(data.line.c_str(), data.cursorPos, font, fsize, SPACING) - fsize * 0.2F;
                DrawTextEx(font, "|", {textPos.x + offset + inputOff, textPos.y}, fsize, SPACING, theme.textPassive);
            }

            BeginScissorMode(0, 0, (int)cWidth, (int)cHeight - (int)lineHeight);
            // Draw console text
            textPos.y -= lineHeight - data.renderOffset; // Start one line up for console strings
            for (const auto& line : data.consoleLines)
            {
                DrawTextEx(font, line.text.c_str(), textPos, fsize, SPACING, line.color);
                textPos.y -= lineHeight;
            }

            textPos = {textPos.x, cHeight - 2 * lineHeight}; // Reset text pos
            const auto suggestions = data.suggestions.size();
            if (suggestions > 0)
            {
                const auto gap = fsize * 0.5F;
                const auto dashOff = MeasureTextEx(font, "-", fsize, SPACING).x + gap * 2;
                for (int i = 0; i < suggestions; ++i) // Iterate from front - sorted descending in quality
                {
                    const Rectangle sBox = {textPos.x, textPos.y, data.maxSuggestionLen, lineHeight};
                    const auto isSelected = i == data.suggestionPos;
                    const auto& color = isSelected ? theme.textActive : theme.textPassive;
                    const auto& cmd = *data.suggestions[i];
                    const char* name = cmd.name.c_str();
                    const char* desc = cmd.description.empty() ? "..." : cmd.description.c_str();
                    const float nOff = MeasureTextEx(font, name, fsize, SPACING).x;

                    DrawRectangleRec(sBox, isSelected ? theme.backLight : theme.backDark);
                    DrawTextEx(font, name, textPos, fsize, SPACING, color);
                    DrawTextEx(font, "-", {textPos.x + nOff + gap, textPos.y}, fsize, SPACING, color);
                    DrawTextEx(font, desc, {textPos.x + nOff + dashOff, textPos.y}, fsize, SPACING, color);
                    textPos.y -= lineHeight;
                }
            }
            EndScissorMode();
        }
    }

    inline void ConsoleHandler::PollControls(ConsoleData& data)
    {
        auto& cursorPos = data.cursorPos;
        auto& historyPos = data.submitPos;
        auto& suggestionPos = data.suggestionPos;
        const auto inputSize = static_cast<int>(data.line.size());
        const auto historySize = static_cast<int>(data.submitHistory.size());
        const auto suggestionSize = data.suggestions.size();

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
                return;
            if (historyPos > data.commandHistoryLen)
            {
                data.submitHistory.pop_back();
            }
            data.submitHistory.push_front(data.line);

            suggestionPos = 0;
            historyPos = -1;
            cursorPos = 0;
            data.submit();
        }
        else if (IsKeyPressed(KEY_TAB) && suggestionPos >= 0 && suggestionSize > 0)
        {
            MAGIQUE_ASSERT(suggestionPos < suggestionSize, "Out of bounds");
            data.line = data.suggestions[suggestionPos]->name;
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
        if (CheckCollisionPointRec(GetMousePosition(), cRect))
        {
            data.renderOffset = std::max(0.0F, data.renderOffset + GetMouseWheelMoveV().y * 10.0F);
        }
    }

    inline void ConsoleHandler::update(ConsoleData& data)
    {

        if (IsKeyPressed(data.openKey)) [[unlikely]]
            data.showConsole = !data.showConsole;

        if (!data.showConsole) [[likely]]
            return;

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

    //================= PARSER =================//

    static bool ParseHasNext(const ConsoleData& data, int& off)
    {
        const char* ptr = data.line.c_str();
        while ((ptr[off] != 0))
        {
            if (isblank(ptr[off]) == 0) // Not whitespace
            {
                return true;
            }
            off++;
        }
        return false;
    }

    inline Command* ParamParser::TypeMismatchError(ConsoleData& d, Param& p, const internal::ParamData& e, int idx)
    {
        const auto* paramString = d.stringParameters[idx].c_str();
        const auto* expected = GetTypesString(e.types);
        const auto* actual = GetTypeString(p.type);
        LOG_WARNING("Type mismatch at \"%s\"! Expected: %s Have: %s", paramString, expected, actual);
        return nullptr;
    }

    inline Command* ParamParser::CountMismatchError(const ConsoleData& data, const Command& cmd)
    {
        const int actual = static_cast<int>(data.parameters.size());
        int expected = 0;
        bool hasOptional = false;
        bool hasVariadic = false;

        for (const auto& defined : cmd.parameters)
        {
            if (defined.optional)
                hasOptional = true;
            else if (defined.variadic)
                hasVariadic = true;
            else
                ++expected;
        }

        if (actual > expected)
        {
            LOG_WARNING(hasVariadic ? "Too many arguments! Expected at least: %d Got: %d"
                                    : "Too many arguments! Expected: %d Got: %d",
                        expected, actual);
            return nullptr;
        }

        if (actual < expected)
        {
            LOG_WARNING(hasOptional ? "Not enough arguments! Expected at least: %d Got: %d"
                                    : "Not enough arguments! Expected: %d Got: %d",
                        expected, actual);
            return nullptr;
        }
        MAGIQUE_ASSERT(false, "error");
        return nullptr;
    }

    inline bool ParamParser::ParseNextParam(ConsoleData& data, int& off, int& paramCount, Param& param)
    {
        const char* ptr = data.line.c_str();
        MAGIQUE_ASSERT(isblank(ptr[off]) == 0, "Internal parse error");

        const int paramStart = off;
        while ((ptr[off] != 0) && isblank(ptr[off]) == 0) // Not whitespace
        {
            off++;
        }
        const int paramEnd = off;
        MAGIQUE_ASSERT(paramStart <= paramEnd, "Internal parse error");

        std::string& paramString = data.getParsedParamString(paramCount);
        paramString.assign(ptr + paramStart, paramEnd - paramStart);

        if (paramString[0] == data.envPrefix && (paramString.size() == 1 || paramString[1] != data.envPrefix))
        {
            const auto it = data.environParams.find(paramString.c_str() + 1);
            if (it == data.environParams.end())
            {
                LOG_WARNING("Environment param not found:%s", paramString.c_str() + 1);
                return false; // Not found but specified
            }
            param.type = it->second.type;
            param.string = it->second.string;
            return true;
        }

        const char* paramPtr = paramString.c_str();
        // Bool cases
        if (strcmp(paramPtr, "False") == 0 || strcmp(paramPtr, "false") == 0 || strcmp(paramPtr, "FALSE") == 0 ||
            strcmp(paramPtr, "OFF") == 0 || strcmp(paramPtr, "off") == 0)
        {
            param.boolean = false;
            param.type = ParamType::BOOL;
        }
        else if (strcmp(paramPtr, "True") == 0 || strcmp(paramPtr, "true") == 0 || strcmp(paramPtr, "TRUE") == 0 ||
                 strcmp(paramPtr, "ON") == 0 || strcmp(paramPtr, "on") == 0)
        {
            param.boolean = true;
            param.type = ParamType::BOOL;
        }
        else // Either number or string
        {
            int dotCount = 0;
            bool isNum = true;
            for (const auto c : paramString)
            {
                if (c == '.')
                {
                    dotCount++;
                    if (dotCount > 1)
                    {
                        isNum = false;
                        break;
                    }
                }
                else if (isdigit(c) == 0) // Not a number
                {
                    isNum = false;
                    break;
                }
            }

            if (isNum)
            {
                param.number = TextToFloat(paramPtr);
                param.type = ParamType::NUMBER;
            }
            else
            {
                param.string = paramPtr;
                param.type = ParamType::STRING;
            }
        }
        paramCount++;
        return true;
    }

    inline const Command* ParamParser::parse(ConsoleData& data)
    {
        int off = 0;
        int commandLen = 0;
        const char* ptr = data.line.c_str();

        // Skip the command name
        while ((ptr[off] != 0) && isblank(ptr[off]) == 0)
        {
            off++;
            commandLen++;
        }

        const Command* command = nullptr;
        for (const auto& cmd : data.commands)
        {
            if (static_cast<int>(cmd.name.size()) == commandLen)
            {
                if (strncmp(cmd.name.c_str(), ptr, commandLen) == 0)
                {
                    command = &cmd;
                    break;
                }
            }
        }

        if (command == nullptr)
        {
            data.line.resize(commandLen);
            LOG_WARNING("No command with name:%s", data.line.c_str());
            return nullptr;
        }

        auto& parsedParams = data.parameters;
        const auto& definedParams = command->parameters;

        parsedParams.clear();

        int paramCount = 0; // Count of the parameters parsed
        while (ParseHasNext(data, off))
        {
            Param param{};
            if (!ParseNextParam(data, off, paramCount, param))
            {
                return nullptr;
            }
            parsedParams.push_back(param);
        }

        // Take care of simple cases
        bool hasVariadic = false;
        int optionals = 0;
        for (const auto& param : definedParams)
        {
            if (param.variadic)
            {
                hasVariadic = true;
            }
            if (param.optional)
            {
                optionals++;
            }
        }

        if (hasVariadic && (int)parsedParams.size() < ((int)definedParams.size() - 1))
        {
            return CountMismatchError(data, *command);
        }
        // No params only allowed if all are optionals or a variadic
        if (parsedParams.empty() && (!hasVariadic && optionals != (int)definedParams.size()))
        {
            return CountMismatchError(data, *command);
        }

        // Must not have more parsed than defined
        if (!hasVariadic && parsedParams.size() > definedParams.size())
        {
            return CountMismatchError(data, *command);
        }

        // Must have exactly as many parsed as defined
        if (!(optionals > 0) && !hasVariadic && parsedParams.size() != definedParams.size())
        {
            return CountMismatchError(data, *command);
        }

        // Must have at least non-optional amount
        if ((optionals > 0) && parsedParams.size() < (definedParams.size() - optionals))
        {
            return CountMismatchError(data, *command);
        }

        int i = 0;
        for (const auto& paramDef : definedParams)
        {
            if (!paramDef.optional && !paramDef.variadic)
            {
                auto& parsedParam = data.parameters[i];
                if (!GetTypeMatches(parsedParam.type, paramDef.types))
                {
                    return TypeMismatchError(data, parsedParam, paramDef, i);
                }
                parsedParam.name = paramDef.name;
            }
            else if (paramDef.optional)
            {
                if (parsedParams.empty() || i == (int)parsedParams.size())
                {
                    for (int j = i; j < static_cast<int>(definedParams.size()); j++)
                    {
                        const auto& paramDefI = definedParams[j];
                        Param param{};
                        param.type = paramDefI.types[0]; // Optionals have the type at 0
                        param.name = paramDefI.name;
                        switch (param.type)
                        {
                        case ParamType::BOOL:
                            param.boolean = paramDefI.boolean;
                            break;
                        case ParamType::NUMBER:
                            param.number = paramDefI.number;
                            break;
                        case ParamType::STRING:
                            param.string = paramDefI.string;
                            break;
                        }
                        data.parameters.push_back(param);
                    }
                    break;
                }

                auto& parsedParam = data.parameters[i];
                if (!GetTypeMatches(parsedParam.type, paramDef.types))
                {
                    return TypeMismatchError(data, parsedParam, paramDef, i);
                }
                parsedParam.name = paramDef.name;
            }
            else
            {
                break;
            }
            i++;
        }

        if (hasVariadic)
        {
            const auto& paramDef = definedParams[i];
            for (int j = i; j < static_cast<int>(parsedParams.size()); j++)
            {
                auto& parsedParam = data.parameters[j];
                if (!GetTypeMatches(parsedParam.type, paramDef.types))
                {
                    return TypeMismatchError(data, parsedParam, paramDef, j);
                }
                parsedParam.name = paramDef.name;
            }
        }

        return command;
    }


} // namespace magique

#endif //MAGIQUE_CONSOLE_DATA_H