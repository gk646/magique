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
        Command c {"hey"};
        /*
        // printName Command
        Command printHello{"printHello"};
        printHello.addParam("name", ParameterType::STRING, true)
            .setFunction([](const std::vector<Parameter>& params)
                         { AddConsoleStringF("Hello %s!", params.front().string); });

        // likes Command
        Command likes{"likes"};
        likes.addParam("person1", ParameterType::STRING)
            .addParam("person2", ParameterType::STRING)
            .addParam("amount", ParameterType::NUMBER)
            .setFunction(
                [](const std::vector<Parameter>& params)
                {
                    if (params.size() > 2) // Last parameter is present
                    {
                        AddConsoleStringF("%s likes %s %d many times!", params[0].string, params[1].string,
                                          params[2].number);
                    }
                    else // Not present
                    {
                        AddConsoleStringF("%s likes %s; but we don't know how much :(", params[0].string,
                                          params[1].string);
                    }
                });
                */
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
        )";
        DrawText(helpText, 10, 500, 18, BLACK);
    }
};

#endif // MAGIQUE_COMMAND_EXAMPLE_H