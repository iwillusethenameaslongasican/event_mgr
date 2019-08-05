#pragma once

#include <functional>
#include <utility>
#include <type_traits>
#include <lua.hpp>

template <typename T> void lua_push_object(lua_State* L, T obj);
template <typename T> T lua_to_object(lua_State* L, int idx);

template <typename T>
T lua_to_native(lua_State* L, int i) { return lua_to_object<T>(L, i); }

template <> inline bool lua_to_native<bool>(lua_State* L, int i) { return lua_toboolean(L, i) != 0; }
template <> inline char lua_to_native<char>(lua_State* L, int i) { return (char)lua_tointeger(L, i); }
template <> inline unsigned char lua_to_native<unsigned char>(lua_State* L, int i) { return (unsigned char)lua_tointeger(L, i); }
template <> inline short lua_to_native<short>(lua_State* L, int i) { return (short)lua_tointeger(L, i); }
template <> inline unsigned short lua_to_native<unsigned short>(lua_State* L, int i) { return (unsigned short)lua_tointeger(L, i); }
template <> inline int lua_to_native<int>(lua_State* L, int i) { return (int)lua_tointeger(L, i); }
template <> inline unsigned int lua_to_native<unsigned int>(lua_State* L, int i) { return (unsigned int)lua_tointeger(L, i); }
template <> inline long lua_to_native<long>(lua_State* L, int i) { return (long)lua_tointeger(L, i); }
template <> inline unsigned long lua_to_native<unsigned long>(lua_State* L, int i) { return (unsigned long)lua_tointeger(L, i); }
template <> inline long long lua_to_native<long long>(lua_State* L, int i) { return lua_tointeger(L, i); }
template <> inline unsigned long long lua_to_native<unsigned long long>(lua_State* L, int i) { return (unsigned long long)lua_tointeger(L, i); }
template <> inline float lua_to_native<float>(lua_State* L, int i) { return (float)lua_tonumber(L, i); }
template <> inline double lua_to_native<double>(lua_State* L, int i) { return lua_tonumber(L, i); }
template <> inline const char* lua_to_native<const char*>(lua_State* L, int i) { return lua_tostring(L, i); }
template <> inline std::string lua_to_native<std::string>(lua_State* L, int i)
{
    const char* str = lua_tostring(L, i);
    return str == nullptr ? "" : str;
}

template <typename T>
void native_to_lua(lua_State* L, T* v) { lua_push_object(L, v); }
inline void native_to_lua(lua_State* L, bool v) { lua_pushboolean(L, v); }
inline void native_to_lua(lua_State* L, char v) { lua_pushinteger(L, v); }
inline void native_to_lua(lua_State* L, unsigned char v) { lua_pushinteger(L, v); }
inline void native_to_lua(lua_State* L, short v) { lua_pushinteger(L, v); }
inline void native_to_lua(lua_State* L, unsigned short v) { lua_pushinteger(L, v); }
inline void native_to_lua(lua_State* L, int v) { lua_pushinteger(L, v); }
inline void native_to_lua(lua_State* L, unsigned int v) { lua_pushinteger(L, v); }
inline void native_to_lua(lua_State* L, long v) { lua_pushinteger(L, v); }
inline void native_to_lua(lua_State* L, unsigned long v) { lua_pushinteger(L, v); }
inline void native_to_lua(lua_State* L, long long v) { lua_pushinteger(L, (lua_Integer)v); }
inline void native_to_lua(lua_State* L, unsigned long long v) { lua_pushinteger(L, (lua_Integer)v); }
inline void native_to_lua(lua_State* L, float v) { lua_pushnumber(L, v); }
inline void native_to_lua(lua_State* L, double v) { lua_pushnumber(L, v); }
inline void native_to_lua(lua_State* L, const char* v) { lua_pushstring(L, v); }
inline void native_to_lua(lua_State* L, char* v) { lua_pushstring(L, v); }
inline void native_to_lua(lua_State* L, const std::string& v) { lua_pushstring(L, v.c_str()); }

using lua_object_function = std::function<int(void *, lua_State *)>;

template <size_t... integers, typename return_type, typename T, typename... arg_types>
return_type call_helper(lua_State* L, T* obj, return_type(T::*func)(arg_types...), std::index_sequence<integers...>&&)
{
    return (obj->*func)(lua_to_native<arg_types>(L, integers + 1)...);
}

template <size_t... integers, typename return_type, typename T, typename... arg_types>
return_type call_helper(lua_State* L, T* obj, return_type(T::*func)(arg_types...) const, std::index_sequence<integers...>&&)
{
    return (obj->*func)(lua_to_native<arg_types>(L, integers + 1)...);
}

template <typename return_type, typename T, typename... arg_types>
lua_object_function lua_adapter(return_type(T::*func)(arg_types...))
{
    return [=](void* obj, lua_State* L)
    {
        native_to_lua(L, call_helper(L, (T*)obj, func, std::make_index_sequence<sizeof...(arg_types)>()));
        return 1;
    }
}

template <typename return_type, typename T, typename... arg_types>
lua_object_function lua_adapter(return_type(T::*func)(arg_types...) const)
{
    return [=](void* obj, lua_State* L)
    {
        native_to_lua(L, call_helper(L, (T*)obj, func, std::make_index_sequence<sizeof...(arg_types)>()));
        return 1;
    }
}

template <typename T, typename... arg_types>
lua_object_function lua_adapter(void(T::*func)(arg_types...))
{
    return [=](void* obj, lua_State* L)
    {
        call_helper(L, (T*)obj, func, std::make_index_sequence<sizeof...(arg_types)>());
        return 0;
    }
}

