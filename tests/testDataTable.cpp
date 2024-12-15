#include <catch_amalgamated.hpp>
#include <magique/persistence/types/DataTable.h>
#include <tuple>

using namespace magique;

TEST_CASE("DataTable multiple types and operations")
{
    DataTable<int, float, std::string> table{"id", "weight", "name"};
    REQUIRE(table.getColumns() == 3);
    REQUIRE(table.getRows() == 0);

    table.addRow({1, 70.5f, std::string("Alice")});
    table.addRow(std::make_tuple(2, 65.0f, std::string("Bob")));
    REQUIRE(table.getRows() == 2);

    REQUIRE(table[0] == std::make_tuple(1, 70.5f, std::string("Alice")));
    REQUIRE(table[1] == std::make_tuple(2, 65.0f, std::string("Bob")));

    REQUIRE(table.getCell<int>(0, "id") == 1);
    REQUIRE(table.getCell<std::string>(1, "name") == "Bob");

    table.set<1>(60.0f, 1);
    REQUIRE(table.getCell<float>(1, "weight") == 60.0f);

    table.setRow(std::make_tuple(3, 80.0f, std::string("Charlie")), 0);
    REQUIRE(table[0] == std::make_tuple(3, 80.0f, "Charlie"));
}

TEST_CASE("DataTable no column names and various assignments")
{
    DataTable<int, double, char> table2;
    REQUIRE(table2.getColumns() == 3);
    REQUIRE(table2.getRows() == 0);

    table2.addRow(std::make_tuple(10, 100.1, 'A'));
    table2.addRow(std::make_tuple(20, 200.2, 'B'));
    REQUIRE(table2.getRows() == 2);
    REQUIRE(table2[0] == std::make_tuple(10, 100.1, 'A'));

    table2.set<0>(15, 0);
    REQUIRE(std::get<0>(table2[0]) == 15);

    table2.setRow(std::make_tuple(25, 250.5, 'Z'), 1);
    REQUIRE(table2[1] == std::make_tuple(25, 250.5, 'Z'));

    table2.addRow(std::make_tuple(30, 300.3, 'C'));
    REQUIRE(table2.getRows() == 3);

    std::vector<std::tuple<int,double,char>> expected {
            {15,100.1,'A'},
            {25,250.5,'Z'},
            {30,300.3,'C'}
    };
    int i=0;
    for (auto it = table2.begin(); it != table2.end(); ++it, ++i)
        REQUIRE(*it == expected[i]);
}


TEST_CASE("DataTable mixed modifications and getCell by index")
{
    DataTable<std::string, int, int> table3{"name", "score", "level"};
    table3.addRow(std::make_tuple("Player1", 100, 1));
    table3.addRow(std::make_tuple("Player2", 200, 2));
    REQUIRE(table3.getRows() == 2);

    REQUIRE(table3.getCell<std::string>(0, 0) == "Player1");
    REQUIRE(table3.getCell<int>(1, 1) == 200);
    REQUIRE(table3.getCell<int>(1, 2) == 2);

    table3.set<1>(300, 1);
    REQUIRE(table3.getCell<int>(1, "score") == 300);

    table3.setRow(std::make_tuple("Hero", 500, 10), 0);
    REQUIRE(table3[0] == std::make_tuple("Hero", 500, 10));
}