#include <catch_amalgamated.hpp>

#include <magique/assets/FileImports.h>
#include <fstream>
#include <magique/util/Logging.h>

TEST_CASE("csv import")
{
    magique::Asset asset{};
    FILE* file = fopen("../../tests/res/csv.csv", "rb");
    if (!file)
    {
        LOG_ERROR("Failed to open file");
    }
    fseek(file, 0, SEEK_END);
    asset.size = ftell(file);
    fseek(file, 0, SEEK_SET);

    asset.data = new char[asset.size];
    fread((void*)asset.data, 1, asset.size, file);
    fclose(file);
    asset.path = "myCSV.csv";

    auto import = magique::FileImportCSV(asset);

    REQUIRE(import.getCell(0, 0) == std::string{"Name"});
    REQUIRE(import.getCell(0, 1) == std::string{"Age"});
    REQUIRE(import.getCell(0, 2) == std::string{"Float"});
    REQUIRE(import.getCell(1, 2) == std::string{"3.123"});
    REQUIRE(import.getCell(1, 1) == std::string{"24"});
    REQUIRE(import.getCell(2, 0) == std::string{"Hannes"});
    REQUIRE(import.getColumns() == 3);
    REQUIRE(import.getRows() == 3);
    REQUIRE(import.getColumn("Name") == 0);
    REQUIRE(import.getColumn("Age") == 1);
    REQUIRE(import.getColumn("Float") == 2);
    REQUIRE(import.hasColumnNames() == true);
}

TEST_CASE("only name row | comma delimiter")
{
    magique::Asset asset{};
    FILE* file = fopen("../../tests/res/csv2.csv", "rb");
    if (!file)
    {
        LOG_ERROR("Failed to open file");
    }
    fseek(file, 0, SEEK_END);
    asset.size = ftell(file);
    fseek(file, 0, SEEK_SET);

    asset.data = new char[asset.size];
    fread((void*)asset.data, 1, asset.size, file);
    fclose(file);
    asset.path = "myCSV.csv";

    auto import = magique::FileImportCSV(asset, ',');

    REQUIRE(import.getCell(0, 0) == std::string{"Name"});
    REQUIRE(import.getCell(0, 1) == std::string{"Age"});
    REQUIRE(import.getColumns() == 3);
    REQUIRE(import.getRows() == 1);
    REQUIRE(import.getColumn("Name") == 0);
    REQUIRE(import.getColumn("Age") == 1);
    REQUIRE(import.getColumn("Float") == 2);
    REQUIRE(import.hasColumnNames() == true);
}

TEST_CASE("trailing delimiter | no names")
{
    magique::Asset asset{};
    FILE* file = fopen("../../tests/res/csv3.csv", "rb");
    if (!file)
    {
        LOG_ERROR("Failed to open file");
    }
    fseek(file, 0, SEEK_END);
    asset.size = ftell(file);
    fseek(file, 0, SEEK_SET);

    asset.data = new char[asset.size];
    fread((void*)asset.data, 1, asset.size, file);
    fclose(file);
    asset.path = "myCSV.csv";

    auto import = magique::FileImportCSV(asset, ';', false);

    REQUIRE(import.getCell(0, 0) == std::string{"Name"});
    REQUIRE(import.getCell(0, 1) == std::string{"Age"});
    REQUIRE(import.getCell(0, 2) == std::string{"Float"});
    REQUIRE(import.getColumns() == 3);
    REQUIRE(import.getRows() == 1);
    REQUIRE(import.hasColumnNames() == false);
}