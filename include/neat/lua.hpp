// TODO add  luaN_registerfunction()

/**
 @file      neat/lua.hpp
 @author    Pim Pieters
 @brief     Collection of helper functions for Lua in C++. Compatible with Lua 5.4.
 @copyright Copyright (c) 2025

 IMPORTANT: Add the following line to one C++ source file before the include to create the implementation:
    #define NEAT_LUA_IMPLEMENTATION

 e.g.:
    // source.cpp
    ...
    ...
    #define NEAT_LUA_IMPLEMENTATION
    #include <neat/lua.hpp

 This library does not include the Lua header, meaning that it has to be imported as well when using neat::lua:
 e.g.
    #include <lua/lua.hpp>
    #include <neat/lua.hpp>

 Alternatively, the header can be included by defining NEAT_LUA_PATH.
 e.g.
    #define NEAT_LUA_PATH <lua/lua.hpp>
    #include <neat/lua.hpp>

 Adds templated versions of common C Lua functions to make use easier. The following templates are supported:
 - int
 - float
 - double
 - bool
 - const char*
 - void*
 - void (only for luaN_poptop nad luaN_call)

 Throughout this documentation the template name T will be used to refer to any of the supported templates as listed
 above. Any other types should be rejected by the compiler. Pointers will always be interpreted as lightuserdata when
 applicable.

 If during the execution of these functions an error occurs (e.g. because the stack was empty when calling luaN_poptop),
 an std::runtime_error will be thrown with an appropriate message. In case these errors are to be expected, these should
 be handled manually. Normal Lua errors, such as calling lua_gettable with an incorrect stack configuration, will still
 result in the expected Lua errors.

 The names of global values and global functions used support nesting by using dots. For example, searching for the
 global "Foo.Bar" will search for the global table Foo and then return its Baz field. These tables can be nested
 themselves, and searches like "Foo.Bar.Baz.Quux" are also valid.

 The following functions are defined, where T represents a template value as defined above:
    // Global functions
    - void luaN_setglobal(lua_State* L, T value, const char* name)
    - T    luaN_getglobal(lua_State* L, const char* name)
    - void luaN_pushglobal(lua_State* L, const char* name)
    - T    luaN_call(lua_State* L, const char* name, Args... args)

    // Stack functions
    - T      luaN_to(lua_State* L, int index)
    - bool   luaN_is(lua_State* L, int index)
    - void   luaN_push(lua_State* L, T value)
    - T      luaN_poptop(lua_State* L)
    - size_t luaN_pushmany(lua_State* L, T... values)

    // Array functions
    - void luaN_toarray(lua_State* L, int index, T* values, size_t count)
    - void luaN_pusharray(lua_State* L, T* values, size_t count)
    - void luaN_poptoparray(lua_State* L, T* values, size_t count)

 */

#ifndef NEAT_LUA_HPP_
#define NEAT_LUA_HPP_

#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#ifdef NEAT_LUA_PATH
#include NEAT_LUA_PATH
#endif  // NEAT_LUA_PATH

