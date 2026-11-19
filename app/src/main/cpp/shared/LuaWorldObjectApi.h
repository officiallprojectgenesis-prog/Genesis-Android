#pragma once

#include <cstdint>

struct lua_State;

namespace WorldRPG::Script {

void InstallWorldObjectLua(lua_State* L);

// Position stored for a client or server object, after sampling an active move.
// False when this id is not an object record.
bool CopyObjectPose(uint32_t id, float& x, float& y, float& z);

}  // namespace WorldRPG::Script
