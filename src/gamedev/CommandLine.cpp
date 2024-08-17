#include <magique/gamedev/CommandLine.h>

#include "internal/globals/CommandLineData.h"
#include "internal/utils/STLUtil.h"

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
        const auto predicate = [](const CommandInfo& cmdInfo, const std::string& s)
        { return strcmp(cmdInfo.name.c_str(), s.c_str()) == 0; };
        UnorderedDelete(global::COMMAND_LINE.commands, name, predicate);
    }

    void SetCommandLineKey(const int key) { global::COMMAND_LINE.openKey = key; }

    void SetMaxCommandHistory(const int len) { global::COMMAND_LINE.maxHistoryLen = len; }

} // namespace magique