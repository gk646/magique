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
            LOG_WARNING("Command with name already exists: %s", command.getName().data());
            return;
        }
        if (!command.cmdFunc)
        {
            LOG_ERROR("Cannot register command with no function: %s", command.getName().data());
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

    Command::Command(std::string_view cmdName, std::string_view description) : name(cmdName), description(description) {}

    Command& Command::addParam(std::string_view param, const std::array<ParamType, 3>& allowedTypes)
    {
        MAGIQUE_ASSERT(parameters.empty() || (!parameters.back().isVariadic && !parameters.back().isOptional),
                       "A variadic/optional parameter must be the last one!");
        MAGIQUE_ASSERT(allowedTypes.size() <= 3, "Only specify the type once!");
        auto& newParam = parameters.emplace_back();
        newParam.name = param;
        int i = 0;
        for (const auto type : allowedTypes)
        {
            newParam.allowedTypes[i++] = type;
        }
        return *this;
    }

    Command& Command::addOptionalNumber(std::string_view param, const float value)
    {
        MAGIQUE_ASSERT(parameters.empty() || !parameters.back().isVariadic,
                       "A variadic parameter must be the last one!");
        auto& newParam = parameters.emplace_back();
        newParam.isOptional = true;
        newParam.name = param;
        newParam.allowedTypes[0] = ParamType::NUMBER;
        newParam.number = value;
        return *this;
    }

    Command& Command::addOptionalString(std::string_view param, std::string_view value)
    {
        MAGIQUE_ASSERT(parameters.empty() || !parameters.back().isVariadic,
                       "A variadic parameter must be the last one!");
        auto& newParam = parameters.emplace_back();
        newParam.isOptional = true;
        newParam.name = param;
        newParam.allowedTypes[0] = ParamType::STRING;
        newParam.string = value;
        return *this;
    }

    Command& Command::addOptionalBool(std::string_view param, const bool value)
    {
        MAGIQUE_ASSERT(parameters.empty() || !parameters.back().isVariadic,
                       "A variadic parameter must be the last one!");
        auto& newParam = parameters.emplace_back();
        newParam.isOptional = true;
        newParam.name = param;
        newParam.allowedTypes[0] = ParamType::BOOL;
        newParam.boolean = value;
        return *this;
    }

    Command& Command::addVariadicParam(const std::initializer_list<ParamType> allowedTypes)
    {
        MAGIQUE_ASSERT(parameters.empty() || (!parameters.back().isVariadic && !parameters.back().isOptional),
                       "A variadic parameter must be the last one!");
        MAGIQUE_ASSERT(allowedTypes.size() <= 3, "Only specify the type once!");
        auto& newParam = parameters.emplace_back();
        newParam.isVariadic = true;
        int i = 0;
        for (const auto type : allowedTypes)
        {
            newParam.allowedTypes[i++] = type;
        }
        return *this;
    }

    Command& Command::setFunction(const CommandFunction& func)
    {
        cmdFunc = func;
        return *this;
    }

    std::string_view Command::getName() const { return name; }

    std::string_view Command::getDescription() const { return description; }

    const std::vector<ParamInfo>& Command::getParamInfo() const { return parameters; }

    //================= Environmental Parameters =================//

    void ConsoleSetEnvParam(std::string_view name, std::string_view value)
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

    bool ConsoleRemoveEnvParam(std::string_view name)
    {
        auto& console = global::CONSOLE_DATA;
        MAGIQUE_ASSERT(!name.empty(), "Passed null");
        return console.envParams.erase(name) > 0;
    }

    const Param* ConsoleGetEnvParam(std::string_view name)
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
