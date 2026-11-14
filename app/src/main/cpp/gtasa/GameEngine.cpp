#include "../shared/DebugFlags.h"
#include "GameEngine.h"
#include "PlayerPedPort.h"
#include "RenderGuard.h"
#include "GTASAOffsets.h"
#include "../shared/ServerWorld.h"
#include "../shared/entity/PedSyncState.h"

namespace WorldRPG::JniBridge { void PostGameLine(int kind, const char* text); }
#include "../hook/SymbolResolver.h"
#include "../hook/MemoryPatcher.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <array>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <memory>

#ifdef __ANDROID__
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sched.h>
#include <semaphore.h>
#include <android/log.h>
#define GTASA_LOG(...) __android_log_print(ANDROID_LOG_INFO, "GameEngine", __VA_ARGS__)
#endif

namespace WorldRPG::Game {

#ifdef __ANDROID__
struct Vec3 { float x, y, z; };
using SetMoveSpeedFn = void (*)(void*, Vec3);
using RequestSpecialModelFn = void (*)(int, const char*, int);
using LoadAllRequestedModelsFn = void (*)(bool);
using SetupPlayerPedFn = void (*)(int);
using FindPlayerPedFn = void* (*)(int);
using TeleportFn = void (*)(void*, Vec3, unsigned char);
using EndUserPauseFn = void (*)();
using RemoveTopFn = void (*)(void*, int, int);
using SwitchOffFn = void (*)();

static RequestSpecialModelFn g_requestSpecial = nullptr;
static LoadAllRequestedModelsFn g_loadAll = nullptr;
static SetupPlayerPedFn g_setupPlayer = nullptr;
static FindPlayerPedFn g_findPlayer = nullptr;
using FindPlayerCoorsFn = Vec3 (*)(int);
static FindPlayerCoorsFn g_findCoors = nullptr;
using RequestModelFn = void (*)(int, int);
static RequestModelFn g_requestModel = nullptr;
using HasModelFn = bool (*)(int);
static HasModelFn g_hasModel = nullptr;
using CivilianCtorFn = void (*)(void*, int, unsigned);
static CivilianCtorFn g_civilianCtor = nullptr;
using AddPedFn = void* (*)(int, unsigned, const Vec3*, bool);
static AddPedFn g_addPed = nullptr;
using WorldAddFn = void (*)(void*);
static WorldAddFn g_worldAdd = nullptr;
using SetHeadingFn = void (*)(void*, float);
static SetHeadingFn g_setHeading = nullptr;
static float* g_pedDensity = nullptr;
static void* g_managePopulation = nullptr;
static std::atomic<float> g_poseX{1754.f};
static std::atomic<float> g_poseY{-1894.f};
static std::atomic<float> g_poseZ{14.f};
static std::atomic<float> g_poseH{0.f};
// FindPlayerHeading, for the radar arrow only. g_poseH stays the sync sample.
static std::atomic<float> g_radarH{0.f};
static std::atomic<float> g_velX{0.f};
static std::atomic<float> g_velY{0.f};
static std::atomic<float> g_velZ{0.f};
static std::atomic<float> g_health{100.f};
static std::atomic<float> g_armor{0.f};
static std::atomic<bool> g_healthOwned{false};
static std::atomic<bool> g_damagePending{false};
static std::atomic<uint32_t> g_damager{0xFFFFFFFFu};
static std::atomic<uint8_t> g_damageWeapon{0};
static std::atomic<int64_t> g_lastDamageMs{0};
static std::atomic<int64_t> g_lastMeleeMs{0};
static std::atomic<bool> g_localFire{false};
static std::atomic<bool> g_localDuck{false};
// CPad::GetPad(0) NewState shorts. LeftStickX is +0, LeftStickY is +2
// (PlayerPedPort.cpp, CPad::GetPedWalkLeftRight / GetPedWalkUpDown).
static std::atomic<int> g_padStickX{0};
static std::atomic<int> g_padStickY{0};
static std::atomic<bool> g_padStickValid{false};
// CCamera::Orientation. CCamera::Process (0x4bbcc0) stores it at this+0x154.
// CTaskSimplePlayerOnFoot::PlayerControlZelda (0x655a28) subtracts that float
// from the stick angle. PC CCameraSA::GetCameraRotation reads VAR_CameraRotation.
static std::atomic<float> g_cameraRot{0.f};
static std::atomic<bool> g_cameraRotValid{false};
static TeleportFn g_teleport = nullptr;
static EndUserPauseFn g_endPause = nullptr;
static RemoveTopFn g_removeTop = nullptr;
static SwitchOffFn g_switchOff = nullptr;
static void* g_mobileMenu = nullptr;
using ScriptProcessFn = void (*)(void*);
static ScriptProcessFn g_originalScript = nullptr;
static std::atomic<float> g_spawnX{1754.f};
static std::atomic<float> g_spawnY{-1894.f};
static std::atomic<float> g_spawnZ{14.f};

static void AllowWrite(void* address, size_t size) {
    long page = sysconf(_SC_PAGESIZE);
    if (page <= 0) page = 4096;
    auto start = reinterpret_cast<uintptr_t>(address) & ~(static_cast<uintptr_t>(page) - 1);
    auto end = (reinterpret_cast<uintptr_t>(address) + size + static_cast<uintptr_t>(page) - 1)
        & ~(static_cast<uintptr_t>(page) - 1);
    mprotect(reinterpret_cast<void*>(start), end - start, PROT_READ | PROT_WRITE);
}

static void ResolvePlayerFns() {
    void* handle = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_GLOBAL);
    if (!handle) return;
    g_requestSpecial = reinterpret_cast<RequestSpecialModelFn>(
        dlsym(handle, "_ZN10CStreaming19RequestSpecialModelEiPKci"));
    g_loadAll = reinterpret_cast<LoadAllRequestedModelsFn>(
        dlsym(handle, "_ZN10CStreaming22LoadAllRequestedModelsEb"));
    g_setupPlayer = reinterpret_cast<SetupPlayerPedFn>(
        dlsym(handle, "_ZN10CPlayerPed14SetupPlayerPedEi"));
    g_findPlayer = reinterpret_cast<FindPlayerPedFn>(dlsym(handle, "_Z13FindPlayerPedi"));
    g_findCoors = reinterpret_cast<FindPlayerCoorsFn>(dlsym(handle, "_Z15FindPlayerCoorsi"));
    g_requestModel = reinterpret_cast<RequestModelFn>(dlsym(handle, "_ZN10CStreaming12RequestModelEii"));
    g_hasModel = reinterpret_cast<HasModelFn>(dlsym(handle, "_ZN10CStreaming14HasModelLoadedEi"));
    g_civilianCtor = reinterpret_cast<CivilianCtorFn>(dlsym(handle, "_ZN12CCivilianPedC1E8ePedTypej"));
    g_addPed = reinterpret_cast<AddPedFn>(dlsym(handle, "_ZN11CPopulation6AddPedE8ePedTypejRK7CVectorb"));
    g_worldAdd = reinterpret_cast<WorldAddFn>(dlsym(handle, "_ZN6CWorld3AddEP7CEntity"));
    g_setHeading = reinterpret_cast<SetHeadingFn>(dlsym(handle, "_ZN10CPlaceable10SetHeadingEf"));
    g_pedDensity = reinterpret_cast<float*>(dlsym(handle, "_ZN11CPopulation20PedDensityMultiplierE"));
    g_managePopulation = dlsym(handle, "_ZN11CPopulation16ManagePopulationEv");
    g_teleport = reinterpret_cast<TeleportFn>(dlsym(handle, "_ZN4CPed8TeleportE7CVectorh"));
    g_endPause = reinterpret_cast<EndUserPauseFn>(dlsym(handle, "_ZN6CTimer12EndUserPauseEv"));
    g_removeTop = reinterpret_cast<RemoveTopFn>(dlsym(handle, "_ZN10MobileMenu15RemoveTopScreenEbb"));
    g_switchOff = reinterpret_cast<SwitchOffFn>(dlsym(handle, "_Z20Menu_SwitchOffToGamev"));
}

static void PlacePlayer() {
    if (!g_findPlayer || !g_teleport) return;
    void* ped = g_findPlayer(0);
    if (!ped) return;
    Vec3 pos{g_spawnX.load(), g_spawnY.load(), g_spawnZ.load()};
    g_teleport(ped, pos, 0);
}

// Same replacement the PC client installs on CRunningScript::Process: the
// original is never called, so main.scm cannot start the intro. The first
// call only builds the local player. MobileSA reaches this by DoGameState
// case 8 (InitialiseGame, then gGameState = 9) and does not set MobileMenu
// +0x6d. That byte is the New Game flag: it sets +0x6c, and MainGameFunc
// then calls DoGameRestart, which is the single-player story.
static bool PlausiblePosition(float x, float y, float z) {
    return std::fabs(x) < 20000.f && std::fabs(y) < 20000.f && z > -200.f && z < 4000.f
        && !(x == 0.f && y == 0.f && z == 0.f);
}

// CPlaceable on 2.10: matrix pointer at +0x18 (position at matrix+0x30),
// otherwise the embedded transform at +0x08 and heading at +0x14.
static void SampleLocalTasks(void* ped);
static void SampleLocalPose() {
    if (!g_findPlayer) return;
    auto* ped = static_cast<unsigned char*>(g_findPlayer(0));
    if (!ped) return;
    float x = 0, y = 0, z = 0, heading = g_poseH.load(), radarHeading = g_radarH.load();
    auto* matrix = *reinterpret_cast<unsigned char**>(ped + 0x18);
    const auto matrixAddr = reinterpret_cast<uintptr_t>(matrix);
    if (matrix && (matrixAddr & 3u) == 0 && matrixAddr > 0x10000u) {
        x = *reinterpret_cast<float*>(matrix + 0x30);
        y = *reinterpret_cast<float*>(matrix + 0x34);
        z = *reinterpret_cast<float*>(matrix + 0x38);
        // Pure-sync still publishes this sample. The radar arrow does not use it:
        // row +0x20 is up, so its X is ~0 (east/west never turns the arrow) and
        // its Y is only pitch (north/south comes out backwards).
        heading = std::atan2(-*reinterpret_cast<float*>(matrix + 0x20),
                              *reinterpret_cast<float*>(matrix + 0x24));
        // FindPlayerHeading (libGTASA 2.10, 0x4f0720): ldp s0,s1,[matrix,#0x10];
        // fneg s0; atan2f. Forward row, atan2(-forward.x, forward.y). 0 is north,
        // east is -pi/2. Position stays +0x30/+0x34 (CPed::Teleport stores X, then Y).
        radarHeading = std::atan2(-*reinterpret_cast<float*>(matrix + 0x10),
                                   *reinterpret_cast<float*>(matrix + 0x14));
    } else {
        x = *reinterpret_cast<float*>(ped + 0x08);
        y = *reinterpret_cast<float*>(ped + 0x0C);
        z = *reinterpret_cast<float*>(ped + 0x10);
        heading = *reinterpret_cast<float*>(ped + 0x14);
        radarHeading = heading;
    }
    if (!PlausiblePosition(x, y, z)) return;
    auto* health = reinterpret_cast<float*>(ped + 0x6AC);
    // CPed armour on libGTASA 2.10 arm64, not the PC field at +0x548.
    // CWidgetPlayerInfo::RenderHealthBar (0x37d770) loads health with
    // `ldr s0, [ped, #0x6ac]`. RenderArmorBar (0x37d93c) loads armour from
    // that same ped pointer with `ldr s1, [ped, #0x6b4]`.
    // CPedDamageResponseCalculator::AccountForPedArmour (0x444044) reads and
    // writes `CPed*` (x1) at the same +0x6B4.
    auto* armor = reinterpret_cast<float*>(ped + 0x6B4);
    auto* speed = reinterpret_cast<float*>(ped + 0x68);
    if (g_healthOwned.load()) {
        const float owned = g_health.load();
        if (*health + 0.5f < owned) g_health = *health;
        else *health = owned;
    } else if (*health > 0.f && *health < 1000.f) {
        g_health = *health;
    }
    if (std::isfinite(*armor) && *armor >= 0.f && *armor < 1000.f) g_armor = *armor;
    g_velX = speed[0];
    g_velY = speed[1];
    g_velZ = speed[2];
    const float dx = x - g_poseX.load();
    const float dy = y - g_poseY.load();
    g_poseX = x;
    g_poseY = y;
    g_poseZ = z;
    g_poseH = heading;
    g_radarH = radarHeading;
    SampleLocalTasks(ped);
    if (dx * dx + dy * dy > 9.f) {
        static int reports = 0;
        if (reports < 8) {
            ++reports;
            ENGINE_LOGI("Local pose %.0f %.0f %.0f heading %.2f", x, y, z, heading);
        }
    }
}

struct RemotePed {
    float x = 0, y = 0, z = 0, heading = 0;
    float vx = 0, vy = 0, vz = 0;
    float shownX = 0, shownY = 0, shownZ = 0, shownH = 0;
    float health = 100.f;
    float armor = 0.f;
    float cameraRotation = 0.f;
    int weapon = 0;
    int wanderDir = 0;
    // Native task presentation — matches mta_neon SLastSyncedPedData fields.
    unsigned int nativeTaskLocomotionMode = 0;   // SNativeTaskLocomotionSync::eMode
    uint8_t nativeTaskWeaponType = 0;             // SNativeTaskWeaponPresentationSync weapon type
    uint16_t nativeTaskAnimGroup = 0;             // SNativeTaskAnimationPresentationSync group
    uint16_t nativeTaskAnimId = 0;                // SNativeTaskAnimationPresentationSync id
    unsigned int physicalTakeoverMode = 0;        // Physical takeover mode from PED_START_SYNC
    uint16_t physicalTakeoverAnimGroup = 0;
    uint16_t physicalTakeoverAnimId = 0;
    float physicalTakeoverProgress = 0.f;
    bool physicalTakeoverPending = false;
    bool dirty = false;
    bool shown = false;
    bool fire = false;
    bool inWater = false;
    bool duck = false;
    bool hosted = false;
    bool wander = false;
    bool animationKilled = false;
    bool healthLocked = false;
    bool armorLocked = false;
    // ENTITY_ADD interior. CClientPed::SetInterior writes the area code at CEntity+0x4B
    // (CClientPed.cpp 1101-1119). CColStore::RequestCollision takes the same value.
    uint8_t interior = 0;
    // Last keysync stick (PC units, -128..128). Drives CPlayerPed::ProcessControl
    // once the controller hook is installed. keysync is false for script peds.
    int stickX = 0;
    int stickY = 0;
    bool keysync = false;
    // ReadPlayerPuresync wrote SVelocitySync (including a zero vector). Applied once.
    bool syncMoveSpeed = false;
    // SPlayerHealthSync / SPlayerArmorSync decoded. Applied once to the body fields.
    bool syncHealth = false;
    bool syncArmor = false;
    // Server ped from the ENTITY_ADD actor list (kind 14).
    bool serverPed = false;
};
static std::mutex g_remoteMutex;
static std::unordered_map<uint32_t, RemotePed> g_remotePeds;
struct RemoteBody {
    void* ped = nullptr;
    std::unique_ptr<std::byte[]> owned;
    int weapon = -1;
    int wanderDir = -1;
    bool ducked = false;
    int64_t lastFight = 0;
    // CClientPed::ArmRemoteStreamInTransformFence (CClientPed.cpp 5914-5945) holds a remote
    // ped until collision is loaded. Owners use the same bIsStaticWaitingForCollision bit
    // in UpdateNativeCollisionAuthorityFence (7074-7108).
    bool collisionFence = false;
    int64_t fenceAtMs = 0;
    // m_remoteStreamInFencePreviousStaticWaitingForCollision (CClientPed.cpp 5934), restored on release (6038).
    bool fencePrevStaticWaiting = false;
    // Last area code written by ApplyPedInterior. Unset until the first write.
    bool interiorWritten = false;
    uint8_t interior = 0;
};
// Desktop CPacketHandler::Packet_EntityAdd (vehicle: SetRotationDegrees, SetHealth) and
// CUnoccupiedVehicleSync::Packet_UnoccupiedVehicleStartSync (CUnoccupiedVehicleSync.cpp 104-158: SetTargetPosition,
// SetTargetRotation, SetMoveSpeed, SetTurnSpeed, SetHealth). Rotation is in degrees, as on the wire.
struct CarSeed {
    bool pending = false;
    bool hasPose = false;
    float x = 0, y = 0, z = 0;
    bool hasRotation = false;
    float rotX = 0, rotY = 0, rotZ = 0;
    bool hasMotion = false;
    float move[3] = {0, 0, 0};
    float turn[3] = {0, 0, 0};
    bool hasHealth = false;
    float health = 0;
};
struct RemoteCar {
    void* body = nullptr;
    std::unique_ptr<std::byte[]> owned;
    int model = 0;
    float x = 0, y = 0, z = 0, heading = 0;
    float shownX = 0, shownY = 0, shownZ = 0, shownH = 0;
    bool shown = false;
    bool hosted = false;
    bool dirty = false;
    // Packet 33 driver block: move and turn speed, written to the body once per packet (motionPending, g_remoteMutex).
    float moveX = 0, moveY = 0, moveZ = 0;
    float turnX = 0, turnY = 0, turnZ = 0;
    bool motionPending = false;
    // Server state the body has to take once (ApplyCarSeed). Filled from the ENTITY_ADD actor (rotation, health)
    // and from VEHICLE_STARTSYNC (everything). Guarded by g_remoteMutex.
    CarSeed seed;
    bool actorSeeded = false;
};
static std::unordered_map<uint32_t, RemoteCar> g_remoteCars;
static std::vector<uint32_t> g_forget;
static std::atomic<bool> g_remoteResetPending{false};
static std::vector<HostedPed> g_hostedSnap;
static std::vector<HostedCar> g_hostedCarSnap;
static std::unordered_map<uint32_t, RemoteBody> g_remoteBodies;
// Phone body caps, kept apart the way desktop keeps the two streamers apart
// (CClientManager.cpp 27-28, Common.h 16-17). Desktop stops at 110 peds and 64 vehicles.
// The phone stays at 48 for each. CopyActors quotas use the same numbers, so a full car
// list cannot take the slots the ped bodies are drawn from.
static constexpr int kMaxRemotePedBodies = 48;
static constexpr int kMaxRemoteVehicleRecords = 48;
// Local player vehicle state and the queue for VEHICLE_INOUT. Both guarded by g_remoteMutex.
static constexpr uint32_t kNoVehicle = 0xFFFFFFFFu;
static uint32_t g_localVehicleId = kNoVehicle;
// Live sample of that car for packet 33 (CopyLocalCar). Written in WatchLocalVehicle, under g_remoteMutex.
static LocalCarState g_localCar;
static bool g_localCarValid = false;
static std::vector<LocalVehicleEvent> g_localVehicleEvents;
static constexpr size_t kMaxLocalVehicleEvents = 16;
// Local bullet-weapon shots (NoteLocalShot / ConsumeLocalShot). Guarded by g_remoteMutex.
static std::vector<LocalShot> g_localShots;
static constexpr size_t kMaxLocalShots = 32;

// HasModelLoaded is not exported. HasVehicleUpgradeLoaded (0x395098) reads
// ms_aInfoForModel[id] with stride 0x14 and treats the byte at +0x10 as loaded
// when it equals 1. A null function pointer used to fail this check forever.
static int StreamingLoadState(int model) {
    if (model < 0 || model > 19999) return -1;
    static unsigned char* table = nullptr;
    static bool looked = false;
    if (!looked) {
        looked = true;
        void* handle = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_GLOBAL);
        if (handle)
            table = static_cast<unsigned char*>(dlsym(handle, "_ZN10CStreaming16ms_aInfoForModelE"));
        if (!table) ENGINE_LOGI("ms_aInfoForModel missing; remote models stay unloaded");
    }
    if (!table) return g_hasModel && g_hasModel(model) ? 1 : -1;
    return table[static_cast<size_t>(model) * 0x14u + 0x10u];
}

static bool EnsureStreamedModel(int model, int limit) {
    if (model < 0 || model > limit) return false;
    if (StreamingLoadState(model) == 1) return true;
    if (g_requestModel) g_requestModel(model, 0x12);
    // A few priority flushes. Later ticks keep requesting until the state
    // flips, so a model that is still streaming is not dropped.
    static int pumps = 0;
    if (g_loadAll && pumps < 6 && StreamingLoadState(model) != 1) {
        ++pumps;
        g_loadAll(true);
    }
    return StreamingLoadState(model) == 1;
}

static bool EnsurePedModel(int model) {
    if (model == 0) return false;
    return EnsureStreamedModel(model, 312);
}

struct ClothesApi {
    bool resolved = false;
    void (*init)() = nullptr;
    void (*rebuild)(void*, bool) = nullptr;
    void (*setTex)(void*, const char*, const char*, int) = nullptr;
    void (*setStat)(unsigned short, float) = nullptr;
    void (*allocData)(void*) = nullptr;
};

static ClothesApi& Clothes() {
    static ClothesApi api;
    if (api.resolved) return api;
    api.resolved = true;
    void* handle = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_GLOBAL);
    if (!handle) return api;
    api.init = reinterpret_cast<void (*)()>(dlsym(handle, "_ZN8CClothes4InitEv"));
    api.rebuild = reinterpret_cast<void (*)(void*, bool)>(
        dlsym(handle, "_ZN8CClothes13RebuildPlayerEP10CPlayerPedb"));
    api.setTex = reinterpret_cast<void (*)(void*, const char*, const char*, int)>(
        dlsym(handle, "_ZN15CPedClothesDesc18SetTextureAndModelEPKcS1_i"));
    api.setStat = reinterpret_cast<void (*)(unsigned short, float)>(
        dlsym(handle, "_ZN6CStats12SetStatValueEtf"));
    api.allocData = reinterpret_cast<void (*)(void*)>(
        dlsym(handle, "_ZN14CPlayerPedData12AllocateDataEv"));
    if (api.init) api.init();
    return api;
}

