#ifndef MAGIQUE_CONSOLE_DATA_H
#define MAGIQUE_CONSOLE_DATA_H

#include <deque>

#include <magique/gamedev/Console.h>
#include <magique/util/RayUtils.h>
#include <magique/util/Math.h>

#include "internal/datastructures/VectorType.h"
#include "internal/globals/EngineConfig.h"
#include "external/raylib-compat/rcore_compat.h"

namespace magique
{
    struct ConsoleHandler final
    {
        static constexpr float HEIGHT_P = 0.3F; // 30% of height is console - 100% of the width ...
        static constexpr int CURSOR_BLINK_TICKS = 30;

        static void draw(const ConsoleData& data);
        static void update(ConsoleData& data);

    private:
        static void PollControls(ConsoleData& data);
    };

    struct ConsoleParameterParser final
    {
        // Returns a ptr to the command to be executed - logs errors internally
        static const Command* parse(ConsoleData& data);

    private:
        static bool ParseNextParam(ConsoleData& data, int& off, int& n, Parameter& param);
    };

    struct ConsoleData final
    {
        // Terminal data
        std::deque<std::string> submitHistory; // Submitted text
        std::deque<std::string> consoleLines;  // Past lines
        std::string line;                      // Current input line

        // Command data
        vector<Command> commands;             // All registered commands
        vector<const Command*> suggestions;   // Command suggestions
        std::vector<Parameter> parameters;    // Command parameters
        vector<std::string> stringParameters; // Storage for the string parmeters

        // State data
        int submitPos = 0;     // which entry of submitHistory to pick
        int cursorPos = 0;     // Cursor position
        int blinkCounter = 0;  //
        int suggestionPos = 0; // which entry of suggestions to pick
        bool showConsole = false;
        bool showCursor = false;

        // Config data
        float fontSize = 15.0F;
        int openKey = KEY_PAGE_UP;
        int commandHistoryLen = 15;
        int terminalHistoryLen = 20;

        // Register default commands
        ConsoleData()
        {
            RegisterConsoleCommand(
                Command{"print"}
                    .addVariadicParams({ParameterType::BOOL, ParameterType::STRING, ParameterType::NUMBER})
                    .setFunction(
                        [](const std::vector<Parameter>& params)
                        {
                            auto printParam = [](const Parameter& param)
                            {
                                switch (param.getType())
                                {
                                case ParameterType::NUMBER:
                                    if (IsWholeNumber(param.getFloat()))
                                    {
                                        AddConsoleStringF("%d", param.getInt());
                                    }
                                    else
                                    {
                                        AddConsoleStringF("%.3f", param.getFloat());
                                    }
                                    break;
                                case ParameterType::BOOL:
                                    if (param.getBool())
                                    {
                                        AddConsoleString("true");
                                    }
                                    else
                                    {
                                        AddConsoleString("false");
                                    }
                                    break;
                                case ParameterType::STRING:
                                    AddConsoleStringF("%s", param.getString());
                                    break;
                                }
                            };
                            for (const auto& param : params)
                            {
                                printParam(param);
                            }
                        }));

            RegisterConsoleCommand(
                Command{"clear"}.setFunction([&](const std::vector<Parameter>& params) { consoleLines.clear(); }));
        }

        void refreshSuggestions()
        {
            suggestions.clear();
            if (line.empty())
                return;
            const size_t inputLen = line.size();
            for (const auto& cmd : commands)
            {
                if (strncmp(cmd.name.c_str(), line.c_str(), inputLen) == 0 && cmd.name.size() != inputLen)
                {
                    suggestions.push_back(&cmd);
                }
            }
        }

        void draw() const { ConsoleHandler::draw(*this); }

        void update() { ConsoleHandler::update(*this); }

        void submit()
        {
            const auto* command = ConsoleParameterParser::parse(*this);
            if (command)
            {
                command->cmdFunc(parameters);
            }
            line.clear();
        }

        void addString(const char* string) { consoleLines.emplace_front(string); }

        void addStringF(const char* fmt, va_list va_args)
        {
            constexpr int MAX_LEN = 150;
            char buf[MAX_LEN]{};
            consoleLines.emplace_front();
            auto& string = consoleLines.front();
            vsnprintf(buf, MAX_LEN, fmt, va_args);
            string.assign(buf);
        }
    };

    namespace global
    {
        inline ConsoleData CONSOLE_DATA{};
    }

    //================= HANDLER =================//

