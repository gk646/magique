#include <catch_amalgamated.hpp>
#include <string>
#include <magique/gamedev/TextFormat.h>

using namespace magique;

TEST_CASE("TextFormattingModule Value Management")
{
    const char* intPlaceholder = "INT_VALUE";
    const char* floatPlaceholder = "FLOAT_VALUE";
    const char* stringPlaceholder = "STRING_VALUE";

    SECTION("Set and Get int values")
    {
        int inputValue = 42;
        FormatSetValue(intPlaceholder, inputValue);

        int* retrievedValue = FormatGetValue<int>(intPlaceholder);
        REQUIRE(retrievedValue != nullptr);
        REQUIRE(*retrievedValue == inputValue);
    }

    SECTION("Set and Get float values")
    {
        float inputValue = 3.14f;
        FormatSetValue(floatPlaceholder, inputValue);

        float* retrievedValue = FormatGetValue<float>(floatPlaceholder);
        REQUIRE(retrievedValue != nullptr);
        REQUIRE(*retrievedValue == inputValue);
    }

    SECTION("Set and Get string values")
    {
        std::string inputValue = "HelloWorld";
        FormatSetValue(stringPlaceholder, inputValue);

        std::string* retrievedValue = FormatGetValue<std::string>(stringPlaceholder);
        REQUIRE(retrievedValue != nullptr);
        REQUIRE(*retrievedValue == inputValue);
    }
}

TEST_CASE("TextFormattingModule Text Formatting")
{
    const char* templateText = "Hello, ${PLAYER_NAME}!";
    const char* placeholder = "PLAYER_NAME";
    std::string value = "Jespar";
    FormatSetValue(placeholder, value);

    const char* formattedText = FormatGetText(templateText);
    REQUIRE(formattedText != nullptr);
    REQUIRE(std::string(formattedText) == "Hello, Jespar!");
}

TEST_CASE("TextFormattingModule Customization")
{
    const char* templateText = "Hello, @{PLAYER_NAME}!";
    const char* placeholder = "PLAYER_NAME";
    std::string value = "Jespar";

    FormatSetPrefix('@');
    FormatSetValue(placeholder, value);

    const char* formattedText = FormatGetText(templateText);
    REQUIRE(formattedText != nullptr);
    REQUIRE(std::string(formattedText) == "Hello, Jespar!");
    FormatSetPrefix('$'); // Set back as other tests rely on it
}

TEST_CASE("Dynamic placeholder value management")
{
    const char* placeholder = "DYNAMIC_VAR";

    SECTION("Set and get int value")
    {
        FormatSetValue(placeholder, 42);
        int* intValue = FormatGetValue<int>(placeholder);
        REQUIRE(intValue != nullptr);
        REQUIRE(*intValue == 42);


        REQUIRE(FormatGetValue<float>(placeholder) == nullptr);
        REQUIRE(FormatGetValue<std::string>(placeholder) == nullptr);
    }

    SECTION("Change type from int to float")
    {
        FormatSetValue(placeholder, 42);
        REQUIRE(*FormatGetValue<int>(placeholder) == 42);


        FormatSetValue(placeholder, 3.14f);
        float* floatValue = FormatGetValue<float>(placeholder);
        REQUIRE(floatValue != nullptr);
        REQUIRE(*floatValue == 3.14f);


        REQUIRE(FormatGetValue<int>(placeholder) == nullptr);


        REQUIRE(FormatGetValue<std::string>(placeholder) == nullptr);
    }

    SECTION("Change type from float to string")
    {
        FormatSetValue(placeholder, 3.14f);
        REQUIRE(*FormatGetValue<float>(placeholder) == 3.14f);


        std::string textValue = "Hello, World!";
        FormatSetValue(placeholder, textValue);

        std::string* retrievedString = FormatGetValue<std::string>(placeholder);
        REQUIRE(retrievedString != nullptr);
        REQUIRE(*retrievedString == textValue);


        REQUIRE(FormatGetValue<float>(placeholder) == nullptr);


        REQUIRE(FormatGetValue<int>(placeholder) == nullptr);
    }

    SECTION("Change type from string to int")
    {
        std::string textValue = "Placeholder";
        FormatSetValue(placeholder, textValue);
        REQUIRE(*FormatGetValue<std::string>(placeholder) == textValue);


        FormatSetValue(placeholder, 99);
        int* intValue = FormatGetValue<int>(placeholder);
        REQUIRE(intValue != nullptr);
        REQUIRE(*intValue == 99);


        REQUIRE(FormatGetValue<std::string>(placeholder) == nullptr);


        REQUIRE(FormatGetValue<float>(placeholder) == nullptr);
    }

    SECTION("Non-existent placeholder returns nullptr")
    {
        REQUIRE(FormatGetValue<int>("NON_EXISTENT") == nullptr);
        REQUIRE(FormatGetValue<float>("NON_EXISTENT") == nullptr);
        REQUIRE(FormatGetValue<std::string>("NON_EXISTENT") == nullptr);
    }
}

TEST_CASE("Multiple placeholders with dynamic types")
{
    const char* intPlaceholder = "INT_VAR";
    const char* floatPlaceholder = "FLOAT_VAR";
    const char* stringPlaceholder = "STRING_VAR";


    FormatSetValue(intPlaceholder, 123);
    REQUIRE(*FormatGetValue<int>(intPlaceholder) == 123);
    REQUIRE(FormatGetValue<float>(intPlaceholder) == nullptr);
    REQUIRE(FormatGetValue<std::string>(intPlaceholder) == nullptr);


    FormatSetValue(floatPlaceholder, 456.78f);
    REQUIRE(*FormatGetValue<float>(floatPlaceholder) == 456.78f);
    REQUIRE(FormatGetValue<int>(floatPlaceholder) == nullptr);
    REQUIRE(FormatGetValue<std::string>(floatPlaceholder) == nullptr);


    std::string textValue = "Catch2 Test";
    FormatSetValue(stringPlaceholder, textValue);
    REQUIRE(*FormatGetValue<std::string>(stringPlaceholder) == textValue);
    REQUIRE(FormatGetValue<int>(stringPlaceholder) == nullptr);
    REQUIRE(FormatGetValue<float>(stringPlaceholder) == nullptr);


    REQUIRE(*FormatGetValue<int>(intPlaceholder) == 123);
    REQUIRE(*FormatGetValue<float>(floatPlaceholder) == 456.78f);
    REQUIRE(*FormatGetValue<std::string>(stringPlaceholder) == textValue);
}