static std::unordered_map<void*, std::unique_ptr<std::byte[]>> g_privatePlayerData;

static void* PlayerDataOf(void* ped) {
    if (!ped) return nullptr;
    void* data = *reinterpret_cast<void**>(static_cast<unsigned char*>(ped) + 0x540);
    const auto addr = reinterpret_cast<uintptr_t>(data);
    if (!data || (addr & 7u) != 0 || addr < 0x10000u) return nullptr;
    return data;
}

static void* EnsurePlayerData(void* ped, bool createData) {
    if (void* data = PlayerDataOf(ped)) return data;
    if (!createData) return nullptr;
    auto block = std::make_unique<std::byte[]>(0x80);
    std::memset(block.get(), 0, 0x80);
    *reinterpret_cast<void**>(static_cast<unsigned char*>(ped) + 0x540) = block.get();
    g_privatePlayerData[ped] = std::move(block);
    return PlayerDataOf(ped);
}

static void DressCj(void* ped, bool createData) {
    if (!ped) return;
    auto& api = Clothes();
    if (!api.rebuild || !api.setTex) {
        ENGINE_LOGI("CJ clothes missing rebuild=%p set=%p",
            reinterpret_cast<void*>(api.rebuild), reinterpret_cast<void*>(api.setTex));
        return;
    }
    if (api.setStat) {
        api.setStat(21, 0.f);
        api.setStat(23, 0.f);
    }
    void* data = EnsurePlayerData(ped, createData);
    if (data && api.allocData) api.allocData(data);
    void* desc = data ? *reinterpret_cast<void**>(static_cast<unsigned char*>(data) + 8) : nullptr;
    if (!desc) {
        ENGINE_LOGI("CJ clothes desc missing ped=%p", ped);
        return;
    }
    api.setTex(desc, "vestblack", "vest", 0);
    api.setTex(desc, "PLAYER_FACE", "HEAD", 1);
    api.setTex(desc, "JEANSDENIM", "JEANS", 2);
    api.setTex(desc, "SNEAKERBINCBLK", "SNEAKER", 3);
    if (g_requestSpecial) g_requestSpecial(0, "player", 26);
    if (g_loadAll) g_loadAll(true);
    api.rebuild(ped, false);
    if (g_loadAll) g_loadAll(true);
    ENGINE_LOGI("CJ dressed ped=%p desc=%p", ped, desc);
}

static void DressLocalPlayer() {
    if (!g_findPlayer) return;
    DressCj(g_findPlayer(0), false);
}

static void ArmRemote(void* body, RemoteBody& stored, int weapon) {
    if (!body || weapon <= 0 || stored.weapon == weapon) return;
    using GiveWeaponFn = void (*)(void*, int, unsigned, bool);
    static GiveWeaponFn giveWeapon = nullptr;
    static bool looked = false;
    if (!looked) {
        looked = true;
        void* handle = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_GLOBAL);
        if (handle)
            giveWeapon = reinterpret_cast<GiveWeaponFn>(dlsym(handle, "_ZN4CPed10GiveWeaponE11eWeaponTypejb"));
    }
    if (!giveWeapon) return;
    giveWeapon(body, weapon, 60u, true);
    stored.weapon = weapon;
}

struct TaskFns {
    bool ready = false;
    void* (*taskNew)(unsigned long) = nullptr;
    void (*wanderCtor)(void*, int, unsigned char, bool, float) = nullptr;
    void (*addPrimary)(void*, void*, bool) = nullptr;
    void (*setSecondary)(void*, void*, int) = nullptr;
    void* (*getSecondary)(void*, int) = nullptr;
    void (*duck)(void*, unsigned short) = nullptr;
    void (*clearDuck)(void*) = nullptr;
    void (*clearTasks)(void*, bool, bool) = nullptr;
    void (*fightCtor)(void*, void*, int, unsigned) = nullptr;
    void (*setMoveState)(void*, int) = nullptr;
    void (*setMoveAnim)(void*) = nullptr;
    void (*worldRemove)(void*) = nullptr;
    void (*carCtor)(void*, int, unsigned char, unsigned char) = nullptr;
    void (*carDtor)(void*) = nullptr;
    // CCarCtrl::CreateCarForScript(int, CVector, unsigned char). Vec3 has the same layout as CVector (3 floats).
    void* (*createCarForScript)(int, Vec3, unsigned char) = nullptr;
    // CVehicle* FindPlayerVehicle(int playerId, bool bIncludeRemote). playerId -1 is the local player.
    void* (*findPlayerVehicle)(int, bool) = nullptr;
    // CPad (libGTASA 2.10). GetPad(0) is the local pad; the getters are CPad members, so they take the pad.
    void* (*getPad)(int) = nullptr;
    int (*padSteering)(void*) = nullptr;       // CPad::GetSteeringLeftRight, -128..128
    int (*padAccelerate)(void*) = nullptr;     // CPad::GetAccelerate, 0..255
    int (*padBrake)(void*) = nullptr;          // CPad::GetBrake, 0..255
    int (*padHandBrake)(void*) = nullptr;      // CPad::GetHandBrake, nonzero when pressed
    bool (*padHorn)(void*, bool) = nullptr;    // CPad::GetHorn(bool), the bool is unused on libGTASA 2.10
    bool (*padLookLeft)(void*, bool) = nullptr;   // CPad::GetLookLeft(bool), true includes the touch look widget
    bool (*padLookRight)(void*, bool) = nullptr;  // CPad::GetLookRight(bool), same
    void* wanderVtable = nullptr;
};

static TaskFns& Tasks() {
    static TaskFns api;
    if (api.ready) return api;
    api.ready = true;
    void* handle = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_GLOBAL);
    if (!handle) return api;
    api.taskNew = reinterpret_cast<void* (*)(unsigned long)>(dlsym(handle, "_ZN5CTasknwEm"));
    api.wanderCtor = reinterpret_cast<void (*)(void*, int, unsigned char, bool, float)>(
        dlsym(handle, "_ZN18CTaskComplexWanderC2Eihbf"));
    api.addPrimary = reinterpret_cast<void (*)(void*, void*, bool)>(
        dlsym(handle, "_ZN16CPedIntelligence26AddTaskPrimaryMaybeInGroupEP5CTaskb"));
    api.setSecondary = reinterpret_cast<void (*)(void*, void*, int)>(
        dlsym(handle, "_ZN12CTaskManager16SetTaskSecondaryEP5CTaski"));
    api.getSecondary = reinterpret_cast<void* (*)(void*, int)>(
        dlsym(handle, "_ZNK12CTaskManager16GetTaskSecondaryEi"));
    api.duck = reinterpret_cast<void (*)(void*, unsigned short)>(
        dlsym(handle, "_ZN16CPedIntelligence20SetTaskDuckSecondaryEt"));
    api.clearDuck = reinterpret_cast<void (*)(void*)>(
        dlsym(handle, "_ZN16CPedIntelligence22ClearTaskDuckSecondaryEv"));
    api.clearTasks = reinterpret_cast<void (*)(void*, bool, bool)>(
        dlsym(handle, "_ZN16CPedIntelligence10ClearTasksEbb"));
    api.fightCtor = reinterpret_cast<void (*)(void*, void*, int, unsigned)>(
        dlsym(handle, "_ZN16CTaskSimpleFightC1EP7CEntityij"));
    api.setMoveState = reinterpret_cast<void (*)(void*, int)>(dlsym(handle, "_ZN4CPed12SetMoveStateE10eMoveState"));
    api.setMoveAnim = reinterpret_cast<void (*)(void*)>(dlsym(handle, "_ZN4CPed11SetMoveAnimEv"));
    api.worldRemove = reinterpret_cast<void (*)(void*)>(dlsym(handle, "_ZN6CWorld6RemoveEP7CEntity"));
    api.carCtor = reinterpret_cast<void (*)(void*, int, unsigned char, unsigned char)>(
        dlsym(handle, "_ZN11CAutomobileC1Eihh"));
    api.carDtor = reinterpret_cast<void (*)(void*)>(dlsym(handle, "_ZN11CAutomobileD1Ev"));
    api.createCarForScript = reinterpret_cast<void* (*)(int, Vec3, unsigned char)>(
        dlsym(handle, "_ZN8CCarCtrl18CreateCarForScriptEi7CVectorh"));
    api.findPlayerVehicle = reinterpret_cast<void* (*)(int, bool)>(dlsym(handle, "_Z17FindPlayerVehicleib"));
    api.getPad = reinterpret_cast<void* (*)(int)>(dlsym(handle, "_ZN4CPad6GetPadEi"));
    api.padSteering = reinterpret_cast<int (*)(void*)>(dlsym(handle, "_ZN4CPad20GetSteeringLeftRightEv"));
    api.padAccelerate = reinterpret_cast<int (*)(void*)>(dlsym(handle, "_ZN4CPad13GetAccelerateEv"));
    api.padBrake = reinterpret_cast<int (*)(void*)>(dlsym(handle, "_ZN4CPad8GetBrakeEv"));
    api.padHandBrake = reinterpret_cast<int (*)(void*)>(dlsym(handle, "_ZN4CPad12GetHandBrakeEv"));
    api.padHorn = reinterpret_cast<bool (*)(void*, bool)>(dlsym(handle, "_ZN4CPad7GetHornEb"));
    api.padLookLeft = reinterpret_cast<bool (*)(void*, bool)>(dlsym(handle, "_ZN4CPad11GetLookLeftEb"));
    api.padLookRight = reinterpret_cast<bool (*)(void*, bool)>(dlsym(handle, "_ZN4CPad12GetLookRightEb"));
    api.wanderVtable = dlsym(handle, "_ZTV26CTaskComplexWanderStandard");
    return api;
}

// Driver controls for LocalCarState, read from the local pad on the game thread. A missing getter or pad leaves
// its field at zero. The pad is the same CPad::GetPad(0) that PlayerPedPort.cpp uses.
static void SamplePadControls(const TaskFns& api, LocalCarState& c) {
    if (!api.getPad) return;
    void* pad = api.getPad(0);
    if (!pad) return;
    if (api.padSteering) c.steering = static_cast<int16_t>(std::clamp(api.padSteering(pad), -128, 128));
    if (api.padAccelerate) c.accelerate = static_cast<uint8_t>(std::clamp(api.padAccelerate(pad), 0, 255));
    if (api.padBrake) c.brake = static_cast<uint8_t>(std::clamp(api.padBrake(pad), 0, 255));
    if (api.padHandBrake) c.handBrake = api.padHandBrake(pad) != 0;
    if (api.padHorn) c.horn = api.padHorn(pad, false);
    if (api.padLookLeft) c.lookLeft = api.padLookLeft(pad, true);
    if (api.padLookRight) c.lookRight = api.padLookRight(pad, true);
}

// On-foot stick and camera, sampled on the game thread for SendPureSync / SendKeySync.
// TheCamera is the verified global (GTASAOffsets.h, nm "TheCamera" at 0xBBA8D0).
// Orientation is +0x154 (CCamera::Process store at 0x4bbcc0; PlayerControlZelda load at 0x655a28).
static void SampleLocalPadAndCamera() {
    TaskFns& api = Tasks();
    if (!api.getPad) {
        g_padStickValid = false;
        static bool loggedPad = false;
        if (!loggedPad) {
            loggedPad = true;
            ENGINE_LOGI("DIAG on-foot stick: CPad::GetPad (_ZN4CPad6GetPadEi) is missing; "
                        "CopyOnFoot still builds the stick from move speed");
        }
    } else {
        void* pad = api.getPad(0);
        if (!pad) {
            g_padStickValid = false;
        } else {
            int16_t sx = 0, sy = 0;
            std::memcpy(&sx, static_cast<unsigned char*>(pad) + 0, sizeof sx);
            std::memcpy(&sy, static_cast<unsigned char*>(pad) + 2, sizeof sy);
            g_padStickX = sx;
            g_padStickY = sy;
            g_padStickValid = true;
        }
    }
    static uintptr_t base = 0;
    static bool looked = false;
    if (!looked) {
        looked = true;
        base = Hook::MemoryPatcher::getLibraryBaseAddress("libGTASA.so");
    }
    if (!base) {
        g_cameraRotValid = false;
        static bool loggedCam = false;
        if (!loggedCam) {
            loggedCam = true;
            ENGINE_LOGI("DIAG GetCameraRotation: libGTASA base missing, TheCamera+0x154 "
                        "(CCameraSA::GetCameraRotation, CCameraSA.cpp 651-654) is not read");
        }
        return;
    }
    const float orient = *reinterpret_cast<const float*>(base + 0xBBA8D0 + 0x154);
    if (!std::isfinite(orient)) {
        g_cameraRotValid = false;
        return;
    }
    g_cameraRot = orient;
    g_cameraRotValid = true;
}

static void* PedIntel(void* ped) {
    if (!ped) return nullptr;
    auto* raw = static_cast<unsigned char*>(ped);
    void* intel = *reinterpret_cast<void**>(raw + 0x538);
    const auto addr = reinterpret_cast<uintptr_t>(intel);
    if (!intel || (addr & 3u) != 0 || addr < 0x10000u) return nullptr;
    return intel;
}

static void SampleLocalTasks(void* ped) {
    TaskFns& api = Tasks();
    void* intel = PedIntel(ped);
    if (!intel || !api.getSecondary) {
        g_localFire = false;
        g_localDuck = false;
        return;
    }
    void* manager = static_cast<unsigned char*>(intel) + 8;
    g_localFire = api.getSecondary(manager, 0) != nullptr;
    g_localDuck = api.getSecondary(manager, 1) != nullptr;
}

static void StartWander(void* body, int direction) {
    TaskFns& api = Tasks();
    void* intel = PedIntel(body);
    if (!intel || !api.taskNew || !api.wanderCtor || !api.addPrimary) return;
    void* task = api.taskNew(0x40);
    if (!task) return;
    std::memset(task, 0, 0x40);
    const unsigned char dir = static_cast<unsigned char>(std::clamp(direction, 0, 7));
    api.wanderCtor(task, 4, dir, true, 0.f);
    if (api.wanderVtable)
        *reinterpret_cast<void**>(task) = static_cast<unsigned char*>(api.wanderVtable) + 16;
    api.addPrimary(intel, task, false);
}

static void PlayPunch(void* body) {
    TaskFns& api = Tasks();
    void* intel = PedIntel(body);
    if (!intel || !api.taskNew || !api.fightCtor || !api.setSecondary) return;
    void* task = api.taskNew(0x80);
    if (!task) return;
    std::memset(task, 0, 0x80);
    void* target = g_findPlayer ? g_findPlayer(0) : nullptr;
    api.fightCtor(task, target, 0, 0u);
    api.setSecondary(static_cast<unsigned char*>(intel) + 8, task, 0);
}

static void PresentOnFoot(void* body, RemoteBody& stored, const RemotePed& ped) {
    TaskFns& api = Tasks();
    const float speed = std::sqrt(ped.vx * ped.vx + ped.vy * ped.vy);
    // A zero packet speed must not force PEDMOVE_STILL every tick. Desktop
    // SetMoveSpeed runs only when the ped-sync velocity flag is set
    // (CPedSync.cpp 434-437). Stamping still here froze the stand and the walk.
    if (api.setMoveState && api.setMoveAnim && speed > 0.03f) {
        const int state = speed > 0.14f ? 6 : 4;
        api.setMoveState(body, state);
        api.setMoveAnim(body);
    }
    void* intel = PedIntel(body);
    if (!intel) return;
    if (ped.duck && !stored.ducked && api.duck) api.duck(intel, 0);
    if (!ped.duck && stored.ducked && api.clearDuck) api.clearDuck(intel);
    stored.ducked = ped.duck;
    if (!ped.fire) return;
    const int64_t now = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    if (now - stored.lastFight < 650) return;
    stored.lastFight = now;
    PlayPunch(body);
}

static bool ReadBodyPos(void* body, float& x, float& y, float& z, float& heading) {
    auto* raw = static_cast<unsigned char*>(body);
    auto* matrix = *reinterpret_cast<unsigned char**>(raw + 0x18);
    const auto matrixAddr = reinterpret_cast<uintptr_t>(matrix);
    if (!matrix || (matrixAddr & 3u) != 0 || matrixAddr < 0x10000u) return false;
    x = *reinterpret_cast<float*>(matrix + 0x30);
    y = *reinterpret_cast<float*>(matrix + 0x34);
    z = *reinterpret_cast<float*>(matrix + 0x38);
    // FindPlayerHeading (libGTASA 2.10, 0x4f0720) reads the forward row at +0x10.
    // +0x20 is up. CMatrix::SetRotateZOnly stores (0, 0, 1) there, so a heading
    // taken from that row is 0 for an upright ped.
    heading = std::atan2(-*reinterpret_cast<float*>(matrix + 0x10), *reinterpret_cast<float*>(matrix + 0x14));
    return PlausiblePosition(x, y, z);
}

static void WriteBodyPos(void* body, float x, float y, float z) {
    auto* raw = static_cast<unsigned char*>(body);
    auto* matrix = *reinterpret_cast<unsigned char**>(raw + 0x18);
    const auto matrixAddr = reinterpret_cast<uintptr_t>(matrix);
    if (matrix && (matrixAddr & 3u) == 0 && matrixAddr > 0x10000u) {
        *reinterpret_cast<float*>(matrix + 0x30) = x;
        *reinterpret_cast<float*>(matrix + 0x34) = y;
        *reinterpret_cast<float*>(matrix + 0x38) = z;
    }
    *reinterpret_cast<float*>(raw + 0x08) = x;
    *reinterpret_cast<float*>(raw + 0x0C) = y;
    *reinterpret_cast<float*>(raw + 0x10) = z;
}

// Desktop CClientVehicle::SetRotationDegrees. The matrix rows are those of CMatrix::SetRotation (Shared/sdk/CMatrix.h
// 253-279), the inverse of the CMatrix::GetRotation that SampleHostedCar reads back, and the current row lengths
// (scale) are kept, as SetRotation does. Rows: right (+0x00), forward (+0x10), up (+0x20), as in SampleHostedCar.
static bool WriteBodyRotation(void* body, float rotXDeg, float rotYDeg, float rotZDeg) {
    auto* raw = static_cast<unsigned char*>(body);
    auto* matrix = *reinterpret_cast<unsigned char**>(raw + 0x18);
    const auto matrixAddr = reinterpret_cast<uintptr_t>(matrix);
    if (!matrix || (matrixAddr & 3u) != 0 || matrixAddr < 0x10000u) return false;
    if (!std::isfinite(rotXDeg) || !std::isfinite(rotYDeg) || !std::isfinite(rotZDeg)) return false;
    constexpr float kDegToRad = 3.14159265f / 180.f;
    const float fCosX = std::cos(-rotXDeg * kDegToRad);
    const float fCosY = std::cos(-rotYDeg * kDegToRad);
    const float fCosZ = std::cos(rotZDeg * kDegToRad);
    const float fSinX = std::sin(-rotXDeg * kDegToRad);
    const float fSinY = std::sin(-rotYDeg * kDegToRad);
    const float fSinZ = std::sin(rotZDeg * kDegToRad);
    auto scaleOf = [matrix](unsigned offset) {
        const auto* v = reinterpret_cast<const float*>(matrix + offset);
        const float len = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        return (len > 1e-6f && std::isfinite(len)) ? len : 1.f;
    };
    const float sx = scaleOf(0x00), sy = scaleOf(0x10), sz = scaleOf(0x20);
    auto* right = reinterpret_cast<float*>(matrix + 0x00);
    auto* front = reinterpret_cast<float*>(matrix + 0x10);
    auto* up = reinterpret_cast<float*>(matrix + 0x20);
    right[0] = fCosY * fCosZ * sx;
    right[1] = fCosY * fSinZ * sx;
    right[2] = fSinY * sx;
    front[0] = (fSinX * fSinY * fCosZ - fCosX * fSinZ) * sy;
    front[1] = (fSinX * fSinY * fSinZ + fCosX * fCosZ) * sy;
    front[2] = -fSinX * fCosY * sy;
    up[0] = -(fCosX * fSinY * fCosZ + fSinX * fSinZ) * sz;
    up[1] = (fCosZ * fSinX - fCosX * fSinY * fSinZ) * sz;
    up[2] = fCosX * fCosY * sz;
    return true;
}

// Desktop CClientVehicle::SetHealth. CVehicle::m_fHealth is at +0x634 (see SampleHostedCar). The wire range of
// SVehicleHealthSync is 0 - 2047.5.
static void WriteBodyHealth(void* body, float health) {
    if (!std::isfinite(health) || health < 0.f || health > 2047.5f) return;
    *reinterpret_cast<float*>(static_cast<unsigned char*>(body) + 0x634) = health;
}

// Live values for packet 56, read on the game thread like the ped snapshot. Desktop
// CUnoccupiedVehicleSync::WriteVehicleInformation uses CClientVehicle::GetPosition, GetRotationDegrees,
// GetMoveSpeed and GetTurnSpeed. GetRotationDegrees (mta_neon_source/Client/mods/deathmatch/logic/
// CClientVehicle.cpp 464) is 2*PI - ConvertMatrixToEulerAngles, wrapped to [0, 360). That equals the
// negated x and y of CMatrix::GetRotation (Shared/sdk/CMatrix.h 236, "0 - ConvertToEulerAngles") modulo
// 360. As in GetRotationMatrix (CMatrix.h 200), each matrix row is divided by its length, so scale is
// ignored. Health is CVehicle::m_fHealth at +0x634: CVehicle::InflictDamage (0x6a7848) does
// `health -= damage` on [this, #1588] and CAutomobile::BlowUpCar (0x67d3ac) stores 0 there.
static float WrapDegrees360(float value) {
    return value - 360.f * std::floor(value / 360.f);
}

