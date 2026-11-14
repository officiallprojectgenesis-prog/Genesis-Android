#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "entity/PedSpawn.h"

namespace WorldRPG::Net {
struct ResourceStartInfo;
}

namespace WorldRPG::ServerWorld {

// Reconnect: forgets every element, its custom data and the resource root of the old session. Radar blips of the
// old session are cleared on the next Tick (game thread). Call before the new session's first packet.
void ResetSession();
// Drops a resource-root placeholder (kind 0) stored by OnResourceStart. A real
// player, ped, vehicle, marker or blip is left alone.
void ForgetPlaceholder(uint32_t id);

void SetLocalPlayer(uint32_t id);
uint32_t LocalPlayer();
void NotePlayerSkin(uint32_t id, int model);

// -1 until PLAYER_LIST or ENTITY_ADD names a model.
int ModelOf(uint32_t id);

bool GetData(uint32_t id, const char* key, double& number, std::string& text, bool& isNumber);

// One custom-data value as GetData reports it (number, or text when number is false).
struct ElementDataValue {
    bool number = false;
    double value = 0;
    std::string text;
};

// Own custom data for one element. Empty when the element has none.
void CopyElementData(uint32_t id, std::vector<std::pair<std::string, ElementDataValue>>& out);

// Nick from the join or from PLAYER_LIST. False when this id has no nick yet.
bool CopyPlayerNick(uint32_t id, std::string& out);
void NotePlayerNick(uint32_t id, const std::string& nick);
// Case-insensitive nick match among players (CClientPlayerManager::Get, default insensitive).
bool FindPlayerByNick(const std::string& nick, uint32_t& id);

void OnEntityAdd(const uint8_t* body, size_t length, size_t bits);
void OnEntityRemove(const uint8_t* body, size_t length, size_t bits);
void OnCustomData(const uint8_t* body, size_t length, size_t bits);
void OnPlayerList(const uint8_t* body, size_t length, size_t bits);
void OnResourceStart(const WorldRPG::Net::ResourceStartInfo& info);
uint32_t LastResourceRoot();

struct ActorSpawn {
    uint32_t id = 0;
    int kind = 0;
    int model = -1;
    float x = 0, y = 0, z = 0, heading = 0;
    // Vehicles only (see Entity::Visual): roll and pitch in degrees, and health (< 0 when unknown).
    float rotX = 0, rotY = 0;
    float health = -1.f;
    bool hosted = false;
    // ENTITY_ADD interior (CPacketHandler common header). Outdoor traffic is 0.
    uint8_t interior = 0;
};
// Copies peds (kind 14) and vehicles (kind 2).
// Both limits negative: the two kinds share `capacity` (a run of vehicles can fill it).
// Otherwise each kind stops at its own limit and peds are copied first, so a full vehicle
// quota cannot take a ped slot. `capacity` is the size of `out`.
int CopyActors(ActorSpawn* out, int capacity, int pedLimit = -1, int vehicleLimit = -1);
void SetActorHost(uint32_t id, bool host);
bool ActorHosted(uint32_t id);
bool ElementPose(uint32_t id, float& x, float& y, float& z);
bool CopyVisual(uint32_t id, ActorSpawn& out);
const char* ElementKindName(uint32_t id);
// Sync time context from ENTITY_ADD (0 when this id has none). Desktop CClientEntity::GetSyncTimeContext.
uint8_t SyncTimeContext(uint32_t id);
// The ENTITY_ADD ped body beyond pose and model (PedSpawn.h). False when the id is not a ped from ENTITY_ADD.
bool CopyPedSpawn(uint32_t id, Entity::PedSpawn& out);
// PLAYER_QUIT (5): CPacketHandler::Packet_PlayerQuit -> CClientGame::QuitPlayer deletes the player element.
void OnPlayerQuit(uint32_t id);
// A remote player this session knows (PLAYER_LIST nick, PLAYER_SPAWN skin, or a body record of kind 1).
bool IsKnownPlayer(uint32_t id);
// A PLAYER_PURESYNC came from this id: it is a player (kind 1) even without a PLAYER_LIST or PLAYER_SPAWN record.
void NotePlayerSeen(uint32_t id);

// Radar blips and the F11 snapshot. Call on the game thread.
void Tick(float playerX, float playerY, float playerZ);
std::string MapSnapshot(float playerX, float playerY, float playerHeading);

} // namespace WorldRPG::ServerWorld
