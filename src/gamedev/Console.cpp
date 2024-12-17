// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/gamedev/Console.h>

#include "internal/globals/ConsoleData.h"
#include "internal/utils/STLUtil.h"

namespace magique
{
    void RegisterConsoleCommand(const Command& command)
    {
        MAGIQUE_ASSERT(command.cmdFunc, "Command is without function!");
        global::CONSOLE_DATA.commands.push_back(command);
    }

    bool UnRegisterCommand(const char* name)
    {
        auto& console = global::CONSOLE_DATA;
        const auto predicate = [](const Command& cmd, const char* s)
        {
            const auto res = strcmp(cmd.name.c_str(), s);
            if (res)
            {
                for (const auto& param : cmd.parameters)
                {
                    free(param.name);
                    if (param.types[0] == ParamType::STRING)
                        free(param.string);
                }
                return true;
            }
            return false;
        };
        int const size = console.commands.size();
        UnorderedDelete(console.commands, name, predicate);
        return console.commands.size() == size;
    }

    void SetConsoleKey(const int key) { global::CONSOLE_DATA.openKey = key; }

    void SetCommandHistorySize(const int len) { global::CONSOLE_DATA.commandHistoryLen = len; }

    void AddConsoleString(const char* text) { global::CONSOLE_DATA.addString(text); }

    void AddConsoleStringF(const char* format, ...)
    {
        va_list va_args;
        va_start(va_args, format);
        global::CONSOLE_DATA.addStringF(format, va_args);
        va_end(va_args);
    }

    //================= COMMAND =================//

    Command::Command(const char* cmdName, const char* description) :
        name(cmdName), description((description != nullptr) ? description : "")
    {
    }

    Command& Command::addParam(const char* name, const std::initializer_list<ParamType> types)
    {
        MAGIQUE_ASSERT(parameters.empty() || (!parameters.back().variadic && !parameters.back().optional),
                       "A variadic/optional parameter must be the last one!");
        MAGIQUE_ASSERT(types.size() <= 3, "Only specify the type once!");
        internal::ParamData data;
        data.name = strdup(name); // Freed in console data
        int i = 0;
        for (const auto type : types)
        {
            data.types[i++] = type;
        }
        parameters.push_back(data);
        return *this;
    }

    Command& Command::addOptionalNumber(const char* name, const float value)
    {
        MAGIQUE_ASSERT(parameters.empty() || !parameters.back().variadic, "A variadic parameter must be the last one!");
        internal::ParamData data;
        data.optional = true;
        data.name = strdup(name); // Freed in console data

        data.types[0] = ParamType::NUMBER;
        data.number = value;
        parameters.push_back(data);
        return *this;
    }

    Command& Command::addOptionalString(const char* name, const char* value)
    {
        MAGIQUE_ASSERT(parameters.empty() || !parameters.back().variadic, "A variadic parameter must be the last one!");
        internal::ParamData data;
        data.optional = true;
        data.name = strdup(name); // Freed in console data

        data.types[0] = ParamType::STRING;
        data.string = strdup(value); // Freed in console data
        parameters.push_back(data);
        return *this;
    }

    Command& Command::addOptionalBool(const char* name, const bool value)
    {
        MAGIQUE_ASSERT(parameters.empty() || !parameters.back().variadic, "A variadic parameter must be the last one!");
        internal::ParamData data;
        data.optional = true;
        data.name = strdup(name); // Freed in console data

        data.types[0] = ParamType::BOOL;
        data.boolean = value;
        parameters.push_back(data);
        return *this;
    }

    Command& Command::addVariadicParam(const std::initializer_list<ParamType> types)
    {
        MAGIQUE_ASSERT(parameters.empty() || (!parameters.back().variadic && !parameters.back().optional),
                       "A variadic parameter must be the last one!");
        MAGIQUE_ASSERT(types.size() <= 3, "Only specify the type once!");
        internal::ParamData data;
        data.variadic = true;
        int i = 0;
        for (const auto type : types)
        {
            data.types[i++] = type;
        }
        parameters.push_back(data);
        return *this;
    }

    Command& Command::setFunction(const CommandFunction& func)
    {
        if (cmdFunc)
        {
            LOG_WARNING("Command function already set. Replacing ...");
        }
        cmdFunc = func;
        return *this;
    }

    //================= Environmental Parameters =================//

    void SetEnvironmentParam(const char* name, const char* value)
    {
        MAGIQUE_ASSERT(name != nullptr, "Passed null");
        MAGIQUE_ASSERT(name[0] != global::CONSOLE_DATA.envPrefix, "Prefix as first char of name not allowed!");
        auto& param = global::CONSOLE_DATA.environParams[name];
        param.name = global::CONSOLE_DATA.environParams.find(name)->first.c_str();
        if (param.type == ParamType::STRING)
        {
            free((void*)param.string);
        }
        param.type = ParamType::STRING;
        param.string = strdup(value);
    }

    void SetEnvironmentParam(const char* name, const float value)
    {
        MAGIQUE_ASSERT(name != nullptr, "Passed null");
        MAGIQUE_ASSERT(name[0] != global::CONSOLE_DATA.envPrefix, "Prefix as first char of name not allowed!");
        auto& param = global::CONSOLE_DATA.environParams[name];
        param.name = global::CONSOLE_DATA.environParams.find(name)->first.c_str();
        if (param.type == ParamType::STRING)
        {
            free((void*)param.string);
        }
        param.type = ParamType::NUMBER;
        param.number = value;
    }

    void SetEnvironmentParam(const char* name, const bool value)
    {
        MAGIQUE_ASSERT(name != nullptr, "Passed null");
        MAGIQUE_ASSERT(name[0] != global::CONSOLE_DATA.envPrefix, "Prefix as first char of name not allowed!");
        auto& param = global::CONSOLE_DATA.environParams[name];
        param.name = global::CONSOLE_DATA.environParams.find(name)->first.c_str();
        if (param.type == ParamType::STRING)
        {
            free((void*)param.string);
        }
        param.type = ParamType::BOOL;
        param.boolean = value;
    }

    bool RemoveEnvironmentParam(const char* name)
    {
        auto& paramTable = global::CONSOLE_DATA.environParams;
        if (paramTable.contains(name))
        {
            auto it = paramTable.find(name);
            if (it->second.type == ParamType::STRING)
                free((void*)it->second.string);
            paramTable.erase(it);
            return true;
        }
        return false;
    }

    const Param* GetEnvironmentParam(const char* name)
    {
        const auto& paramTable = global::CONSOLE_DATA.environParams;
        const auto it = paramTable.find(name);
        if (it == paramTable.end())
        {
            return nullptr;
        }
        return &it->second;
    }

    void SetEnvironmentSetCallback(const std::function<void(const Param& param)>& func)
    {
        global::CONSOLE_DATA.environChangeCallback = func;
    }

    void SetEnvironmentParamPrefix(const char prefix) { global::CONSOLE_DATA.envPrefix = prefix; }


} // namespace magique