static bool SampleHostedCar(void* body, HostedCar& out) {
    float x = 0, y = 0, z = 0, heading = 0;
    if (!ReadBodyPos(body, x, y, z, heading)) return false;
    auto* raw = static_cast<unsigned char*>(body);
    auto* matrix = *reinterpret_cast<unsigned char**>(raw + 0x18);
    // Rows: right (+0x00), forward (+0x10), up (+0x20). Position (+0x30) came from ReadBodyPos.
    auto unitRow = [matrix](unsigned offset, float (&row)[3]) {
        const auto* v = reinterpret_cast<const float*>(matrix + offset);
        const float len = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        if (!(len > 1e-6f) || !std::isfinite(len)) return false;
        row[0] = v[0] / len;
        row[1] = v[1] / len;
        row[2] = v[2] / len;
        return true;
    };
    float right[3] = {}, front[3] = {}, up[3] = {};
    if (!unitRow(0x00, right) || !unitRow(0x10, front) || !unitRow(0x20, up)) return false;

    // CMatrix::GetRotation (CMatrix.h 236): (-fRotX, -fRotY, fRotZ) in radians.
    const float fRotY = std::atan2(right[2], std::sqrt(right[0] * right[0] + right[1] * right[1]));
    const float fRotZ = std::atan2(right[1], right[0]);
    const float fSinZ = -std::sin(fRotZ);
    const float fCosZ = std::cos(fRotZ);
    const float fRotX = std::atan2(up[0] * fSinZ + up[1] * fCosZ, front[0] * fSinZ + front[1] * fCosZ);
    constexpr float kRadToDeg = 180.f / 3.14159265f;

    out.x = x;
    out.y = y;
    out.z = z;
    out.rotX = WrapDegrees360(-fRotX * kRadToDeg);
    out.rotY = WrapDegrees360(-fRotY * kRadToDeg);
    out.rotZ = WrapDegrees360(fRotZ * kRadToDeg);

    // CPhysical move speed (+0x68) and turn speed (+0x74), world units per frame.
    const auto* move = reinterpret_cast<const float*>(raw + 0x68);
    const auto* turn = reinterpret_cast<const float*>(raw + 0x74);
    out.vx = move[0];
    out.vy = move[1];
    out.vz = move[2];
    out.tx = turn[0];
    out.ty = turn[1];
    out.tz = turn[2];
    out.health = *reinterpret_cast<const float*>(raw + 0x634);

    const float values[] = {out.health, out.x, out.y, out.z, out.rotX, out.rotY, out.rotZ, out.vx, out.vy, out.vz, out.tx, out.ty, out.tz};
    for (float v : values) {
        if (!std::isfinite(v)) return false;
    }
    return true;
}

// libGTASA 2.10 arm64. CPlaceable::CPlaceable (0x4ebd58) stores _ZTV10CPlaceable+0x10
// (Itanium: the object vptr skips the offset-to-top and typeinfo). Desktop
// IsPlaceableVTBL compares against that pointer (CPlaceableSA.h:29, 0x863C40 on x86).
static bool IsPlaceableVtable(void* body) {
    static void* placeable = nullptr;
    static bool looked = false;
    if (!looked) {
        looked = true;
        void* handle = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_GLOBAL);
        auto* symbol = handle ? static_cast<unsigned char*>(dlsym(handle, "_ZTV10CPlaceable")) : nullptr;
        if (symbol) placeable = symbol + 16;
    }
    if (!placeable || !body) return false;
    return *reinterpret_cast<void**>(body) == placeable;
}

static void DropBody(void* body, bool car, bool pooled = false) {
    if (!body) return;
    // Desktop CVehicleSA::~CVehicleSA (CVehicleSA.cpp:400-438) and
    // CPlayerPedSA::~CPlayerPedSA (CPlayerPedSA.cpp:141-148) skip CWorld::Remove
    // and the game destructor once the interface vtable is CPlaceable: the game
    // has already destroyed it. Remove's first virtual call is vtable+0x20
    // (CWorld::Remove at 0x5073b4). Past CPlaceable's two slots that word is
    // MI_CRANE_HARNESS (0x8862d6), and the branch SIGBUSes.
    // Device: 2026-11-13 18:46:58, libGTASA base 0x6cfae14000, fault
    // 0x6cfb69a2d6 = base+0x8862d6, from DropCarLocked.
    if (IsPlaceableVtable(body)) {
        ENGINE_LOGI("DIAG drop skipped: body %p vtable is CPlaceable", body);
        return;
    }
    TaskFns& api = Tasks();
    if (api.worldRemove) api.worldRemove(body);
    if (!car) return;
    if (pooled) {
        // Pool-owned vehicle (CreateCarForScript): call the deleting destructor (Itanium ABI vtable slot 1)
        // so the pool slot is released, like the desktop's delete.
        auto** vtable = *reinterpret_cast<void***>(body);
        reinterpret_cast<void (*)(void*)>(vtable[1])(body);
    } else if (api.carDtor) {
        api.carDtor(body);
    }
}

static bool EnsureWorldModel(int model) {
    return EnsureStreamedModel(model, 611);
}

static int g_gameFrame = 0;

static void NoteGameFrame() {
    static unsigned* counter = nullptr;
    static bool looked = false;
    if (!looked) {
        looked = true;
        void* handle = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_GLOBAL);
        if (handle)
            counter = static_cast<unsigned*>(dlsym(handle, "_ZN6CTimer14m_FrameCounterE"));
    }
    if (counter) g_gameFrame = static_cast<int>(*counter);
    else ++g_gameFrame;
}

static bool PedModelSettled(int model) {
    static std::unordered_map<int, int> readyAt;
    if (StreamingLoadState(model) != 1) {
        readyAt.erase(model);
        EnsurePedModel(model);
        return false;
    }
    auto it = readyAt.find(model);
    if (it == readyAt.end() || g_gameFrame < it->second) {
        if (g_loadAll) g_loadAll(true);
        readyAt[model] = g_gameFrame;
        return false;
    }
    return g_gameFrame - it->second >= 3;
}
struct GroundApi {
    bool looked = false;
    void (*setState)(void*, int) = nullptr;
    float (*findGround)(float, float, float, bool*, void**) = nullptr;
    void (*standCtor)(void*, int, bool, bool, float) = nullptr;
};

static GroundApi& Ground() {
    static GroundApi api;
    if (api.looked) return api;
    api.looked = true;
    void* handle = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_GLOBAL);
    if (!handle) return api;
    api.setState = reinterpret_cast<void (*)(void*, int)>(
        dlsym(handle, "_ZN4CPed11SetPedStateE9ePedState"));
    api.findGround = reinterpret_cast<float (*)(float, float, float, bool*, void**)>(
        dlsym(handle, "_ZN6CWorld21FindGroundZFor3DCoordEfffPbPP7CEntity"));
    api.standCtor = reinterpret_cast<void (*)(void*, int, bool, bool, float)>(
        dlsym(handle, "_ZN21CTaskSimpleStandStillC1Eibbf"));
    return api;
}

// CClientPed::ArmRemoteStreamInTransformFence (CClientPed.cpp 5914-5945) and
// ClearRemoteStreamInTransformFence release (5990-5996). Minimum 100 ms, timeout
// 1500 ms (CClientPed.cpp 62-63). The entity flag bit is bIsStaticWaitingForCollision
// (CEntitySA.h 131). CEntity::SetIsStatic (libGTASA 0x33ad14) writes bit 2 of the
// flag word at +0x28, so bit 18 of that word is the same field.
struct ColStoreApi {
    bool looked = false;
    void (*request)(const Vec3*, int) = nullptr;
    bool (*loaded)(const Vec3*, int) = nullptr;
};

static ColStoreApi& ColStore() {
    static ColStoreApi api;
    if (api.looked) return api;
    api.looked = true;
    void* handle = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_GLOBAL);
    if (!handle) return api;
    api.request = reinterpret_cast<void (*)(const Vec3*, int)>(
        dlsym(handle, "_ZN9CColStore16RequestCollisionERK7CVectori"));
    api.loaded = reinterpret_cast<bool (*)(const Vec3*, int)>(
        dlsym(handle, "_ZN9CColStore18HasCollisionLoadedERK7CVectori"));
    return api;
}

static void SetStaticWaitingForCollision(void* body, bool waiting) {
    if (!body) return;
    auto* flags = reinterpret_cast<uint64_t*>(static_cast<unsigned char*>(body) + 0x28);
    if (waiting) *flags |= (1ull << 18);
    else *flags &= ~(1ull << 18);
}

// CEntitySA::IsStaticWaitingForCollision (CEntitySA.h 304). Same bit as the store above.
// CPhysical::AddToMovingList (0x4e0bd4) loads it as ldrb [x0, #0x2a], tbnz #2.
static bool IsStaticWaitingForCollision(void* body) {
    if (!body) return false;
    return (*reinterpret_cast<uint64_t*>(static_cast<unsigned char*>(body) + 0x28) & (1ull << 18)) != 0;
}

// CEntitySA::IsStatic (CEntitySA.h 289, bIsStatic is bit 2 of the flag word).
// CEntity::SetIsStatic (0x33ad14) stores it: ldr x8, [x0, #0x28]; bfi x8, x9, #2, #1; str x8, [x0, #0x28].
static bool IsEntityStatic(void* body) {
    if (!body) return false;
    return (*reinterpret_cast<uint64_t*>(static_cast<unsigned char*>(body) + 0x28) & (1ull << 2)) != 0;
}

// The moving-list node of a CPhysical. CPhysical::AddToMovingList (0x4e0bcc) loads it with
// ldr x8, [x19, #0xe0] and returns when it is set; it stores the new node there at 0x4e0be8.
static bool OnMovingList(void* body) {
    if (!body) return false;
    return *reinterpret_cast<void**>(static_cast<unsigned char*>(body) + 0xE0) != nullptr;
}

// CPhysicalSA::AddToMovingList (CPhysicalSA.cpp 244). libGTASA 2.10 0x4e0bbc returns early when the
// node is already set or the ped is still static-waiting, so a second call is a no-op.
static void AddToMovingList(void* body) {
    using Fn = void (*)(void*);
    static Fn fn = nullptr;
    static bool looked = false;
    if (!looked) {
        looked = true;
        if (void* handle = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_GLOBAL))
            fn = reinterpret_cast<Fn>(dlsym(handle, "_ZN9CPhysical15AddToMovingListEv"));
        if (!fn) ENGINE_LOGI("DIAG CPhysical::AddToMovingList missing; fenced peds stay off the moving list");
    }
    if (fn && body) fn(body);
}

// CPhysicalSA::SetFrozen (CPhysicalSA.cpp 389-396).
// libGTASA 2.10 stores the physical flag word at CPhysical+0x64
// (CPhysical::CPhysical writes it at 0x4e04d8). CPhysical::ApplyMoveSpeed (0x4e1104)
// skips integration when bit 5 or bit 13 is set (mask 0x2020). Bit 13 is
// bDontApplySpeed and bit 2 is bDisableFriction (CPhysicalSA.h 42-55).
// Mass is the float at +0xB0: ApplyMoveForce (0x4e0df0) divides by it, and the
// constructor stores q0 there (0x4e04a0). PHYSICAL_MAXMASS is 99999.
static void SetPhysicalFrozen(void* body, bool frozen) {
    if (!body) return;
    auto* raw = static_cast<unsigned char*>(body);
    auto* flags = reinterpret_cast<uint32_t*>(raw + 0x64);
    const float mass = *reinterpret_cast<float*>(raw + 0xB0);
    constexpr uint32_t kDontApplySpeed = 1u << 13;
    constexpr uint32_t kDisableFriction = 1u << 2;
    constexpr float kMaxMass = 99999.f;
    if (frozen) *flags |= kDontApplySpeed;
    else *flags &= ~kDontApplySpeed;
    if (frozen || mass >= kMaxMass) *flags |= kDisableFriction;
    else *flags &= ~kDisableFriction;
}

// CClientPed::SetInterior (CClientPed.cpp 1101-1119) writes CEntity::SetAreaCode.
// CEntryExitManager::SetAreaCodeForVisibleObjects reads and writes the byte at
// +0x4B (0x3cc420, 0x3cc43c). CTaskSimpleWaitUntilAreaCodesMatch::ProcessPed
// compares the same byte (0x5f938c).
static void ApplyPedInterior(void* body, RemoteBody& stored, uint8_t interior) {
    if (!body) return;
    const bool changed = !stored.interiorWritten || stored.interior != interior;
    static_cast<unsigned char*>(body)[0x4B] = interior;
    if (!changed) return;
    const unsigned previous = stored.interiorWritten ? stored.interior : 0u;
    stored.interiorWritten = true;
    stored.interior = interior;
    if (previous == interior) return;
    static int noted = 0;
    if (noted < 4) {
        ++noted;
        // CClientEntity::SetInterior also fires onClientElementInteriorChange
        // (CClientEntity.cpp 1332-1338). The local-player branch calls
        // CWorld::SetCurrentArea (CClientPed.cpp 1109-1116) through CGame::currArea.
        ENGINE_LOGI("DIAG CClientPed::SetInterior wrote area %u (was %u) at CEntity+0x4B. "
                    "onClientElementInteriorChange is not fired (CClientEntity::SetInterior, CClientEntity.cpp 1332-1338). "
                    "CWorld::SetCurrentArea is the local-player branch (CClientPed.cpp 1109-1116).",
                    static_cast<unsigned>(interior), previous);
    }
}

static int64_t GameNowMs() {
    return static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
}

// Holds the ped at the network pose until CColStore::HasCollisionLoaded, or 1500 ms.
// Returns true while the fence is still holding.
static bool HoldPedCollisionFence(void* body, RemoteBody& stored, RemotePed& ped) {
    if (!stored.collisionFence) return false;
    constexpr int64_t kMinMs = 100;
    constexpr int64_t kTimeoutMs = 1500;
    const int64_t elapsed = GameNowMs() - stored.fenceAtMs;
    Vec3 pos{ped.x, ped.y, ped.z};
    const bool loaded = ColStore().loaded && ColStore().loaded(&pos, ped.interior);
    if (elapsed >= kTimeoutMs || (elapsed >= kMinMs && loaded)) {
        // ClearRemoteStreamInTransformFence (CClientPed.cpp 6038): restore the flag saved at arm.
        SetStaticWaitingForCollision(body, stored.fencePrevStaticWaiting);
        // ClearRemoteStreamInTransformFence calls ApplyPhysicalFreezeState while
        // the fence flag is still set (CClientPed.cpp 6038-6048), so that call
        // leaves bDontApplySpeed on. The next StreamedInPulse owns the bit through
        // UpdateRemoteReplicaPhysicsFence (CClientPed.cpp 7140-7152), which is not
        // ported. This fence also covers a hosted ped; desktop ArmRemoteStreamInTransformFence
        // returns first when m_bIsSyncing (CClientPed.cpp 5917). Clearing the bit
        // here lets that ped move after the hold.
        SetPhysicalFrozen(body, false);
        // CClientPed.cpp 6040-6046: "GTA's stock mission collision gate puts a physical back on the moving list
        // after clearing bStaticWaitingForCollision ... a required resume when construction left it out of the list."
        // Without this the body is never processed after the hold (ROADMAP_PEDS_SYNC suspect 1).
        const bool listedBefore = OnMovingList(body);
        if (!stored.fencePrevStaticWaiting && !IsEntityStatic(body) && !WorldRPG::Debug::Flag("nomovelist"))
            AddToMovingList(body);
        stored.collisionFence = false;
        static int released = 0;
        if (released < 8) {
            ++released;
            ENGINE_LOGI("DIAG ped collision fence released elapsed=%lld loaded=%d interior=%u static=%d movingList=%d->%d",
                static_cast<long long>(elapsed), loaded ? 1 : 0, static_cast<unsigned>(ped.interior),
                IsEntityStatic(body) ? 1 : 0, listedBefore ? 1 : 0, OnMovingList(body) ? 1 : 0);
        }
        return false;
    }
    PlaceRemotePlayerPed(body, ped.x, ped.y, ped.z, ped.heading);
    return true;
}

static void ArmPedCollisionFence(void* body, const RemotePed& ped, RemoteBody& stored) {
    // ArmRemoteStreamInTransformFence returns for the syncer (CClientPed.cpp 5917, m_bIsSyncing).
    // The other early-outs (frozen, in a vehicle, attached, synced position anim) are not tracked here yet.
    if (ped.hosted) return;
    stored.collisionFence = true;
    stored.fenceAtMs = GameNowMs();
    stored.fencePrevStaticWaiting = IsStaticWaitingForCollision(body);
    SetStaticWaitingForCollision(body, true);
    // ArmRemoteStreamInTransformFence (CClientPed.cpp 5937-5938): static-waiting, then SetFrozen(true).
    SetPhysicalFrozen(body, true);
    auto& cols = ColStore();
    if (!cols.request || !cols.loaded) {
        static bool missing = false;
        if (!missing) {
            missing = true;
            ENGINE_LOGI("DIAG ped collision fence: CColStore::RequestCollision=%p HasCollisionLoaded=%p",
                reinterpret_cast<void*>(cols.request), reinterpret_cast<void*>(cols.loaded));
        }
        return;
    }
    const Vec3 pos{ped.x, ped.y, ped.z};
    cols.request(&pos, ped.interior);
}

// One getGroundPosition waiting on the game thread. Lua runs on mta-net
// (nativePulse), and CWorld::FindGroundZFor3DCoord reads collision.
struct GroundRequest {
    float x = 0, y = 0, z = 0;
    float result = 0;
    bool ok = false;
    bool done = false;
    bool cancelled = false;
    std::mutex mu;
    std::condition_variable cv;
};
static std::mutex g_groundMu;
static std::shared_ptr<GroundRequest> g_groundWaiter;
static thread_local bool t_onGameThread = false;

static void ServiceGroundQueries() {
    std::shared_ptr<GroundRequest> req;
    {
        std::lock_guard<std::mutex> lock(g_groundMu);
        req = std::move(g_groundWaiter);
    }
    if (!req) return;
    {
        std::lock_guard<std::mutex> lock(req->mu);
        if (req->cancelled) return;
    }
    auto& api = Ground();
    float result = 0.f;
    const bool ok = api.findGround != nullptr;
    if (ok) {
        // CWorldSA::FindGroundZFor3DPosition (CWorldSA.cpp 708-729) passes null, null.
        // libGTASA 2.10 arm64 null-checks both (cbz x20 at 0x50f460, cbz x19 at 0x50f46c)
        // and returns 0 when the vertical line misses.
        result = api.findGround(req->x, req->y, req->z, nullptr, nullptr);
    }
    {
        std::lock_guard<std::mutex> lock(req->mu);
        if (req->cancelled) return;
        req->result = result;
        req->ok = ok;
        req->done = true;
    }
    req->cv.notify_one();
}

bool FindGroundZFor3DPosition(float x, float y, float z, float& out) {
    auto& api = Ground();
    if (!api.findGround) {
        // DIAG: CWorld::FindGroundZFor3DCoord is not in this libGTASA.
        static bool logged = false;
        if (!logged) {
            logged = true;
            ENGINE_LOGE("DIAG getGroundPosition: CWorld::FindGroundZFor3DCoord missing "
                        "(_ZN6CWorld21FindGroundZFor3DCoordEfffPbPP7CEntity)");
        }
        return false;
    }
    if (t_onGameThread) {
        out = api.findGround(x, y, z, nullptr, nullptr);
        return true;
    }
    auto req = std::make_shared<GroundRequest>();
    req->x = x;
    req->y = y;
    req->z = z;
    {
        std::lock_guard<std::mutex> lock(g_groundMu);
        g_groundWaiter = req;
    }
    std::unique_lock<std::mutex> lock(req->mu);
    // Serviced on the game thread from WorldRPG_NoStoryScript (the CRunningScript::Process hook).
    const bool finished = req->cv.wait_for(lock, std::chrono::milliseconds(200), [&] { return req->done; });
    if (!finished) {
        req->cancelled = true;
        lock.unlock();
        {
            std::lock_guard<std::mutex> slot(g_groundMu);
            if (g_groundWaiter == req) g_groundWaiter.reset();
        }
        ENGINE_LOGE("DIAG getGroundPosition: game thread did not run CWorld::FindGroundZFor3DCoord "
                    "(hook CRunningScript::Process, WorldRPG_NoStoryScript)");
        return false;
    }
    if (!req->ok) return false;
    out = req->result;
    return true;
}

// VEHICLE_INOUT seat requests for other players (NoteRemoteSeat / NoteRemoteUnseat). The network thread writes them under
// g_remoteMutex. ApplyRemoteSeats runs on the game thread and removes a request once it is applied.
struct RemoteSeatRequest {
    uint32_t vehicleId = 0xFFFFFFFFu;
    uint8_t seat = 0;
    bool seated = false;
    uint32_t serial = 0;
    int ticks = 0;
};
static std::unordered_map<uint32_t, RemoteSeatRequest> g_seatRequests;
static uint32_t g_seatSerial = 0;
// About 10 s at 60 fps. A request whose ped or car body never appears is dropped after this.
constexpr int kSeatRequestTicks = 600;
// Game thread only: the car each remote ped is seated in by ApplyRemoteSeat.
struct SeatedPed {
    uint32_t vehicleId = 0xFFFFFFFFu;
    uint8_t seat = 0;
};
static std::unordered_map<uint32_t, SeatedPed> g_seatedPeds;

// Ends a seat this thread applied earlier. The caller holds g_remoteMutex. The car body must still exist; without it
// the record is only forgotten. Reads g_remoteBodies and g_remoteCars directly.
static void UnseatLocked(uint32_t pedId) {
    auto seated = g_seatedPeds.find(pedId);
    if (seated == g_seatedPeds.end()) return;
    const uint32_t vehicleId = seated->second.vehicleId;
    g_seatedPeds.erase(seated);
    auto b = g_remoteBodies.find(pedId);
    auto c = g_remoteCars.find(vehicleId);
    if (b == g_remoteBodies.end() || !b->second.ped) return;
    if (c == g_remoteCars.end() || !c->second.body) return;
    UnseatRemotePedFromCar(b->second.ped, c->second.body);
}

