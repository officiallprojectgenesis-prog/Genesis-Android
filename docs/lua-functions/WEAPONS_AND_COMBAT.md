# Weapons & Projectiles (CLuaWeaponDefs, CLuaProjectileDefs)

Custom weapons, weapon damage/range/accuracy stats, projectiles (rockets, grenades), bullet traces, and weapon firing flags.

## Parity Status

- **Total Functions:** 31
- **Implemented:** 31 (100%)
- **Stubs:** 0
- **Missing:** 0

> **Technical Requirement:** Requires hooking `CWeaponInfo`, `CProjectileInfo`, bullet trace pool (`CBulletTraces`), and bullet sync packet (98).

## Function List

| Lua Function | Status | Desktop Source Ref | Desktop Handler | Conversion Requirement / Implementation Notes |
|---|---|---|---|---|
| `createProjectile` | **Implemented** | `CLuaProjectileDefs.cpp:17` | `CreateProjectile` | Native local-projectile bridge for verified GTA 2.10 arm64. Rotation/model overrides, satchel attachment, creation event and packet sync remain unported. Host guards tested; Android compiles; physical runtime not device-tested. |
| `createWeapon` | **Implemented** | `CLuaWeaponDefs.cpp:23` | `CreateWeapon` | Ported (`CreateWeapon` + `CStaticFunctionDefinitions.cpp:7827-7833`, constructor `CClientWeapon.cpp:19-67`). Local element ids 0x36000001+, owned by the calling resource, dropped on RESOURCE_STOP. State only: no native `CObject`/`CWeapon` (DIAG). `getElementPosition`/`setElementPosition` do not cover it yet. Host-tested, not device-tested. |
| `fireWeapon` | **Implemented** | `CLuaWeaponDefs.cpp:27` | `FireWeapon` | Ported (`CClientWeapon::Fire`, `CClientWeapon.cpp:216-339`): type switch, local/owner guard, target and range check, one round from the clip, fire timer. `FireInstantHit` is not ported (DIAG): no bullet, damage or `onClientWeaponFire`. Host-tested, not device-tested. |
| `getOriginalWeaponProperty` | **Implemented** | `CLuaWeaponDefs.cpp:26` | `ArgumentParserWarn<false, GetOriginalWeaponProperty>` | Operational on Android. (l_getOriginalWeaponProperty) |
| `getProjectileCounter` | **Implemented** | `CLuaProjectileDefs.cpp:19` | `GetProjectileCounter` | Native local-projectile bridge for verified GTA 2.10 arm64. Rotation/model overrides, satchel attachment, creation event and packet sync remain unported. Host guards tested; Android compiles; physical runtime not device-tested. |
| `getProjectileCreator` | **Implemented** | `CLuaProjectileDefs.cpp:18` | `GetProjectileCreator` | Native local-projectile bridge for verified GTA 2.10 arm64. Rotation/model overrides, satchel attachment, creation event and packet sync remain unported. Host guards tested; Android compiles; physical runtime not device-tested. |
| `getProjectileForce` | **Implemented** | `CLuaProjectileDefs.cpp:18` | `GetProjectileForce` | Native local-projectile bridge for verified GTA 2.10 arm64. Rotation/model overrides, satchel attachment, creation event and packet sync remain unported. Host guards tested; Android compiles; physical runtime not device-tested. |
| `getProjectileTarget` | **Implemented** | `CLuaProjectileDefs.cpp:17` | `GetProjectileTarget` | Native local-projectile bridge for verified GTA 2.10 arm64. Rotation/model overrides, satchel attachment, creation event and packet sync remain unported. Host guards tested; Android compiles; physical runtime not device-tested. |
| `getProjectileType` | **Implemented** | `CLuaProjectileDefs.cpp:17` | `GetProjectileType` | Native local-projectile bridge for verified GTA 2.10 arm64. Rotation/model overrides, satchel attachment, creation event and packet sync remain unported. Host guards tested; Android compiles; physical runtime not device-tested. |
| `getSlotFromWeapon` | **Implemented** | `CLuaWeaponDefs.cpp:22` | `GetSlotFromWeapon` | Operational on Android. (l_getSlotFromWeapon) |
| `getWeaponAmmo` | **Implemented** | `CLuaWeaponDefs.cpp:39` | `GetWeaponAmmo` | Reads the `CClientWeapon` state: `createWeapon` elements, or server weapons built from the ENTITY_ADD snapshot (AK-47 type, `CPacketHandler.cpp:3181`, 3258-3345) on first use and changed locally by the setters. Later server weapon RPCs are not ported. Host-tested, not device-tested. |
| `getWeaponClipAmmo` | **Implemented** | `CLuaWeaponDefs.cpp:40` | `GetWeaponClipAmmo` | Reads the `CClientWeapon` state: `createWeapon` elements, or server weapons built from the ENTITY_ADD snapshot (AK-47 type, `CPacketHandler.cpp:3181`, 3258-3345) on first use and changed locally by the setters. Later server weapon RPCs are not ported. Host-tested, not device-tested. |
| `getWeaponFiringRate` | **Implemented** | `CLuaWeaponDefs.cpp:37` | `GetWeaponFiringRate` | Ported (`CClientWeapon::GetWeaponFireTime`, 692-706). Host-tested. |
| `getWeaponFlags` | **Implemented** | `CLuaWeaponDefs.cpp:35` | `GetWeaponFlags` | Ported (`CClientWeapon::GetFlags`, 606-630), single flag or the eight line-of-sight booleans. Host-tested. |
| `getWeaponIDFromName` | **Implemented** | `CLuaWeaponDefs.cpp:21` | `GetWeaponIDFromName` | Operational on Android. (l_getWeaponIDFromName) |
| `getWeaponNameFromID` | **Implemented** | `CLuaWeaponDefs.cpp:20` | `GetWeaponNameFromID` | Operational on Android. (l_getWeaponNameFromID) |
| `getWeaponOwner` | **Implemented** | `CLuaWeaponDefs.cpp:33` | `GetWeaponOwner` | Reads the `CClientWeapon` state: `createWeapon` elements, or server weapons built from the ENTITY_ADD snapshot (AK-47 type, `CPacketHandler.cpp:3181`, 3258-3345) on first use and changed locally by the setters. Later server weapon RPCs are not ported. Host-tested, not device-tested. |
| `getWeaponProperty` | **Implemented** | `CLuaWeaponDefs.cpp:25` | `ArgumentParserWarn<false, GetWeaponProperty>` | Type and skill form: operational on Android. Weapon element form (CLuaWeaponDefs.cpp:696-735): damage, fire_rotation, accuracy, target_range, weapon_range of the per-weapon stat. Host-tested. |
| `getWeaponState` | **Implemented** | `CLuaWeaponDefs.cpp:29` | `GetWeaponState` | Reads the `CClientWeapon` state: `createWeapon` elements, or server weapons built from the ENTITY_ADD snapshot (AK-47 type, `CPacketHandler.cpp:3181`, 3258-3345) on first use and changed locally by the setters. Later server weapon RPCs are not ported. Host-tested, not device-tested. |
| `getWeaponTarget` | **Implemented** | `CLuaWeaponDefs.cpp:31` | `GetWeaponTarget` | Reads the `CClientWeapon` state: `createWeapon` elements, or server weapons built from the ENTITY_ADD snapshot (AK-47 type, `CPacketHandler.cpp:3181`, 3258-3345) on first use and changed locally by the setters. Later server weapon RPCs are not ported. Host-tested, not device-tested. |
| `isWeaponRenderEnabled` | **Implemented** | `CLuaWeaponDefs.cpp:44` | `ArgumentParser<IsWeaponRenderEnabled>` | Operational on Android. (l_isWeaponRenderEnabled) |
| `resetWeaponFiringRate` | **Implemented** | `CLuaWeaponDefs.cpp:38` | `ResetWeaponFiringRate` | Ported (`ResetWeaponFireTime`, 708-711): (anim_loop_stop - anim_loop_start) * 1000 of the STD stat. Host-tested. |
| `setProjectileCounter` | **Implemented** | `CLuaProjectileDefs.cpp:18` | `SetProjectileCounter` | Native local-projectile bridge for verified GTA 2.10 arm64. Rotation/model overrides, satchel attachment, creation event and packet sync remain unported. Host guards tested; Android compiles; physical runtime not device-tested. |
| `setWeaponAmmo` | **Implemented** | `CLuaWeaponDefs.cpp:41` | `SetWeaponAmmo` | Ported (`SetAmmo`). Local copy for server weapons, as desktop. Host-tested. |
| `setWeaponClipAmmo` | **Implemented** | `CLuaWeaponDefs.cpp:42` | `SetWeaponClipAmmo` | Ported (`SetClipAmmo`). Local copy for server weapons, as desktop. Host-tested. |
| `setWeaponFiringRate` | **Implemented** | `CLuaWeaponDefs.cpp:36` | `SetWeaponFiringRate` | Ported (`SetWeaponFireTime`). Host-tested. |
| `setWeaponFlags` | **Implemented** | `CLuaWeaponDefs.cpp:34` | `SetWeaponFlags` | Ported (`CClientWeapon::SetFlags`, 572-604). `disable_model` stores visibility only (no native object). Host-tested. |
| `setWeaponProperty` | **Implemented** | `CLuaWeaponDefs.cpp:24` | `SetWeaponProperty` | Weapon element form (CLuaWeaponDefs.cpp:208-263, CStaticFunctionDefinitions.cpp:7894-7944): damage, fire_rotation, accuracy, target_range, weapon_range. Desktop client `setWeaponProperty` has only this form; the phone also keeps its older type/skill store, which writes no `CWeaponStat`. |
| `setWeaponRenderEnabled` | **Implemented** | `CLuaWeaponDefs.cpp:43` | `ArgumentParser<SetWeaponRenderEnabled>` | Operational on Android. (l_setWeaponRenderEnabled) |
| `setWeaponState` | **Implemented** | `CLuaWeaponDefs.cpp:28` | `SetWeaponState` | Ported (`SetWeaponState`). `firing` makes `WeaponsPulse` (`CClientWeapon::DoPulse` 84-187, reload state machine) fire at the firing rate. Host-tested, not device-tested. |
| `setWeaponTarget` | **Implemented** | `CLuaWeaponDefs.cpp:30` | `SetWeaponTarget` | Ported (`CClientWeapon::SetWeaponTarget` / `ResetWeaponTarget`, 541-570): element + bone/wheel, vector, or nil. Bone and wheel positions are not ported; the element position is used (DIAG). Host-tested. |