namespace neat {

// [-0, +0, e] Set a global value. Supports nested names.
template <typename T> void luaN_setglobal(lua_State* L, T value, const char* name);

// [-0, +0, e] Get a global value. Supports nested names.
template <typename T> T luaN_getglobal(lua_State* L, const char* name);

// [-0, +1, e] Push a global onto the top of the stack. Supports nested names. Pushes nil if the global could not be found.
inline void luaN_pushglobal(lua_State* L, const char* name);

// [-0, +0, e] Call a Lua function with arguments and returns the result (or returns void). The arguments are in
// the same order as the lua function. Supports nested names.
template <typename T, typename... Args> T luaN_call(lua_State* L, const char* name, Args const&... args);

// [-0, +0, e] Register a C function. Equivalent to lua_registerfunction, except it supports nested objects.
inline void luaN_registerfunction(lua_State* L, const char* name, lua_CFunction function);

// [-0, +0, -] Get a value in the stack. Equivalent to lua_toXXX (e.g. luaN_to<const char*> is equivalent to luaN_tostring).
template <typename T> T luaN_to(lua_State* L, int index);

// [-0, +0, -] Get an array of values from the stack. Destination pointer and count have to be provided manually. Uses luaN_to internally.
template <typename T> void luaN_toarray(lua_State* L, int index, T* destination, size_t count);

// [-0, +0, -] Check if a value at an index is a type. Equivalent to lua_isXXX (e.g. luaN_is<const char*> is equivalent to lua_isstring).
template <typename T> bool luaN_is(lua_State* L, int index);

// [-0, +1, -] Push a value onto the stack. Equivalent to lua_pushXXX (e.g. luaN_push<const char*> is equivalent to lua_pushstring).
template <typename T> void luaN_push(lua_State* L, T value);

// [-0, +N, -] Push multiple values onto the stack and returns the amount of values pushed. Uses luaN_push internally.
template <typename... T> size_t luaN_pushmany(lua_State* L, T... values);

// [-1, +0, e] Pop the top value of the stack and returns its value, or void if the value is to be discarded.
template <typename T> T luaN_poptop(lua_State* L);

// [-0, +1, e] Push an array of values into a new table onto the stack.
template <typename T> void luaN_pusharray(lua_State* L, T* values, size_t count);

// [-1, +0, e] Pop an array of values from the stack.
template <typename T> void luaN_poptoparray(lua_State* L, T* destination, size_t count);

// Split a nested string into a vector of table names and the name of the variable.
inline std::tuple<std::vector<std::string>, std::string> __luaN_splitnestedname(const char* str);

// [-0, +1, e] Create or retrieve a global nested table and pushes it into the top of the stack. On failure, nil is pushed.
inline void __luaN_pushglobaltable(const char* fullname, lua_State* L, const std::vector<std::string>& names, bool create_if_not_exist);

#pragma region Template implementation

template <typename T>
void luaN_setglobal(lua_State* L, T value, const char* name) {
    auto [tables, variable] = __luaN_splitnestedname(name);
    if (tables.empty()) {
        luaN_push(L, value);
        lua_setglobal(L, name);
    } else {
        __luaN_pushglobaltable(name, L, tables, true);  // +1, set global table
        if (lua_isnil(L, -1)) {
            std::stringstream stream;
            stream << "Could not set global '" << name << "'. Could not create table!";
            throw std::runtime_error(stream.str());
            lua_pop(L, 1);
        } else {
            luaN_push(L, value);                    // +1, push value
            lua_setfield(L, -2, variable.c_str());  // -1, set global field
            lua_pop(L, 1);                          // -1, pop the global table
        }
    }
}

template <typename T> T luaN_getglobal(lua_State* L, const char* name) {
    luaN_pushglobal(L, name);
    return luaN_poptop<T>(L);
}

/* Specific implementation for luaN_call with no return*/
template <typename... Args> void luaN_call(lua_State* L, const char* name, Args const&... args) {
    luaN_pushglobal(L, name);
    int arg_count = luaN_pushmany(L, args...);
    lua_call(L, arg_count, 0);
    return;
}

template <typename T, typename... Args> T luaN_call(lua_State* L, const char* name, Args const&... args) {
    luaN_pushglobal(L, name);
    int arg_count = luaN_pushmany(L, args...);
    lua_call(L, arg_count, 1);
    return luaN_poptop<T>(L);
}

template <typename T> void luaN_toarray(lua_State* L, int index, T* destination, size_t count) {
    for (int i = 0; i < (int)count; i++) {
        lua_pushinteger(L, i + 1);  // +1, push index
        if (index < 0) {
            lua_gettable(L, index - 1);  // +0, note: -1 because we already used negative indices, and a value was pushed
        } else {
            lua_gettable(L, index);  // +0
        }
        destination[i] = luaN_poptop<T>(L);
    }
}

template <typename T> void luaN_pusharray(lua_State* L, T* values, size_t count) {
    lua_newtable(L);
    for (int i = 0; i < (int)count; i++) {
        lua_pushinteger(L, i + 1);   // +1, push key
        luaN_push<T>(L, values[i]);  // +1, push value
        lua_settable(L, -3);         // -2, set the value
    }
}

template <typename T> T luaN_poptop(lua_State* L) {
    T value = luaN_to<T>(L, -1);
    lua_pop(L, 1);
    return value;
}

template <typename T> void luaN_poptoparray(lua_State* L, T* destination, size_t count) {
    luaN_toarray(L, -1, destination, count);
    lua_pop(L, 1);
}

template <typename... T> size_t luaN_pushmany(lua_State* L, T... values) {
    size_t count = 0;
    ([&] {
        luaN_push(L, values);
        count++;
    }(),
     ...);
    return count;
}

#pragma endregion

}  // namespace neat

