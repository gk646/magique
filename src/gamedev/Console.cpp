#include <magique/gamedev/Console.h>

#include <raylib/config.h>
#include "internal/globals/ConsoleData.h"
#include "internal/utils/STLUtil.h"

namespace magique
{
    void RegisterConsoleCommand(const std::string& name, const std::string& description,
                                const std::function<void()>& func)
    {
        /*
        auto& cmd = global::CONSOLE_DATA;
        for (auto& info : cmd.commands)
        {
            if (strcmp(info.name.c_str(), name.c_str()) == 0)
            {
                info.name = name;
                info.description = description;
               // info.func = func;
                return;
            }
        }
        */
        // cmd.commands.emplace_back(name, description, func);
    }

    bool UnRegisterCommand(const std::string& name)
    {
        /*
        auto& console = global::CONSOLE_DATA;
        const auto predicate = [](const CommandInfo& cmdInfo, const std::string& s)
        { return strcmp(cmdInfo.name.c_str(), s.c_str()) == 0; };
        int const size = console.commands.size();
        UnorderedDelete(console.commands, name, predicate);
        return console.commands.size() == size;
        */
    }

    void SetConsoleKey(const int key) { global::CONSOLE_DATA.openKey = key; }

    void SetCommandHistorySize(const int len) { global::CONSOLE_DATA.commandHistoryLen = len; }

    void AddConsoleString(const char* text) {}

    //================= COMMAND =================//

    Command::Command(const char* cmdName, const char* description) :
        name(cmdName), description(description ? description : "")
    {
    }


} // namespace magique