    inline void ConsoleHandler::draw(const ConsoleData& data)
    {
        if (data.showConsole) [[unlikely]]
        {
            const auto& font = global::ENGINE_CONFIG.font;
            const auto fsize = data.fontSize;
            const auto lineHeight = fsize * 1.3F;
            constexpr auto spacing = 0.5F;
            const auto inputOff = MeasureTextEx(font, "> ", fsize, spacing).x;

            const auto cWidth = static_cast<float>(GetScreenWidth());
            const auto cHeight = HEIGHT_P * static_cast<float>(GetScreenHeight());
            const Rectangle cRect = {0, 0, cWidth, cHeight};
            const Rectangle inputBox = {0, cHeight - lineHeight, cWidth, lineHeight};

            // Draw shape
            DrawRectangleRec(cRect, ColorAlpha(DARKGRAY, 0.75F));
            DrawRectangleLinesEx(cRect, 2.0F, ColorAlpha(DARKGRAY, 0.85F));
            DrawRectangleRec(inputBox, ColorAlpha(LIGHTGRAY, 0.35F));

            // Draw input text
            Vector2 textPos = {5, cHeight - lineHeight};

            DrawTextEx(font, "> ", textPos, fsize, spacing, LIGHTGRAY);
            DrawTextEx(font, data.line.c_str(), {textPos.x + inputOff, textPos.y}, fsize, spacing, LIGHTGRAY);
            if (data.showCursor)
            {
                const float offset = MeasureTextUpTo(data.line.c_str(), data.cursorPos, font, fsize, spacing);
                DrawTextEx(font, "|", {textPos.x + offset + inputOff, textPos.y}, fsize, spacing, DARKGRAY);
            }

            // Draw console text
            textPos.y -= lineHeight; // Start one line up for terminal strings
            for (const auto& line : data.consoleLines)
            {
                if (textPos.y < 0)
                    break;
                DrawTextEx(font, line.c_str(), textPos, fsize, spacing, LIGHTGRAY);
                textPos.y -= lineHeight;
            }

            const auto suggestions = data.suggestions.size();
            if (suggestions > 0)
            {
                for (int i = 0; i < suggestions; ++i)
                {
                    /*
                    const auto sy = y - fsize * static_cast<float>(i + 1); // suggestion-y
                    const char* txt = data.suggestions[i]->name.c_str();
                    DrawTextEx(font, txt, {tx, sy}, fsize, 1.0F, DARKGRAY);
                    txt = suggestions[i]->description.c_str();
                    DrawRightBoundText(font, txt, {x + width, sy}, fsize, 1.0F, DARKGRAY);
                    */
                }
            }
        }
    }

    inline void ConsoleHandler::PollControls(ConsoleData& data)
    {
        auto& cursorPos = data.cursorPos;
        auto& historyPos = data.submitPos;
        auto& suggestionPos = data.submitPos;
        const auto inputSize = static_cast<int>(data.line.size());
        const auto historySize = static_cast<int>(data.submitHistory.size());
        const auto suggestionSize = data.suggestions.size();

        if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && cursorPos > 0)
        {
            data.line.erase(cursorPos - 1, 1);
            cursorPos--;
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
            if (historyPos > data.commandHistoryLen)
            {
                data.submitHistory.pop_back();
            }
            data.submitHistory.push_front(data.line);

            historyPos = -1;
            cursorPos = 0;
            data.submit();
        }
        else if (IsKeyPressed(KEY_TAB) && suggestionPos >= 0)
        {
            MAGIQUE_ASSERT(suggestionPos < suggestionSize, "Out of bounds");
            data.line = data.suggestions[suggestionPos]->name;
            cursorPos = static_cast<int>(data.line.size());
            suggestionPos = -1;
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

    inline bool ConsoleParameterParser::ParseNextParam(ConsoleData& data, int& off, int& paramCount, Parameter& param)
    {
        const char* ptr = data.line.c_str();
        MAGIQUE_ASSERT(isblank(ptr[off]) == 0, "Internal parse error");

        const int paramStart = off;
        while ((ptr[off] != 0) && isblank(ptr[off]) == 0) // Not whitespace
        {
            off++;
        }
        const int paramEnd = off;
        MAGIQUE_ASSERT(paramStart < paramEnd, "Internal parse error");

        std::string* paramString;
        // Reuse existing string memory
        if (data.stringParameters.size() > paramCount)
        {
            paramString = &data.stringParameters[paramCount];
        }
        else
        {
            data.stringParameters.emplace_back();
            paramString = &data.stringParameters.back();
        }
        paramString->assign(ptr + paramStart, paramEnd - paramStart);

        const char* paramPtr = paramString->c_str();
        // Bool cases
        if (strcmp(paramPtr, "False") == 0 || strcmp(paramPtr, "false") == 0 || strcmp(paramPtr, "FALSE") == 0 ||
            strcmp(paramPtr, "OFF") == 0 || strcmp(paramPtr, "off") == 0)
        {
            param.boolean = false;
            param.type = ParameterType::BOOL;
        }
        else if (strcmp(paramPtr, "True") == 0 || strcmp(paramPtr, "true") == 0 || strcmp(paramPtr, "TRUE") == 0 ||
                 strcmp(paramPtr, "ON") == 0 || strcmp(paramPtr, "on") == 0)
        {
            param.boolean = true;
            param.type = ParameterType::BOOL;
        }
        else // Either number or string
        {
            int dotCount = 0;
            bool isNum = true;
            for (const auto c : *paramString)
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
                param.type = ParameterType::NUMBER;
            }
            else
            {
                param.string = paramPtr;
                param.type = ParameterType::STRING;
            }
        }
        paramCount++;
        return true;
    }

    inline const Command* ConsoleParameterParser::parse(ConsoleData& data)
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

        // Parse parameters - don't clear string parameters to reuse string memory
        data.parameters.clear();

        int paramCount = 0;
        while (ParseHasNext(data, off))
        {
            Parameter param{};
            if (!ParseNextParam(data, off, paramCount, param))
            {
                return nullptr;
            }
            data.parameters.push_back(param);
        }
        return command;
    }


} // namespace magique

#endif //MAGIQUE_CONSOLE_DATA_H