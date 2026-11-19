#pragma once
#include <cstdint>
struct lua_State;
namespace WorldRPG::Script {
void InstallMarkerLua(lua_State* L);
void MarkerRelease(uint32_t id);
}
