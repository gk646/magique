#include <iostream>

#include "lua.hpp"

#include <cxutil/cxio.h>
#include <cxutil/cxtime.h>


int set_position(lua_State *L)
{
    int entity_id = luaL_checkinteger(L, 1);
    double x = luaL_checknumber(L, 2);
    double y = luaL_checknumber(L, 3);

    // Assume you have a way to access your ECS and components
    // registry.set<PositionComponent>(entity_id, {x, y});

    // std::cout << "Entity " << entity_id << " position set to (" << x << ", " << y << ").\n";
    return 0;
}

// Function to register your C++ function to Lua
void register_functions(lua_State *L)
{
    lua_register(L, "set_position", set_position);
}

int main()
{
    lua_State *L = luaL_newstate();
    luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON);


    luaL_openlibs(L);
    register_functions(L);

    // Load and run the Lua script
    if (luaL_dofile(L, "../res/script.luac") != LUA_OK)
    {
        std::cerr << "Error running script: " << lua_tostring(L, -1) << std::endl;
        lua_close(L);
        return -1;
    }

    cxstructs::now();


    for (int frame = 0; frame < 100; ++frame)
    {
        lua_getglobal(L, "update_entities");
        lua_pushinteger(L, frame); // Pass current frame number to Lua
        if (lua_pcall(L, 1, 0, 0) != LUA_OK)
        {
            std::cerr << "Error: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1); // Remove error message from the stack
        }
    }

    cxstructs::printTime<std::chrono::nanoseconds>();

    lua_close(L);
    return 0;
}