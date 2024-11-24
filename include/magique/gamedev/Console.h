#ifndef MAGIQUE_COMMANDLINE_H
#define MAGIQUE_COMMANDLINE_H

#include <string>
#include <magique/internal/InternalTypes.h>
#include <magique/internal/PlatformIncludes.h>

//===============================================
// Console Module
//===============================================
// .....................................................................
// This module allows to register custom command that can be executed in game.
// It's drawn on top of the game and consumes all input if opened.
// Controls:
//      - PAGE_UP:    opens/closes the command window
//      - ENTER:      submits the current input text
//      - TAB:        autocomplete the current selection
//      - ARROW_UP:   insert previous submitted text or move autocomplete selection up (if shown)
//      - ARROW_DOWN: insert next submitted text or move autocomplete selection down (if shown)
//
// Allows to register custom commands. Commands are executed with its name followed by a variable amount of parameters.
// All parameters are separated by at least 1 whitespace:
//     - setLighting on
//     - setPlayerPos 512 1024
// The parameters are parsed and validated automatically. See the ParameterType enum for parsing rules.
// If the number of parameters and theirs types do not match that of the command, it is not executed!
// Default commands:
//      - print (NUMBER | BOOL | STRING) ...  / prints all given parameters
//      - shutdown                            / closes the program via Game::shutdown
//
// Note: All logged messages are automatically displayed in the terminal as well (from util/Logging.h)
// Note: See examples/headers/CommandExample.h on how to build and execute commands
// .....................................................................

namespace magique
{
    //================= CONTROL =================//

    // Sets the key which opens the command line
    // Note: Can be set to an invalid key to "lock" the console
    // Default: KEY_PAGE_UP
    void SetConsoleKey(int key);

    // Sets the maximum length of the command history
    // Default: 10
    void SetCommandHistorySize(int len);

    //================= INTERACTION =================//

    // Adds a string to the console in a new line
    void AddConsoleString(const char* text);

    // Adds a formatted string to the console in a new line - same as printf()
    void AddConsoleStringF(const char* format, ...);

    //================= COMMANDS =================//

    // Registers a custom command with the given name and description
    // Will replace the existing command with the same name with a warning (if exists)
    void RegisterConsoleCommand(const Command& command);

    // Returns true if the command with the given name is successfully removed
    bool UnRegisterCommand(const std::string& name);

    // Function is passed the parsed parameters
    using CommandFunction = std::function<void(const std::vector<Parameter>& params)>;

    struct Command final
    {
        // Creates a new command instance with the specified name and optional description
        //      - cmdName: the name of the command - must not contain whitespace - can contain numbers
        explicit Command(const char* cmdName, const char* description = nullptr);

        // Adds a new parameters that accepts any only arguments with the given type
        Command& addParam(const char* name, ParameterType type, bool optional = false);

        // Adds a new parameters that accepts any of the specified types
        Command& addParam(const char* name, std::initializer_list<ParameterType> types, bool optional = false);

        // Adds a variable amount of parameters that must have any of the given types
        // Note: MUST be the last parameter added to this command
        Command& addVariadicParams(std::initializer_list<ParameterType> types);

        // Specifies the function that's executed with the parsed parameters if match the type and count specified
        Command& setFunction(const CommandFunction& func);

    private:
        CommandFunction func;
        std::vector<internal::ParameterData> parameters;
        std::string name;
        std::string description;
        befriend(ConsoleData, ConsoleParameterParser, ConsoleHandler)
    };


} // namespace magique

#endif //MAGIQUE_COMMANDLINE_H