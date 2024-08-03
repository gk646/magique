#include <magique/gamedev/CommandLine.h>

#include "internal/globals/CommandLineData.h"

namespace magique
{
    void RegisterCommand(const std::string& name, const std::string& description, const std::function<void()>& func)
    {
        auto& cmd = global::COMMAND_LINE;
        for (auto& info : cmd.commands)
        {
            if (strcmp(info.name.c_str(), name.c_str()) == 0)
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
        auto& cmds = global::COMMAND_LINE.commands;
        for (auto it = cmds.begin(); it != cmds.end();)
        {
            if (strcmp(it->name.c_str(), name.c_str()) == 0)
            {
                cmds.erase_unordered(it);
                return;
            }
        }
    }

    void SetCommandLineKey(const int key) { global::COMMAND_LINE.openKey = key; }

    void SetMaxCommandHistory(const int len) { global::COMMAND_LINE.maxHistoryLen = len; }

} // namespace magique