static void UnseatApplied(uint32_t pedId) {
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    UnseatLocked(pedId);
}

// Caller holds g_remoteMutex. Called before a car body is dropped: every ped seated in it leaves the seat first.
static void UnseatPedsInCarLocked(uint32_t vehicleId) {
    std::vector<uint32_t> peds;
    for (const auto& [pedId, seated] : g_seatedPeds) {
        if (seated.vehicleId == vehicleId) peds.push_back(pedId);
    }
    for (uint32_t pedId : peds) UnseatLocked(pedId);
}

// Game thread. The local player's ped leaves `body` when it is inside it, with the same task the remote peds use
// (desktop InternalRemoveFromVehicle, see UnseatRemotePedFromCar). True when the ped is not in that vehicle
// afterwards, so the body may be destroyed. False when the ped could not be taken out.
static bool EjectLocalPedFrom(void* body) {
    TaskFns& api = Tasks();
    if (!body || !api.findPlayerVehicle) return true;
    if (api.findPlayerVehicle(-1, false) != body) return true;
    void* ped = g_findPlayer ? g_findPlayer(0) : nullptr;
    if (!ped || !UnseatRemotePedFromCar(ped, body)) return false;
    return api.findPlayerVehicle(-1, false) != body;
}

// Game thread, caller holds g_remoteMutex. Destroys the body of a server car that is going away (ENTITY_REMOVE or
// the reconnect reset). The desktop's vehicle destructor removes every occupant first (CClientVehicle::~CClientVehicle
// calls RemoveFromVehicle on each one); here the remote peds leave through UnseatPedsInCarLocked and the local ped
// through EjectLocalPedFrom. When the local ped cannot be taken out the body is left in the world instead of being
// destroyed under it: a destroyed vehicle with the player inside leaves the ped with a dangling vehicle pointer.
static void DropCarLocked(uint32_t id, RemoteCar& car) {
    if (!car.body) return;
    if (EjectLocalPedFrom(car.body)) {
        DropBody(car.body, true, !car.owned);
    } else {
        ENGINE_LOGE("DIAG vehicle %u body kept: the local ped is still inside (no seat-out task available)", id);
        (void)car.owned.release();  // the game still uses this memory
    }
    car.body = nullptr;
}

// One request on the game thread. True when it is finished (applied, or nothing to do). False keeps it for the next tick.
static bool ApplyRemoteSeat(uint32_t pedId, const RemoteSeatRequest& req) {
    if (!req.seated) {
        UnseatApplied(pedId);
        return true;
    }
    auto seated = g_seatedPeds.find(pedId);
    if (seated != g_seatedPeds.end() && seated->second.vehicleId == req.vehicleId && seated->second.seat == req.seat) {
        return true;
    }
    void* body = nullptr;
    void* car = nullptr;
    int model = 0;
    {
        std::lock_guard<std::mutex> lock(g_remoteMutex);
        auto b = g_remoteBodies.find(pedId);
        if (b != g_remoteBodies.end()) body = b->second.ped;
        auto c = g_remoteCars.find(req.vehicleId);
        if (c != g_remoteCars.end()) {
            car = c->second.body;
            model = c->second.model;
        }
    }
    if (!body || !car) return false;
    // Desktop WarpIntoVehicle (CClientPed.cpp 3383) calls RemoveFromVehicle before it seats the ped.
    if (seated != g_seatedPeds.end()) UnseatApplied(pedId);
    // Driver seat 0 is the driver door. A passenger seat is mapped to its game door by the desktop table, and a seat
    // the model does not have is dropped. The ped stays out of the car in that case.
    unsigned char gameDoor = 0;
    if (req.seat != 0) {
        gameDoor = PassengerGameDoor(model, req.seat);
        if (gameDoor == 0xFF) {
            ENGINE_LOGI("VehInOut: ped %u seat %u has no seat on model %d", pedId, static_cast<unsigned>(req.seat), model);
            return true;
        }
    }
    const bool seatedOk = req.seat == 0 ? SeatRemotePedInCar(body, car)
                                        : SeatRemotePedAsPassenger(body, car, static_cast<int>(gameDoor));
    if (!seatedOk) return false;
    // The seated skip in ApplyRemotePeds returns before its ClearRemoteStick, so a stick set before the seat is cleared here.
    ClearRemoteStick(body);
    g_seatedPeds[pedId] = SeatedPed{req.vehicleId, req.seat};
    ENGINE_LOGI("VehInOut: ped %u seated in vehicle %u seat %u", pedId, req.vehicleId, static_cast<unsigned>(req.seat));
    return true;
}

static void ApplyRemoteSeats() {
    std::vector<std::pair<uint32_t, RemoteSeatRequest>> work;
    {
        std::lock_guard<std::mutex> lock(g_remoteMutex);
        for (const auto& [pedId, req] : g_seatRequests) work.emplace_back(pedId, req);
    }
    for (const auto& [pedId, req] : work) {
        const bool done = ApplyRemoteSeat(pedId, req);
        std::lock_guard<std::mutex> lock(g_remoteMutex);
        auto it = g_seatRequests.find(pedId);
        if (it == g_seatRequests.end() || it->second.serial != req.serial) continue;  // replaced while applying
        if (done) {
            g_seatRequests.erase(it);
        } else if (++it->second.ticks >= kSeatRequestTicks) {
            ENGINE_LOGI("VehInOut: seat request for ped %u dropped, body not ready", pedId);
            g_seatRequests.erase(it);
        }
    }
}

// Writes a pending CarSeed into the body, once. Game thread. `localVehicle` is the car the local ped is in: its
// pose and speeds belong to the local driver (the desktop never applies remote state to the vehicle the local player
// drives), so only the health is taken for it.
static void ApplyCarSeed(uint32_t vehicleId, void* body, bool localVehicle) {
    CarSeed seed;
    {
        std::lock_guard<std::mutex> lock(g_remoteMutex);
        auto found = g_remoteCars.find(vehicleId);
        if (found == g_remoteCars.end() || !found->second.seed.pending) return;
        seed = found->second.seed;
        found->second.seed = CarSeed{};
        if (seed.hasPose && !localVehicle) {
            found->second.shownX = seed.x;
            found->second.shownY = seed.y;
            found->second.shownZ = seed.z;
        }
    }
    if (!localVehicle) {
        if (seed.hasPose) WriteBodyPos(body, seed.x, seed.y, seed.z);
        if (seed.hasRotation) WriteBodyRotation(body, seed.rotX, seed.rotY, seed.rotZ);
        if (seed.hasMotion) {
            auto* raw = static_cast<unsigned char*>(body);
            std::memcpy(raw + 0x68, seed.move, sizeof seed.move);
            std::memcpy(raw + 0x74, seed.turn, sizeof seed.turn);
        }
    }
    if (seed.hasHealth) WriteBodyHealth(body, seed.health);
    static int reported = 0;
    if (reported < 6) {
        ++reported;
        ENGINE_LOGI("Server vehicle %u seeded: pose %d rot %d (%.1f %.1f %.1f) motion %d health %d (%.1f)%s", vehicleId,
            seed.hasPose ? 1 : 0, seed.hasRotation ? 1 : 0, seed.rotX, seed.rotY, seed.rotZ, seed.hasMotion ? 1 : 0,
            seed.hasHealth ? 1 : 0, seed.health, localVehicle ? " [local vehicle: health only]" : "");
    }
}

// Packet 33 driver block: desktop CClientVehicle::SetMoveSpeed and SetTurnSpeed, called from CNetAPI::ReadVehiclePuresync
// (CNetAPI.cpp 1420-1421). CPhysical::SetMoveSpeed is not exported, so the CPhysical fields are written at +0x68 (move)
// and +0x74 (turn), the offsets PlaceRemotePlayerPed and SampleHostedCar already use.
static void ApplyCarMotion(uint32_t vehicleId, void* body) {
    float move[3] = {0, 0, 0};
    float turn[3] = {0, 0, 0};
    {
        std::lock_guard<std::mutex> lock(g_remoteMutex);
        auto found = g_remoteCars.find(vehicleId);
        if (found == g_remoteCars.end() || !found->second.motionPending) return;
        found->second.motionPending = false;
        move[0] = found->second.moveX;
        move[1] = found->second.moveY;
        move[2] = found->second.moveZ;
        turn[0] = found->second.turnX;
        turn[1] = found->second.turnY;
        turn[2] = found->second.turnZ;
    }
    auto* raw = static_cast<unsigned char*>(body);
    std::memcpy(raw + 0x68, move, sizeof move);
    std::memcpy(raw + 0x74, turn, sizeof turn);
}

// Desktop CClientPed::EnterVehicle (CClientPed.cpp:9886, request written at 10061) and ExitVehicle
// (CClientPed.cpp:10096, request written at 10178) send REQUEST_IN / REQUEST_OUT only after the local ped
// is allowed to act. On Android the native enter button
// starts GTA's animation first, so the change is watched here instead. Each frame FindPlayerVehicle(-1, false)
// returns the CVehicle the local ped is in; it is mapped to a server ElementID through g_remoteCars. A change
// of mapped vehicle queues a leave for the old one and an enter for the new one. The network thread turns
// those events into REQUEST_IN / REQUEST_OUT (NeonJoinProtocol.cpp, SendVehicleInOut).
static void WatchLocalVehicle() {
    TaskFns& api = Tasks();
    if (!api.findPlayerVehicle) return;
    void* vehicle = api.findPlayerVehicle(-1, false);
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    uint32_t current = kNoVehicle;
    int model = 0;
    if (vehicle) {
        for (const auto& [id, car] : g_remoteCars) {
            if (car.body == vehicle) {
                current = id;
                model = car.model;
                break;
            }
        }
    }
    // Packet 33 source (CopyLocalCar). Sampled every frame while the ped is in a mapped standard car, before
    // the unchanged-vehicle return below, so the snapshot stays current. SampleHostedCar does not take the mutex.
    HostedCar sample;
    const bool sampled = current != kNoVehicle && model >= 400 && model <= 611 && SampleHostedCar(vehicle, sample);
    g_localCarValid = sampled;
    if (sampled) {
        LocalCarState& c = g_localCar;
        c = LocalCarState{};
        c.vehicleId = current;
        c.model = model;
        c.x = sample.x;
        c.y = sample.y;
        c.z = sample.z;
        c.rotX = sample.rotX;
        c.rotY = sample.rotY;
        c.rotZ = sample.rotZ;
        c.vx = sample.vx;
        c.vy = sample.vy;
        c.vz = sample.vz;
        c.tx = sample.tx;
        c.ty = sample.ty;
        c.tz = sample.tz;
        c.health = sample.health;
        SamplePadControls(api, c);
    }
    if (current == g_localVehicleId) return;
    if (g_localVehicleId != kNoVehicle) {
        g_localVehicleEvents.push_back(LocalVehicleEvent{g_localVehicleId, false});
        ENGINE_LOGI("VehInOut: local ped left vehicle %u", g_localVehicleId);
    }
    if (current != kNoVehicle) {
        g_localVehicleEvents.push_back(LocalVehicleEvent{current, true});
        ENGINE_LOGI("VehInOut: local ped entered vehicle %u", current);
    }
    if (g_localVehicleEvents.size() > kMaxLocalVehicleEvents) {
        ENGINE_LOGI("VehInOut: event queue full, dropping oldest");
        g_localVehicleEvents.erase(g_localVehicleEvents.begin());
    }
    g_localVehicleId = current;
}

// CNetAPI::ReadPlayerPuresync writes that arrived on the network thread.
// SetMoveSpeed is CPhysical +0x68 (the same field SampleLocalPose reads).
// SetHealth / SetArmor are CPed +0x6AC / +0x6B4. Each flag is consumed once.
static void ApplySyncedPlayerFields(uint32_t id, void* body) {
    if (!body) return;
    bool doSpeed = false, doHealth = false, doArmor = false;
    float vx = 0.f, vy = 0.f, vz = 0.f, health = 0.f, armor = 0.f;
    {
        std::lock_guard<std::mutex> lock(g_remoteMutex);
        auto found = g_remotePeds.find(id);
        if (found == g_remotePeds.end()) return;
        RemotePed& ped = found->second;
        if (ped.syncMoveSpeed) {
            doSpeed = true;
            vx = ped.vx;
            vy = ped.vy;
            vz = ped.vz;
            ped.syncMoveSpeed = false;
        }
        if (ped.syncHealth) {
            doHealth = true;
            health = ped.health;
            ped.syncHealth = false;
        }
        if (ped.syncArmor) {
            doArmor = true;
            armor = ped.armor;
            ped.syncArmor = false;
        }
    }
    auto* raw = static_cast<unsigned char*>(body);
    if (doSpeed) {
        auto* speed = reinterpret_cast<float*>(raw + 0x68);
        speed[0] = vx;
        speed[1] = vy;
        speed[2] = vz;
    }
    if (doHealth && std::isfinite(health)) *reinterpret_cast<float*>(raw + 0x6AC) = health;
    if (doArmor && std::isfinite(armor)) *reinterpret_cast<float*>(raw + 0x6B4) = armor;
    // CClientPed::StreamedInPulse (CClientPed.cpp 4738-4748) writes a locked health
    // through InternalSetHealth (3677-3718) and a locked armor through CPed::SetArmor.
    // Both game setters store the float (CPedSA.h 407 and 410). The dead-to-alive
    // branch of InternalSetHealth recreates the ped; that hook is logged below.
    bool healthLocked = false, armorLocked = false;
    float lockedHealth = 0.f, lockedArmor = 0.f;
    if (!CopyPedVitalsLock(id, healthLocked, lockedHealth, armorLocked, lockedArmor)) return;
    if (healthLocked && std::isfinite(lockedHealth)) {
        auto* current = reinterpret_cast<float*>(raw + 0x6AC);
        if (*current <= 0.f && lockedHealth > 0.f) {
            static int noted = 0;
            if (noted < 4) {
                ++noted;
                ENGINE_LOGI("DIAG CClientPed::InternalSetHealth dead-to-alive path is not run "
                            "(CClientPed.cpp 3682-3709: RemoveFromVehicle, Respawn, ReCreateModel, WarpIntoVehicle). "
                            "The health field is still written.");
            }
        }
        *current = lockedHealth;
    }
    if (armorLocked && std::isfinite(lockedArmor))
        *reinterpret_cast<float*>(raw + 0x6B4) = lockedArmor;
}

// #region agent log
static void DebugPedPose(const char* hypothesis, const char* branch, uint32_t id, void* body, const RemotePed& ped, bool fence) {
    static int ticks = 0;
    if ((++ticks % 30) != 1) return;
    float bx = 0, by = 0, bz = 0, frontZ = 0, upZ = 0, placeZ = 0;
    uint32_t phys = 0;
    if (body) {
        auto* raw = static_cast<unsigned char*>(body);
        placeZ = *reinterpret_cast<float*>(raw + 0x10);
        phys = *reinterpret_cast<uint32_t*>(raw + 0x64);
        auto* matrix = *reinterpret_cast<unsigned char**>(raw + 0x18);
        const auto matrixAddr = reinterpret_cast<uintptr_t>(matrix);
        if (matrix && (matrixAddr & 3u) == 0 && matrixAddr > 0x10000u) {
            bx = *reinterpret_cast<float*>(matrix + 0x30);
            by = *reinterpret_cast<float*>(matrix + 0x34);
            bz = *reinterpret_cast<float*>(matrix + 0x38);
            frontZ = *reinterpret_cast<float*>(matrix + 0x18);
            upZ = *reinterpret_cast<float*>(matrix + 0x28);
        }
    }
    const long long now = static_cast<long long>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    char line[640];
    std::snprintf(line, sizeof(line),
        "{\"sessionId\":\"dc8159\",\"hypothesisId\":\"%s\",\"location\":\"GameEngine.cpp:ApplyRemotePeds\","
        "\"message\":\"ped pose\",\"data\":{\"branch\":\"%s\",\"id\":%u,\"fence\":%s,\"hosted\":%s,\"keysync\":%s,"
        "\"nx\":%.2f,\"ny\":%.2f,\"nz\":%.2f,\"bx\":%.2f,\"by\":%.2f,\"bz\":%.2f,\"placeZ\":%.2f,"
        "\"frontZ\":%.3f,\"upZ\":%.3f,\"phys\":%u,\"vx\":%.3f,\"vy\":%.3f,\"sx\":%d,\"sy\":%d},\"timestamp\":%lld}\n",
        hypothesis, branch, id, fence ? "true" : "false", ped.hosted ? "true" : "false", ped.keysync ? "true" : "false",
        ped.x, ped.y, ped.z, bx, by, bz, placeZ, frontZ, upZ, phys, ped.vx, ped.vy, ped.stickX, ped.stickY, now);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_INFO, "DBGDC8159", "%s", line);
#endif
    // ROADMAP_PEDS_SYNC 0.4: no file I/O on the game thread. The line stays in logcat (tag DBGDC8159).
}
// #endregion

