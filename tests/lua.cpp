#include <cstring>
#include <lua5.4/lua.hpp>

#define NEAT_LUA_IMPLEMENTATION
#include "neat/lua.hpp"
#include "neat/test.hpp"

using namespace neat;

lua_State* create_lua_test_environment() {
    const char* code = R"#(
        my_add = function(a, b, c)
            return a + b + c;
        end;
        
        -- Has a default argument
        my_multiply = function(a, b, c) 
            c = c or 2;
            return a * b * c; 
        end;

        dot_product_and_add = function(x1, y1, x2, y2, add)
            return x1 * x2 + y1 * y2 + add;
        end;
    )#";

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    if (luaL_dostring(L, code) != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        std::cerr << "Could not execute base Lua code: " << error << std::endl;
        exit(1);
    }

    return L;
}

void test_set_and_get_global(void) {
    lua_State* L     = create_lua_test_environment();
    int        stack = lua_gettop(L);

    luaN_setglobal(L, 333, "my_value");
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack);

    NEAT_TEST_ASSERT_EQ(luaN_getglobal<int>(L, "my_value"), 333);
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack);
}

void test_set_and_get_nested_global(void) {
    lua_State* L     = create_lua_test_environment();
    int        stack = lua_gettop(L);

    luaN_setglobal(L, 123, "Storage.User.Id");
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack);

    NEAT_TEST_ASSERT_EQ(luaN_getglobal<int>(L, "Storage.User.Id"), 123);
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack);

    try {
        // Expect a runtime error here, as there is a typo
        NEAT_TEST_ASSERT_EQ(luaN_getglobal<int>(L, "Storage.Usr.Id"), 123);
    } catch (const std::runtime_error&) {
        NEAT_TEST_ASSERT(true);
    } catch (...) {
        NEAT_TEST_ASSERT(false);
    }
}

void test_push_and_poptop(void) {
    lua_State* L     = create_lua_test_environment();
    int        stack = lua_gettop(L);

    // test stack size
    luaN_push(L, 3);
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack + 1);
    luaN_push(L, 3);
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack + 2);
    luaN_poptop<void>(L);
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack + 1);
    luaN_poptop<void>(L);
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack);

    // test integer
    luaN_push(L, 3);
    NEAT_TEST_ASSERT_EQ(luaN_poptop<int>(L), 3);
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack);

    // test float
    luaN_push(L, 3.0f);
    NEAT_TEST_ASSERT_EQ(luaN_poptop<float>(L), 3.0f);
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack);

    // test double
    luaN_push(L, 3.0);
    NEAT_TEST_ASSERT_EQ(luaN_poptop<double>(L), 3.0);
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack);

    // test boolean
    luaN_push(L, true);
    NEAT_TEST_ASSERT_EQ(luaN_poptop<bool>(L), true);
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack);

    // test boolean
    luaN_push(L, "hello!");
    NEAT_TEST_ASSERT_EQ(std::strcmp(luaN_poptop<const char*>(L), "hello!"), 0);
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack);

    // test void*
    int* ptr = new int;
    luaN_push<void*>(L, ptr);
    NEAT_TEST_ASSERT_EQ(luaN_poptop<void*>(L), (void*)ptr);
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack);
    delete ptr;

    // test void
    luaN_push(L, 3);
    luaN_poptop<void>(L);
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack);
}

void test_call_function(void) {
    lua_State* L     = create_lua_test_environment();
    int        stack = lua_gettop(L);

    int add_result = luaN_call<int, int, double, float>(L, "my_add", 1, 2.0, 3.0f);
    NEAT_TEST_ASSERT_EQ(add_result, 6);
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack);

    int mul_result = luaN_call<int, int, int>(L, "my_multiply", 5, 5);
    NEAT_TEST_ASSERT_EQ(mul_result, 50);
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack);

    int dot_result = luaN_call<int, float, float, double, double, int>(L, "dot_product_and_add", 1.0f, 2.0f, 3.0, 9.0, 100);
    NEAT_TEST_ASSERT_EQ(dot_result, 21 + 100);
    NEAT_TEST_ASSERT_EQ(lua_gettop(L), stack);
}

int main() {
    NEAT_TEST_RUN(test_set_and_get_global);
    NEAT_TEST_RUN(test_set_and_get_nested_global);
    NEAT_TEST_RUN(test_call_function);
    NEAT_TEST_RUN(test_push_and_poptop);

    NEAT_TEST_PRINT_STATS();
}