#endif  // NEAT_LUA_HPP_

#ifdef NEAT_LUA_IMPLEMENTATION

#pragma region Implementations

// neat::luaN_to implementations

template <> inline int neat::luaN_to(lua_State* L, int index) {
    return lua_tointeger(L, index);
}

template <> inline float neat::luaN_to(lua_State* L, int index) {
    return static_cast<float>(lua_tonumber(L, index));
}

template <> inline double neat::luaN_to(lua_State* L, int index) {
    return lua_tonumber(L, index);
}

template <> inline bool neat::luaN_to(lua_State* L, int index) {
    return static_cast<bool>(lua_toboolean(L, index));
}

template <> inline const char* neat::luaN_to(lua_State* L, int index) {
    return lua_tostring(L, index);
}

template <> inline void* neat::luaN_to(lua_State* L, int index) {
    return lua_touserdata(L, index);
}

// neat::luaN_is implementations

template <> inline bool neat::luaN_is<int>(lua_State* L, int index) {
    return static_cast<bool>(lua_isinteger(L, index));
}

template <> inline bool neat::luaN_is<float>(lua_State* L, int index) {
    return static_cast<bool>(lua_isnumber(L, index));
}

template <> inline bool neat::luaN_is<double>(lua_State* L, int index) {
    return static_cast<bool>(lua_isnumber(L, index));
}

template <> inline bool neat::luaN_is<bool>(lua_State* L, int index) {
    return static_cast<bool>(lua_isboolean(L, index));
}

template <> inline bool neat::luaN_is<const char*>(lua_State* L, int index) {
    return static_cast<bool>(lua_isstring(L, index));
}

template <> inline bool neat::luaN_is<void*>(lua_State* L, int index) {
    return static_cast<bool>(lua_islightuserdata(L, index));
}

// neat::luaN_push implementations

template <> inline void neat::luaN_push(lua_State* L, int value) {
    lua_pushinteger(L, value);
}

template <> inline void neat::luaN_push(lua_State* L, float value) {
    lua_pushnumber(L, static_cast<double>(value));
}

template <> inline void neat::luaN_push(lua_State* L, double value) {
    lua_pushnumber(L, value);
}

template <> inline void neat::luaN_push(lua_State* L, bool value) {
    lua_pushboolean(L, static_cast<int>(value));
}

template <> inline void neat::luaN_push(lua_State* L, const char* value) {
    lua_pushstring(L, value);
}

template <> inline void neat::luaN_push(lua_State* L, void* value) {
    lua_pushlightuserdata(L, value);
}

// neat::luaN_poptop implementations

// Special case for void, which just pops the top element

template <> inline void neat::luaN_poptop(lua_State* L) {
    lua_pop(L, 1);
}

// neat::luaN_pushglobal implementation

inline void neat::luaN_pushglobal(lua_State* L, const char* name) {
    auto [tables, global] = __luaN_splitnestedname(name);
    if (tables.empty()) {
        lua_getglobal(L, global.c_str());
    } else {
        __luaN_pushglobaltable(name, L, tables, false);  // +1, push global table
        if (lua_isnil(L, -1)) {
            std::stringstream stream;
            stream << "Could not push global '" << name << "'. Could not access nested table!";
            // nil is already pushed
        } else {
            lua_getfield(L, -1, global.c_str());  // +1, push the field from the global
            lua_remove(L, -2);                    // -1, pop the global table from the stack
        }
    }
}