static void ApplyRemotePeds() {
    NoteGameFrame();
    if (WorldRPG::Debug::Flag("noremote")) return;
    if (g_remoteResetPending.exchange(false)) {
        // The whole reset runs under g_remoteMutex: the network thread adds cars and seat requests of the new
        // session under the same mutex (NoteRemoteVehicle, NoteVehicleStartSync, NoteRemoteSeat). A record it added
        // before this tick is dropped here with the rest; cars and peds come back from the ServerWorld actor table
        // on this same tick (CopyActors below), which JNI nativeConnect cleared before the new session started.
        std::lock_guard<std::mutex> lock(g_remoteMutex);
        // Leave every seat while the car bodies still exist, then drop the requests.
        std::vector<uint32_t> seated;
        for (const auto& entry : g_seatedPeds) seated.push_back(entry.first);
        for (uint32_t pedId : seated) UnseatLocked(pedId);
        g_seatedPeds.clear();
        g_seatRequests.clear();
        for (auto& entry : g_remoteBodies) {
            ClearRemoteStick(entry.second.ped);
            DropBody(entry.second.ped, false);
        }
        g_remoteBodies.clear();
        // The local ped leaves its car before any car body is destroyed (DropCarLocked).
        for (auto& entry : g_remoteCars) DropCarLocked(entry.first, entry.second);
        g_remoteCars.clear();
        // A new session starts on foot: drop the stale vehicle, its packet 33 sample, any queued VEHICLE_INOUT
        // events and shots, and the hosted snapshots of the old session's element ids.
        g_localVehicleId = kNoVehicle;
        g_localCarValid = false;
        g_localVehicleEvents.clear();
        g_localShots.clear();
        g_hostedSnap.clear();
        g_hostedCarSnap.clear();
        ENGINE_LOGI("Remote world reset for reconnect");
    }
    {
        ServerWorld::ActorSpawn actors[kMaxRemotePedBodies + kMaxRemoteVehicleRecords];
        const int count = ServerWorld::CopyActors(actors, kMaxRemotePedBodies + kMaxRemoteVehicleRecords,
                                                   kMaxRemotePedBodies, kMaxRemoteVehicleRecords);
        // Observer interpolation (PedSyncState). Runs before g_remoteMutex is taken, because
        // SamplePose pushes the pose through NoteRemotePed, which locks the same mutex.
        for (int i = 0; i < count; ++i) {
            if (actors[i].kind == 14) WorldRPG::Sync::PedSyncState::SamplePose(actors[i].id);
        }
        std::lock_guard<std::mutex> lock(g_remoteMutex);
        for (int i = 0; i < count; ++i) {
            const auto& actor = actors[i];
            if (actor.kind == 14) {
                const bool firstSeen = g_remotePeds.find(actor.id) == g_remotePeds.end();
                auto& ped = g_remotePeds[actor.id];
                ped.serverPed = true;
                if (firstSeen) ENGINE_LOGI("DIAG ped actor id=%u model=%d hosted=%d pos=%.1f,%.1f,%.1f", actor.id, actor.model, actor.hosted ? 1 : 0, actor.x, actor.y, actor.z);
                ped.hosted = actor.hosted;
                ped.interior = actor.interior;
                if (!ped.dirty && !ped.shown && actor.model >= 0) {
                    ped.x = actor.x;
                    ped.y = actor.y;
                    ped.z = actor.z;
                    ped.heading = actor.heading;
                    ped.dirty = true;
                }
            } else if (actor.kind == 2 && actor.model >= 400) {
                auto& car = g_remoteCars[actor.id];
                car.model = actor.model;
                car.hosted = actor.hosted;
                // Desktop Packet_EntityAdd, vehicle case: SetRotationDegrees and SetHealth. Once per record; a
                // VEHICLE_STARTSYNC seed that arrived first is newer and is kept.
                if (!car.actorSeeded) {
                    car.actorSeeded = true;
                    if (!car.seed.pending) {
                        constexpr float kRadToDeg = 180.f / 3.14159265f;
                        car.seed.hasRotation = true;
                        car.seed.rotX = actor.rotX;
                        car.seed.rotY = actor.rotY;
                        car.seed.rotZ = actor.heading * kRadToDeg;
                        car.seed.hasHealth = actor.health >= 0.f;
                        car.seed.health = actor.health;
                        car.seed.pending = true;
                    }
                }
                if (!car.dirty && !car.shown) {
                    car.x = actor.x;
                    car.y = actor.y;
                    car.z = actor.z;
                    car.heading = actor.heading;
                    car.dirty = true;
                }
            }
        }
        TaskFns& api = Tasks();
        for (uint32_t id : g_forget) {
            // This loop holds g_remoteMutex, so the locked variants are used. A seated ped leaves its seat, and a
            // forgotten car first unseats the peds inside it.
            UnseatLocked(id);
            UnseatPedsInCarLocked(id);
            g_seatRequests.erase(id);
            auto body = g_remoteBodies.find(id);
            if (body != g_remoteBodies.end()) {
                ClearRemoteStick(body->second.ped);
                DropBody(body->second.ped, false);
                g_remoteBodies.erase(body);
            }
            auto car = g_remoteCars.find(id);
            if (car != g_remoteCars.end()) {
                DropCarLocked(id, car->second);
                g_remoteCars.erase(car);
            }
            g_remotePeds.erase(id);
        }
        g_forget.clear();
        (void)api;
    }
    std::unordered_map<uint32_t, RemotePed> pending;
    {
        std::lock_guard<std::mutex> lock(g_remoteMutex);
        for (auto& [id, ped] : g_remotePeds) pending.emplace(id, ped);
    }
    for (auto& [id, ped] : pending) {
        if (!PlausiblePosition(ped.x, ped.y, ped.z)) continue;
        auto live = g_remotePeds.find(id);
        if (live == g_remotePeds.end()) continue;
        if (id == ServerWorld::LocalPlayer()) continue;
        int model = ServerWorld::ModelOf(id);
        // Model 0 is the shared CJ clump. Rebuilding it on a second ped
        // replaces the local player. A complete body keeps the other player visible.
        if (model <= 0 || model > 312) model = 7;
        void* body = nullptr;
        auto found = g_remoteBodies.find(id);
        if (found == g_remoteBodies.end()) {
            static int spawnedFrame = -1;
            if (g_gameFrame == spawnedFrame) continue;
            if (!PedModelSettled(model)) {
                static int waits = 0;
                if (waits < 4) {
                    ++waits;
                    ENGINE_LOGI("Server ped %u waiting model %d state %d",
                        id, model, StreamingLoadState(model));
                }
                continue;
            }
            // The actor list is not distance-sorted. A full cap of far peds left
            // none around the local spawn (126,-114 while bodies sat at 757 and 2490).
            // Desktop streams by distance. Drop the farthest body when this ped is closer.
            if (g_remoteBodies.size() >= static_cast<size_t>(kMaxRemotePedBodies)) {
                const float lx = g_poseX.load();
                const float ly = g_poseY.load();
                auto dist2 = [&](float x, float y) {
                    const float dx = x - lx;
                    const float dy = y - ly;
                    return dx * dx + dy * dy;
                };
                const float mine = dist2(ped.x, ped.y);
                uint32_t farId = 0;
                float farD = -1.f;
                for (const auto& entry : g_remoteBodies) {
                    if (g_seatedPeds.count(entry.first)) continue;
                    auto pit = g_remotePeds.find(entry.first);
                    if (pit == g_remotePeds.end()) continue;
                    const float d = dist2(pit->second.x, pit->second.y);
                    if (d > farD) {
                        farD = d;
                        farId = entry.first;
                    }
                }
                if (farId == 0 || mine >= farD) continue;
                auto far = g_remoteBodies.find(farId);
                if (far == g_remoteBodies.end()) continue;
                ClearRemoteStick(far->second.ped);
                DropBody(far->second.ped, false);
                g_remoteBodies.erase(far);
            }
            spawnedFrame = g_gameFrame;
            // CClientPed::SetPosition (CClientPed.cpp 1064-1098) and Packet_EntityAdd
            // (CPacketHandler.cpp 4054) write the network position. No ground offset.
            const Vec3 pos{ped.x, ped.y, ped.z};
            body = CreateRemotePlayerPed(model, pos.x, pos.y, pos.z, ped.heading);
            if (!body) continue;
            RemoteBody stored;
            ArmRemote(body, stored, ped.weapon);
            stored.ped = body;
            ArmPedCollisionFence(body, ped, stored);
            {
                static int spawnLists = 0;
                if (spawnLists < 8) {
                    ++spawnLists;
                    ENGINE_LOGI("DIAG ped spawn id=%u hosted=%d fence=%d static=%d staticWaiting=%d movingList=%d",
                        id, ped.hosted ? 1 : 0, stored.collisionFence ? 1 : 0, IsEntityStatic(body) ? 1 : 0,
                        IsStaticWaitingForCollision(body) ? 1 : 0, OnMovingList(body) ? 1 : 0);
                }
            }
            // Packet_EntityAdd calls SetInterior after the ped exists (CPacketHandler.cpp 4412,
            // CClientPed::SetInterior 1101-1119).
            ApplyPedInterior(body, stored, ped.interior);
            g_remoteBodies.emplace(id, std::move(stored));
            live->second.shownX = pos.x;
            live->second.shownY = pos.y;
            live->second.shownZ = pos.z;
            live->second.shownH = ped.heading;
            live->second.shown = true;
            ApplySyncedPlayerFields(id, body);
            DebugPedPose("H1", "spawn", id, body, ped, true);
            static int created = 0;
            if (created < 6) {
                ++created;
                ENGINE_LOGI("Server ped %u model %d at %.0f %.0f %.0f", id, model, pos.x, pos.y, pos.z);
            }
            continue;
        }
        body = found->second.ped;
        if (!body) continue;
        ApplyPedInterior(body, found->second, live->second.interior);
        // A ped in a seat is placed by the seat task. The stream-in fence returns
        // when the ped occupies a vehicle (CClientPed.cpp 5918).
        if (g_seatedPeds.count(id)) {
            ApplySyncedPlayerFields(id, body);
            if (found->second.collisionFence) {
                SetStaticWaitingForCollision(body, false);
                SetPhysicalFrozen(body, false);
                found->second.collisionFence = false;
            }
        } else if (HoldPedCollisionFence(body, found->second, live->second)) {
            ApplySyncedPlayerFields(id, body);
            DebugPedPose("H5", "fence", id, body, live->second, true);
            continue;
        }
        ArmRemote(body, found->second, ped.weapon);
        // A seated ped is placed by the seat task (ApplyRemoteSeat), so the on-foot pose below must not move it.
        if (g_seatedPeds.count(id)) continue;
        if (ped.hosted && ped.wander && found->second.wanderDir != ped.wanderDir) {
            StartWander(body, ped.wanderDir);
            found->second.wanderDir = ped.wanderDir;
        }
        if (!ped.hosted && found->second.wanderDir >= 0) {
            TaskFns& api = Tasks();
            void* intel = PedIntel(body);
            if (intel && api.clearTasks) api.clearTasks(intel, true, true);
            found->second.wanderDir = -1;
        }
        const bool controlled = !ped.hosted && ped.keysync && RemoteControlHookActive();
        if (controlled) {
            // Plan step A: the stick drives CPlayerPed::ProcessControl. The per-tick
            // matrix lerp below would fight it, so only re-anchor when the body
            // drifts more than 8 m from the server position.
            SetRemoteStick(body, ped.stickX, ped.stickY, ped.fire);
            float bx = 0, by = 0, bz = 0, bh = 0;
            if (ReadBodyPos(body, bx, by, bz, bh)) {
                const float ex = ped.x - bx;
                const float ey = ped.y - by;
                const float ez = ped.z - bz;
                if (ex * ex + ey * ey + ez * ez > 8.f * 8.f)
                    PlaceRemotePlayerPed(body, ped.x, ped.y, ped.z, ped.heading);
            }
            live->second.shown = true;
            // The pad hook only drives the stick. libGTASA reads fire from the touch widget, not from the pad
            // (PlayerPedPort.cpp, RemotePlayerProcessControl), so the punch for a remote fire key comes from
            // PresentOnFoot, as it does without the hook.
            PresentOnFoot(body, found->second, ped);
            ApplySyncedPlayerFields(id, body);
            DebugPedPose("H4", "stick", id, body, ped, false);
            continue;
        }
        ClearRemoteStick(body);
        const float dx = ped.x - live->second.shownX;
        const float dy = ped.y - live->second.shownY;
        const float dz = ped.z - live->second.shownZ;
        const float dist2 = dx * dx + dy * dy + dz * dz;
        if (ped.hosted && live->second.shown && dist2 < 64.f) {
            float hx = 0, hy = 0, hz = 0, hh = live->second.shownH;
            if (ReadBodyPos(body, hx, hy, hz, hh)) {
                auto* speed = reinterpret_cast<float*>(static_cast<unsigned char*>(body) + 0x68);
                live->second.shownX = hx;
                live->second.shownY = hy;
                live->second.shownZ = hz;
                live->second.shownH = hh;
                std::lock_guard<std::mutex> lock(g_remoteMutex);
                auto snap = g_remotePeds.find(id);
                if (snap != g_remotePeds.end()) {
                    snap->second.x = hx;
                    snap->second.y = hy;
                    snap->second.z = hz;
                    snap->second.heading = hh;
                    snap->second.vx = speed[0];
                    snap->second.vy = speed[1];
                    snap->second.vz = speed[2];
                    snap->second.shownX = hx;
                    snap->second.shownY = hy;
                    snap->second.shownZ = hz;
                    snap->second.shownH = hh;
                }
            }
            ApplySyncedPlayerFields(id, body);
            DebugPedPose("H2", "hosted", id, body, ped, false);
            continue;
        }
        if (!live->second.shown || dist2 > 20.f * 20.f) {
            live->second.shownX = ped.x;
            live->second.shownY = ped.y;
            live->second.shownZ = ped.z;
            live->second.shownH = ped.heading;
            live->second.shown = true;
            PlaceRemotePlayerPed(body, ped.x, ped.y, ped.z, ped.heading);
            ApplySyncedPlayerFields(id, body);
            DebugPedPose("H1", "snap", id, body, ped, false);
        } else {
            // CClientPed::SetTargetPosition / UpdateTargetPosition (CClientPed.cpp 8657-8822)
            // spread the error across one sync interval. The player ped has no m_interp
            // fields, and CClientPed::Interpolate is not called from a ped task, so this
            // step stays the existing 0.35 position blend. A second algorithm is not added.
            static bool loggedInterp = false;
            if (!loggedInterp) {
                loggedInterp = true;
                ENGINE_LOGI("DIAG player position: CClientPed::UpdateTargetPosition "
                            "(CClientPed.cpp 8765-8822) is not wired; keeping the 0.35 position step. "
                            "CPed::SetTargetRotation is SetRemotePedHeading (+0x6C4/+0x6C8).");
            }
            live->second.shownX += dx * 0.35f;
            live->second.shownY += dy * 0.35f;
            live->second.shownZ += dz * 0.35f;
            if (ped.keysync) {
                // CClientPed::SetTargetRotation(float) (CClientPed.cpp 5332-5339) writes
                // the heading at once. ReadPlayerPuresync and ReadKeysync use that overload.
                live->second.shownH = ped.heading;
            } else {
                float turn = ped.heading - live->second.shownH;
                constexpr float pi = 3.14159265f;
                while (turn > pi) turn -= 2.f * pi;
                while (turn < -pi) turn += 2.f * pi;
                live->second.shownH += turn * 0.35f;
            }
            auto* raw = static_cast<unsigned char*>(body);
            auto* matrix = *reinterpret_cast<unsigned char**>(raw + 0x18);
            const auto matrixAddr = reinterpret_cast<uintptr_t>(matrix);
            if (matrix && (matrixAddr & 3u) == 0 && matrixAddr > 0x10000u) {
                *reinterpret_cast<float*>(matrix + 0x30) = live->second.shownX;
                *reinterpret_cast<float*>(matrix + 0x34) = live->second.shownY;
                *reinterpret_cast<float*>(matrix + 0x38) = live->second.shownZ;
            }
            *reinterpret_cast<float*>(raw + 0x08) = live->second.shownX;
            *reinterpret_cast<float*>(raw + 0x0C) = live->second.shownY;
            *reinterpret_cast<float*>(raw + 0x10) = live->second.shownZ;
            DebugPedPose("H3", "lerp", id, body, ped, false);
        }
        // CPlaceable::SetHeading is not exported on libGTASA 2.10. Yaw is
        // CMatrix::SetRotateZOnly plus fCurrentRotation / fTargetRotation.
        SetRemotePedHeading(body, live->second.shownH);
        // The snap branch already consumed the one-shot flags. The lerp branch has not.
        ApplySyncedPlayerFields(id, body);
        if (std::fabs(ped.vx) + std::fabs(ped.vy) + std::fabs(ped.vz) > 0.001f) {
            static SetMoveSpeedFn setMove = nullptr;
            static bool looked = false;
            if (!looked) {
                looked = true;
                void* handle = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_GLOBAL);
                if (handle) {
                    setMove = reinterpret_cast<SetMoveSpeedFn>(dlsym(handle, "_ZN10CPhysical12SetMoveSpeedERK7CVector"));
                    if (!setMove)
                        setMove = reinterpret_cast<SetMoveSpeedFn>(dlsym(handle, "_ZN10CPhysical12SetMoveSpeedE7CVector"));
                }
            }
            if (setMove) {
                Vec3 speed{ped.vx, ped.vy, ped.vz};
                setMove(body, speed);
            }
        }

        // Native task presentation — matches mta_neon CClientPed::UpdateNativeTaskAnimationPresentation
        // Apply locomotion mode, weapon type, animation, onfire/inwater, and animation kill
        // to the remote ped body on the game thread after spatial sync.
        if (ped.animationKilled && found->second.ped) {
            // ucFlags & 0x80: clear synced anim state so the ped is no longer
            // permanently excluded from native-task locomotion presentation.
            TaskFns& api = Tasks();
            void* intel = PedIntel(found->second.ped);
            if (intel && api.clearTasks) api.clearTasks(intel, true, true);
            live->second.animationKilled = false;
        }
        if (ped.fire != found->second.ducked) {
            // OnFire state — GTA uses CPed::SetOnFire(bool). For now we track
            // it in RemoteBody for future native call integration.
            found->second.ducked = ped.fire;
        }
        if (ped.inWater) {
            // IsInWater state — tracked for future native call integration.
            // No immediate GTA native available without further dlsym work.
        }
        if (ped.nativeTaskLocomotionMode != 0 && !ped.hosted) {
            // SNativeTaskLocomotionSync::eMode applied via controller state stick inputs.
            // WALK/RUN/SPRINT modes map to left-stick magnitude thresholds matching
            // mta_neon CClientPed::ApplyNativeTaskLocomotionPresentation.
            // Full implementation requires CTaskManagerSA equivalent on Android.
        }
        if (ped.nativeTaskWeaponType != 0 && !ped.hosted) {
            // SNativeTaskWeaponPresentationSync weapon type — future integration
            // point for creating/destroying weapon fire tasks on remote peds.
        }
        if (ped.nativeTaskAnimGroup != 0 || ped.nativeTaskAnimId != 0) {
            // SNativeTaskAnimationPresentationSync — future integration point
            // for playing/blending animations on remote ped clumps via
            // RpAnimBlendClumpGetAssociation equivalent on Android.
        }
        if (ped.physicalTakeoverPending && found->second.ped) {
            // Physical takeover from PED_START_SYNC seeds airborne/climb anchor.
            // Matches mta_neon CClientPed::SetNativeTaskPhysicalTakeoverState.
            // Full implementation requires CTaskSimpleClimb equivalent on Android.
            live->second.physicalTakeoverPending = false;
        }

        RemotePed animPed = ped;
        if (std::fabs(animPed.vx) + std::fabs(animPed.vy) < 0.02f) {
            const float step = std::sqrt(dx * dx + dy * dy);
            if (step > 0.02f && step < 8.f) {
                animPed.vx = dx * 0.35f;
                animPed.vy = dy * 0.35f;
            }
        }
        if (!ped.hosted) PresentOnFoot(body, found->second, animPed);
    }
    std::vector<HostedPed> hosted;
    {
        std::lock_guard<std::mutex> lock(g_remoteMutex);
        for (auto& [id, ped] : g_remotePeds) {
            if (!ped.hosted) continue;
            HostedPed sample;
            sample.id = id;
            sample.x = ped.shown ? ped.shownX : ped.x;
            sample.y = ped.shown ? ped.shownY : ped.y;
            sample.z = ped.shown ? ped.shownZ : ped.z;
            sample.heading = ped.shown ? ped.shownH : ped.heading;
            sample.vx = ped.vx;
            sample.vy = ped.vy;
            sample.vz = ped.vz;
            sample.health = ped.health;
            // WritePedInformation reads the live ped: GetPosition, GetMoveSpeed, GetHealth, GetArmor,
            // and GetCurrentRotation (CPedSA.h 433). That float is CPed +0x6C4 on libGTASA 2.10
            // (CPed::UpdatePosition 0x597b3c). Health and armour stay +0x6AC and +0x6B4.
            auto bodyIt = g_remoteBodies.find(id);
            void* body = bodyIt == g_remoteBodies.end() ? nullptr : bodyIt->second.ped;
            if (body) {
                float hx = 0, hy = 0, hz = 0, hh = sample.heading;
                if (ReadBodyPos(body, hx, hy, hz, hh)) {
                    sample.x = hx;
                    sample.y = hy;
                    sample.z = hz;
                    sample.heading = hh;
                    const float current = *reinterpret_cast<const float*>(static_cast<unsigned char*>(body) + 0x6C4);
                    if (std::isfinite(current)) sample.heading = current;
                    auto* speed = reinterpret_cast<float*>(static_cast<unsigned char*>(body) + 0x68);
                    sample.vx = speed[0];
                    sample.vy = speed[1];
                    sample.vz = speed[2];
                }
                auto* raw = static_cast<unsigned char*>(body);
                const float health = *reinterpret_cast<float*>(raw + 0x6AC);
                const float armor = *reinterpret_cast<float*>(raw + 0x6B4);
                if (std::isfinite(health) && std::isfinite(armor)) {
                    sample.health = health;
                    sample.armor = armor;
                    sample.bodyVitals = true;
                }
            }
            hosted.push_back(sample);
        }
        g_hostedSnap.swap(hosted);
    }
    TaskFns& api = Tasks();
    struct CarPose { uint32_t id; int model; float x, y, z, heading; bool hosted; };
    std::vector<CarPose> cars;
    {
        std::lock_guard<std::mutex> lock(g_remoteMutex);
        for (const auto& [id, car] : g_remoteCars)
            cars.push_back({id, car.model, car.x, car.y, car.z, car.heading, car.hosted});
    }
    // The car the local ped is in. Its body is moved by the local driver, not by server poses.
    void* const localVehicle = api.findPlayerVehicle ? api.findPlayerVehicle(-1, false) : nullptr;
    for (const auto& car : cars) {
        const uint32_t id = car.id;
        if (!PlausiblePosition(car.x, car.y, car.z) || car.model < 400 || car.model > 611) continue;
        auto found = g_remoteCars.find(id);
        if (found == g_remoteCars.end()) continue;
        void* body = found->second.body;
        if (!body) {
            if (!EnsureWorldModel(car.model) || g_remoteCars.size() > static_cast<size_t>(kMaxRemoteVehicleRecords)) continue;
            if (api.createCarForScript) {
                // Desktop CPoolsSA::AddVehicle: CCarCtrl::CreateCarForScript(model, CVector(), 0) (pool-owned, added to the world).
                body = api.createCarForScript(car.model, Vec3{car.x, car.y, car.z}, 0);
                if (!body) continue;
            } else {
                if (!api.carCtor || !g_worldAdd) continue;
                found->second.owned = std::make_unique<std::byte[]>(0x2000);
                body = found->second.owned.get();
                std::memset(body, 0, 0x2000);
                api.carCtor(body, car.model, 2, 0);
                g_worldAdd(body);
            }
            found->second.body = body;
            found->second.shownX = car.x;
            found->second.shownY = car.y;
            found->second.shownZ = car.z;
            found->second.shownH = car.heading;
            found->second.shown = true;
            WriteBodyPos(body, car.x, car.y, car.z);
            if (g_setHeading) g_setHeading(body, car.heading);
            // Server rotation and health from ENTITY_ADD or VEHICLE_STARTSYNC. A new body starts upright at the
            // constructor health of 1000, which packets 56 and 33 would otherwise report back to the server.
            ApplyCarSeed(id, body, false);
            static int created = 0;
            if (created < 6) {
                ++created;
                ENGINE_LOGI("Server vehicle %u model %d at %.0f %.0f %.0f", id, car.model, car.x, car.y, car.z);
            }
            continue;
        }
        ApplyCarSeed(id, body, body == localVehicle);
        if (body == localVehicle) {
            // The local ped is in this car. The desktop applies remote vehicle state (CNetAPI::ReadVehiclePuresync,
            // Packet_UnoccupiedVehicleSync) only to vehicles other players sync; the local player's own vehicle is
            // driven by the game. Once the server seats the ped it sends VEHICLE_STOPSYNC, so the car is not hosted
            // and the pose lerp below would write the last server pose into the body on every frame. The record
            // follows the live body instead, so the lerp starts from the right place when the ped leaves.
            float lx = 0, ly = 0, lz = 0, lh = 0;
            if (ReadBodyPos(body, lx, ly, lz, lh)) {
                std::lock_guard<std::mutex> lock(g_remoteMutex);
                auto live = g_remoteCars.find(id);
                if (live != g_remoteCars.end()) {
                    HostedCar pose;
                    live->second.x = live->second.shownX = lx;
                    live->second.y = live->second.shownY = ly;
                    live->second.z = live->second.shownZ = lz;
                    // Same heading convention as the server's rz (CarPose heading is rz in radians).
                    if (SampleHostedCar(body, pose)) live->second.heading = live->second.shownH = pose.rotZ * 3.14159265f / 180.f;
                    live->second.motionPending = false;
                }
            }
            continue;
        }
        if (!car.hosted) ApplyCarMotion(id, body);
        if (car.hosted) continue;
        const float dx = car.x - found->second.shownX;
        const float dy = car.y - found->second.shownY;
        const float dz = car.z - found->second.shownZ;
        if (dx * dx + dy * dy + dz * dz > 30.f * 30.f) {
            found->second.shownX = car.x;
            found->second.shownY = car.y;
            found->second.shownZ = car.z;
        } else {
            found->second.shownX += dx * 0.35f;
            found->second.shownY += dy * 0.35f;
            found->second.shownZ += dz * 0.35f;
        }
        found->second.shownH = car.heading;
        WriteBodyPos(body, found->second.shownX, found->second.shownY, found->second.shownZ);
        if (g_setHeading) g_setHeading(body, car.heading);
    }

    // Hosted vehicles for packet 56 (SendUnoccupiedVehicleSync). Sampled here on the game thread,
    // then swapped in as one snapshot, the same way g_hostedSnap is built above.
    std::vector<HostedCar> hostedCars;
    for (const auto& car : cars) {
        if (!car.hosted) continue;
        void* body = nullptr;
        {
            std::lock_guard<std::mutex> lock(g_remoteMutex);
            auto found = g_remoteCars.find(car.id);
            if (found != g_remoteCars.end() && found->second.hosted) body = found->second.body;
        }
        HostedCar sample;
        if (!body || !SampleHostedCar(body, sample)) continue;
        sample.id = car.id;
        hostedCars.push_back(sample);
    }
    {
        std::lock_guard<std::mutex> lock(g_remoteMutex);
        g_hostedCarSnap.swap(hostedCars);
    }
    // After the car loop, so the car bodies that a seat needs exist for this tick.
    ApplyRemoteSeats();
    WatchLocalVehicle();
}

