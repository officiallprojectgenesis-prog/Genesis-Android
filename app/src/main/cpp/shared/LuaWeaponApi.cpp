#include "LuaWeaponApi.h"
#include "LuaProjectileApi.h"

#include "LuaVMStub.h"
#include "LuaApiExtra.h"
#include "ServerWorld.h"

#include "lua.hpp"

#include <cmath>
#include <cstring>
#include <string>
#include <vector>

namespace WorldRPG::Script {
namespace {

#include "LuaWeaponData.inc"

const char* kWeaponNames[] = {
    "Fist", "Brassknuckle", "Golfclub", "Nightstick", "Knife", "Bat", "Shovel", "Poolstick", "Katana", "Chainsaw",
    "Dildo", "Dildo", "Vibrator", "Vibrator", "Flower", "Cane", "Grenade", "Teargas", "Molotov", "", "", "",
    "Colt 45", "Silenced", "Deagle", "Shotgun", "Sawed-off", "Combat Shotgun", "Uzi", "MP5", "AK-47", "M4", "Tec-9",
    "Rifle", "Sniper", "Rocket Launcher", "Rocket Launcher HS", "Flamethrower", "Minigun", "Satchel", "Bomb",
    "Spraycan", "Fire Extinguisher", "Camera", "Nightvision", "Infrared", "Parachute",
};

struct Override { int id; int skill; std::string property; double a, b, c; bool triple; };
std::vector<Override> g_overrides;
bool g_diagWeapon = false;

int SkillOf(lua_State* L, int index) {
    if (lua_isnumber(L, index)) {
        const int skill = static_cast<int>(lua_tointeger(L, index));
        return skill >= 0 && skill <= 2 ? skill : -1;
    }
    const char* name = lua_tostring(L, index);
    if (!name) return -1;
    if (std::strcmp(name, "poor") == 0) return 0;
    if (std::strcmp(name, "std") == 0) return 1;
    if (std::strcmp(name, "pro") == 0) return 2;
    return -1;
}

int WeaponOf(lua_State* L, int index) {
    if (lua_isnumber(L, index)) {
        const int id = static_cast<int>(lua_tointeger(L, index));
        return id >= 0 && id <= 46 ? id : -1;
    }
    const char* name = lua_tostring(L, index);
    if (!name) return -1;
    for (int id = 0; id < static_cast<int>(sizeof(kWeaponNames) / sizeof(kWeaponNames[0])); ++id) {
        if (kWeaponNames[id][0] != '\0' && std::strcmp(kWeaponNames[id], name) == 0) return id;
    }
    return -1;
}

const WStat* FindStat(int id, int skill) {
    for (const WStat& stat : kWeaponStats)
        if (stat.id == id && stat.skill == skill) return &stat;
    return nullptr;
}

Override* FindOverride(int id, int skill, const char* property) {
    for (Override& item : g_overrides)
        if (item.id == id && item.skill == skill && item.property == property) return &item;
    return nullptr;
}

int FlagBit(const char* property) {
    if (std::strcmp(property, "flag_aim_no_auto") == 0) return 0x1;
    if (std::strcmp(property, "flag_aim_arm") == 0) return 0x2;
    if (std::strcmp(property, "flag_aim_1st_person") == 0) return 0x4;
    if (std::strcmp(property, "flag_aim_free") == 0) return 0x8;
    if (std::strcmp(property, "flag_move_and_aim") == 0) return 0x10;
    if (std::strcmp(property, "flag_move_and_shoot") == 0) return 0x20;
    if (std::strcmp(property, "flag_type_throw") == 0) return 0x100;
    if (std::strcmp(property, "flag_type_heavy") == 0) return 0x200;
    if (std::strcmp(property, "flag_type_constant") == 0) return 0x400;
    if (std::strcmp(property, "flag_type_dual") == 0) return 0x800;
    if (std::strcmp(property, "flag_anim_reload") == 0) return 0x1000;
    if (std::strcmp(property, "flag_anim_crouch") == 0) return 0x2000;
    if (std::strcmp(property, "flag_anim_reload_loop") == 0) return 0x4000;
    if (std::strcmp(property, "flag_anim_reload_long") == 0) return 0x8000;
    if (std::strcmp(property, "flag_shot_slows") == 0) return 0x10000;
    if (std::strcmp(property, "flag_shot_rand_speed") == 0) return 0x20000;
    if (std::strcmp(property, "flag_shot_anim_abrupt") == 0) return 0x40000;
    if (std::strcmp(property, "flag_shot_expands") == 0) return 0x80000;
    return 0;
}

bool PushProperty(lua_State* L, const WStat& stat, const char* property) {
    const int flag = FlagBit(property);
    if (flag != 0) {
        lua_pushboolean(L, (stat.flags & flag) != 0);
        return true;
    }
    if (std::strcmp(property, "fire_offset") == 0) {
        lua_pushnumber(L, stat.ox);
        lua_pushnumber(L, stat.oy);
        lua_pushnumber(L, stat.oz);
        return true;
    }
    auto pushInt = [&](int value) { lua_pushinteger(L, value); };
    auto pushFloat = [&](float value) { lua_pushnumber(L, value); };
    if (std::strcmp(property, "weapon_range") == 0) pushFloat(stat.weapon_range);
    else if (std::strcmp(property, "target_range") == 0) pushFloat(stat.target_range);
    else if (std::strcmp(property, "accuracy") == 0) pushFloat(stat.accuracy);
    else if (std::strcmp(property, "life_span") == 0) pushFloat(stat.life_span);
    else if (std::strcmp(property, "firing_speed") == 0) pushFloat(stat.firing_speed);
    else if (std::strcmp(property, "spread") == 0) pushFloat(stat.spread);
    else if (std::strcmp(property, "move_speed") == 0) pushFloat(stat.move_speed);
    else if (std::strcmp(property, "anim_loop_start") == 0) pushFloat(stat.anim_loop_start);
    else if (std::strcmp(property, "anim_loop_stop") == 0) pushFloat(stat.anim_loop_stop);
    else if (std::strcmp(property, "anim_loop_bullet_fire") == 0) pushFloat(stat.anim_loop_bullet_fire);
    else if (std::strcmp(property, "anim2_loop_start") == 0) pushFloat(stat.anim2_loop_start);
    else if (std::strcmp(property, "anim2_loop_stop") == 0) pushFloat(stat.anim2_loop_stop);
    else if (std::strcmp(property, "anim2_loop_bullet_fire") == 0) pushFloat(stat.anim2_loop_bullet_fire);
    else if (std::strcmp(property, "anim_breakout_time") == 0) pushFloat(stat.anim_breakout_time);
    else if (std::strcmp(property, "radius") == 0) pushFloat(stat.radius);
    else if (std::strcmp(property, "damage") == 0) pushInt(stat.damage);
    else if (std::strcmp(property, "maximum_clip_ammo") == 0) pushInt(stat.maximum_clip_ammo);
    else if (std::strcmp(property, "flags") == 0) pushInt(stat.flags);
    else if (std::strcmp(property, "anim_group") == 0) pushInt(stat.anim_group);
    else if (std::strcmp(property, "fire_type") == 0) pushInt(stat.fire_type);
    else if (std::strcmp(property, "model") == 0) pushInt(stat.model);
    else if (std::strcmp(property, "model2") == 0) pushInt(stat.model2);
    else if (std::strcmp(property, "weapon_slot") == 0) pushInt(stat.weapon_slot);
    else if (std::strcmp(property, "skill_level") == 0) pushInt(stat.skill_level);
    else if (std::strcmp(property, "required_skill_level") == 0) pushInt(stat.required_skill_level);
    else if (std::strcmp(property, "aim_offset") == 0) pushInt(stat.aim_offset);
    else if (std::strcmp(property, "default_combo") == 0) pushInt(stat.default_combo);
    else if (std::strcmp(property, "combos_available") == 0) pushInt(stat.combos_available);
    else return false;
    return true;
}

int ReadProperty(lua_State* L, bool original) {
    const int id = WeaponOf(L, 1);
    const int skill = SkillOf(L, 2);
    const char* property = lua_tostring(L, 3);
    if (id < 0 || skill < 0 || !property) {
        lua_pushboolean(L, 0);
        return 1;
    }
    if (!original) {
        // A flag read uses the flags word, including a setWeaponProperty override of that word.
        // Desktop IsFlagSet (CStaticFunctionDefinitions::SetWeaponPropertyFlag 2657-2676).
        const int bit = FlagBit(property);
        if (bit != 0) {
            const WStat* stat = FindStat(id, skill);
            if (!stat) {
                lua_pushboolean(L, 0);
                return 1;
            }
            int flags = stat->flags;
            if (Override* word = FindOverride(id, skill, "flags")) flags = static_cast<int>(word->a);
            lua_pushboolean(L, (flags & bit) != 0);
            return 1;
        }
        if (Override* item = FindOverride(id, skill, property)) {
            if (item->triple) {
                lua_pushnumber(L, item->a);
                lua_pushnumber(L, item->b);
                lua_pushnumber(L, item->c);
                return 3;
            }
            if (FlagBit(property) != 0) lua_pushboolean(L, item->a != 0);
            else if (std::strcmp(property, "damage") == 0 || std::strcmp(property, "flags") == 0 ||
                     std::strcmp(property, "maximum_clip_ammo") == 0 || std::strcmp(property, "model") == 0)
                lua_pushinteger(L, static_cast<lua_Integer>(item->a));
            else lua_pushnumber(L, item->a);
            return 1;
        }
    }
    const WStat* stat = FindStat(id, skill);
    if (!stat) {
        lua_pushboolean(L, 0);
        return 1;
    }
    const int before = lua_gettop(L);
    if (!PushProperty(L, *stat, property)) {
        lua_pushboolean(L, 0);
        return 1;
    }
    return lua_gettop(L) - before;
}

int l_getWeaponProperty(lua_State* L) { return ReadProperty(L, false); }
int l_getOriginalWeaponProperty(lua_State* L) { return ReadProperty(L, true); }

int l_setWeaponProperty(lua_State* L) {
    // CLuaWeaponDefs::SetWeaponProperty writes CWeaponStat. The phone stores the value.
    const int id = WeaponOf(L, 1);
    const int skill = SkillOf(L, 2);
    const char* property = lua_tostring(L, 3);
    const WStat* stat = FindStat(id, skill);
    if (id < 22 || id > 42 || skill < 0 || !property || !stat) {
        lua_pushboolean(L, 0);
        return 1;
    }
    const bool fire = std::strcmp(property, "fire_offset") == 0;
    const int flag = FlagBit(property);
    if (flag != 0) {
        // SetWeaponPropertyFlag (CStaticFunctionDefinitions.cpp 2657-2676): set or clear the bit.
        // The same value already stored returns false.
        if (!lua_isboolean(L, 4)) {
            lua_pushboolean(L, 0);
            return 1;
        }
        int flags = stat->flags;
        if (Override* word = FindOverride(id, skill, "flags")) flags = static_cast<int>(word->a);
        const bool enable = lua_toboolean(L, 4) != 0;
        if (enable == ((flags & flag) != 0)) {
            lua_pushboolean(L, 0);
            return 1;
        }
        if (enable) flags |= flag;
        else flags &= ~flag;
        Override next{id, skill, "flags", static_cast<double>(flags), 0, 0, false};
        if (Override* word = FindOverride(id, skill, "flags")) *word = next;
        else g_overrides.push_back(next);
        if (!g_diagWeapon) {
            g_diagWeapon = true;
            LUA_LOGE("DIAG setWeaponProperty: CWeaponStat is not written; the value is stored");
        }
        lua_pushboolean(L, 1);
        return 1;
    }
    if (!fire) {
        if (!PushProperty(L, *stat, property)) {
            lua_pushboolean(L, 0);
            return 1;
        }
        lua_pop(L, 1);
    }
    Override next{id, skill, property, 0, 0, 0, false};
    if (std::strcmp(property, "fire_offset") == 0) {
        if (!lua_isnumber(L, 4) || !lua_isnumber(L, 5) || !lua_isnumber(L, 6)) {
            lua_pushboolean(L, 0);
            return 1;
        }
        next.triple = true;
        next.a = lua_tonumber(L, 4);
        next.b = lua_tonumber(L, 5);
        next.c = lua_tonumber(L, 6);
    } else if (lua_type(L, 4) == LUA_TBOOLEAN) {
        next.a = lua_toboolean(L, 4) ? 1 : 0;
    } else if (lua_isnumber(L, 4)) {
        next.a = lua_tonumber(L, 4);
    } else {
        lua_pushboolean(L, 0);
        return 1;
    }
    if (Override* item = FindOverride(id, skill, property)) *item = next;
    else g_overrides.push_back(next);
    if (!g_diagWeapon) {
        g_diagWeapon = true;
        LUA_LOGE("DIAG setWeaponProperty: CWeaponStat is not written; the value is stored");
    }
    lua_pushboolean(L, 1);
    return 1;
}

// CLuaWeaponDefs.cpp:288-307,363-415,606-648; CClientWeapon.h:54,82,102,105.
// These are queries of server-created weapon elements, not ped-held weapons.
// Until native CClientWeapon firing / RPC updates are ported, values are the
// last ENTITY_ADD snapshot. No local ammo consumption or weapon is fabricated.
bool ReadWeapon(lua_State* L, Entity::WeaponSpawn& weapon) {
    return lua_istable(L, 1) && ServerWorld::CopyWeaponSpawn(ElementIdOf(L, 1), weapon);
}

int BadWeapon(lua_State* L) {
    LUA_LOGE("weapon query: expected a live weapon element");
    lua_pushboolean(L, 0);
    return 1;
}

int GetWeaponAmmo(lua_State* L) {
    Entity::WeaponSpawn weapon;
    if (!ReadWeapon(L, weapon)) return BadWeapon(L);
    lua_pushnumber(L, weapon.ammo);
    return 1;
}

int GetWeaponClipAmmo(lua_State* L) {
    Entity::WeaponSpawn weapon;
    if (!ReadWeapon(L, weapon)) return BadWeapon(L);
    lua_pushnumber(L, weapon.clipAmmo);
    return 1;
}

int GetWeaponState(lua_State* L) {
    Entity::WeaponSpawn weapon;
    if (!ReadWeapon(L, weapon)) return BadWeapon(L);
    // CLuaFunctionParseHelpers.cpp:374-378 has only these three enum strings.
    static const char* states[] = {"ready", "firing", "reloading"};
    lua_pushstring(L, weapon.state < 3 ? states[weapon.state] : "");
    return 1;
}

int GetWeaponOwner(lua_State* L) {
    Entity::WeaponSpawn weapon;
    if (!ReadWeapon(L, weapon)) return BadWeapon(L);
    if (weapon.owner == 0xFFFFFFFFu ||
        (weapon.owner != ServerWorld::LocalPlayer() && !ServerWorld::IsKnownPlayer(weapon.owner))) {
        lua_pushboolean(L, 0);
    } else {
        PushElement(L, weapon.owner);
    }
    return 1;
}

int GetWeaponTarget(lua_State* L) {
    Entity::WeaponSpawn weapon;
    if (!ReadWeapon(L, weapon)) return BadWeapon(L);
    switch (weapon.targetType) {
        case 0:
            lua_pushnumber(L, weapon.targetX);
            lua_pushnumber(L, weapon.targetY);
            lua_pushnumber(L, weapon.targetZ);
            return 3;
        case 1:
            if (weapon.target != 0xFFFFFFFFu) PushElement(L, weapon.target);
            else lua_pushnil(L);
            return 1;
        case 2:
            lua_pushnil(L);
            return 1;
        default:
            lua_pushboolean(L, 0);
            return 1;
    }
}

}  // namespace

void InstallWeaponLua(lua_State* L) {
    InstallProjectileLua(L);
    const struct { const char* name; lua_CFunction fn; } binds[] = {
        {"getWeaponAmmo", GetWeaponAmmo},
        {"getWeaponClipAmmo", GetWeaponClipAmmo},
        {"getWeaponState", GetWeaponState},
        {"getWeaponOwner", GetWeaponOwner},
        {"getWeaponTarget", GetWeaponTarget},
    };
    for (const auto& bind : binds) {
        lua_pushcfunction(L, bind.fn);
        lua_setglobal(L, bind.name);
    }
    lua_pushcfunction(L, l_getWeaponProperty);
    lua_setglobal(L, "getWeaponProperty");
    lua_pushcfunction(L, l_getOriginalWeaponProperty);
    lua_setglobal(L, "getOriginalWeaponProperty");
    lua_pushcfunction(L, l_setWeaponProperty);
    lua_setglobal(L, "setWeaponProperty");
}

}  // namespace WorldRPG::Script
