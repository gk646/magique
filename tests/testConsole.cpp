#include <catch_amalgamated.hpp>

#define MAGIQUE_TEST_MODE

#include <magique/gamedev/Console.h>
#include <raylib/raylib.h>

#include "internal/globals/ConsoleData.h"

using namespace magique;

TEST_CASE("fuzzing console")
{
    global::CONSOLE_DATA.init();
    // Buffer to hold fuzzed input data
    std::string fuzzInput(100, '\0');

    // Run the fuzz test for a large number of iterations
    for (int iteration = 0; iteration < 10000; ++iteration)
    {
        // Randomize input length
        int inputLength = GetRandomValue(15, 50);

        // Generate random characters, with a 10% chance of being a space
        for (int i = 0; i < inputLength; ++i)
        {
            if (GetRandomValue(1, 100) <= 25) // 10% chance
            {
                fuzzInput[i] = ' '; // Insert a space
            }
            else
            {
                fuzzInput[i] = static_cast<char>(GetRandomValue(33, 126)); // Printable ASCII excluding space
            }
        }

        // Assign fuzzed input to the console's global data
        global::CONSOLE_DATA.line = fuzzInput;

        // Perform parsing
        auto parseResult = ParamParser::parse(global::CONSOLE_DATA);

        // Check the result (adapt this assertion as per your expected behavior)
        REQUIRE(parseResult == nullptr); // Expect no commands registered by default
    }
}

TEST_CASE("fuzzing with random parameters and additional commands")
{
    Command printHello{"printHello"};
    printHello.addParam("name", {ParameterType::STRING});
    printHello.setFunction([](const std::vector<Parameter>& params)
                           { AddConsoleStringF("Hello %s!", params.front().getString()); });

    Command likes{"likes"};
    likes.addParam("person1", {ParameterType::STRING});
    likes.addParam("person2", {ParameterType::STRING});
    likes.addParam("amount", {ParameterType::NUMBER});
    likes.setFunction(
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
    greet.addParam("name", {ParameterType::STRING});
    greet.addParam("timeOfDay", {ParameterType::STRING});
    greet.setFunction([](const std::vector<Parameter>& params)
                      { AddConsoleStringF("Good %s, %s!", params[1].getString(), params[0].getString()); });

    // addNumbers Command
    Command addNumbers{"addNumbers", "Adds two numbers, with the second number being optional"};
    addNumbers.addParam("num1", {ParameterType::NUMBER})
        .addOptionalNumber("num2", 0.0f)
        .setFunction(
            [](const std::vector<Parameter>& params)
            {
                float sum = params[0].getFloat() + (params.size() > 1 ? params[1].getFloat() : 0.0f);
                AddConsoleStringF("The sum is: %.2f", sum);
            });

    // logMessages Command
    Command logMessages{"logMessages", "Logs a series of messages"};
    logMessages.addParam("prefix", {ParameterType::STRING});
    logMessages.addVariadicParam({ParameterType::STRING});
    logMessages.setFunction(
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
    global::CONSOLE_DATA.init();
    for (const Command& cmd : global::CONSOLE_DATA.commands)
    {
        for (int i = 0; i < 1000; ++i)
        {
            std::string commandLine = cmd.name;
            commandLine.append(" ");
            int paramCount = GetRandomValue(5, 10);
            for (int i = 0; i < paramCount; ++i)
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
            auto parseResult = ParamParser::parse(global::CONSOLE_DATA);
        }
    }
}