template <typename T, typename... arg_types>
lua_object_function lua_adapter(void(T::*func)(arg_types...) const)
{
    return [=](void* obj, lua_State* L)
    {
        call_helper(L, (T*)obj, func, std::make_index_sequence<sizeof...(arg_types)>());
        return 0;
    }
}

template <typename T>
lua_object_function lua_adapter(int(T::func*)(lua_State* L))
{
    return [=](void* obj, lua_State* L)
    {
        T* this_obj = (T*)obj;
        return (this_obj->*func)(L);
    }
}

template <typename T>
lua_object_function lua_adapter(int(T::func*)(lua_State* L) const)
{
    return [=](void* obj, lua_State* L)
    {
        T* this_obj = (T*)obj;
        return (this_obj->*func)(L);
    }
}

enum class lua_member_type {
  member_none,
  member_char,
  member_short,
  member_int,
  member_int64,
  member_time,
  member_bool,
  member_float,
  member_double,
  member_string,
  member_std_str,
  member_function
};

struct lua_member_item {
  const char *name;
  lua_member_type type;
  int offset;
  size_t size;
  bool readonly;
  lua_object_function func;
};

template <typename T>
void lua_push_object(lua_State* L, T obj)
{
    // 禁止将对象的父类指针push到lua栈中，以避免在使用lua_push_object时发生指针转换(通过将类定义成final来避免)
    // 使用了萃取来移除类的引用，常量等属性
    static_assert(std::is_final<typename std::remove_pointer<T>::type>::value, "T should be declared final!!!");

    if (obj == nullptr)
    {
        lua_pushnil(L);
        return;
    }

    // 取出对象放在栈顶
    lua_getfield(L, LUA_REGISTERYINDEX, "__objects__"); 
    if (lua_isnil(L, -1)) // 如果__objects__表不存在
    {
        lua_pop(L, -1);
        lua_newtable(L);

        lua_newtable(L);
        lua_pushstring(L, "v");
        lua_setfield(L, -2, "__mode"); // 设置table.value是weak的
        lua_setmetatable(L, -2);

        lua_pushvalue(L, -1);
        lua_setfield(L, LUA_REGISTERYINDEX, "__objects__");
    }

    // stack: __objects__
    if(lua_rawgetp(L, -1, obj) != LUA_TTABLE) // 如果对象在__objects__表中不存在
    {
        lua_pop(L, 1);

        lua_newtable(L);
        lua_pushstring(L, "__pointer__");
        lua_pushlightuserdata(L, obj);
        lua_rawset(L, -3);

        // stack: __objects__, table
        const char* meta_name = obj->lua_get_meta_data();
        luaL_getmetatable(L, meta_name);
        if(lua_isnil(L, -1))
        {
            lua_remove(L, -1);
            lua_register_class(L, obj);
            luaL_getmetatable(L, meta_name);
        }

        lua_setmetatable(L. -2);

        // stack: __objects__, table
        lua_pushvalue(L, -1);
        lua_rawsetp(L, -3, obj);
    }
    lua_remove(L, -2);
}

#define DECLARE_LUA_CLASS(classname)                                           \
  const char *lua_get_meta_name() { return "_class_meta:" #classname; }        \
  lua_member_item *lua_get_meta_data();

#define DECLARE_CLASS_BEGIN(classname)                                         \
  lua_member_item *classname::lua_get_meta_data() {                            \
    typedef classname class_type;                                              \
    static lua_member_item lua_member_list[] = {

#define DECLARE_CLASS_END()                                                    \
    { nullptr, lua_member_type::member_none, 0, 0, false, nullptr }            \
  };                                                                           \
  return lua_member_list;                                                      \
}

#define EXPORT_LUA_CHAR(Member) {#Member, lua_member_type::member_char, offsetof(class_type, Member), sizeof(class_type::Member), false, nullptr},
#define EXPORT_LUA_SHORT(Member) {#Member, lua_member_type::member_short, offsetof(class_type, Member), sizeof(class_type::Member), false, nullptr},
#define EXPORT_LUA_INT(Member) {#Member, lua_member_type::member_int, offsetof(class_type, Member), sizeof(class_type::Member), false, nullptr},
#define EXPORT_LUA_INT64(Member) {#Member, lua_member_type::member_int64, offsetof(class_type, Member), sizeof(class_type::Member), false, nullptr},
#define EXPORT_LUA_TIME(Member) {#Member, lua_member_type::member_time, offsetof(class_type, Member), sizeof(class_type::Member), false, nullptr},
#define EXPORT_LUA_BOOL(Member) {#Member, lua_member_type::member_bool, offsetof(class_type, Member), sizeof(class_type::Member), false, nullptr},
#define EXPORT_LUA_FLOAT(Member) {#Member, lua_member_type::member_float, offsetof(class_type, Member), sizeof(class_type::Member), false, nullptr},
#define EXPORT_LUA_DOUBLE(Member) {#Member, lua_member_type::member_double, offsetof(class_type, Member), sizeof(class_type::Member), false, nullptr},
#define EXPORT_LUA_STRING(Member) {#Member, lua_member_type::member_string, offsetof(class_type, Member), sizeof(class_type::Member), false, nullptr},
#define EXPORT_LUA_STD_STR(Member) {#Member, lua_member_type::member_std_str, offsetof(class_type, Member), sizeof(class_type::Member), false, nullptr},
#define EXPORT_LUA_FUNCTION(Member) {#Member, lua_member_type::member_function, 0, 0, false, lua_adapter(&class_type::Member)},

