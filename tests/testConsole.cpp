#include <catch_amalgamated.hpp>

#include <magique/core/Console.h>
#include <raylib/raylib.h>

#include "internal/globals/ConsoleData.h"

using namespace magique;

TEST_CASE("fuzzing console")
{
    global::ENGINE_DATA.gameInstance = new Game();
    global::CONSOLE_DATA.init();
    // Buffer to hold fuzzed input data
    std::string fuzzInput(100, '\0');

    // Run the fuzz test for a large number of iterations
    for (int iteration = 0; iteration < 100; ++iteration)
    {
        // Randomize input length
        const int inputLength = GetRandomValue(15, 50);

        // Generate random characters, with a 10% chance of being a space
        for (int i = 0; i < inputLength; ++i)
        {
            if (GetRandomValue(1, 100) <= 25)
            {
                fuzzInput[i] = ' ';
            }
            else
            {
                fuzzInput[i] = static_cast<char>(GetRandomValue(33, 126));
            }
        }

        global::CONSOLE_DATA.line = fuzzInput;
        auto parseResult = ParamParser::ParseCommand(StringSplit(global::CONSOLE_DATA.line, ' '));
        REQUIRE(parseResult == nullptr);
    }
}

TEST_CASE("fuzzing with random Params and additional commands")
{
    Command printHello{"printHello"};
    printHello.addParam("name", {ParamType::STRING});
    printHello.setFunction([](const std::vector<Param>& params)
                           { ConsoleAddStringF("Hello %s!", params.front().getString()); });

    Command likes{"likes"};
    likes.addParam("person1", {ParamType::STRING});
    likes.addParam("person2", {ParamType::STRING});
    likes.addParam("amount", {ParamType::NUMBER});
    likes.setFunction(
        [](const std::vector<Param>& params)
        {
            if (params.size() > 2) // Last Param is present
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

    // setDefaults Command (Only Optional Params)
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
    global::CONSOLE_DATA.init();
    for (const Command& cmd : global::CONSOLE_DATA.commands)
    {
        for (int i = 0; i < 50; ++i)
        {
            std::string commandLine = cmd.getName().data();
            commandLine.append(" ");
            int paramCount = GetRandomValue(5, 10);
            for (int b = 0; b < paramCount; ++b)
            {
                int paramLength = GetRandomValue(1, 10);
                for (int j = 0; j < paramLength; ++j)
                {
                    if (GetRandomValue(1, 100) <= 10)
                    {
                        commandLine.append(" ");
                    }
                    else
                    {
                        char randomChar = static_cast<char>(GetRandomValue(33, 126));
                        commandLine.push_back(randomChar);
                    }
                }

                if (i < paramCount - 1)
                {
                    commandLine.append(" ");
                }
            }
            global::CONSOLE_DATA.line = commandLine;
            auto parseResult = ParamParser::ParseCommand(StringSplit(global::CONSOLE_DATA.line, ' '));
        }
    }
}
