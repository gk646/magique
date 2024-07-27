#include <vector>

#include <magique/gamedev/CommandLine.h>

#include "internal/globals/CommandLineData.h"

namespace magique
{
    void RegisterCommand(const std::string& name, const std::string& description, const std::function<void()>& func)
    {
        auto& cmd = global::COMMAND_LINE;
        for (auto& info : cmd.commands)
        {
            if (info.name == name)
            {
                info.name = name;
                info.description = description;
                info.func = func;
                return;
            }
        }
        cmd.commands.emplace_back(name, description, func);
    }

    void UnRegisterCommand(const std::string& name)
    {
        std::erase_if(global::COMMAND_LINE.commands, [&](const CommandInfo& info) { return info.name == name; });
    }

    void SetCommandLineKey(const int key) { global::COMMAND_LINE.openKey = key; }

    void SetMaxCommandHistory(const int len) { global::COMMAND_LINE.maxHistoryLen = len; }


} // namespace magique