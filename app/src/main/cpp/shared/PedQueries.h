#pragma once
#include <array>
#include <cstdint>
#include "entity/PedSpawn.h"

namespace WorldRPG::Game { struct PedRpcOp; }
namespace WorldRPG::PedQueries {
constexpr unsigned Slots = 13;
struct Weapon {
    uint32_t type = 0, total = 0, clip = 0;
};
struct Snapshot {
    uint64_t generation = 0;
    bool player = false, streamed = false, dead = false, networkDead = false;
    float health = 100.f;
    int taskType = -1; // no event-response-nontemp task
    unsigned currentSlot = 0;
    std::array<Weapon, Slots> weapons{};
    std::array<uint16_t, Slots> cachedAmmo{};
};
// Client state survives stream-out, but not element deletion or session reset.
void Ensure(uint32_t id, bool player);
void Create(uint32_t id, bool player, const Entity::PedSpawn* spawn = nullptr);
void Forget(uint32_t id);
void Reset();
bool Copy(uint32_t id, Snapshot& out);
bool Current(uint32_t id, uint64_t generation);
uint64_t Generation(uint32_t id);
uint64_t WeaponRpc(uint32_t id, const Game::PedRpcOp& op);
void Health(uint32_t id, float health, bool clearDead = true);
void Wasted(uint32_t id);
void Spawn(uint32_t id);
// Base inventory for a new GTA body, after offscreen RPCs updated the client cache.
bool CopyRestore(uint32_t id, Snapshot& out);
void StreamOut(uint32_t id, const Snapshot& native);
int WeaponSlot(unsigned type);
bool IsDead(const Snapshot& state);
bool Ammo(const Snapshot& state, unsigned slot, bool clip, uint16_t& out);
}
