#ifndef MAGIQUE_COMMANDLINE_H
#define MAGIQUE_COMMANDLINE_H

#include <functional>
#include <string>

//-----------------------------------------------
// CommandLine Module
//-----------------------------------------------
// .....................................................................
// This module allow to register dynamic command that can be executed in game.
// Pressing the open key (default: KEY_GRAVE) opens the command line.
// Then it works like any other command line with suggestions and autocomplete
// Is automatically drawn on the lower half of the screen and input is handled
// ENTER        -> submits the current input text
// TAB          -> autocompletes if there is only 1 suggestion left
// ARROW_UP     -> sets the previous submitted text as current text
// ARROW_DOWN   -> sets the next submitted text as current text
// .....................................................................

namespace magique
{
    // Registers a custom command with the given name and description
    // Will replace the existing command with the same name with a warning (if any)
    void RegisterCommand(const std::string& name, const std::string& description, const std::function<void()>& func);

    // Removes the command
    void UnRegisterCommand(const std::string& name);

    //----------------- CONTROL -----------------//

    // Sets the key which opens the command line
    // Default: KEY_GRAVE (top left next to key 1) / same as many popular games like skyrim
    void SetCommandLineKey(int key);

    // Sets the maximum length of the command history
    // Default: 10
    void SetMaxCommandHistory(int len);

} // namespace magique

#endif //MAGIQUE_COMMANDLINE_H