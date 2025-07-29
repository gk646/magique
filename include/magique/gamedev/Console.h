// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_COMMANDLINE_H
#define MAGIQUE_COMMANDLINE_H

#include <functional>
#include <string>
#include <vector>
#include <magique/internal/InternalTypes.h>

//===============================================
// Console Module
//===============================================
// .....................................................................
// This module allows to register custom command that can be executed in game.
// It's drawn on top of the game and consumes all input if opened. It also features autocomplete and sorted suggestions
// Controls:
//      - PAGE_UP:    opens/closes the command window
//      - ENTER:      submits the current input text
//      - TAB:        autocomplete the current selection
//      - ARROW_UP:   insert previous submitted text or move autocomplete selection up (if shown)
//      - ARROW_DOWN: insert next submitted text (or clear if at last position) or move autocomplete selection down (if shown)
//
// Allows to register custom commands. Commands are executed with its name followed by a variable amount of parameters.
// All parameters are separated by at least 1 whitespace:
//     - setLighting on
//     - setPlayerPos 512 1024
// The parameters are parsed and validated automatically. See the ParameterType enum for parsing rules.
// If the number of parameters or their types do not match that of the command, it is not executed!
// For the list of default commands look at the commands section.
//
// Environmental params are parameters (with a type) identified by a string. To get/add/remove them see the Command section.
// They can be referenced and used as input parameters to commands with a prefix (default: $) e.g:
//      - print $GAME_NAME $M_VERSION $$
// If the first char of any argument is the prefix (unless there's two) its interpreted as environment param automatically.
//
// Note: All logged messages are automatically displayed in the console as well (from util/Logging.h)
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
    // Builtin commands:
    //      - help: no args
    //           - Shows a quick help note
    //      - print: [ (STRING | BOOL | NUMBER) ... ]
    //           - Takes any amount of variables of any type and prints each to a new line to the console
    //      - clear: no args
    //           - clears the terminal
    //      - shutdown: no args
    //           - shuts down the game by calling Game::shutDown()
    //      - define: STRING (STRING | BOOL | NUMBER)
    //           - creates/sets an environment param with the given name to the given value (and type)
    //      - undef: STRING
    //           - removes the environment param with the given name
    //   namespaces:
    //      - m: Contains various functions that call magique API functions of the same (similar) name

    // Registers a custom command with the given name and description
    // Will replace the existing command with the same name with a warning (if exists)
    void RegisterConsoleCommand(const Command& command);

    // Returns true if the command with the given name is successfully removed
    bool UnRegisterCommand(const char* name);

    // Returns ture if a command with the given name was found and executed (must have no params)
    bool ExecuteCommand(const char* name);

    // Function is passed the parsed parameters - function is only called if the parsed parameters match the definition
    using CommandFunction = std::function<void(const std::vector<Param>& params)>;

    struct Command final
    {
        // Creates a new command instance with the specified name and optional description
        //      - cmdName: the name of the command - must not contain whitespace - can contain numbers
        explicit Command(const char* cmdName, const char* description = nullptr);

        // Adds a new parameters that accepts any of the specified types
        Command& addParam(const char* name, std::initializer_list<ParamType> types);

        // Adds a new optional parameter - if not specified will have the provided value
        // Note: optional params have to be last, but can be followed by other optional params (exclusive with variadic)
        Command& addOptionalNumber(const char* name, float value);
        Command& addOptionalString(const char* name, const char* value);
        Command& addOptionalBool(const char* name, bool value);

        // Adds a parameter that matches a variable amount of parsed params - parsed params must have any of the given types
        // Note: MUST be the last parameter added to this command (exclusive with optionals)
        Command& addVariadicParam(std::initializer_list<ParamType> types);

        // Specifies the function that's executed with the parsed parameters if match the type and count specified
        Command& setFunction(const CommandFunction& func);

        // Returns the name of the command
        const char* getName() const;

    private:
        CommandFunction cmdFunc;
        std::vector<internal::ParamData> parameters;
        std::string name;
        std::string description;
        befriend(ConsoleData, ParamParser, ConsoleHandler);
        befriend(bool UnRegisterCommand(const char*), void RegisterConsoleCommand(const Command&))
        befriend(bool ExecuteCommand(const char* name))
    };

    //================= Environmental Parameters =================//
    // Builtin environmental parameters:
    //      - GAME_NAME: STRING | name of the game as specified in the Game constructor

    // Sets (or creates) the environment param with the given name to the given value
    // Note: you can dynamically switch the type of the same param - float stands for the NUMBER type
    void SetEnvironmentParam(const char* name, const char* value);
    void SetEnvironmentParam(const char* name, float value);
    void SetEnvironmentParam(const char* name, bool value);

    // Returns true if the environment param with the given name was successfully removed
    bool RemoveEnvironmentParam(const char* name);

    // Returns the environment param with the given name
    // Note: returned pointer might become invalid (don't save it)
    // Failure: returns nullptr
    const Param* GetEnvironmentParam(const char* name);

    // Sets the callback called after any environment param was changed with the changed param
    void SetEnvironmentSetCallback(const std::function<void(const Param& param)>& func);

    // Sets the prefix used to denote a environment param
    // Default: $
    void SetEnvironmentParamPrefix(char prefix = '$');

} // namespace magique

#endif //MAGIQUE_COMMANDLINE_H