#ifndef MAGIQUE_CONSOLE_EXAMPLE_H
#define MAGIQUE_CONSOLE_EXAMPLE_H

#include <magique/magique.hpp>

using namespace magique;

struct Example final : Game
{
    void onStartup(AssetLoader& loader) override
    {
        SetGameState({}); // Set empty gamestate - needs to be set in a real game

        // printName Command
        Command printHello{"printHello"};
        printHello.addParam("name", {ParamType::STRING});
        printHello.setFunction([](const std::vector<Param>& params)
                               { ConsoleAddStringF("Hello %s!", params.front().getString()); });

        // likes Command
        Command likes{"likes"};
        likes.addParam("person1", {ParamType::STRING});
        likes.addParam("person2", {ParamType::STRING});
        likes.addParam("amount", {ParamType::NUMBER});
        likes.setFunction(
            [](const std::vector<Param>& params)
            {
                if (params.size() > 2) // Last parameter is present
                {
                    ConsoleAddStringF("%s likes %s %d many times!", params[0].getString(), params[1].getString(),
                                      params[2].getInt());
                }
                else // Not present
                {
                    ConsoleAddStringF("%s likes %s; but we don't know how much :(", params[0].getString(),
                                      params[1].getString());
                }
            });

        Command greet{"greet", "Greets a user with a friendly message"};
        greet.addParam("name", {ParamType::STRING});
        greet.addParam("timeOfDay", {ParamType::STRING});
        greet.setFunction([](const std::vector<Param>& params)
                          { ConsoleAddStringF("Good %s, %s!", params[1].getString(), params[0].getString()); });

        // addNumbers Command
        Command addNumbers{"addNumbers", "Adds two numbers, with the second number being optional"};
        addNumbers.addParam("num1", {ParamType::NUMBER})
            .addOptionalNumber("num2", 0.0f)
            .setFunction(
                [](const std::vector<Param>& params)
                {
                    float sum = params[0].getFloat() + (params.size() > 1 ? params[1].getFloat() : 0.0f);
                    ConsoleAddStringF("The sum is: %.2f", sum);
                });

        // logMessages Command
        Command logMessages{"logMessages", "Logs a series of messages"};
        logMessages.addParam("prefix", {ParamType::STRING});
        logMessages.addVariadicParam({ParamType::STRING});
        logMessages.setFunction(
            [](const std::vector<Param>& params)
            {
                ConsoleAddStringF("Messages prefixed by '%s':", params[0].getString());
                for (size_t i = 1; i < params.size(); ++i)
                {
                    ConsoleAddStringF("- %s", params[i].getString());
                }
            });

        // setDefaults Command (Only Optional Parameters)
        Command setDefaults{"setDefaults", "Sets default configuration values"};
        setDefaults.addOptionalString("configName", "default")
            .addOptionalNumber("value", 42.0f)
            .addOptionalBool("isEnabled", true)
            .setFunction(
                [](const std::vector<Param>& params)
                {
                    ConsoleAddStringF("Setting config '%s' to value %.2f, enabled: %s", params[0].getString(),
                                      params[1].getFloat(), params[2].getBool() ? "true" : "false");
                });

        // broadcast Command (Only Variadic Parameters)
        Command broadcast{"broadcast", "Broadcasts messages to all users"};
        broadcast.addParam("what", {ParamType::STRING})
            .addVariadicParam({ParamType::STRING})
            .setFunction(
                [](const std::vector<Param>& params)
                {
                    for (const auto& param : params)
                    {
                        ConsoleAddStringF("Broadcasting: %s", param.getString());
                    }
                });

        ConsoleRegisterCommand(greet);
        ConsoleRegisterCommand(addNumbers);
        ConsoleRegisterCommand(logMessages);
        ConsoleRegisterCommand(setDefaults);
        ConsoleRegisterCommand(broadcast);
        ConsoleRegisterCommand(printHello);
        ConsoleRegisterCommand(likes);
    }

    void onUpdateGame(GameState gameState) override {}

    void onDrawGame(GameState gameState, Camera2D& camera2D) override
    {
        const char* helpText = R"(
Open the console with PAGE_UP.

Parameter Types:
BOOL: ( ON | on | OFF | off | true | TRUE | false | FALSE)
NUMBER: list of numbers with a optionally single ".": 123 123.456
STRING: not a BOOL or NUMBER

Default commands:
print (NUMBER | BOOL | STRING) ...
clear

Look at the commands defined in the example and try to invoke them!
	- "greet User Morning"
	- "broadcast
        )";
        DrawTextEx(GetEngineFont(), helpText, {10, 250}, 18, 1, BLACK);
    }
};

#endif // MAGIQUE_CONSOLE_EXAMPLE_H
