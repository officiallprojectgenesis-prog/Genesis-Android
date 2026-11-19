#pragma once
#include "SpatialWorld.h"
struct lua_State;
namespace WorldRPG::Script {
void InstallSpatialLua(lua_State* L);
bool SpatialShapeAlive(uint32_t id);
bool CopySpatialShape(uint32_t id, Spatial::Element& out);
bool SpatialSetArea(uint32_t id, unsigned value, bool dimension);
void SpatialRelease(uint32_t id);
void SpatialDropResource(const std::string& owner,uint16_t netId,uint32_t generation);
void PulseSpatial();
void ResetSpatial();
}
