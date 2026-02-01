// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/gamedev/Console.h>

#include "internal/globals/ConsoleData.h"

namespace magique
{
    void ConsoleSetOpenKey(const int key) { global::CONSOLE_DATA.openKey = key; }

    void ConsoleSetHistorySize(const int len) { global::CONSOLE_DATA.commandHistoryLen = len; }

    void ConsoleAddString(const char* text) { global::CONSOLE_DATA.addString(text); }

    void ConsoleAddStringF(const char* format, ...)
    {
        va_list va_args;
        va_start(va_args, format);
        global::CONSOLE_DATA.addStringF(format, va_args);
        va_end(va_args);
    }

    void ConsoleRegisterCommand(const Command& command)
    {
        auto& console = global::CONSOLE_DATA;
        auto* cmd = console.getCommand(command.getName());
        if (cmd != nullptr)
        {
            LOG_WARNING("Command with name already exists: %s", command.getName().c_str());
            return;
        }
        if (!command.cmdFunc)
        {
            LOG_ERROR("Cannot register command with no function: %s", command.getName().c_str());
            return;
        }
        global::CONSOLE_DATA.commands.emplace_back(command);
    }

    bool ConsoleRemoveCommand(const std::string_view& name)
    {
        const auto predicate = [&](const Command& cmd)
        {
            return cmd.getName() == name;
        };
        return std::erase_if(global::CONSOLE_DATA.commands, predicate) > 0;
    }

    bool ConsoleExecuteCommand(const std::string_view& name)
    {
        auto& console = global::CONSOLE_DATA;
        MAGIQUE_ASSERT(!name.empty(), "Command name is null!");
        const auto* cmd = console.getCommand(name);
        if (cmd == nullptr)
        {
            LOG_WARNING("Cannot execute command:%s : No such command", name);
            return false;
        }

        if (!cmd->getParamInfo().empty())
        {
            LOG_WARNING("Cannot execute command:%s : Needs parameters", name);
            return false;
        }
        cmd->cmdFunc({});
        return true;
    }

    //================= COMMAND =================//

    Command::Command(const char* cmdName, const char* description) :
        name(cmdName), description((description != nullptr) ? description : "")
    {
    }

    Command& Command::addParam(std::string_view name, const std::array<ParamType, 3>& allowedTypes)
    {
        MAGIQUE_ASSERT(parameters.empty() || (!parameters.back().isVariadic && !parameters.back().isOptional),
                       "A variadic/optional parameter must be the last one!");
        MAGIQUE_ASSERT(allowedTypes.size() <= 3, "Only specify the type once!");
        ParamInfo data;
        data.name = name;
        int i = 0;
        for (const auto type : allowedTypes)
        {
            data.allowedTypes[i++] = type;
        }
        parameters.push_back(data);
        return *this;
    }

    Command& Command::addOptionalNumber(const char* name, const float value)
    {
        MAGIQUE_ASSERT(parameters.empty() || !parameters.back().isVariadic,
                       "A variadic parameter must be the last one!");
        ParamInfo data;
        data.isOptional = true;
        data.name = name;

        data.allowedTypes[0] = ParamType::NUMBER;
        data.number = value;
        parameters.push_back(data);
        return *this;
    }

    Command& Command::addOptionalString(const char* name, const char* value)
    {
        MAGIQUE_ASSERT(parameters.empty() || !parameters.back().isVariadic,
                       "A variadic parameter must be the last one!");
        ParamInfo data;
        data.isOptional = true;
        data.name = name;

        data.allowedTypes[0] = ParamType::STRING;
        data.string = strdup(value);
        parameters.push_back(data);
        return *this;
    }

    Command& Command::addOptionalBool(const char* name, const bool value)
    {
        MAGIQUE_ASSERT(parameters.empty() || !parameters.back().isVariadic,
                       "A variadic parameter must be the last one!");
        ParamInfo data;
        data.isOptional = true;
        data.name = name;

        data.allowedTypes[0] = ParamType::BOOL;
        data.boolean = value;
        parameters.push_back(data);
        return *this;
    }

    Command& Command::addVariadicParam(const std::initializer_list<ParamType> allowedTypes)
    {
        MAGIQUE_ASSERT(parameters.empty() || (!parameters.back().isVariadic && !parameters.back().isOptional),
                       "A variadic parameter must be the last one!");
        MAGIQUE_ASSERT(allowedTypes.size() <= 3, "Only specify the type once!");
        ParamInfo data;
        data.isVariadic = true;
        int i = 0;
        for (const auto type : allowedTypes)
        {
            data.allowedTypes[i++] = type;
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

    const std::string& Command::getName() const { return name; }

    const std::string& Command::getDescription() const { return description; }

    const std::vector<ParamInfo>& Command::getParamInfo() const { return parameters; }

    //================= Environmental Parameters =================//

    void ConsoleSetEnvParam(const std::string_view& name, const std::string_view& value)
    {
        auto& console = global::CONSOLE_DATA;
        MAGIQUE_ASSERT(!name.empty(), "Passed null");
        MAGIQUE_ASSERT(name[0] != console.envPrefix, "Prefix as first char of name not allowed!");
        auto& param = console.envParams[name];
        param = Param{name, value};
        if (console.envChangeCallback)
        {
            console.envChangeCallback(param);
        }
    }

    bool ConsoleRemoveEnvParam(const std::string_view& name)
    {
        auto& console = global::CONSOLE_DATA;
        MAGIQUE_ASSERT(!name.empty(), "Passed null");
        return console.envParams.erase(name) > 0;
    }

    const Param* ConsoleGetEnvParam(const std::string_view& name)
    {
        const auto& paramTable = global::CONSOLE_DATA.envParams;
        const auto it = paramTable.find(name);
        if (it == paramTable.end())
        {
            return nullptr;
        }
        return &it->second;
    }

    void ConsoleSetEnvParamCallback(const std::function<void(const Param& param)>& func)
    {
        global::CONSOLE_DATA.envChangeCallback = func;
    }

    void ConsoleSetEnvParamPrefix(const char prefix) { global::CONSOLE_DATA.envPrefix = prefix; }


} // namespace magique