extern "C" void WorldRPG_NoAmbientPeds() {}

// Registration wardrobe preview. Lua runs on mta-net; the body, camera, clock and
// clothes run here on the game thread. One new ped body per call (skill cap).
namespace {
constexpr int kScriptPedSlots = 4;
constexpr int kClothesSlots = 4;
constexpr uint32_t kFirstScriptPed = 0x300001u;

struct ScriptPedSlot {
    uint32_t id = 0;
    int model = 0;
    float x = 0, y = 0, z = 0, heading = 0;
    bool used = false;
    bool spawn = false;
    bool drop = false;
    bool pose = false;
    bool clothes = false;
    bool frozen = false;
    void* body = nullptr;
    char texture[kClothesSlots][40]{};
    char cloth[kClothesSlots][40]{};
    bool slotOn[kClothesSlots]{};
};

std::mutex g_scriptMu;
ScriptPedSlot g_scriptPeds[kScriptPedSlots];
uint32_t g_nextScriptPed = kFirstScriptPed;

struct ScriptCam {
    float x = 0, y = 0, z = 0, lx = 0, ly = 0, lz = 0;
    bool on = false;
};
ScriptCam g_scriptCam;
bool g_scriptCamRestore = false;
bool g_scriptTimeOn = false;
int g_scriptHour = 12, g_scriptMinute = 0;
bool g_scriptWeatherOn = false;
int g_scriptWeather = 0;

struct CamFns {
    bool looked = false;
    void (*fixed)(void*, const float*, const float*) = nullptr;
    // libGTASA exports TakeControlNoEntity(CVector const&, short, int), not (int, int).
    void (*take)(void*, const float*, short, int) = nullptr;
    void (*restore)(void*) = nullptr;
    void* camera = nullptr;
};
CamFns& CamApi() {
    static CamFns api;
    if (api.looked) return api;
    api.looked = true;
#ifdef __ANDROID__
    void* handle = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_GLOBAL);
    if (!handle) return api;
    api.fixed = reinterpret_cast<void (*)(void*, const float*, const float*)>(
        dlsym(handle, "_ZN7CCamera26SetCamPositionForFixedModeERK7CVectorS2_"));
    api.take = reinterpret_cast<void (*)(void*, const float*, short, int)>(
        dlsym(handle, "_ZN7CCamera19TakeControlNoEntityERK7CVectorsi"));
    api.restore = reinterpret_cast<void (*)(void*)>(dlsym(handle, "_ZN7CCamera7RestoreEv"));
    const uintptr_t base = Hook::MemoryPatcher::getLibraryBaseAddress("libGTASA.so");
    if (base) api.camera = reinterpret_cast<void*>(base + 0xBBA8D0);
    if (!api.fixed || !api.take || !api.camera) {
        ENGINE_LOGI("DIAG setCameraMatrix: fixed=%p take=%p camera=%p restore=%p",
            reinterpret_cast<void*>(api.fixed), reinterpret_cast<void*>(api.take),
            api.camera, reinterpret_cast<void*>(api.restore));
    }
#endif
    return api;
}

struct WorldFns {
    bool looked = false;
    // CClock::SetGameClock(hour, minute, unknown). The export takes three bytes.
    void (*clock)(unsigned char, unsigned char, unsigned char) = nullptr;
    void (*weather)(short) = nullptr;
};
WorldFns& WorldApi() {
    static WorldFns api;
    if (api.looked) return api;
    api.looked = true;
#ifdef __ANDROID__
    void* handle = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_GLOBAL);
    if (!handle) return api;
    api.clock = reinterpret_cast<void (*)(unsigned char, unsigned char, unsigned char)>(
        dlsym(handle, "_ZN6CClock12SetGameClockEhhh"));
    api.weather = reinterpret_cast<void (*)(short)>(dlsym(handle, "_ZN8CWeather12ForceWeatherEs"));
    if (!api.clock)
        ENGINE_LOGI("DIAG setTime: CClock::SetGameClock missing");
    if (!api.weather)
        ENGINE_LOGI("DIAG setWeather: CWeather::ForceWeather missing");
#endif
    return api;
}

void ApplyScriptClothes(void* ped, const ScriptPedSlot& slot) {
    auto& api = Clothes();
    if (!api.rebuild || !api.setTex) {
        ENGINE_LOGI("DIAG addPedClothes: rebuild=%p set=%p",
            reinterpret_cast<void*>(api.rebuild), reinterpret_cast<void*>(api.setTex));
        return;
    }
    void* data = EnsurePlayerData(ped, true);
    if (data && api.allocData) api.allocData(data);
    void* desc = data ? *reinterpret_cast<void**>(static_cast<unsigned char*>(data) + 8) : nullptr;
    if (!desc) {
        ENGINE_LOGI("DIAG addPedClothes: clothes desc missing");
        return;
    }
    for (int i = 0; i < kClothesSlots; ++i) {
        if (!slot.slotOn[i]) api.setTex(desc, nullptr, nullptr, i);
        else api.setTex(desc, slot.texture[i], slot.cloth[i], i);
    }
    api.rebuild(ped, false);
}

void ServiceScriptStage() {
    ScriptPedSlot spawn{};
    bool doSpawn = false;
    void* drop = nullptr;
    ScriptPedSlot dress{};
    bool doDress = false;
    ScriptPedSlot pose{};
    bool doPose = false;
    ScriptCam cam{};
    bool restoreCam = false;
    bool timeOn = false;
    int hour = 0, minute = 0, weather = 0;
    bool weatherOn = false;
    {
        std::lock_guard<std::mutex> lock(g_scriptMu);
        cam = g_scriptCam;
        restoreCam = g_scriptCamRestore && !g_scriptCam.on;
        g_scriptCamRestore = false;
        timeOn = g_scriptTimeOn;
        hour = g_scriptHour;
        minute = g_scriptMinute;
        weatherOn = g_scriptWeatherOn;
        weather = g_scriptWeather;
        g_scriptTimeOn = false;
        g_scriptWeatherOn = false;
        for (auto& slot : g_scriptPeds) {
            if (!slot.used) continue;
            if (slot.drop && slot.body) {
                drop = slot.body;
                slot.body = nullptr;
                slot.drop = false;
                slot.used = false;
                break;
            }
            if (slot.spawn && !slot.body && !doSpawn) {
                spawn = slot;
                slot.spawn = false;
                doSpawn = true;
            }
        }
    }
    if (drop) ReleasePlayerPed(drop);
    if (doSpawn) {
        const float rad = spawn.heading;
        void* body = CreateRemotePlayerPed(spawn.model, spawn.x, spawn.y, spawn.z, rad, true);
        std::lock_guard<std::mutex> lock(g_scriptMu);
        bool kept = false;
        for (auto& slot : g_scriptPeds) {
            if (slot.id != spawn.id || !slot.used || slot.drop) continue;
            slot.body = body;
            kept = true;
            if (body) {
                SetPhysicalFrozen(body, slot.frozen);
                slot.pose = false;
                if (slot.clothes) {
                    dress = slot;
                    slot.clothes = false;
                    doDress = true;
                }
            }
            ENGINE_LOGI("WARDROBE createPed id=%u model=%d body=%p", slot.id, slot.model, body);
            break;
        }
        if (body && !kept) ReleasePlayerPed(body);
    }
    {
        std::lock_guard<std::mutex> lock(g_scriptMu);
        for (auto& slot : g_scriptPeds) {
            if (!slot.used || !slot.body) continue;
            if (slot.pose) {
                pose = slot;
                slot.pose = false;
                doPose = true;
            }
            if (slot.clothes && !doDress) {
                dress = slot;
                slot.clothes = false;
                doDress = true;
            }
        }
    }
    if (doPose && pose.body) {
        PlaceRemotePlayerPed(pose.body, pose.x, pose.y, pose.z, pose.heading);
        SetPhysicalFrozen(pose.body, pose.frozen);
    }
    if (doDress && dress.body) ApplyScriptClothes(dress.body, dress);
    if (timeOn) {
        auto& api = WorldApi();
        if (api.clock) api.clock(static_cast<unsigned char>(hour), static_cast<unsigned char>(minute), 0);
    }
    if (weatherOn) {
        auto& api = WorldApi();
        if (api.weather) api.weather(static_cast<short>(weather));
    }
    if (restoreCam) {
        auto& api = CamApi();
        if (api.camera && api.restore) api.restore(api.camera);
    } else if (cam.on) {
        auto& api = CamApi();
        if (api.camera && api.fixed && api.take) {
            const float pos[3] = {cam.x, cam.y, cam.z};
            const float up[3] = {0.f, 0.f, 1.f};
            const float look[3] = {cam.lx, cam.ly, cam.lz};
            api.fixed(api.camera, pos, up);
            api.take(api.camera, look, static_cast<short>(2), 1);
        }
    }
}

}  // namespace

uint32_t NoteScriptPed(int model, float x, float y, float z, float rotationDegrees) {
    if (model < 0 || model > 312 || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) return 0;
    float rad = rotationDegrees * 0.01745329251f;
    constexpr float kPi = 3.14159265f;
    if (std::isfinite(rad)) {
        while (rad < -kPi) rad += kPi * 2.f;
        while (rad > kPi) rad -= kPi * 2.f;
    } else {
        rad = 0.f;
    }
    std::lock_guard<std::mutex> lock(g_scriptMu);
    for (auto& slot : g_scriptPeds) {
        if (slot.used) continue;
        slot = {};
        slot.used = true;
        slot.spawn = true;
        slot.frozen = true;
        slot.id = g_nextScriptPed++;
        if (g_nextScriptPed < kFirstScriptPed) g_nextScriptPed = kFirstScriptPed;
        slot.model = model;
        slot.x = x;
        slot.y = y;
        slot.z = z;
        slot.heading = rad;
        return slot.id;
    }
    return 0;
}

bool ReleaseScriptPed(uint32_t id) {
    std::lock_guard<std::mutex> lock(g_scriptMu);
    for (auto& slot : g_scriptPeds) {
        if (!slot.used || slot.id != id) continue;
        slot.drop = true;
        slot.used = slot.body != nullptr;
        if (!slot.body) slot.used = false;
        return true;
    }
    return false;
}

bool ScriptPedAlive(uint32_t id) {
    std::lock_guard<std::mutex> lock(g_scriptMu);
    for (const auto& slot : g_scriptPeds)
        if (slot.used && slot.id == id && !slot.drop) return true;
    return false;
}

void NoteScriptClothes(uint32_t id, const char* texture, const char* model, int type, bool remove) {
    if (type < 0 || type >= kClothesSlots) return;
    std::lock_guard<std::mutex> lock(g_scriptMu);
    for (auto& slot : g_scriptPeds) {
        if (!slot.used || slot.id != id) continue;
        slot.slotOn[type] = !remove && texture && model && texture[0] && model[0];
        if (slot.slotOn[type]) {
            std::snprintf(slot.texture[type], sizeof slot.texture[type], "%s", texture);
            std::snprintf(slot.cloth[type], sizeof slot.cloth[type], "%s", model);
        }
        slot.clothes = true;
        return;
    }
}

void NoteScriptRotation(uint32_t id, float rotationDegrees) {
    float rad = rotationDegrees * 0.01745329251f;
    constexpr float kPi = 3.14159265f;
    if (!std::isfinite(rad)) return;
    while (rad < -kPi) rad += kPi * 2.f;
    while (rad > kPi) rad -= kPi * 2.f;
    std::lock_guard<std::mutex> lock(g_scriptMu);
    for (auto& slot : g_scriptPeds) {
        if (!slot.used || slot.id != id) continue;
        slot.heading = rad;
        slot.pose = true;
        return;
    }
}

void NoteScriptFrozen(uint32_t id, bool frozen) {
    std::lock_guard<std::mutex> lock(g_scriptMu);
    for (auto& slot : g_scriptPeds) {
        if (!slot.used || slot.id != id) continue;
        slot.frozen = frozen;
        slot.pose = true;
        return;
    }
}

void NoteScriptCamera(float x, float y, float z, float lookX, float lookY, float lookZ) {
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ||
        !std::isfinite(lookX) || !std::isfinite(lookY) || !std::isfinite(lookZ))
        return;
    std::lock_guard<std::mutex> lock(g_scriptMu);
    g_scriptCam = {x, y, z, lookX, lookY, lookZ, true};
}

void ClearScriptCamera() {
    std::lock_guard<std::mutex> lock(g_scriptMu);
    if (g_scriptCam.on) g_scriptCamRestore = true;
    g_scriptCam.on = false;
}

bool NoteScriptTime(int hour, int minute) {
    if (hour < 0 || hour > 23 || minute < 0 || minute > 60) return false;
    std::lock_guard<std::mutex> lock(g_scriptMu);
    g_scriptHour = hour;
    g_scriptMinute = minute;
    g_scriptTimeOn = true;
    return true;
}

bool NoteScriptWeather(int weather) {
    if (weather < 0 || weather > 255) return false;
    std::lock_guard<std::mutex> lock(g_scriptMu);
    g_scriptWeather = weather;
    g_scriptWeatherOn = true;
    return true;
}



extern "C" void WorldRPG_NoStoryScript(void*) {
    t_onGameThread = true;
    ServiceGroundQueries();
    ServiceScriptStage();
    static int depth = 0;
    static bool ready = false;
    if (depth > 0) return;
    if (ready) {
        ++depth;
        if (g_pedDensity) *g_pedDensity = 0.f;
        SampleLocalPose();
        SampleLocalPadAndCamera();
        ApplyRemotePeds();
        ServerWorld::Tick(g_poseX.load(), g_poseY.load(), g_poseZ.load());
        --depth;
        return;
    }
    ++depth;
    if (g_requestSpecial && g_loadAll && g_setupPlayer) {
        g_requestSpecial(0, "player", 26);
        g_loadAll(true);
        g_setupPlayer(0);
        PlacePlayer();
        DressLocalPlayer();
        if (g_endPause) g_endPause();
        ENGINE_LOGI("Player created without main.scm");
    }
    ready = true;
    ApplyRemotePeds();
    --depth;
}

// Desktop CMultiplayerSA.cpp:1415-1416 writes a ret at the start of
// CEntryExitManager::Update ("they crash when you enter anyway").
// libGTASA 2.10 arm64: CGame::Process calls Update, which calls
// CEntryExit::TransitionFinished(FindPlayerPed). That virtual-calls
// CPed::Teleport (vtable + 0x78) and the slot is not executable code
// (logcat 2026-11-13 18:31:09, SIGSEGV SEGV_ACCERR, TransitionFinished+1552).
static void DisableEntryExits() {
    constexpr std::uint32_t kOriginalWord = 0xd10283ffu;  // sub sp, sp, #0xa0
    constexpr std::uint32_t kReturnWord = 0xd65f03c0u;    // ret

    void* handle = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_GLOBAL);
    if (!handle) {
        ENGINE_LOGE("CEntryExitManager::Update: libGTASA.so not loaded");
        return;
    }
    void* fn = dlsym(handle, "_ZN17CEntryExitManager6UpdateEv");
    if (!fn) {
        ENGINE_LOGE("CEntryExitManager::Update symbol missing");
        return;
    }

    auto* site = static_cast<unsigned char*>(fn);
    std::uint32_t word = 0;
    std::memcpy(&word, site, sizeof(word));
    if (word == kReturnWord) return;
    if (word != kOriginalWord) {
        ENGINE_LOGE("CEntryExitManager::Update: unexpected word 0x%08x", word);
        return;
    }

    const long pageSizeLong = sysconf(_SC_PAGESIZE);
    if (pageSizeLong <= 0) {
        ENGINE_LOGE("CEntryExitManager::Update: sysconf failed");
        return;
    }
    const auto pageSize = static_cast<std::uintptr_t>(pageSizeLong);
    const auto addr = reinterpret_cast<std::uintptr_t>(site);
    void* page = reinterpret_cast<void*>(addr & ~(pageSize - 1));
    if (mprotect(page, pageSize, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        ENGINE_LOGE("CEntryExitManager::Update: mprotect RWX failed");
        return;
    }
    std::memcpy(site, &kReturnWord, sizeof(kReturnWord));
    if (mprotect(page, pageSize, PROT_READ | PROT_EXEC) != 0) {
        ENGINE_LOGE("CEntryExitManager::Update: mprotect RX failed");
    }
    __builtin___clear_cache(reinterpret_cast<char*>(site), reinterpret_cast<char*>(site) + 4);
    ENGINE_LOGI("Disabled CEntryExitManager::Update (desktop CMultiplayerSA.cpp:1416)");
}

// Crash: null m_pWidgets[159], write at +0xb1. CWidgetRegionGang::Update+168
// (0x381434) loads that slot and stores the gang widget enabled byte
// (CWidget+0x59). The slot is CWidgetReplay; null makes the store address 0xb1
// (SIGSEGV on MainThread, Idle -> CTouchInterface::Update+108 at 0x36fae0,
// 2026-11-13 21:47:22 and 23:23:51). Desktop has no touch widgets and returns
// from CReplay::Update (CMultiplayerSA.cpp:1739) without constructing
// CWidgetRegionGang. Do not allocate one.
//
// The store is the only use of slot 159. The rest of Update (SetEnabled,
// CWidgetRegion::Update, ManageAlpha, BuildRects) null-checks its other
// loads. Patch that one strb to a bl. The prologue has already saved x30,
// and the epilogue reloads it before the tail call, so this bl does not
// change the return to CTouchInterface::Update. An entry trampoline is the
// wrong tool here: installHook copies 16 bytes and enters the replacement
// with BR, and a bad copy freezes Idle inside CTouchInterface::Update.
// x8 is the widget (or null), w9 is the byte, x0 is already `this`.
__attribute__((naked)) static void GangReplaySlotStore() {
    __asm__ volatile(
        "cbz x8, 1f\n\t"
        "strb w9, [x8, #0xb1]\n\t"
        "1:\n\t"
        "ret\n\t");
}

static bool WriteExecutable(void* site, const void* bytes, size_t size) {
    const long pageSizeLong = sysconf(_SC_PAGESIZE);
    if (pageSizeLong <= 0) return false;
    const auto pageSize = static_cast<std::uintptr_t>(pageSizeLong);
    const auto first = reinterpret_cast<std::uintptr_t>(site);
    const auto start = first & ~(pageSize - 1);
    const auto end = (first + size + pageSize - 1) & ~(pageSize - 1);
    if (mprotect(reinterpret_cast<void*>(start), end - start,
            PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        return false;
    }
    std::memcpy(site, bytes, size);
    __builtin___clear_cache(reinterpret_cast<char*>(site),
        reinterpret_cast<char*>(site) + size);
    mprotect(reinterpret_cast<void*>(start), end - start, PROT_READ | PROT_EXEC);
    return true;
}

static void InstallGangRegionUpdateSkip() {
    // Update+0x9c .. +0xa8. The last word is the crashing store.
    constexpr std::uint32_t kLoadSlot = 0xf9427e88u;    // ldr x8, [x20, #0x4f8]
    constexpr std::uint32_t kLoadByte = 0x39416669u;    // ldrb w9, [x19, #0x59]
    constexpr std::uint32_t kMoveThis = 0xaa1303e0u;    // mov x0, x19
    constexpr std::uint32_t kStoreByte = 0x3902c509u;   // strb w9, [x8, #0xb1]
    constexpr std::uintptr_t kStoreOffset = 0xa8;

    void* handle = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_GLOBAL);
    if (!handle) {
        ENGINE_LOGE("CWidgetRegionGang::Update: libGTASA.so not loaded");
        return;
    }
    void* fn = dlsym(handle, "_ZN17CWidgetRegionGang6UpdateEv");
    if (!fn) {
        ENGINE_LOGE("CWidgetRegionGang::Update: symbol missing");
        return;
    }

    auto* site = static_cast<unsigned char*>(fn) + kStoreOffset;
    std::uint32_t words[4] = {};
    std::memcpy(words, site - 12, sizeof(words));
    if (words[0] != kLoadSlot || words[1] != kLoadByte || words[2] != kMoveThis) {
        ENGINE_LOGE("CWidgetRegionGang::Update: unexpected prefix %08x %08x %08x",
            words[0], words[1], words[2]);
        return;
    }
    if (words[3] != kStoreByte) {
        ENGINE_LOGE("CWidgetRegionGang::Update: store is 0x%08x, left unchanged", words[3]);
        return;
    }

    const auto stub = reinterpret_cast<std::uintptr_t>(&GangReplaySlotStore);
    const auto at = reinterpret_cast<std::uintptr_t>(site);
    const std::int64_t imm = (static_cast<std::int64_t>(stub) - static_cast<std::int64_t>(at)) >> 2;
    constexpr std::int64_t kBlImm = 1LL << 25;
    if (imm < -kBlImm || imm >= kBlImm) {
        ENGINE_LOGE("CWidgetRegionGang::Update: stub out of bl range");
        return;
    }
    const std::uint32_t branch = 0x94000000u | (static_cast<std::uint32_t>(imm) & 0x03ffffffu);
    if (!WriteExecutable(site, &branch, sizeof(branch))) {
        ENGINE_LOGE("CWidgetRegionGang::Update: mprotect failed");
        return;
    }
    ENGINE_LOGI("DIAG skip gang replay store when m_pWidgets[159] is null (strb at +0xb1)");
}

