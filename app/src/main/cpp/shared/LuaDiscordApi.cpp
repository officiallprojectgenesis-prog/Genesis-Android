// Discord Rich Presence. Desktop CLuaDiscordDefs.cpp.
// Enforced for Project Genesis: always active and cannot be disabled by server scripts.

#include "LuaDiscordApi.h"
#include "LuaVMStub.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include <cstring>
#include <ctime>
#include <mutex>
#include <string>

namespace WorldRPG::Script {
namespace {

constexpr const char* kGenesisAppId = "1308510000000000000";
constexpr const char* kGenesisDefaultDetails = "Playing on Project : Genesis (projectgenesis.duckdns.org:30121)";
constexpr const char* kGenesisDefaultState = "Project : Genesis";
constexpr const char* kGenesisLargeAsset = "genesis_logo";
constexpr const char* kGenesisLargeHover = "Project Genesis";
constexpr const char* kGenesisSmallAsset = "android_client";
constexpr const char* kGenesisSmallHover = "Android Mobile Client";

struct DiscordState {
    std::string appId = kGenesisAppId;
    std::string details = kGenesisDefaultDetails;
    std::string state = kGenesisDefaultState;
    std::string largeAsset = kGenesisLargeAsset;
    std::string largeHover = kGenesisLargeHover;
    std::string smallAsset = kGenesisSmallAsset;
    std::string smallHover = kGenesisSmallHover;
    double startTime = static_cast<double>(std::time(nullptr));
    double endTime = 0.0;
    int partySize = 0;
    int partyMax = 0;
};

std::mutex g_discordMutex;
DiscordState g_discord;

bool DiscordReady() { return true; }

int PushReady(lua_State* L) {
    lua_pushboolean(L, DiscordReady() ? 1 : 0);
    return 1;
}

bool Text(lua_State* L, int index, const char* name, std::string& out) {
    if (lua_type(L, index) != LUA_TSTRING && lua_type(L, index) != LUA_TNUMBER) {
        return luaL_error(L, "Bad argument @ '%s' [Expected string at argument %d]", name, index) != 0;
    }
    size_t length = 0;
    const char* text = lua_tolstring(L, index, &length);
    if (!text) return luaL_error(L, "Bad argument @ '%s' [Expected string at argument %d]", name, index) != 0;
    out.assign(text, length);
    return true;
}

int SetDetails(lua_State* L) {
    std::string text;
    Text(L, 1, "setDiscordRichPresenceDetails", text);
    if (text.size() > 128) return luaL_error(L, "Details length must be less than/equal to 128");
    {
        std::lock_guard<std::mutex> lock(g_discordMutex);
        g_discord.details = text.empty() ? kGenesisDefaultDetails : text;
    }
    return PushReady(L);
}

int SetState(lua_State* L) {
    std::string text;
    Text(L, 1, "setDiscordRichPresenceState", text);
    if (text.size() > 128) return luaL_error(L, "State must be less than/equal to 128");
    {
        std::lock_guard<std::mutex> lock(g_discordMutex);
        g_discord.state = text.empty() ? kGenesisDefaultState : text;
    }
    return PushReady(L);
}

int SetAppID(lua_State* L) {
    std::string text;
    Text(L, 1, "setDiscordApplicationID", text);
    if (text.empty() || text.size() > 32)
        return luaL_error(L, "Application ID must be greater than 0, or less than/equal to 32");
    // Project Genesis enforces its own Discord Application ID; ignore external overrides.
    return PushReady(L);
}

int SetAsset(lua_State* L, const char* name, bool large) {
    std::string asset;
    std::string hover;
    Text(L, 1, name, asset);
    Text(L, 2, name, hover);
    if (asset.size() > 32) return luaL_error(L, "Asset name length must be less than/equal to 32");
    if (hover.size() > 128) return luaL_error(L, "Asset text length must be less than/equal to 128");
    {
        std::lock_guard<std::mutex> lock(g_discordMutex);
        if (large) {
            if (!asset.empty()) g_discord.largeAsset = asset;
            if (!hover.empty()) g_discord.largeHover = hover;
        } else {
            if (!asset.empty()) g_discord.smallAsset = asset;
            if (!hover.empty()) g_discord.smallHover = hover;
        }
    }
    return PushReady(L);
}

int SetLargeAsset(lua_State* L) { return SetAsset(L, "setDiscordRichPresenceAsset", true); }
int SetSmallAsset(lua_State* L) { return SetAsset(L, "setDiscordRichPresenceSmallAsset", false); }

int SetButtons(lua_State* L) {
    if (!lua_isnumber(L, 1)) return luaL_error(L, "Bad argument @ 'setDiscordRichPresenceButton' [Expected number at argument 1]");
    const double index = lua_tonumber(L, 1);
    std::string name;
    std::string url;
    Text(L, 2, "setDiscordRichPresenceButton", name);
    Text(L, 3, "setDiscordRichPresenceButton", url);
    if (index < 1.0 || index >= 3.0) return PushReady(L);
    const unsigned button = static_cast<unsigned>(index);
    if (button < 1 || button > 2) return PushReady(L);
    if (name.size() > 32) return luaL_error(L, "Button name length must be less than/equal to 32");
    if (url.size() > 128) return luaL_error(L, "Button URL length must be less than/equal to 128");
    if (url.compare(0, 8, "https://") != 0 && url.compare(0, 8, "mtasa://") != 0)
        return luaL_error(L, "Button URL should include the https:// or mtasa:// link");
    return PushReady(L);
}

int SetTime(lua_State* L, const char* name, bool isStart) {
    if (!lua_isnumber(L, 1)) return luaL_error(L, "Bad argument @ '%s' [Expected number at argument 1]", name);
    const double value = lua_tonumber(L, 1);
    {
        std::lock_guard<std::mutex> lock(g_discordMutex);
        if (isStart) g_discord.startTime = value;
        else g_discord.endTime = value;
    }
    return PushReady(L);
}

int SetStartTime(lua_State* L) { return SetTime(L, "setDiscordRichPresenceStartTime", true); }
int SetEndTime(lua_State* L) { return SetTime(L, "setDiscordRichPresenceEndTime", false); }

int SetPartySize(lua_State* L) {
    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
        return luaL_error(L, "Bad argument @ 'setDiscordRichPresencePartySize' [Expected number]");
    const int size = static_cast<int>(lua_tonumber(L, 1));
    const int maxSize = static_cast<int>(lua_tonumber(L, 2));
    if (size > maxSize)
        return luaL_error(L, "Party size must be less than or equal to max party size");
    {
        std::lock_guard<std::mutex> lock(g_discordMutex);
        g_discord.partySize = size;
        g_discord.partyMax = maxSize;
    }
    return PushReady(L);
}

int ResetData(lua_State* L) {
    // Restore Project Genesis branding instead of disabling Rich Presence.
    std::lock_guard<std::mutex> lock(g_discordMutex);
    g_discord = DiscordState{};
    return PushReady(L);
}

int GetUserID(lua_State* L) {
    lua_pushstring(L, "");
    return 1;
}

}  // namespace

void InstallDiscordLua(lua_State* L) {
    const struct { const char* name; lua_CFunction fn; } binds[] = {
        {"setDiscordApplicationID", SetAppID},
        {"setDiscordRichPresenceDetails", SetDetails},
        {"setDiscordRichPresenceState", SetState},
        {"setDiscordRichPresenceAsset", SetLargeAsset},
        {"setDiscordRichPresenceSmallAsset", SetSmallAsset},
        {"setDiscordRichPresenceButton", SetButtons},
        {"setDiscordRichPresenceStartTime", SetStartTime},
        {"setDiscordRichPresenceEndTime", SetEndTime},
        {"setDiscordRichPresencePartySize", SetPartySize},
        {"resetDiscordRichPresenceData", ResetData},
        {"isDiscordRichPresenceConnected", PushReady},
        {"getDiscordRichPresenceUserID", GetUserID},
    };
    for (const auto& bind : binds) {
        lua_pushcfunction(L, bind.fn);
        lua_setglobal(L, bind.name);
    }
}

}  // namespace WorldRPG::Script