// utility function implementations
inline std::tuple<std::vector<std::string>, std::string> neat::__luaN_splitnestedname(const char* str) {
    const char               delimiter = '.';
    size_t                   start     = 0;
    size_t                   end;
    std::string              token;
    std::vector<std::string> tables;
    std::string              string(str);

    while ((end = string.find(delimiter, start)) != std::string::npos) {
        token = string.substr(start, end - start);
        start = end + 1;
        tables.push_back(token);
    }

    std::string variable = string.substr(start);
    return std::make_tuple(tables, variable);
}

inline void neat::__luaN_pushglobaltable(const char* fullname, lua_State* L, const std::vector<std::string>& names, bool create_if_not_exist) {
    // Note: on failure, nil is pushed, even if an error is thrown.

    if (names.empty()) {
        lua_pushnil(L);  // +1, push nil to signify error
        std::stringstream stream;
        stream << "Attempting to push a global table for '" << fullname << "'. List of names is empty!";
        throw std::runtime_error(stream.str());
    }

    for (const auto& name : names) {
        if (name.empty()) {
            lua_pushnil(L);  // +1, push nil to signify error
            std::stringstream stream;
            stream << "Attempting to push a global table for '" << fullname << "'. One of the names is empty! Did you accidentally write '..' in the name?";
            throw std::runtime_error(stream.str());
        }
    }

    // Create the global table
    lua_getglobal(L, names[0].c_str());  // +1, get the first global table
    if (lua_isnil(L, -1)) {
        if (create_if_not_exist) {
            lua_pop(L, 1);                       // -1, pop the nil value
            lua_newtable(L);                     // +1, create the table
            lua_setglobal(L, names[0].c_str());  // -1, make the table global
            lua_getglobal(L, names[0].c_str());  // +1, retrieve the global table again, which should exist at this point
        } else {
            std::stringstream stream;
            stream << "Attempting to push a global table for '" << fullname << "'. Global table '" << names[0] << "' does not exist!";
            throw std::runtime_error(stream.str());
        }
    } else if (!lua_istable(L, -1)) {
        lua_pop(L, 1);   // -1, remove the incorrectly typed global from the stack
        lua_pushnil(L);  // +1, push nil to signify error
        std::stringstream stream;
        stream << "Attempting to push a global table for '" << fullname << "'. Global variable '" << names[0] << "' already exists, but is not a table!";
        throw std::runtime_error(stream.str());
    }

    // Create the nested table
    for (size_t i = 1; i < names.size(); i++) {
        lua_getfield(L, -1, names[i].c_str());  // +1, push field of table
        if (lua_isnil(L, -1)) {
            if (create_if_not_exist) {
                lua_pop(L, 1);                          // -1, pop the nil value
                lua_newtable(L);                        // +1, create the table
                lua_setfield(L, -2, names[i].c_str());  // -1, set the value of the parent table
                lua_getfield(L, -1, names[i].c_str());  // +1, push the subtable to the top of the stack, which should exist at this point
            } else {
                lua_pop(L, 1);   // -1, pop the nil value
                lua_pop(L, 1);   // -1, pop the parent table
                lua_pushnil(L);  // +1, push nil to signify error
                std::stringstream stream;
                stream << "Attempting to push a global table for '" << fullname << "'. Subtable '" << names[i] << "' at index " << i << " does not exist!";
                throw std::runtime_error(stream.str());
            }
        } else if (!lua_istable(L, -1)) {
            lua_pop(L, 1);   // -1, pop the incorrectly typed table from the stack
            lua_pop(L, 1);   // -1, pop the parent table from the stack
            lua_pushnil(L);  // +1, push nil to signify error
            std::stringstream stream;
            stream << "Attempting to push a global table for '" << fullname << "'. Subtable '" << names[i] << "' at index " << i << " already exists, but is not a table!";
            throw std::runtime_error(stream.str());
        }
        lua_remove(L, -2);  // -1, pop the parent table from the stack
    }
}

#pragma endregionImplementations

#endif