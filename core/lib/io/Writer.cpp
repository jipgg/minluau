#include <userdata_template.hpp>
#include "lib.hpp"
using Writer_ut = Userdata_template<libio::Writer>;
static const char* tname = "ioWriter";
template<> const char* Writer_ut::type_name(){return tname;}

namespace libio {
static const Writer_ut::Registry namecall = {
    {"write", [](lua_State* L, Writer& r) -> int {
        size_t buf_len;
        char* buf = static_cast<char*>(luaL_checkbuffer(L, 2, &buf_len));
        r->write(buf, buf_len);
        return 0;
    }},
    {"print", [](lua_State* L, Writer& r) -> int {
        int top = lua_gettop(L);
        std::string message;
        const std::string separator{", "};
        for (int i{2}; i <= top; ++i) {
            size_t len;
            const char* str = luaL_tolstring(L, i, &len);
            message += std::string_view(str, len);
            message += separator;
        }
        message.resize(message.size() - separator.size());
        (*r) << message << std::endl; // for files too
        return 0;
    }},
    {"put", [](lua_State* L, Writer& r) -> int {
        r->put(luaL_checkinteger(L, 2));
        return 0;
    }},
    {"reset", [](lua_State* L, Writer& r) -> int {
        r->seekp(0, std::ios::beg);
        r->clear();
        return 0;
    }},
    {"try_offset", [](lua_State* L, Writer& w) -> int {
        auto old_pos = w->tellp();
        bool success = true;
        w->seekp(luaL_checkinteger(L, 2), std::ios::beg);
        if (w->fail()) {
            w->seekp(old_pos);
            w->clear();
            success = false;
        }
        lua_pushboolean(L, success);
        return 1;
    }},
};

static const Writer_ut::Registry index = {
    {"eof", [](lua_State* L, Writer& w) -> int {
        lua_pushboolean(L, w->eof());
        return 1;
    }},
    {"offset", [](lua_State* L, Writer& r) -> int {
        lua_pushinteger(L, r->tellp());
        return 1;
    }},
};
static const Writer_ut::Registry newindex = {
    {"offset", [](lua_State* L, Writer& r) -> int {
        auto old_pos = r->tellp();
        r->seekp(luaL_checkinteger(L, 3), std::ios::beg);
        if (r->fail()) {
            r->seekp(old_pos);
            r->clear(std::ios::failbit);
            luaL_errorL(L, "offset out of range.");
        }
        return 0;
    }},
};

Writer& new_writer(lua_State* L, Writer&& v) {
    if (not Writer_ut::initialized(L)) {
        lumin_adduserdatatype(tname);
        Writer_ut::init(L, Writer_ut::init_info{
            .index = index,
            .newindex = newindex,
            .namecall = namecall,
        });
    }
    return Writer_ut::new_udata(L, std::move(v));
}
Writer& check_writer(lua_State* L, int idx) {
    return Writer_ut::check_udata(L, idx);
}
}
