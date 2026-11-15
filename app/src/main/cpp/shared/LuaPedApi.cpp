// Ped model and player Lua queries that do not call into CPed / CPlayerPed.
// Ped, clothes, bone, weapon, task, voice, jetpack and mission-text functions stay
// unregistered: those hooks are not wired, and a stored flag would not be the desktop call.

#include "LuaApiExtra.h"
#include "LuaVMStub.h"
#include "ServerWorld.h"
#include "../gtasa/GameEngine.h"
#include "../gtasa/RemoteVehicle.h"
#include "../net/raknet/RakNetWrapper.h"

extern "C" {
#include "lua.h"
}

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>

namespace WorldRPG::Script {
namespace {

// Nametag fields from CClientPlayer. Showing starts true. Color is white until a
// script sets an override. A team color is not applied: OnPlayerList reads a team
// id and drops it, so no player has a team. Cleared when InstallPedLua runs, which
// is each new Lua VM (a new session).
struct Nametag {
    bool showing = true;
    bool colorOverride = false;
    unsigned char red = 255;
    unsigned char green = 255;
    unsigned char blue = 255;
    // Empty until setPlayerNametagText. The displayed text is the nick until then
    // (CClientPlayer::SetNametagText(m_strNick) in the constructor).
    bool textOverride = false;
    std::string text;
};

std::unordered_map<uint32_t, Nametag> g_nametags;

bool IsPlayer(lua_State* L, int index, uint32_t& id) {
    if (!lua_istable(L, index)) return false;
    id = ElementIdOf(L, index);
    if (id == 0 || id == 0xFFFFFFFFu) return false;
    const char* kind = ServerWorld::ElementKindName(id);
    return kind && std::strcmp(kind, "player") == 0;
}

Nametag& TagFor(uint32_t id) { return g_nametags[id]; }

int Fail(lua_State* L, const char* name, const char* message) {
    LUA_LOGE("%s: %s", name, message);
    lua_pushboolean(L, 0);
    return 1;
}

int PushFalse(lua_State* L) {
    lua_pushboolean(L, 0);
    return 1;
}

// CLuaFunctionDefs.Util.cpp:13-54. Only stock models are available: the port's
// engineRequestModel is still a stub, so both custom-model walks are empty.
// The stock validity set follows CPlayerManager::IsValidPlayerModel
// (Server/mods/deathmatch/logic/CPlayerManager.cpp:338-352). Do not infer model
// availability from a streamed ped: the desktop also lists unloaded models.
int GetValidPedModels(lua_State* L) {
    // ReadBool(includeCustom, true) does not gate the desktop body on HasErrors:
    // even a non-boolean argument returns the stock list, with custom=false.
    lua_newtable(L);
    int index = 0;
    for (int model = 0; model <= 312; ++model) {
        if (model == 74 || model == 149 || model == 208) continue;
        lua_pushnumber(L, model);
        lua_rawseti(L, -2, ++index);
    }
    return 1;
}

// Nick stored at join or from PLAYER_LIST. "Player" is the previous fixed text,
// used only until a nick is stored for this id.
std::string PlayerNameOf(uint32_t id) {
    std::string nick;
    if (ServerWorld::CopyPlayerNick(id, nick)) return nick;
    return "Player";
}

bool ReadStringArg(lua_State* L, int index, std::string& out) {
    const int type = lua_type(L, index);
    if (type != LUA_TSTRING && type != LUA_TNUMBER) return false;
    size_t length = 0;
    const char* text = lua_tolstring(L, index, &length);
    if (!text) return false;
    out.assign(text, length);
    return true;
}

// UTF-16 code units of the bytes up to the first 0, matching MbUTF8ToUTF16 on a C string.
size_t Utf16Units(const std::string& text) {
    size_t units = 0;
    const unsigned char* s = reinterpret_cast<const unsigned char*>(text.data());
    const unsigned char* end = s + text.size();
    while (s < end && *s != 0) {
        const unsigned char lead = *s;
        size_t need = 1;
        uint32_t code = lead;
        if (lead < 0x80) {
            need = 1;
            code = lead;
        } else if ((lead & 0xE0) == 0xC0) {
            need = 2;
            code = lead & 0x1Fu;
        } else if ((lead & 0xF0) == 0xE0) {
            need = 3;
            code = lead & 0x0Fu;
        } else if ((lead & 0xF8) == 0xF0) {
            need = 4;
            code = lead & 0x07u;
        } else {
            ++units;
            ++s;
            continue;
        }
        if (s + need > end) {
            ++units;
            break;
        }
        bool ok = true;
        for (size_t i = 1; i < need; ++i) {
            if ((s[i] & 0xC0) != 0x80) {
                ok = false;
                break;
            }
            code = (code << 6) | (s[i] & 0x3Fu);
        }
        if (!ok) {
            ++units;
            ++s;
            continue;
        }
        units += code >= 0x10000u ? 2 : 1;
        s += need;
    }
    return units;
}

// CClientGame::IsNametagValid (CClientGame.cpp 1673-1694).
// MIN_PLAYER_NAMETAG_LENGTH 1, MAX_PLAYER_NAMETAG_LENGTH 64 (CClientCommon.h).
// The character walk indexes the C string by the UTF-16 length, as the desktop does.
bool NametagValid(const std::string& text) {
    const size_t units = Utf16Units(text);
    if (units < 1 || units > 64) return false;
    for (size_t i = 0; i < units; ++i) {
        const unsigned char byte = i < text.size() ? static_cast<unsigned char>(text[i]) : 0;
        if (byte < 32) return false;
    }
    return true;
}

// CScriptArgReader::ReadNumber into a signed int, then the unsigned char cast in
// SetPlayerNametagColor. Numeric strings are accepted. NaN and anything else fail.
bool ReadChannel(lua_State* L, int index, unsigned char& channel, const char*& error) {
    const int type = lua_type(L, index);
    if (type != LUA_TNUMBER && type != LUA_TSTRING) {
        error = "expected a number";
        return false;
    }
    if (!lua_isnumber(L, index)) {
        error = "expected a number, got a non-convertible string";
        return false;
    }
    const double number = lua_tonumber(L, index);
    if (std::isnan(number)) {
        error = "expected a number, got NaN";
        return false;
    }
    channel = static_cast<unsigned char>(static_cast<int>(number));
    return true;
}

// Desktop CLuaPlayerDefs::GetPlayerName (CLuaPlayerDefs.cpp 325-349).
// CClientPlayer::GetNick. The nick is the join name, then the PLAYER_LIST name.
int GetPlayerName(lua_State* L) {
    uint32_t id = 0;
    if (!IsPlayer(L, 1, id)) return Fail(L, "getPlayerName", "expected a player");
    const std::string name = PlayerNameOf(id);
    lua_pushlstring(L, name.data(), name.size());
    return 1;
}

bool IsPlayerId(uint32_t id) {
    if (id == 0 || id == 0xFFFFFFFFu) return false;
    const char* kind = ServerWorld::ElementKindName(id);
    return kind && std::strcmp(kind, "player") == 0;
}

// Desktop CLuaPlayerDefs::GetPlayerFromName (CLuaPlayerDefs.cpp 351-376).
// getPlayerFromNick is the same function. Match is case-insensitive
// (CClientPlayerManager::Get default, CClientPlayerManager.cpp 117-136).
int GetPlayerFromName(lua_State* L) {
    std::string name;
    if (!ReadStringArg(L, 1, name)) return Fail(L, "getPlayerFromName", "expected a string");
    uint32_t id = 0;
    if (!ServerWorld::FindPlayerByNick(name, id) || !IsPlayerId(id)) return PushFalse(L);
    PushElement(L, id);
    return 1;
}

// Desktop CLuaPlayerDefs::GetPlayerNametagText (CLuaPlayerDefs.cpp 378-402).
int GetPlayerNametagText(lua_State* L) {
    uint32_t id = 0;
    if (!IsPlayer(L, 1, id)) return Fail(L, "getPlayerNametagText", "expected a player");
    const auto found = g_nametags.find(id);
    const std::string text = (found != g_nametags.end() && found->second.textOverride)
                                 ? found->second.text
                                 : PlayerNameOf(id);
    lua_pushlstring(L, text.data(), text.size());
    return 1;
}

// Desktop CLuaPlayerDefs::GetPlayerPing (CLuaPlayerDefs.cpp 452-473).
// CClientPlayer::GetPing (CClientPlayer.h 48): the local player uses g_pNet->GetPing,
// which is RakNetWrapper::GetPing. A remote player's m_uiPing starts at 0 and the
// latency write in CNetAPI is not stored, so a remote player stays at 0.
int GetPlayerPing(lua_State* L) {
    uint32_t id = 0;
    if (!IsPlayer(L, 1, id)) return Fail(L, "getPlayerPing", "expected a player");
    int ping = 0;
    if (id == ServerWorld::LocalPlayer()) {
        if (WorldRPG::Net::RakNetWrapper* net = WorldRPG::Net::RakNetWrapper::Live()) ping = net->GetPing();
    }
    lua_pushnumber(L, ping);
    return 1;
}

// Desktop CLuaPlayerDefs::GetPlayerTeam (CLuaPlayerDefs.cpp 475-499).
// OnPlayerList reads a team id and drops it, so no player has a team element.
int GetPlayerTeam(lua_State* L) {
    uint32_t id = 0;
    if (!IsPlayer(L, 1, id)) return Fail(L, "getPlayerTeam", "expected a player");
    (void)id;
    lua_pushboolean(L, 0);
    return 1;
}

// Desktop CLuaPlayerDefs::GetPlayerNametagColor (CLuaPlayerDefs.cpp:404-429).
// CClientPlayer::GetNametagColor (CClientPlayer.cpp:142-166): the override, otherwise
// the team color, otherwise white. No team is stored, so the middle branch is white.
int GetPlayerNametagColor(lua_State* L) {
    uint32_t id = 0;
    if (!IsPlayer(L, 1, id)) return Fail(L, "getPlayerNametagColor", "expected a player");
    const auto found = g_nametags.find(id);
    const bool overridden = found != g_nametags.end() && found->second.colorOverride;
    const unsigned char red = overridden ? found->second.red : 255;
    const unsigned char green = overridden ? found->second.green : 255;
    const unsigned char blue = overridden ? found->second.blue : 255;
    lua_pushnumber(L, red);
    lua_pushnumber(L, green);
    lua_pushnumber(L, blue);
    return 3;
}

// Desktop CLuaPlayerDefs::GetPlayerSerial (CLuaPlayerDefs.cpp:770-777).
// g_pCore->GetNetwork()->GetSerial is RakNetWrapper::GetSerial.
int GetPlayerSerial(lua_State* L) {
    char serial[64] = {};
    if (WorldRPG::Net::RakNetWrapper* net = WorldRPG::Net::RakNetWrapper::Live())
        net->GetSerial(serial, sizeof(serial));
    lua_pushstring(L, serial);
    return 1;
}

// Desktop CLuaPlayerDefs::GetPlayerUserName (CLuaPlayerDefs.cpp:764-768).
// The desktop function returns false. There is no username to read.
int GetPlayerUserName(lua_State* L) {
    lua_pushboolean(L, 0);
    return 1;
}

// Desktop CLuaPlayerDefs::IsPlayerNametagShowing (CLuaPlayerDefs.cpp:431-450).
// CClientPlayer starts with the nametag showing (CClientPlayer.cpp:76 and 240).
int IsPlayerNametagShowing(lua_State* L) {
    uint32_t id = 0;
    if (!IsPlayer(L, 1, id)) return Fail(L, "isPlayerNametagShowing", "expected a player");
    const auto found = g_nametags.find(id);
    const bool showing = found == g_nametags.end() || found->second.showing;
    lua_pushboolean(L, showing ? 1 : 0);
    return 1;
}

// Desktop CLuaPlayerDefs::SetPlayerNametagText (CLuaPlayerDefs.cpp 653-676) and
// CStaticFunctionDefinitions::SetPlayerNametagText (CStaticFunctionDefinitions.cpp 2170-2182).
// RUN_CHILDREN does not run: players are not given children on the phone, so
// CountChildren() is 0 and the walk is skipped on desktop too.
int SetPlayerNametagText(lua_State* L) {
    uint32_t id = 0;
    std::string text;
    if (!IsPlayer(L, 1, id)) return Fail(L, "setPlayerNametagText", "expected a player");
    if (!ReadStringArg(L, 2, text)) return Fail(L, "setPlayerNametagText", "expected a string");
    if (!NametagValid(text)) return PushFalse(L);
    Nametag& tag = TagFor(id);
    tag.textOverride = true;
    tag.text = std::move(text);
    lua_pushboolean(L, 1);
    return 1;
}

// Desktop CLuaPlayerDefs::SetPlayerNametagColor (CLuaPlayerDefs.cpp:678-733) and
// CStaticFunctionDefinitions::SetPlayerNametagColor (CStaticFunctionDefinitions.cpp:2185-2199).
// Two calls: (player, r, g, b) sets an override; (player, false) clears it.
// (player, true) is rejected. RUN_CHILDREN does not run: no element has children,
// so CountChildren() is 0 and the walk is skipped on desktop too.
int SetPlayerNametagColor(lua_State* L) {
    uint32_t id = 0;
    if (!IsPlayer(L, 1, id)) return Fail(L, "setPlayerNametagColor", "expected a player");
    if (lua_type(L, 2) == LUA_TBOOLEAN) {
        if (lua_toboolean(L, 2)) return PushFalse(L);
        Nametag& tag = TagFor(id);
        tag.colorOverride = false;
        tag.red = 255;
        tag.green = 255;
        tag.blue = 255;
        lua_pushboolean(L, 1);
        return 1;
    }
    const char* error = nullptr;
    unsigned char red = 0, green = 0, blue = 0;
    if (!ReadChannel(L, 2, red, error) || !ReadChannel(L, 3, green, error) || !ReadChannel(L, 4, blue, error))
        return Fail(L, "setPlayerNametagColor", error ? error : "expected a number");
    Nametag& tag = TagFor(id);
    tag.colorOverride = true;
    tag.red = red;
    tag.green = green;
    tag.blue = blue;
    lua_pushboolean(L, 1);
    return 1;
}

// Desktop CLuaPlayerDefs::SetPlayerNametagShowing (CLuaPlayerDefs.cpp:735-760) and
// CStaticFunctionDefinitions::SetPlayerNametagShowing (CStaticFunctionDefinitions.cpp:2202-2214).
// The showing flag is a boolean, not a number. Same empty child walk as the color setter.
int SetPlayerNametagShowing(lua_State* L) {
    uint32_t id = 0;
    if (!IsPlayer(L, 1, id)) return Fail(L, "setPlayerNametagShowing", "expected a player");
    if (lua_type(L, 2) != LUA_TBOOLEAN) return Fail(L, "setPlayerNametagShowing", "expected a boolean");
    TagFor(id).showing = lua_toboolean(L, 2) != 0;
    lua_pushboolean(L, 1);
    return 1;
}

// CScriptArgReader::ReadUserData<CClientPed>: a ped or a player.
bool IsPed(lua_State* L, int index, uint32_t& id) {
    if (!lua_istable(L, index)) return false;
    id = ElementIdOf(L, index);
    if (id == 0 || id == 0xFFFFFFFFu) return false;
    const char* kind = ServerWorld::ElementKindName(id);
    return kind && (std::strcmp(kind, "player") == 0 || std::strcmp(kind, "ped") == 0);
}

// Desktop CClientPed::GetOccupiedVehicle / GetOccupiedVehicleSeat. The local player is in the
// car the game reports (WatchLocalVehicle); its seat is 0 because the phone only requests the
// driver seat (NeonJoinProtocol.cpp VEHICLE_REQUEST_IN). Other peds are seated by
// VEHICLE_INOUT (PacketHandler.cpp ApplyRemoteVehicleInOut). A ped that ENTITY_ADD placed in a
// vehicle is not seated yet (the warp logs DIAG), so it reports no vehicle.
bool PedOccupiedVehicle(uint32_t pedId, uint32_t& vehicleId, uint8_t& seat) {
    if (pedId == ServerWorld::LocalPlayer()) {
        vehicleId = WorldRPG::Game::LocalVehicleId();
        seat = 0;
    } else if (!WorldRPG::RemoteVehicle::FindOccupant(pedId, vehicleId, seat)) {
        return false;
    }
    if (vehicleId == 0 || vehicleId == 0xFFFFFFFFu) return false;
    const char* kind = ServerWorld::ElementKindName(vehicleId);
    return kind && std::strcmp(kind, "vehicle") == 0;
}

// CLuaPedDefs.cpp:834-857.
int GetPedOccupiedVehicle(lua_State* L) {
    uint32_t id = 0, vehicle = 0;
    uint8_t seat = 0;
    if (!IsPed(L, 1, id)) return Fail(L, "getPedOccupiedVehicle", "expected a ped");
    if (!PedOccupiedVehicle(id, vehicle, seat)) return PushFalse(L);
    PushElement(L, vehicle);
    return 1;
}

// CLuaPedDefs.cpp:859-880 and CStaticFunctionDefinitions.cpp:1972-1980.
int GetPedOccupiedVehicleSeat(lua_State* L) {
    uint32_t id = 0, vehicle = 0;
    uint8_t seat = 0;
    if (!IsPed(L, 1, id)) return Fail(L, "getPedOccupiedVehicleSeat", "expected a ped");
    if (!PedOccupiedVehicle(id, vehicle, seat)) return PushFalse(L);
    lua_pushinteger(L, seat);
    return 1;
}

// CLuaPedDefs.cpp:1214-1238 and CStaticFunctionDefinitions.cpp:1966-1970. A bad argument is nil.
int IsPedInVehicle(lua_State* L) {
    uint32_t id = 0, vehicle = 0;
    uint8_t seat = 0;
    if (!IsPed(L, 1, id)) {
        LUA_LOGE("isPedInVehicle: expected a ped");
        lua_pushnil(L);
        return 1;
    }
    lua_pushboolean(L, PedOccupiedVehicle(id, vehicle, seat) ? 1 : 0);
    return 1;
}

}  // namespace

void BindPedLua(lua_State* L) {
    g_nametags.clear();
    const struct {
        const char* name;
        lua_CFunction fn;
    } binds[] = {
        {"getValidPedModels", GetValidPedModels},
        {"getPedOccupiedVehicle", GetPedOccupiedVehicle},
        {"getPedOccupiedVehicleSeat", GetPedOccupiedVehicleSeat},
        {"isPedInVehicle", IsPedInVehicle},
        {"getPlayerFromName", GetPlayerFromName},
        {"getPlayerFromNick", GetPlayerFromName},
        {"getPlayerName", GetPlayerName},
        {"getPlayerNametagColor", GetPlayerNametagColor},
        {"getPlayerNametagText", GetPlayerNametagText},
        {"getPlayerPing", GetPlayerPing},
        {"getPlayerSerial", GetPlayerSerial},
        {"getPlayerTeam", GetPlayerTeam},
        {"getPlayerUserName", GetPlayerUserName},
        {"isPlayerNametagShowing", IsPlayerNametagShowing},
        {"setPlayerNametagColor", SetPlayerNametagColor},
        {"setPlayerNametagShowing", SetPlayerNametagShowing},
        {"setPlayerNametagText", SetPlayerNametagText},
    };
    for (const auto& bind : binds) {
        lua_pushcfunction(L, bind.fn);
        lua_setglobal(L, bind.name);
    }
}

}  // namespace WorldRPG::Script

extern "C" void InstallPedLua(lua_State* L) {
    WorldRPG::Script::BindPedLua(L);
}