// RenderQueue::Flush (0x267878) on the multithreaded path sets byte +0x410
// and tail-calls OS_SemaphoreWait. GraphicsThread (0x267968) clears that
// byte only after it has drained the ring and run both SetSafe calls, then
// posts the semaphore. Live hang, 2026-11-13 23:39, engine pid 12704:
// MainThread was in sem_wait on that semaphore (value 0xfffffffe, return
// into RQVertexBuffer::Set at +0x120, Idle -> CFont::RenderFontBuffer),
// while GraphicsThread was in usleep(30) at GraphicsThread+0x2b0 and the
// byte was already 0. The post never woke the waiter, so Idle never
// returned. Completion is the byte, not the semaphore.
using RenderQueueFlushFn = void (*)(void*);
static RenderQueueFlushFn g_renderQueueFlush = nullptr;
static std::uintptr_t g_gtasaBase = 0;

static void* RenderQueueFlushSemaphore() {
    if (!g_gtasaBase) return nullptr;
    // RenderQueue::Initialize stores the semaphore through the pointer at
    // this link address (adrp 0x84e000 + 0x608).
    constexpr std::uintptr_t kSemSlot = 0x84e608;
    auto* slot = *reinterpret_cast<void**>(g_gtasaBase + kSemSlot);
    if (!slot) return nullptr;
    return *reinterpret_cast<void**>(slot);
}

static void RenderQueueFlush(void* queue) {
    auto* bytes = static_cast<unsigned char*>(queue);
    // +0x3e8 is the multithreaded flag set by RenderQueue::RenderQueue.
    if (!bytes[0x3e8]) {
        if (g_renderQueueFlush) g_renderQueueFlush(queue);
        return;
    }
    __atomic_store_n(bytes + 0x410, static_cast<unsigned char>(1), __ATOMIC_RELEASE);
    int waits = 0;
    while (__atomic_load_n(bytes + 0x410, __ATOMIC_ACQUIRE)) {
        sched_yield();
        if (!__atomic_load_n(bytes + 0x410, __ATOMIC_ACQUIRE)) break;
        usleep(200);
        if (++waits == 5000)
            ENGINE_LOGI("DIAG RenderQueue::Flush waited 1s for the graphics thread");
    }
    // GraphicsThread posts after clearing the byte. Consume the token when
    // it has arrived. Do not block if the post was the one that never woke
    // the old waiter: the byte is the completion condition.
    if (void* sem = RenderQueueFlushSemaphore()) {
        for (int i = 0; i < 4; ++i) {
            if (sem_trywait(static_cast<sem_t*>(sem)) != 0) break;
        }
    }
}

static void InstallRenderQueueFlush(std::uintptr_t base) {
    constexpr std::uint32_t kOriginalWord = 0xf81e0ff3u;  // str x19, [sp, #-0x20]!
    g_gtasaBase = base;
    void* handle = dlopen("libGTASA.so", RTLD_NOLOAD | RTLD_GLOBAL);
    if (!handle) {
        ENGINE_LOGE("RenderQueue::Flush: libGTASA.so not loaded");
        return;
    }
    void* fn = dlsym(handle, "_ZN11RenderQueue5FlushEv");
    if (!fn) {
        ENGINE_LOGE("RenderQueue::Flush: symbol missing");
        return;
    }
    std::uint32_t word = 0;
    std::memcpy(&word, fn, sizeof(word));
    if (word != kOriginalWord) {
        ENGINE_LOGE("RenderQueue::Flush: unexpected word 0x%08x", word);
        return;
    }
    // First 16 bytes are str/stp/add/mov of x19. None are PC-relative.
    static Hook::MemoryPatcher patcher;
    const auto hooked = patcher.installHook(reinterpret_cast<uintptr_t>(fn),
        reinterpret_cast<uintptr_t>(&RenderQueueFlush));
    if (!hooked.success) {
        ENGINE_LOGE("RenderQueue::Flush hook failed: %s", hooked.errorMessage.c_str());
        return;
    }
    g_renderQueueFlush = reinterpret_cast<RenderQueueFlushFn>(hooked.trampolineAddress);
    ENGINE_LOGI("DIAG RenderQueue::Flush waits on byte +0x410, not the semaphore");
}

bool BeginServerSession() {
    static bool leftMenu = false;
    const uintptr_t base = Hook::MemoryPatcher::getLibraryBaseAddress("libGTASA.so");
    if (leftMenu) {
        if (!base) return true;
        constexpr uintptr_t kPendingPause = 0x00D3D664;
        constexpr uintptr_t kGotMobileMenu = 0x008519B0;
        auto* pending = reinterpret_cast<uint8_t*>(base + kPendingPause);
        uintptr_t menu = *reinterpret_cast<uintptr_t*>(base + kGotMobileMenu);
        AllowWrite(pending, 1);
        const bool menuUp = menu && (*reinterpret_cast<int32_t*>(menu + 0x24) != 0 ||
            *reinterpret_cast<uintptr_t*>(menu + 0x30) != 0);
        if (*pending || menuUp) {
            *pending = 0;
            if (menuUp && g_removeTop) {
                g_mobileMenu = reinterpret_cast<void*>(menu);
                for (int i = 0; i < 8 && *reinterpret_cast<int32_t*>(menu + 0x24) > 0; ++i)
                    g_removeTop(g_mobileMenu, 1, 1);
                if (g_switchOff) g_switchOff();
            }
            ::WorldRPG::JniBridge::PostGameLine(9, "open");
        }
        return true;
    }
    if (!base) return false;
    // libGTASA 2.10 ARM64. DoGameState is the stock switch MobileSA copies.
    constexpr uintptr_t kGameState = 0x00D3D640;
    constexpr uintptr_t kGotMobileMenu = 0x008519B0;
    constexpr uintptr_t kScriptProcess = 0x003f3f18;
    auto* state = reinterpret_cast<int32_t*>(base + kGameState);
    AllowWrite(state, sizeof(int32_t));
    ResolvePlayerFns();
    static bool scriptBlocked = false;
    if (!scriptBlocked) {
        static Hook::MemoryPatcher patcher;
        const auto hooked = patcher.installHook(base + kScriptProcess,
            reinterpret_cast<uintptr_t>(&WorldRPG_NoStoryScript));
        scriptBlocked = hooked.success;
        g_originalScript = hooked.success
            ? reinterpret_cast<ScriptProcessFn>(hooked.trampolineAddress) : nullptr;
        (void)g_originalScript;
        if (!hooked.success) {
            ENGINE_LOGE("Could not block CRunningScript::Process: %s", hooked.errorMessage.c_str());
            return false;
        }
        ENGINE_LOGI("Blocked CRunningScript::Process; main.scm will not run");
        if (!InstallRemoteControlHook())
            ENGINE_LOGE("Remote players keep the velocity fallback (no controller hook)");
        if (!InstallRenderGuard())
            ENGINE_LOGE("RenderGuard not installed; the sortLODs crash is unguarded");
        if (!InstallLocalBulletFireHook())
            ENGINE_LOGE("Local bullet sync (PLAYER_BULLETSYNC) disabled: CWeapon::Fire hook missing");
        ENGINE_LOGI("Ped symbols add=%p ctor=%p heading=%p density=%p manage=%p",
            reinterpret_cast<void*>(g_addPed), reinterpret_cast<void*>(g_civilianCtor),
            reinterpret_cast<void*>(g_setHeading), static_cast<void*>(g_pedDensity), g_managePopulation);
        if (g_managePopulation) {
            const auto population = patcher.installHook(reinterpret_cast<uintptr_t>(g_managePopulation),
                reinterpret_cast<uintptr_t>(&WorldRPG_NoAmbientPeds));
            if (population.success) ENGINE_LOGI("Stopped local ped population");
            else ENGINE_LOGE("Could not stop local ped population: %s", population.errorMessage.c_str());
        } else {
            ENGINE_LOGE("CPopulation::ManagePopulation was not exported");
        }
        DisableEntryExits();
        InstallGangRegionUpdateSkip();
        InstallRenderQueueFlush(base);
    }
    // Idle opens MobileMenu::InitForPause when this byte is set. The story
    // script is what normally consumes it. Keep it clear until the world is up.
    constexpr uintptr_t kPendingPause = 0x00D3D664;
    auto* pending = reinterpret_cast<uint8_t*>(base + kPendingPause);
    AllowWrite(pending, 1);
    *pending = 0;
    uintptr_t menu = *reinterpret_cast<uintptr_t*>(base + kGotMobileMenu);
    g_mobileMenu = reinterpret_cast<void*>(menu);
    const int32_t current = *state;
    if (current == 7) {
        // +0x6d is New Game and +0x6c is the restart MainGameFunc turns
        // into DoGameRestart. Both stay clear so the story does not start.
        if (menu) {
            auto* flags = reinterpret_cast<uint8_t*>(menu + 0x6c);
            AllowWrite(flags, 2);
            flags[0] = 0;
            flags[1] = 0;
        }
        *state = 8;
        ENGINE_LOGI("Server start: gGameState 7 -> 8 without the New Game flag");
        return true;
    }
    if (current < 9) return true;
    // Idle can open the pause a frame after the state reaches 9. Keep
    // clearing it until that frame has happened and the menu is gone.
    static int seenPlaying = 0;
    ++seenPlaying;
    const int32_t screens = menu ? *reinterpret_cast<int32_t*>(menu + 0x24) : -1;
    const uintptr_t top = menu ? *reinterpret_cast<uintptr_t*>(menu + 0x30) : 0;
    if (seenPlaying == 30) {
        ENGINE_LOGI("After start: screens=%d top=%p pending cleared", screens, reinterpret_cast<void*>(top));
    }
    const bool menuUp = screens != 0 || top != 0;
    // Unload only frees menu textures. The pause screen stays until
    // RemoveTopScreen pops it, which is what the Resume button does.
    if (menuUp && g_removeTop && g_mobileMenu) {
        for (int i = 0; i < 8 && *reinterpret_cast<int32_t*>(menu + 0x24) > 0; ++i)
            g_removeTop(g_mobileMenu, 1, 1);
        if (g_switchOff) g_switchOff();
        if (seenPlaying < 4) ENGINE_LOGI("Closed the GTA pause menu");
    }
    if (g_endPause) g_endPause();
    if (seenPlaying > 120 && !menuUp) leftMenu = true;
    return true;
}
void ApplyServerSpawn(float x, float y, float z) {
    g_spawnX = x;
    g_spawnY = y;
    g_spawnZ = z;
    g_poseX = x;
    g_poseY = y;
    g_poseZ = z;
    g_health = 100.f;
    g_armor = 0.f;
    g_healthOwned = false;
    g_damagePending = false;
    PlacePlayer();
}
void CopySpawn(float& x, float& y, float& z) {
    x = g_spawnX.load();
    y = g_spawnY.load();
    z = g_spawnZ.load();
}
void CopyPose(float& x, float& y, float& z, float& heading) {
    x = g_poseX.load();
    y = g_poseY.load();
    z = g_poseZ.load();
    heading = g_poseH.load();
}
float CopyRadarHeading() { return g_radarH.load(); }
void CopyBody(float& x, float& y, float& z, float& heading,
    float& vx, float& vy, float& vz, float& health) {
    CopyPose(x, y, z, heading);
    vx = g_velX.load();
    vy = g_velY.load();
    vz = g_velZ.load();
    health = g_health.load();
}
float CopyLocalArmor() { return g_armor.load(); }
void NoteDamage(uint32_t attacker, float amount, uint8_t weapon) {
    if (attacker == 0 || attacker == 0xFFFFFFFFu || amount <= 0.f) return;
    const int64_t now = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    std::atomic<int64_t>& stamp = weapon < 16 ? g_lastMeleeMs : g_lastDamageMs;
    const int64_t gate = weapon < 16 ? 650 : 150;
    int64_t previous = stamp.load();
    if (now - previous < gate) return;
    if (!stamp.compare_exchange_strong(previous, now)) return;
    float health = g_health.load();
    health = std::max(0.f, health - amount);
    g_health = health;
    g_healthOwned = true;
    g_damager = attacker;
    g_damageWeapon = weapon;
    g_damagePending = true;
}
bool ConsumeDamage(uint32_t& attacker, uint8_t& weapon) {
    if (!g_damagePending.exchange(false)) return false;
    attacker = g_damager.load();
    weapon = g_damageWeapon.load();
    return attacker != 0 && attacker != 0xFFFFFFFFu;
}
void NoteRemoteAim(uint32_t id, float heading, float camera, int stickX, int stickY, bool fire, bool duck) {
    if (id == 0 || id == 0xFFFFFFFFu || id == ServerWorld::LocalPlayer()) return;
    const float pi = 3.14159265f;
    while (heading > pi) heading -= 2.f * pi;
    while (heading < -pi) heading += 2.f * pi;
    const float along = -static_cast<float>(stickY) / 128.f;
    const float side = static_cast<float>(stickX) / 128.f;
    const float mag = std::sqrt(along * along + side * side);
    const float scale = mag > 0.7f ? 0.18f : 0.09f;
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    auto& ped = g_remotePeds[id];
    ped.heading = heading;
    ped.fire = fire;
    ped.duck = duck;
    // CClientPed::SetControllerState replaces the whole controller (CClientPed.cpp 1460-1484).
    // A zero stick overwrites the previous one. ReadKeysync does not call SetMoveSpeed.
    ped.stickX = stickX;
    ped.stickY = stickY;
    ped.cameraRotation = camera;
    ped.keysync = true;
    // With the controller hook the game moves the ped from the stick, so the
    // velocity fallback stays off while the stick is held. A zero stick clears
    // the stored speed PresentOnFoot turns into SetMoveState. ReadKeysync does
    // not call SetMoveSpeed; the body write is the next pure sync's SVelocitySync.
    if (mag > 0.08f) {
        if (!RemoteControlHookActive()) {
            // Inverse of CClientPed: stick Y negative is forward along the camera.
            const float input = std::atan2(-static_cast<float>(stickX), -static_cast<float>(stickY));
            const float desired = input - camera;
            ped.vx = -std::sin(desired) * scale;
            ped.vy = std::cos(desired) * scale;
        }
    } else {
        ped.vx = 0.f;
        ped.vy = 0.f;
    }
    ped.dirty = true;
}
void NoteRemoteWeapon(uint32_t id, int weapon) {
    if (id == 0 || id == 0xFFFFFFFFu || weapon <= 0 || weapon > 46) return;
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    g_remotePeds[id].weapon = weapon;
}
bool RemoteFacingLocal(uint32_t id, float facing, float reach) {
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    auto found = g_remotePeds.find(id);
    if (found == g_remotePeds.end()) return false;
    const float dx = g_poseX.load() - found->second.x;
    const float dy = g_poseY.load() - found->second.y;
    if (dx * dx + dy * dy > reach * reach) return false;
    const float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.05f) return true;
    const float fx = -std::sin(facing);
    const float fy = std::cos(facing);
    return (fx * dx + fy * dy) / len > 0.35f;
}
void CopyOnFoot(float heading, float vx, float vy, OnFootKeys& keys) {
    keys.fire = g_localFire.load();
    keys.duck = g_localDuck.load();
    keys.padStickZero = false;
    const float pi = 3.14159265f;
    // CNetAPI::WritePlayerPuresync / WriteKeysync: SCameraRotationSync is GetCameraRotation,
    // the float CTaskSimplePlayerOnFoot subtracts from the stick angle (TheCamera+0x154).
    float camera = g_cameraRotValid.load() ? g_cameraRot.load() : 0.f;
    const float turns = pi * 2.f;
    camera = camera - turns * std::floor((camera + pi) / turns);
    keys.camera = camera;
    // CPad::GetPad(0) shorts. A released stick stays 0 even while CPhysical+0x68 is still moving.
    if (g_padStickValid.load()) {
        const int rawX = g_padStickX.load();
        const int rawY = g_padStickY.load();
        if (rawX == 0 && rawY == 0) {
            keys.stickX = 0;
            keys.stickY = 0;
            keys.padStickZero = true;
            return;
        }
        // SFullKeysyncSync stretches -128..128 into a char with *127/128 (SyncStructures.h 998-1001).
        auto wireStick = [](int raw) {
            const int scaled = static_cast<int>(std::lround(static_cast<float>(raw) * 127.f / 128.f));
            return std::clamp(scaled, -127, 127);
        };
        keys.stickX = wireStick(rawX);
        keys.stickY = wireStick(rawY);
        return;
    }
    const float speed = std::sqrt(vx * vx + vy * vy);
    if (speed < 0.02f) {
        keys.stickX = 0;
        keys.stickY = 0;
        return;
    }
    const float desired = std::atan2(-vx, vy);
    const float input = desired + camera;
    const float mag = speed > 0.16f ? 108.f : 59.f;
    keys.stickX = static_cast<int>(std::lround(std::clamp(-std::sin(input) * mag, -127.f, 127.f) * 127.f / 128.f));
    keys.stickY = static_cast<int>(std::lround(std::clamp(-std::cos(input) * mag, -127.f, 127.f) * 127.f / 128.f));
    (void)heading;
}
void NoteRemotePlayerSync(uint32_t id, float x, float y, float z, float heading,
    float vx, float vy, float vz, bool hasVelocity,
    int stickX, int stickY, float camera, bool fire, bool duck,
    bool hasHealth, float health, bool hasArmor, float armor) {
    if (id == 0 || id == 0xFFFFFFFFu || id == ServerWorld::LocalPlayer()) return;
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    auto& ped = g_remotePeds[id];
    ped.x = x;
    ped.y = y;
    ped.z = z;
    ped.heading = heading;
    ped.vx = vx;
    ped.vy = vy;
    ped.vz = vz;
    ped.syncMoveSpeed = hasVelocity;
    // SetControllerState replacement. Sticks are already in pad units (-128..128).
    ped.stickX = stickX;
    ped.stickY = stickY;
    ped.cameraRotation = camera;
    ped.fire = fire;
    ped.duck = duck;
    ped.keysync = true;
    if (hasHealth && std::isfinite(health)) {
        ped.health = health;
        ped.syncHealth = true;
    }
    if (hasArmor && std::isfinite(armor)) {
        ped.armor = armor;
        ped.syncArmor = true;
    }
    ped.dirty = true;
}
void NoteRemotePed(uint32_t id, float x, float y, float z, float heading, float vx, float vy, float vz, bool applyVelocity) {
    if (id == 0 || id == 0xFFFFFFFFu) return;
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    auto& ped = g_remotePeds[id];
    ped.x = x;
    ped.y = y;
    ped.z = z;
    ped.heading = heading;
    // CPedSync::Packet_PedSync sets move speed only when ucFlags & 0x04
    // (CPedSync.cpp 434-437). A position-only packet must not clear the last speed.
    if (applyVelocity) {
        ped.vx = vx;
        ped.vy = vy;
        ped.vz = vz;
    }
    ped.dirty = true;
}
void SetPedHost(uint32_t id, bool host) {
    if (id == 0 || id == 0xFFFFFFFFu) return;
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    g_remotePeds[id].hosted = host;
    if (!host) g_remotePeds[id].wander = false;
}
void ForgetRemote(uint32_t id) {
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    g_forget.push_back(id);
}

