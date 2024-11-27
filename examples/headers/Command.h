#ifndef MAGIQUE_COMMAND_EXAMPLE_H
#define MAGIQUE_COMMAND_EXAMPLE_H

#define MAGIQUE_EXAMPLE

#include <raylib/raylib.h>
#include <magique/core/Game.h>
#include <magique/gamedev/Console.h>

using namespace magique;

struct Example final : Game
{
    void onStartup(AssetLoader& loader) override
    {
        // printName Command
        Command printHello{"printHello"};
        printHello.addParam("name", {ParameterType::STRING})
            .setFunction([](const std::vector<Parameter>& params)
                         { AddConsoleStringF("Hello %s!", params.front().getString()); });

        // likes Command
        Command likes{"likes"};
        likes.addParam("person1", {ParameterType::STRING})
            .addParam("person2", {ParameterType::STRING})
            .addParam("amount", {ParameterType::NUMBER})
            .setFunction(
                [](const std::vector<Parameter>& params)
                {
                    if (params.size() > 2) // Last parameter is present
                    {
                        AddConsoleStringF("%s likes %s %d many times!", params[0].getString(), params[1].getString(),
                                          params[2].getInt());
                    }
                    else // Not present
                    {
                        AddConsoleStringF("%s likes %s; but we don't know how much :(", params[0].getString(),
                                          params[1].getString());
                    }
                });
        Command greet{"greet", "Greets a user with a friendly message"};
        greet.addParam("name", {ParameterType::STRING})
            .addParam("timeOfDay", {ParameterType::STRING})
            .setFunction([](const std::vector<Parameter>& params)
                         { AddConsoleStringF("Good %s, %s!", params[1].getString(), params[0].getString()); });

        // addNumbers Command (Normal + Optional Parameters)
        Command addNumbers{"addNumbers", "Adds two numbers, with the second number being optional"};
        addNumbers.addParam("num1", {ParameterType::NUMBER})
            .addOptionalNumber("num2", 0.0f)
            .setFunction(
                [](const std::vector<Parameter>& params)
                {
                    float sum = params[0].getFloat() + (params.size() > 1 ? params[1].getFloat() : 0.0f);
                    AddConsoleStringF("The sum is: %.2f", sum);
                });

        // logMessages Command (Normal + Variadic Parameters)
        Command logMessages{"logMessages", "Logs a series of messages"};
        logMessages.addParam("prefix", {ParameterType::STRING})
            .addVariadicParam({ParameterType::STRING})
            .setFunction(
                [](const std::vector<Parameter>& params)
                {
                    AddConsoleStringF("Messages prefixed by '%s':", params[0].getString());
                    for (size_t i = 1; i < params.size(); ++i)
                    {
                        AddConsoleStringF("- %s", params[i].getString());
                    }
                });

        // setDefaults Command (Only Optional Parameters)
        Command setDefaults{"setDefaults", "Sets default configuration values"};
        setDefaults.addOptionalString("configName", "default")
            .addOptionalNumber("value", 42.0f)
            .addOptionalBool("isEnabled", true)
            .setFunction(
                [](const std::vector<Parameter>& params)
                {
                    AddConsoleStringF("Setting config '%s' to value %.2f, enabled: %s", params[0].getString(),
                                      params[1].getFloat(), params[2].getBool() ? "true" : "false");
                });

        // broadcast Command (Only Variadic Parameters)
        Command broadcast{"broadcast", "Broadcasts messages to all users"};
        broadcast.addParam("what", {ParameterType::STRING})
            .addVariadicParam({ParameterType::STRING})
            .setFunction(
                [](const std::vector<Parameter>& params)
                {
                    for (const auto& param : params)
                    {
                        AddConsoleStringF("Broadcasting: %s", param.getString());
                    }
                });

        RegisterConsoleCommand(greet);
        RegisterConsoleCommand(addNumbers);
        RegisterConsoleCommand(logMessages);
        RegisterConsoleCommand(setDefaults);
        RegisterConsoleCommand(broadcast);
        RegisterConsoleCommand(printHello);
        RegisterConsoleCommand(likes);
    }

    void updateGame(GameState gameState) override {}

    void drawGame(GameState gameState, Camera2D& camera2D) override
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
        DrawText(helpText, 10, 500, 18, BLACK);
    }
};

#endif // MAGIQUE_COMMAND_EXAMPLE_H