#include <catch_amalgamated.hpp>
#include <string>
#include <magique/ui/TextFormat.h>

using namespace magique;

TEST_CASE("TextFormattingModule Value Management")
{
    const char* intPlaceholder = "INT_VALUE";
    const char* floatPlaceholder = "FLOAT_VALUE";
    const char* stringPlaceholder = "STRING_VALUE";

    SECTION("Set and Get int values")
    {
        int inputValue = 42;
        SetFormatValue(intPlaceholder, inputValue);

        int* retrievedValue = GetFormatValue<int>(intPlaceholder);
        REQUIRE(retrievedValue != nullptr);
        REQUIRE(*retrievedValue == inputValue);
    }

    SECTION("Set and Get float values")
    {
        float inputValue = 3.14f;
        SetFormatValue(floatPlaceholder, inputValue);

        float* retrievedValue = GetFormatValue<float>(floatPlaceholder);
        REQUIRE(retrievedValue != nullptr);
        REQUIRE(*retrievedValue == inputValue);
    }

    SECTION("Set and Get string values")
    {
        std::string inputValue = "HelloWorld";
        SetFormatValue(stringPlaceholder, inputValue);

        std::string* retrievedValue = GetFormatValue<std::string>(stringPlaceholder);
        REQUIRE(retrievedValue != nullptr);
        REQUIRE(*retrievedValue == inputValue);
    }
}

TEST_CASE("TextFormattingModule Text Formatting")
{
    const char* templateText = "Hello, ${PLAYER_NAME}!";
    const char* placeholder = "PLAYER_NAME";
    std::string value = "Jespar";
    SetFormatValue(placeholder, value);

    const char* formattedText = FormatGetText(templateText);
    REQUIRE(formattedText != nullptr);
    REQUIRE(std::string(formattedText) == "Hello, Jespar!");
}

TEST_CASE("TextFormattingModule Customization")
{
    const char* templateText = "Hello, @{PLAYER_NAME}!";
    const char* placeholder = "PLAYER_NAME";
    std::string value = "Jespar";

    SetFormatPrefix('@');
    SetFormatValue(placeholder, value);

    const char* formattedText = FormatGetText(templateText);
    REQUIRE(formattedText != nullptr);
    REQUIRE(std::string(formattedText) == "Hello, Jespar!");
    SetFormatPrefix('$'); // Set back as other tests rely on it
}

TEST_CASE("Dynamic placeholder value management")
{
    const char* placeholder = "DYNAMIC_VAR";

    SECTION("Set and get int value")
    {
        SetFormatValue(placeholder, 42);
        int* intValue = GetFormatValue<int>(placeholder);
        REQUIRE(intValue != nullptr);
        REQUIRE(*intValue == 42);


        REQUIRE(GetFormatValue<float>(placeholder) == nullptr);
        REQUIRE(GetFormatValue<std::string>(placeholder) == nullptr);
    }

    SECTION("Change type from int to float")
    {
        SetFormatValue(placeholder, 42);
        REQUIRE(*GetFormatValue<int>(placeholder) == 42);


        SetFormatValue(placeholder, 3.14f);
        float* floatValue = GetFormatValue<float>(placeholder);
        REQUIRE(floatValue != nullptr);
        REQUIRE(*floatValue == 3.14f);


        REQUIRE(GetFormatValue<int>(placeholder) == nullptr);


        REQUIRE(GetFormatValue<std::string>(placeholder) == nullptr);
    }

    SECTION("Change type from float to string")
    {
        SetFormatValue(placeholder, 3.14f);
        REQUIRE(*GetFormatValue<float>(placeholder) == 3.14f);


        std::string textValue = "Hello, World!";
        SetFormatValue(placeholder, textValue);

        std::string* retrievedString = GetFormatValue<std::string>(placeholder);
        REQUIRE(retrievedString != nullptr);
        REQUIRE(*retrievedString == textValue);


        REQUIRE(GetFormatValue<float>(placeholder) == nullptr);


        REQUIRE(GetFormatValue<int>(placeholder) == nullptr);
    }

    SECTION("Change type from string to int")
    {
        std::string textValue = "Placeholder";
        SetFormatValue(placeholder, textValue);
        REQUIRE(*GetFormatValue<std::string>(placeholder) == textValue);


        SetFormatValue(placeholder, 99);
        int* intValue = GetFormatValue<int>(placeholder);
        REQUIRE(intValue != nullptr);
        REQUIRE(*intValue == 99);


        REQUIRE(GetFormatValue<std::string>(placeholder) == nullptr);


        REQUIRE(GetFormatValue<float>(placeholder) == nullptr);
    }

    SECTION("Non-existent placeholder returns nullptr")
    {
        REQUIRE(GetFormatValue<int>("NON_EXISTENT") == nullptr);
        REQUIRE(GetFormatValue<float>("NON_EXISTENT") == nullptr);
        REQUIRE(GetFormatValue<std::string>("NON_EXISTENT") == nullptr);
    }
}

TEST_CASE("Multiple placeholders with dynamic types")
{
    const char* intPlaceholder = "INT_VAR";
    const char* floatPlaceholder = "FLOAT_VAR";
    const char* stringPlaceholder = "STRING_VAR";


    SetFormatValue(intPlaceholder, 123);
    REQUIRE(*GetFormatValue<int>(intPlaceholder) == 123);
    REQUIRE(GetFormatValue<float>(intPlaceholder) == nullptr);
    REQUIRE(GetFormatValue<std::string>(intPlaceholder) == nullptr);


    SetFormatValue(floatPlaceholder, 456.78f);
    REQUIRE(*GetFormatValue<float>(floatPlaceholder) == 456.78f);
    REQUIRE(GetFormatValue<int>(floatPlaceholder) == nullptr);
    REQUIRE(GetFormatValue<std::string>(floatPlaceholder) == nullptr);


    std::string textValue = "Catch2 Test";
    SetFormatValue(stringPlaceholder, textValue);
    REQUIRE(*GetFormatValue<std::string>(stringPlaceholder) == textValue);
    REQUIRE(GetFormatValue<int>(stringPlaceholder) == nullptr);
    REQUIRE(GetFormatValue<float>(stringPlaceholder) == nullptr);


    REQUIRE(*GetFormatValue<int>(intPlaceholder) == 123);
    REQUIRE(*GetFormatValue<float>(floatPlaceholder) == 456.78f);
    REQUIRE(*GetFormatValue<std::string>(stringPlaceholder) == textValue);
}