// Reconnect (P3.1): the network thread clears the record set; the game thread
// drops the bodies in ApplyRemotePeds on its next tick.
void RequestRemoteReset() {
    {
        std::lock_guard<std::mutex> lock(g_remoteMutex);
        g_remotePeds.clear();
        g_forget.clear();
    }
    g_remoteResetPending = true;
}
void NoteRemoteVehicle(uint32_t id, int model, float x, float y, float z, float heading) {
    if (id == 0 || id == 0xFFFFFFFFu || !PlausiblePosition(x, y, z)) return;
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    auto& car = g_remoteCars[id];
    if (model >= 400) car.model = model;
    car.x = x;
    car.y = y;
    car.z = z;
    car.heading = heading;
    car.dirty = true;
}
// Desktop CUnoccupiedVehicleSync::Packet_UnoccupiedVehicleStartSync (CUnoccupiedVehicleSync.cpp 104-158): target
// position, target rotation, move speed, turn speed and health are set on the vehicle before AddVehicle. The values are
// applied to the body on the game thread (ApplyCarSeed). rot is in degrees.
void NoteVehicleStartSync(uint32_t id, const float pos[3], const float rot[3], const float move[3], const float turn[3],
    float health) {
    if (id == 0 || id == 0xFFFFFFFFu || !pos || !rot || !move || !turn) return;
    if (!PlausiblePosition(pos[0], pos[1], pos[2])) return;
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    auto& car = g_remoteCars[id];
    car.x = pos[0];
    car.y = pos[1];
    car.z = pos[2];
    car.heading = rot[2] * 3.14159265f / 180.f;
    car.dirty = true;
    CarSeed& seed = car.seed;
    seed.hasPose = true;
    seed.x = pos[0];
    seed.y = pos[1];
    seed.z = pos[2];
    seed.hasRotation = true;
    seed.rotX = rot[0];
    seed.rotY = rot[1];
    seed.rotZ = rot[2];
    seed.hasMotion = true;
    for (int i = 0; i < 3; ++i) {
        seed.move[i] = std::isfinite(move[i]) ? move[i] : 0.f;
        seed.turn[i] = std::isfinite(turn[i]) ? turn[i] : 0.f;
    }
    seed.hasHealth = true;
    seed.health = health;
    seed.pending = true;
}
// Desktop CPacketHandler::Packet_Vehicle_InOut seats or unseats the ped. The request is applied in ApplyRemoteSeats.
void NoteRemoteSeat(uint32_t pedId, uint32_t vehicleId, uint8_t seat) {
    if (pedId == 0 || pedId == 0xFFFFFFFFu || vehicleId == 0 || vehicleId == 0xFFFFFFFFu) return;
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    RemoteSeatRequest& req = g_seatRequests[pedId];
    req.vehicleId = vehicleId;
    req.seat = seat;
    req.seated = true;
    req.serial = ++g_seatSerial;
    req.ticks = 0;
}
void NoteRemoteUnseat(uint32_t pedId) {
    if (pedId == 0 || pedId == 0xFFFFFFFFu) return;
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    RemoteSeatRequest& req = g_seatRequests[pedId];
    req.vehicleId = kNoVehicle;
    req.seat = 0;
    req.seated = false;
    req.serial = ++g_seatSerial;
    req.ticks = 0;
}
void NoteRemoteVehicleMotion(uint32_t id, float mx, float my, float mz, float tx, float ty, float tz) {
    if (id == 0 || id == 0xFFFFFFFFu) return;
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    // Only a car that already has a pose (NoteRemoteVehicle from the same packet, or VEHICLE_START). Creating one here
    // would let CopyRemotePose report a car at the origin.
    auto found = g_remoteCars.find(id);
    if (found == g_remoteCars.end()) return;
    RemoteCar& car = found->second;
    car.moveX = mx;
    car.moveY = my;
    car.moveZ = mz;
    car.turnX = tx;
    car.turnY = ty;
    car.turnZ = tz;
    car.motionPending = true;
}
void RequestWander(uint32_t id, int direction) {
    if (id == 0 || id == 0xFFFFFFFFu) return;
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    auto& ped = g_remotePeds[id];
    ped.wander = true;
    ped.wanderDir = direction < 0 ? 0 : direction;
}
bool CopyRemotePose(uint32_t id, float& x, float& y, float& z) {
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    auto found = g_remotePeds.find(id);
    if (found != g_remotePeds.end()) {
        x = found->second.shown ? found->second.shownX : found->second.x;
        y = found->second.shown ? found->second.shownY : found->second.y;
        z = found->second.shown ? found->second.shownZ : found->second.z;
        return true;
    }
    auto car = g_remoteCars.find(id);
    if (car == g_remoteCars.end()) return false;
    x = car->second.shown ? car->second.shownX : car->second.x;
    y = car->second.shown ? car->second.shownY : car->second.y;
    z = car->second.shown ? car->second.shownZ : car->second.z;
    return true;
}
int CopyHostedPeds(HostedPed* out, int capacity) {
    if (!out || capacity <= 0) return 0;
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    const int count = std::min(capacity, static_cast<int>(g_hostedSnap.size()));
    for (int i = 0; i < count; ++i) out[i] = g_hostedSnap[static_cast<size_t>(i)];
    return count;
}
int CopyHostedCars(HostedCar* out, int max) {
    if (!out || max <= 0) return 0;
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    const int count = std::min(max, static_cast<int>(g_hostedCarSnap.size()));
    for (int i = 0; i < count; ++i) out[i] = g_hostedCarSnap[static_cast<size_t>(i)];
    return count;
}
// VEHICLE_INOUT source queue (see WatchLocalVehicle). The network thread drains it from Pulse.
bool ConsumeLocalVehicleEvent(LocalVehicleEvent& out) {
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    if (g_localVehicleEvents.empty()) return false;
    out = g_localVehicleEvents.front();
    g_localVehicleEvents.erase(g_localVehicleEvents.begin());
    return true;
}
uint32_t LocalVehicleId() {
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    return g_localVehicleId;
}
bool CopyLocalCar(LocalCarState& out) {
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    if (!g_localCarValid) return false;
    out = g_localCar;
    return true;
}
// Desktop has no queue: CClientGame::PostWeaponFire sends at once. The queue only bridges the game thread
// (CWeapon::Fire hook) and the network thread (SendBulletSync). At the cap the oldest shot is dropped.
void NoteLocalShot(int weapon, const float* start, const float* end) {
    if (!end) return;
    LocalShot shot;
    shot.weapon = weapon;
    if (start) {
        shot.sx = start[0];
        shot.sy = start[1];
        shot.sz = start[2];
    } else {
        float heading = 0.f;
        CopyPose(shot.sx, shot.sy, shot.sz, heading);
    }
    shot.ex = end[0];
    shot.ey = end[1];
    shot.ez = end[2];
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    if (g_localShots.size() >= kMaxLocalShots) g_localShots.erase(g_localShots.begin());
    g_localShots.push_back(shot);
}
bool ConsumeLocalShot(LocalShot& out) {
    std::lock_guard<std::mutex> lock(g_remoteMutex);
    if (g_localShots.empty()) return false;
    out = g_localShots.front();
    g_localShots.erase(g_localShots.begin());
    return true;
}

#else
bool BeginServerSession() { return false; }
void ApplyServerSpawn(float, float, float) {}
void CopySpawn(float& x, float& y, float& z) { x = y = z = 0.f; }
void CopyPose(float& x, float& y, float& z, float& heading) { x = y = z = heading = 0.f; }
float CopyRadarHeading() { return 0.f; }
void CopyBody(float& x, float& y, float& z, float& heading, float& vx, float& vy, float& vz, float& health) {
    x = y = z = heading = vx = vy = vz = 0.f;
    health = 100.f;
}
float CopyLocalArmor() { return 0.f; }
bool FindGroundZFor3DPosition(float, float, float, float&) { return false; }
void NoteDamage(uint32_t, float, uint8_t) {}
bool ConsumeDamage(uint32_t&, uint8_t&) { return false; }
void NoteRemoteAim(uint32_t, float, float, int, int, bool, bool) {}
void NoteRemoteWeapon(uint32_t, int) {}
bool RemoteFacingLocal(uint32_t, float, float) { return false; }
void CopyOnFoot(float, float, float, OnFootKeys& keys) { keys = {}; }
void NoteRemotePlayerSync(uint32_t, float, float, float, float, float, float, float, bool, int, int, float, bool, bool, bool, float, bool, float) {}
void NoteRemotePed(uint32_t, float, float, float, float, float, float, float, bool) {}
void SetPedHost(uint32_t, bool) {}
void ForgetRemote(uint32_t) {}
void RequestRemoteReset() {}
void NoteRemoteVehicle(uint32_t, int, float, float, float, float) {}
void NoteVehicleStartSync(uint32_t, const float*, const float*, const float*, const float*, float) {}
void NoteRemoteSeat(uint32_t, uint32_t, uint8_t) {}
void NoteRemoteUnseat(uint32_t) {}
void NoteRemoteVehicleMotion(uint32_t, float, float, float, float, float, float) {}
void RequestWander(uint32_t, int) {}
bool CopyRemotePose(uint32_t, float& x, float& y, float& z) { x = y = z = 0.f; return false; }
int CopyHostedPeds(HostedPed*, int) { return 0; }
int CopyHostedCars(HostedCar*, int) { return 0; }
bool ConsumeLocalVehicleEvent(LocalVehicleEvent&) { return false; }
uint32_t LocalVehicleId() { return 0xFFFFFFFFu; }
bool CopyLocalCar(LocalCarState& out) {
    out = LocalCarState{};
    return false;
}
void NoteLocalShot(int, const float*, const float*) {}
bool ConsumeLocalShot(LocalShot& out) {
    out = LocalShot{};
    return false;
}
void SetPedNativeTaskLocomotion(uint32_t, unsigned int) {}
void SetPedNativeTaskWeapon(uint32_t, uint8_t) {}
void SetPedNativeTaskAnimation(uint32_t, uint16_t, uint16_t) {}
void SetPedNativeTaskPhysicalTakeover(uint32_t, unsigned int, uint16_t, uint16_t, float) {}
void SetPedCameraRotation(uint32_t, float) {}
void SetPedOnFire(uint32_t, bool) {}
void SetPedInWater(uint32_t, bool) {}
void KillPedAnimation(uint32_t) {}
void LockPedHealth(uint32_t, float) {}
void LockPedArmor(uint32_t, float) {}
void LockPedHealthCurrent(uint32_t) {}
void LockPedArmorCurrent(uint32_t) {}
bool CopyPedVitalsLock(uint32_t, bool& healthLocked, float& health, bool& armorLocked, float& armor) {
    healthLocked = false;
    armorLocked = false;
    health = 0.f;
    armor = 0.f;
    return false;
}
#endif

// ---------------------------------------------------------------------------
// EngineOffsets factory
// ---------------------------------------------------------------------------

EngineOffsets EngineOffsets::forVersion(const char* version) {
    EngineOffsets offsets{};
    std::memset(&offsets, 0, sizeof(offsets));

    if (!version) return offsets;

    // Source of truth: GTASAOffsets (verified ARM64 v2.10 table extracted from
    // the SA-MP-2.10 source and cross-checked against libGTASA_v210.so).
    //
    // Only entries whose ARM64 call signature is proven to match the
    // corresponding typedef in GameEngine::Impl are mapped here. Entries are
    // deliberately left zero when the verified table documents a DIFFERENT
    // function or an unproven signature, because calling them through the
    // wrong prototype would corrupt the engine state or segfault:
    //  - requestModel: table entry 0x391FF0 is CStreaming::RemoveModel
    //    (the exported CStreaming::RequestModel sits at 0x3949e0 — pending
    //    ABI probe before wiring). Model requests go through ScriptCommand.
    //  - getPlayerPed: table entry 0x671BBC is FindPlayerSpeed, not a
    //    FindPlayerPed accessor.
    //  - ped/vehicle/object create+delete and position setters: table entries
    //    are CWorld::Add/Remove (CEntity* parameter) and PLT hooks with
    //    engine-internal signatures; none matches the handle-based typedefs.
    if (std::strcmp(version, "2.10") == 0) {
        const GTASAOffsets table = GTASAOffsets::forVersion("2.10");

        offsets.loadAllModels  = table.loadAllModels;   // void()            @ 0x396D1C
        offsets.hasModelLoaded = table.hasModelLoaded;  // bool(int)         @ 0x395098

        ENGINE_LOGI("EngineOffsets: v2.10 mapped %d/%d signature-proven offsets "
                    "(%d verified offsets available in GTASAOffsets)",
                    (offsets.loadAllModels != 0) + (offsets.hasModelLoaded != 0), 2,
                    table.countValidOffsets());
    } else {
        ENGINE_LOGE("EngineOffsets: unknown version '%s', all offsets zero", version);
    }

    return offsets;
}

// ---------------------------------------------------------------------------
// GameEngine implementation
// ---------------------------------------------------------------------------

struct GameEngine::Impl {
    bool initialized = false;
    std::atomic<bool> paused{false};
    EngineOffsets offsets{};
    Hook::SymbolResolver resolver;
    uintptr_t gtasaBase = 0;
#ifdef __ANDROID__
    // Handle do dlopen("libGTASA.so"). Mantido aberto durante toda a vida do
    // engine: fechar descarregaria os hooks que apontam para dentro dessa lib.
    void* gtasaHandle = nullptr;
#endif

    // Function pointer typedefs for engine calls
    using CreatePedFn = uint32_t (*)(int modelId, float x, float y, float z, float heading);
    using DeletePedFn = void (*)(uint32_t handle);
    using SetPosFn = void (*)(uint32_t handle, float x, float y, float z);
    using GetPosFn = void (*)(uint32_t handle, float* x, float* y, float* z);
    using SetRotFn = void (*)(uint32_t handle, float rx, float ry, float rz);
    using SetHealthFn = void (*)(uint32_t handle, float health);
    using GetHealthFn = float (*)(uint32_t handle);
    using CreateVehicleFn = uint32_t (*)(int modelId, float x, float y, float z, float heading);
    using DeleteVehicleFn = void (*)(uint32_t handle);
    using SetVelFn = void (*)(uint32_t handle, float vx, float vy, float vz);
    using CreateObjectFn = uint32_t (*)(int modelId, float x, float y, float z);
    using DeleteObjectFn = void (*)(uint32_t handle);
    using RequestModelFn = void (*)(int modelId);
    using LoadModelsFn = void (*)();
    using HasModelLoadedFn = bool (*)(int modelId);
    using GetPlayerPedFn = uint32_t (*)();
    using SetPlayerControlFn = void (*)(bool enable);

    template<typename Fn>
    Fn getFunction(uintptr_t offset) const {
        if (offset == 0 || gtasaBase == 0) return nullptr;
        return reinterpret_cast<Fn>(gtasaBase + offset);
    }
};

GameEngine::GameEngine() : pImpl_(std::make_unique<Impl>()) {}
GameEngine::~GameEngine() = default;

bool GameEngine::initialize(const EngineOffsets& offsets) {
#ifdef __ANDROID__
    // O libGTASA.so e' empacotado em jniLibs mas NAO e' linkado pelo build
    // (nao esta em target_link_libraries). Sem um dlopen explicito, ele nunca
    // entra na memoria do processo e getLibraryBaseAddress devolve 0.
    // RTLD_NOW | RTLD_GLOBAL: GLOBAL e' necessario para que os simbolos do
    // libGTASA fiquem visiveis a dlsym dos nossos hooks.
    pImpl_->gtasaBase = Hook::MemoryPatcher::getLibraryBaseAddress("libGTASA.so");
    if (pImpl_->gtasaBase == 0) {
        GTASA_LOG("libGTASA.so nao carregado; tentando dlopen");
        void* h = dlopen("libGTASA.so", RTLD_NOW | RTLD_GLOBAL);
        if (!h) {
            const char* err = dlerror();
            ENGINE_LOGE("GameEngine::initialize: dlopen(libGTASA.so) falhou: %s",
                        err ? err : "(sem detalhe)");
            return false;
        }
        GTASA_LOG("libGTASA.so carregado em %p", h);
        pImpl_->gtasaHandle = h;
        pImpl_->gtasaBase = Hook::MemoryPatcher::getLibraryBaseAddress("libGTASA.so");
    }
    if (pImpl_->gtasaBase == 0) {
        ENGINE_LOGE("GameEngine::initialize: libGTASA.so not found in memory");
        return false;
    }

    pImpl_->offsets = offsets;
    pImpl_->initialized = true;

    ENGINE_LOGI("GameEngine::initialize: libGTASA.so base=%p, hooks ready",
                reinterpret_cast<void*>(pImpl_->gtasaBase));
    return true;
#else
    (void)offsets;
    ENGINE_LOGE("GameEngine::initialize: non-Android platform, stub only");
    return false;
#endif
}

bool GameEngine::isReady() const {
    return pImpl_->initialized && pImpl_->gtasaBase != 0;
}

void GameEngine::pause() {
    pImpl_->paused.store(true);
    ENGINE_LOGI("GameEngine: paused");
}

void GameEngine::resume() {
    pImpl_->paused.store(false);
    ENGINE_LOGI("GameEngine: resumed");
}

bool GameEngine::isPaused() const {
    return pImpl_->paused.load();
}

// --- Ped operations ---

PedHandle GameEngine::spawnPed(int modelId, float x, float y, float z, float heading) {
    auto fn = pImpl_->getFunction<Impl::CreatePedFn>(pImpl_->offsets.createPed);
    if (!fn) {
        ENGINE_LOGE("spawnPed: function not available");
        return INVALID_PED;
    }
    PedHandle h = fn(modelId, x, y, z, heading);
    ENGINE_LOGI("spawnPed: model=%d pos=(%.1f,%.1f,%.1f) -> handle=%u", modelId, x, y, z, h);
    return h;
}

void GameEngine::deletePed(PedHandle handle) {
    auto fn = pImpl_->getFunction<Impl::DeletePedFn>(pImpl_->offsets.deletePed);
    if (fn) fn(handle);
}

void GameEngine::setPedPosition(PedHandle handle, float x, float y, float z) {
    auto fn = pImpl_->getFunction<Impl::SetPosFn>(pImpl_->offsets.setPedPosition);
    if (fn) fn(handle, x, y, z);
}

void GameEngine::getPedPosition(PedHandle handle, float& x, float& y, float& z) {
    auto fn = pImpl_->getFunction<Impl::GetPosFn>(pImpl_->offsets.getPedPosition);
    if (fn) fn(handle, &x, &y, &z);
    else { x = y = z = 0.0f; }
}

void GameEngine::setPedRotation(PedHandle handle, float rx, float ry, float rz) {
    auto fn = pImpl_->getFunction<Impl::SetRotFn>(pImpl_->offsets.setPedRotation);
    if (fn) fn(handle, rx, ry, rz);
}

void GameEngine::setPedHealth(PedHandle handle, float health) {
    auto fn = pImpl_->getFunction<Impl::SetHealthFn>(pImpl_->offsets.setPedHealth);
    if (fn) fn(handle, health);
}

float GameEngine::getPedHealth(PedHandle handle) {
    auto fn = pImpl_->getFunction<Impl::GetHealthFn>(pImpl_->offsets.getPedHealth);
    return fn ? fn(handle) : 0.0f;
}

// --- Vehicle operations ---

VehicleHandle GameEngine::spawnVehicle(int modelId, float x, float y, float z, float heading) {
    auto fn = pImpl_->getFunction<Impl::CreateVehicleFn>(pImpl_->offsets.createVehicle);
    if (!fn) {
        ENGINE_LOGE("spawnVehicle: function not available");
        return INVALID_VEHICLE;
    }
    VehicleHandle h = fn(modelId, x, y, z, heading);
    ENGINE_LOGI("spawnVehicle: model=%d pos=(%.1f,%.1f,%.1f) -> handle=%u", modelId, x, y, z, h);
    return h;
}

void GameEngine::deleteVehicle(VehicleHandle handle) {
    auto fn = pImpl_->getFunction<Impl::DeleteVehicleFn>(pImpl_->offsets.deleteVehicle);
    if (fn) fn(handle);
}

void GameEngine::setVehiclePosition(VehicleHandle handle, float x, float y, float z) {
    auto fn = pImpl_->getFunction<Impl::SetPosFn>(pImpl_->offsets.setVehiclePosition);
    if (fn) fn(handle, x, y, z);
}

void GameEngine::getVehiclePosition(VehicleHandle handle, float& x, float& y, float& z) {
    auto fn = pImpl_->getFunction<Impl::GetPosFn>(pImpl_->offsets.getVehiclePosition);
    if (fn) fn(handle, &x, &y, &z);
    else { x = y = z = 0.0f; }
}

void GameEngine::setVehicleRotation(VehicleHandle handle, float rx, float ry, float rz) {
    auto fn = pImpl_->getFunction<Impl::SetRotFn>(pImpl_->offsets.setVehicleRotation);
    if (fn) fn(handle, rx, ry, rz);
}

void GameEngine::setVehicleVelocity(VehicleHandle handle, float vx, float vy, float vz) {
    auto fn = pImpl_->getFunction<Impl::SetVelFn>(pImpl_->offsets.setVehicleVelocity);
    if (fn) fn(handle, vx, vy, vz);
}

void GameEngine::setVehicleHealth(VehicleHandle handle, float health) {
    auto fn = pImpl_->getFunction<Impl::SetHealthFn>(pImpl_->offsets.setVehicleHealth);
    if (fn) fn(handle, health);
}

// --- Object operations ---

ObjectHandle GameEngine::spawnObject(int modelId, float x, float y, float z) {
    auto fn = pImpl_->getFunction<Impl::CreateObjectFn>(pImpl_->offsets.createObject);
    if (!fn) return INVALID_OBJECT;
    return fn(modelId, x, y, z);
}

void GameEngine::deleteObject(ObjectHandle handle) {
    auto fn = pImpl_->getFunction<Impl::DeleteObjectFn>(pImpl_->offsets.deleteObject);
    if (fn) fn(handle);
}

void GameEngine::setObjectPosition(ObjectHandle handle, float x, float y, float z) {
    auto fn = pImpl_->getFunction<Impl::SetPosFn>(pImpl_->offsets.setObjectPosition);
    if (fn) fn(handle, x, y, z);
}

void GameEngine::setObjectRotation(ObjectHandle handle, float rx, float ry, float rz) {
    auto fn = pImpl_->getFunction<Impl::SetRotFn>(pImpl_->offsets.setObjectRotation);
    if (fn) fn(handle, rx, ry, rz);
}

// --- Model loading ---

void GameEngine::requestModel(int modelId) {
    auto fn = pImpl_->getFunction<Impl::RequestModelFn>(pImpl_->offsets.requestModel);
    if (fn) fn(modelId);
}

void GameEngine::loadModels() {
    auto fn = pImpl_->getFunction<Impl::LoadModelsFn>(pImpl_->offsets.loadAllModels);
    if (fn) fn();
}

bool GameEngine::hasModelLoaded(int modelId) {
    auto fn = pImpl_->getFunction<Impl::HasModelLoadedFn>(pImpl_->offsets.hasModelLoaded);
    return fn ? fn(modelId) : false;
}

// --- Player ---

PedHandle GameEngine::getPlayerPed() {
    auto fn = pImpl_->getFunction<Impl::GetPlayerPedFn>(pImpl_->offsets.getPlayerPed);
    return fn ? fn() : INVALID_PED;
}

void GameEngine::setPlayerControl(bool enable) {
    auto fn = pImpl_->getFunction<Impl::SetPlayerControlFn>(pImpl_->offsets.setPlayerControl);
    if (fn) fn(enable);
}

} // namespace WorldRPG::Game