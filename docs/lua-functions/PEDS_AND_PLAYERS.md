# Peds & Players (CLuaPedDefs, CLuaPlayerDefs)

Ped animations, tasks, fighting styles, bone transforms, clothes, stats, voice, wanted levels, nametags, and player targeting.

## Parity Status

- **Total Functions:** 206
- **Implemented:** 39 (18.9%)
- **Stubs:** 72
- **Missing:** 95

> **Technical Requirement:** Requires hooking libGTASA.so `CPed` / `CPlayerPed` (`_ZN10CPlayerPed...`), `CPedIK`, `CTask` hierarchy (wander, fight, duck), and clothing pools.

## Function List

| Lua Function | Status | Desktop Source Ref | Desktop Handler | Conversion Requirement / Implementation Notes |
|---|---|---|---|---|
| `acquireMissionText` | Missing | `CLuaPlayerDefs.cpp:140` | `ArgumentParser<AcquireMissionText>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `acquirePedNativeCouple` | *Stub* | `CLuaPedDefs.cpp:346` | `AcquirePedNativeCouple` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `acquirePedNativeCouplePresentation` | *Stub* | `CLuaPedDefs.cpp:350` | `AcquirePedNativeCouplePresentation` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `acquirePedNativeEventProfile` | Missing | `CLuaPedDefs.cpp:338` | `AcquirePedNativeEventProfile` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `acquirePedNativeGroup` | *Stub* | `CLuaPedDefs.cpp:341` | `AcquirePedNativeGroup` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `acquirePedNativePointArm` | *Stub* | `CLuaPedDefs.cpp:354` | `AcquirePedNativePointArm` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `addPedClothes` | **Implemented** | `CLuaPedDefs.cpp:272` | `AddPedClothes` | Operational on Android. (l_addPedClothes) |
| `addPedNativeBikeJackTask` | Missing | `CLuaPedDefs.cpp:363` | `AddPedNativeBikeJackTask` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `addPedNativeDamageEvent` | Missing | `CLuaPedDefs.cpp:362` | `AddPedNativeDamageEvent` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `addPedNativeDamageResponseEvent` | *Stub* | `CLuaPedDefs.cpp:359` | `AddPedNativeDamageResponseEvent` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `addPedNativeGunAimedAtEvent` | Missing | `CLuaPedDefs.cpp:358` | `AddPedNativeGunAimedAtEvent` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `canPedBeDraggedOut` | *Stub* | `CLuaPedDefs.cpp:400` | `ArgumentParser<CanPedBeDraggedOut>` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `canPedBeKnockedOffBike` | *Stub* | `CLuaPedDefs.cpp:381` | `CanPedBeKnockedOffBike` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `cancelPedCarryObject` | Missing | `CLuaPedDefs.cpp:332` | `CancelPedCarryObject` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `clearMissionHelp` | Missing | `CLuaPlayerDefs.cpp:145` | `ArgumentParser<ClearMissionHelp>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `clearMissionTexts` | Missing | `CLuaPlayerDefs.cpp:144` | `ArgumentParser<ClearMissionTexts>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `createPed` | **Implemented** | `CLuaPedDefs.cpp:267` | `CreatePed` | Operational on Android. (l_createPed) |
| `detonateSatchels` | Missing | `CLuaPedDefs.cpp:268` | `DetonateSatchels` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `doesPedHaveJetPack` | **Implemented** | `CLuaPedDefs.cpp:430` | `DoesPedHaveJetPack` | Operational on Android. (DoesPedHaveJetPack) |
| `forcePlayerMap` | Missing | `CLuaPlayerDefs.cpp:132` | `ForcePlayerMap` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getElementBoneMatrix` | Missing | `CLuaPedDefs.cpp:378` | `ArgumentParserWarn<false, GetElementBoneMatrix>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getElementBonePosition` | Missing | `CLuaPedDefs.cpp:375` | `ArgumentParserWarn<false, GetElementBonePosition>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getElementBoneQuaternion` | Missing | `CLuaPedDefs.cpp:377` | `ArgumentParserWarn<false, GetElementBoneQuaternion>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getElementBoneRotation` | Missing | `CLuaPedDefs.cpp:376` | `ArgumentParserWarn<false, GetElementBoneRotation>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getLocalPlayer` | **Implemented** | `CLuaPlayerDefs.cpp:100` | `GetLocalPlayer` | Operational on Android. (l_getLocalPlayer) |
| `getPedAmmoInClip` | **Implemented** | `CLuaPedDefs.cpp:421` | `GetPedAmmoInClip` | Game-thread GTA ped snapshot plus per-element client cache seeded by ENTITY_ADD/PLAYER_LIST and weapon RPCs. Queries preserve death tasks/network revival, optional weapon slot and uint16 ammo semantics; clip ammo requires a streamed body. Stream-out saves inventory and offscreen RPCs update the cache before body restoration; generation guards reject removed/reused IDs. ARM64 layout is instruction-guarded. Native reload, sync-driven weapons and streaming still need device validation; this is handler coverage, not full gameplay parity. |
| `getPedAnalogControlState` | Missing | `CLuaPedDefs.cpp:389` | `GetPedAnalogControlState` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `getPedAnimation` | Missing | `CLuaPedDefs.cpp:382` | `GetPedAnimation` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `getPedAnimationLength` | Missing | `CLuaPedDefs.cpp:385` | `ArgumentParser<GetPedAnimationLength>` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `getPedAnimationProgress` | Missing | `CLuaPedDefs.cpp:383` | `ArgumentParser<GetPedAnimationProgress>` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `getPedAnimationSpeed` | Missing | `CLuaPedDefs.cpp:384` | `ArgumentParser<GetPedAnimationSpeed>` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `getPedArmor` | **Implemented** | `CLuaPedDefs.cpp:408` | `ArgumentParserWarn<false, GetPedArmor>` | Operational on Android. (l_getPedArmor) |
| `getPedBonePosition` | *Stub* | `CLuaPedDefs.cpp:426` | `GetPedBonePosition` | Currently stubbed: `DefaultValue(kZero3)`. Needs native engine wiring. |
| `getPedCameraRotation` | *Stub* | `CLuaPedDefs.cpp:396` | `GetPedCameraRotation` | Currently stubbed: `DefaultValue(kZero)`. Needs native engine wiring. |
| `getPedCarriedObject` | Missing | `CLuaPedDefs.cpp:330` | `GetPedCarriedObject` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `getPedClothes` | Missing | `CLuaPedDefs.cpp:427` | `GetPedClothes` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `getPedContactElement` | **Implemented** | `CLuaPedDefs.cpp:411` | `GetPedContactElement` | Operational on Android. (GetPedContactElement) |
| `getPedControlState` | *Stub* | `CLuaPedDefs.cpp:388` | `ArgumentParserWarn<false, GetPedControlState>` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `getPedFightingStyle` | Missing | `CLuaPedDefs.cpp:391` | `GetPedFightingStyle` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `getPedMoveState` | **Implemented** | `CLuaPedDefs.cpp:428` | `GetPedMoveState` | Operational on Android. (l_getPedMoveState) |
| `getPedNativeCoupleDiagnostic` | *Stub* | `CLuaPedDefs.cpp:349` | `GetPedNativeCoupleDiagnostic` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `getPedNativeGroupDiagnostic` | *Stub* | `CLuaPedDefs.cpp:344` | `GetPedNativeGroupDiagnostic` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `getPedNativeVehicleResponse` | *Stub* | `CLuaPedDefs.cpp:360` | `GetPedNativeVehicleResponse` | Currently stubbed: `DefaultValue(kNil)`. Needs native engine wiring. |
| `getPedNoiseLevel` | Missing | `CLuaPedDefs.cpp:329` | `GetPedNoiseLevel` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `getPedOccupiedVehicle` | **Implemented** | `CLuaPedDefs.cpp:423` | `GetPedOccupiedVehicle` | Operational on Android. (GetPedOccupiedVehicle) |
| `getPedOccupiedVehicleSeat` | **Implemented** | `CLuaPedDefs.cpp:424` | `GetPedOccupiedVehicleSeat` | Operational on Android. (GetPedOccupiedVehicleSeat) |
| `getPedOxygenLevel` | Missing | `CLuaPedDefs.cpp:407` | `GetPedOxygenLevel` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `getPedPhysicalProofs` | *Stub* | `CLuaPedDefs.cpp:404` | `ArgumentParser<GetPedPhysicalProofs>` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `getPedRotation` | **Implemented** | `CLuaPedDefs.cpp:379` | `GetPedRotation` | Operational on Android. (l_getPedRotation) |
| `getPedSimplestTask` | *Stub* | `CLuaPedDefs.cpp:413` | `GetPedSimplestTask` | Currently stubbed: `DefaultValue(kEmptyString)`. Needs native engine wiring. |
| `getPedStat` | *Stub* | `CLuaPedDefs.cpp:406` | `GetPedStat` | Currently stubbed: `l_num0: lua_pushinteger(L, 0)`. Needs native engine wiring. |
| `getPedStayInSamePlace` | *Stub* | `CLuaPedDefs.cpp:402` | `ArgumentParser<GetPedStayInSamePlace>` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `getPedSuffersCriticalHits` | *Stub* | `CLuaPedDefs.cpp:399` | `ArgumentParser<GetPedSuffersCriticalHits>` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `getPedTarget` | *Stub* | `CLuaPedDefs.cpp:414` | `GetPedTarget` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `getPedTargetCollision` | **Implemented** | `CLuaPedDefs.cpp:417` | `GetPedTargetCollision` | Operational on Android. (GetPedTargetCollision) |
| `getPedTargetEnd` | **Implemented** | `CLuaPedDefs.cpp:416` | `GetPedTargetEnd` | Operational on Android. (GetPedTargetEnd) |
| `getPedTargetStart` | **Implemented** | `CLuaPedDefs.cpp:415` | `GetPedTargetStart` | Operational on Android. (GetPedTargetStart) |
| `getPedTask` | *Stub* | `CLuaPedDefs.cpp:412` | `GetPedTask` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `getPedTaskSequenceProgress` | *Stub* | `CLuaPedDefs.cpp:425` | `ArgumentParser<GetPedTaskSequenceProgress>` | Currently stubbed: `DefaultValue(kZero)`. Needs native engine wiring. |
| `getPedTotalAmmo` | **Implemented** | `CLuaPedDefs.cpp:422` | `GetPedTotalAmmo` | Game-thread GTA ped snapshot plus per-element client cache seeded by ENTITY_ADD/PLAYER_LIST and weapon RPCs. Queries preserve death tasks/network revival, optional weapon slot and uint16 ammo semantics; clip ammo requires a streamed body. Stream-out saves inventory and offscreen RPCs update the cache before body restoration; generation guards reject removed/reused IDs. ARM64 layout is instruction-guarded. Native reload, sync-driven weapons and streaming still need device validation; this is handler coverage, not full gameplay parity. |
| `getPedVoice` | Missing | `CLuaPedDefs.cpp:374` | `GetPedVoice` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `getPedWalkingStyle` | Missing | `CLuaPedDefs.cpp:386` | `GetPedMoveAnim` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `getPedWeapon` | **Implemented** | `CLuaPedDefs.cpp:418` | `GetPedWeapon` | Game-thread GTA ped snapshot plus per-element client cache seeded by ENTITY_ADD/PLAYER_LIST and weapon RPCs. Queries preserve death tasks/network revival, optional weapon slot and uint16 ammo semantics; clip ammo requires a streamed body. Stream-out saves inventory and offscreen RPCs update the cache before body restoration; generation guards reject removed/reused IDs. ARM64 layout is instruction-guarded. Native reload, sync-driven weapons and streaming still need device validation; this is handler coverage, not full gameplay parity. |
| `getPedWeaponAccuracy` | *Stub* | `CLuaPedDefs.cpp:420` | `ArgumentParser<GetPedWeaponAccuracy>` | Currently stubbed: `l_num0: lua_pushinteger(L, 0)`. Needs native engine wiring. |
| `getPedWeaponShootingRate` | *Stub* | `CLuaPedDefs.cpp:419` | `ArgumentParser<GetPedWeaponShootingRate>` | Currently stubbed: `l_num0: lua_pushinteger(L, 0)`. Needs native engine wiring. |
| `getPedWeaponSlot` | **Implemented** | `CLuaPedDefs.cpp:380` | `GetPedWeaponSlot` | Game-thread GTA ped snapshot plus per-element client cache seeded by ENTITY_ADD/PLAYER_LIST and weapon RPCs. Queries preserve death tasks/network revival, optional weapon slot and uint16 ammo semantics; clip ammo requires a streamed body. Stream-out saves inventory and offscreen RPCs update the cache before body restoration; generation guards reject removed/reused IDs. ARM64 layout is instruction-guarded. Native reload, sync-driven weapons and streaming still need device validation; this is handler coverage, not full gameplay parity. |
| `getPlayerFromName` | **Implemented** | `CLuaPlayerDefs.cpp:108` | `GetPlayerFromName` | Operational on Android. (GetPlayerFromName) |
| `getPlayerFromNick` | **Implemented** | `CLuaPlayerDefs.cpp:107` | `GetPlayerFromName` | Operational on Android. (GetPlayerFromName) |
| `getPlayerHudComponentProperty` | Missing | `CLuaPlayerDefs.cpp:137` | `ArgumentParser<GetPlayerHudComponentProperty>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getPlayerMapBoundingBox` | Missing | `CLuaPlayerDefs.cpp:135` | `GetPlayerMapBoundingBox` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getPlayerMapOpacity` | Missing | `CLuaPlayerDefs.cpp:136` | `ArgumentParser<GetPlayerMapOpacity>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getPlayerMoney` | **Implemented** | `CLuaPlayerDefs.cpp:110` | `GetPlayerMoney` | Operational on Android. (l_getPlayerMoney) |
| `getPlayerName` | **Implemented** | `CLuaPlayerDefs.cpp:101` | `GetPlayerName` | Operational on Android. (GetPlayerName) |
| `getPlayerNametagColor` | **Implemented** | `CLuaPlayerDefs.cpp:103` | `GetPlayerNametagColor` | Operational on Android. (GetPlayerNametagColor) |
| `getPlayerNametagText` | **Implemented** | `CLuaPlayerDefs.cpp:102` | `GetPlayerNametagText` | Operational on Android. (GetPlayerNametagText) |
| `getPlayerPing` | **Implemented** | `CLuaPlayerDefs.cpp:105` | `GetPlayerPing` | Operational on Android. (GetPlayerPing) |
| `getPlayerScriptDebugLevel` | Missing | `CLuaPlayerDefs.cpp:112` | `ArgumentParser<GetPlayerScriptDebugLevel>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getPlayerSerial` | **Implemented** | `CLuaPlayerDefs.cpp:129` | `GetPlayerSerial` | Operational on Android. (GetPlayerSerial) |
| `getPlayerTeam` | **Implemented** | `CLuaPlayerDefs.cpp:106` | `GetPlayerTeam` | Operational on Android. (GetPlayerTeam) |
| `getPlayerUserName` | *Stub* | `CLuaPlayerDefs.cpp:128` | `GetPlayerUserName` | Currently stubbed: `GetPlayerUserName: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `getPlayerWantedLevel` | *Stub* | `CLuaPlayerDefs.cpp:111` | `GetPlayerWantedLevel` | Currently stubbed: `l_num0: lua_pushinteger(L, 0)`. Needs native engine wiring. |
| `givePedWeapon` | Missing | `CLuaPedDefs.cpp:276` | `GivePedWeapon` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `givePlayerMoney` | Missing | `CLuaPlayerDefs.cpp:119` | `GivePlayerMoney` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isPedBleeding` | Missing | `CLuaPedDefs.cpp:409` | `ArgumentParser<IsPedBleeding>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isPedChoking` | **Implemented** | `CLuaPedDefs.cpp:435` | `IsPedChoking` | Operational on Android. (IsPedChoking) |
| `isPedDead` | **Implemented** | `CLuaPedDefs.cpp:437` | `IsPedDead` | Game-thread GTA ped snapshot plus per-element client cache seeded by ENTITY_ADD/PLAYER_LIST and weapon RPCs. Queries preserve death tasks/network revival, optional weapon slot and uint16 ammo semantics; clip ammo requires a streamed body. Stream-out saves inventory and offscreen RPCs update the cache before body restoration; generation guards reject removed/reused IDs. ARM64 layout is instruction-guarded. Native reload, sync-driven weapons and streaming still need device validation; this is handler coverage, not full gameplay parity. |
| `isPedDoingGangDriveby` | *Stub* | `CLuaPedDefs.cpp:390` | `IsPedDoingGangDriveby` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `isPedDoingTask` | *Stub* | `CLuaPedDefs.cpp:434` | `IsPedDoingTask` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `isPedDucked` | **Implemented** | `CLuaPedDefs.cpp:436` | `IsPedDucked` | Operational on Android. (IsPedDucked) |
| `isPedFootBloodEnabled` | Missing | `CLuaPedDefs.cpp:395` | `IsPedFootBloodEnabled` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isPedFrozen` | Missing | `CLuaPedDefs.cpp:394` | `IsPedFrozen` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isPedHeadless` | Missing | `CLuaPedDefs.cpp:393` | `IsPedHeadless` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isPedInVehicle` | **Implemented** | `CLuaPedDefs.cpp:431` | `IsPedInVehicle` | Operational on Android. (IsPedInVehicle) |
| `isPedMissionActor` | *Stub* | `CLuaPedDefs.cpp:397` | `ArgumentParser<IsPedMissionActor>` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `isPedNativeAmbientCopWanderTask` | *Stub* | `CLuaPedDefs.cpp:320` | `ArgumentParser<IsPedNativeAmbientCopWanderTask>` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `isPedNativeCoupleActive` | *Stub* | `CLuaPedDefs.cpp:348` | `IsPedNativeCoupleActive` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `isPedNativeCouplePresentationActive` | *Stub* | `CLuaPedDefs.cpp:353` | `IsPedNativeCouplePresentationActive` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `isPedNativeEventProfileActive` | Missing | `CLuaPedDefs.cpp:340` | `IsPedNativeEventProfileActive` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isPedNativeGroupActive` | *Stub* | `CLuaPedDefs.cpp:343` | `IsPedNativeGroupActive` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `isPedNativePointArmActive` | Missing | `CLuaPedDefs.cpp:357` | `IsPedNativePointArmActive` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isPedNativeTaskReady` | Missing | `CLuaPedDefs.cpp:308` | `ArgumentParser<IsPedNativeTaskReady>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isPedNeverTargeted` | *Stub* | `CLuaPedDefs.cpp:403` | `ArgumentParser<IsPedNeverTargeted>` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `isPedOnGround` | *Stub* | `CLuaPedDefs.cpp:433` | `IsPedOnGround` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `isPedOnlyDamagedByPlayer` | *Stub* | `CLuaPedDefs.cpp:401` | `ArgumentParser<IsPedOnlyDamagedByPlayer>` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `isPedReloadingWeapon` | Missing | `CLuaPedDefs.cpp:438` | `ArgumentParserWarn<false, IsPedReloadingWeapon>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isPedStoryProtected` | Missing | `CLuaPedDefs.cpp:398` | `ArgumentParser<IsPedStoryProtected>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isPedUsingNativeWalkingStyle` | Missing | `CLuaPedDefs.cpp:387` | `IsPedUsingNativeWalkingStyle` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isPedWearingJetpack` | Missing | `CLuaPedDefs.cpp:432` | `DoesPedHaveJetPack` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isPlayerCrosshairVisible` | Missing | `CLuaPlayerDefs.cpp:113` | `ArgumentParser<IsPlayerCrosshairVisible>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isPlayerHudComponentVisible` | **Implemented** | `CLuaPlayerDefs.cpp:109` | `IsPlayerHudComponentVisible` | Operational on Android. (l_isPlayerHudComponentVisible) |
| `isPlayerMapForced` | Missing | `CLuaPlayerDefs.cpp:133` | `IsPlayerMapForced` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isPlayerMapVisible` | Missing | `CLuaPlayerDefs.cpp:134` | `IsPlayerMapVisible` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isPlayerNametagShowing` | **Implemented** | `CLuaPlayerDefs.cpp:104` | `IsPlayerNametagShowing` | Operational on Android. (IsPlayerNametagShowing) |
| `killPed` | Missing | `CLuaPedDefs.cpp:269` | `KillPed` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `killPedTask` | Missing | `CLuaPedDefs.cpp:439` | `ArgumentParser<killPedTask>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `pickUpPedObject` | Missing | `CLuaPedDefs.cpp:328` | `PickUpPedObject` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `playPedVoiceLine` | Missing | `CLuaPedDefs.cpp:372` | `ArgumentParser<PlayPedVoiceLine>` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `putDownPedObject` | Missing | `CLuaPedDefs.cpp:331` | `PutDownPedObject` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `releaseMissionText` | Missing | `CLuaPlayerDefs.cpp:146` | `ArgumentParser<ReleaseMissionText>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `releasePedNativeCouple` | *Stub* | `CLuaPedDefs.cpp:347` | `ReleasePedNativeCouple` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `releasePedNativeCouplePresentation` | *Stub* | `CLuaPedDefs.cpp:352` | `ReleasePedNativeCouplePresentation` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `releasePedNativeEventProfile` | Missing | `CLuaPedDefs.cpp:339` | `ReleasePedNativeEventProfile` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `releasePedNativeGroup` | *Stub* | `CLuaPedDefs.cpp:342` | `ReleasePedNativeGroup` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `releasePedNativePointArm` | *Stub* | `CLuaPedDefs.cpp:356` | `ReleasePedNativePointArm` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `removePedClothes` | **Implemented** | `CLuaPedDefs.cpp:273` | `RemovePedClothes` | Operational on Android. (l_removePedClothes) |
| `removePedFromVehicle` | *Stub* | `CLuaPedDefs.cpp:275` | `RemovePedFromVehicle` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `resetPedVoice` | Missing | `CLuaPedDefs.cpp:270` | `ArgumentParser<ResetPedVoice>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `resetPlayerHudComponentProperty` | Missing | `CLuaPlayerDefs.cpp:125` | `ArgumentParser<ResetPlayerHudComponentProperty>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `restorePedNativeVehicleResponse` | *Stub* | `CLuaPedDefs.cpp:361` | `RestorePedNativeVehicleResponse` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setElementBoneMatrix` | Missing | `CLuaPedDefs.cpp:282` | `ArgumentParserWarn<false, SetElementBoneMatrix>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setElementBonePosition` | Missing | `CLuaPedDefs.cpp:279` | `ArgumentParserWarn<false, SetElementBonePosition>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setElementBoneQuaternion` | Missing | `CLuaPedDefs.cpp:281` | `ArgumentParserWarn<false, SetElementBoneQuaternion>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setElementBoneRotation` | Missing | `CLuaPedDefs.cpp:280` | `ArgumentParserWarn<false, SetElementBoneRotation>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setPedAchieveHeading` | Missing | `CLuaPedDefs.cpp:312` | `ArgumentParser<SetPedAchieveHeading>` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedAimAt` | *Stub* | `CLuaPedDefs.cpp:313` | `ArgumentParser<SetPedAimAt>` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedAimTarget` | Missing | `CLuaPedDefs.cpp:300` | `SetPedAimTarget` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedAnalogControlState` | Missing | `CLuaPedDefs.cpp:292` | `SetPedAnalogControlState` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedAnimation` | *Stub* | `CLuaPedDefs.cpp:286` | `SetPedAnimation` | Currently stubbed: `TrueResult: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `setPedAnimationProgress` | Missing | `CLuaPedDefs.cpp:287` | `SetPedAnimationProgress` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedAnimationSpeed` | Missing | `CLuaPedDefs.cpp:288` | `SetPedAnimationSpeed` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedArmor` | Missing | `CLuaPedDefs.cpp:303` | `ArgumentParser<SetPedArmor>` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedBleeding` | Missing | `CLuaPedDefs.cpp:371` | `ArgumentParser<SetPedBleeding>` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedCameraRotation` | Missing | `CLuaPedDefs.cpp:299` | `SetPedCameraRotation` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedCanBeDraggedOut` | *Stub* | `CLuaPedDefs.cpp:366` | `ArgumentParser<SetPedCanBeDraggedOut>` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedCanBeKnockedOffBike` | *Stub* | `CLuaPedDefs.cpp:285` | `SetPedCanBeKnockedOffBike` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedCarryObject` | Missing | `CLuaPedDefs.cpp:327` | `SetPedCarryObject` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedChatWith` | Missing | `CLuaPedDefs.cpp:311` | `ArgumentParser<SetPedChatWith>` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedControlState` | *Stub* | `CLuaPedDefs.cpp:291` | `ArgumentParserWarn<false, SetPedControlState>` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `setPedDoingGangDriveby` | Missing | `CLuaPedDefs.cpp:293` | `SetPedDoingGangDriveby` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedDriveBy` | *Stub* | `CLuaPedDefs.cpp:336` | `ArgumentParser<SetPedDriveBy>` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedDriveMission` | *Stub* | `CLuaPedDefs.cpp:335` | `ArgumentParser<SetPedDriveMission>` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedDriveTo` | *Stub* | `CLuaPedDefs.cpp:334` | `ArgumentParser<SetPedDriveTo>` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedDriveWander` | Missing | `CLuaPedDefs.cpp:333` | `ArgumentParser<SetPedDriveWander>` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedEnterVehicle` | *Stub* | `CLuaPedDefs.cpp:304` | `ArgumentParser<SetPedEnterVehicle>` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedExitVehicle` | *Stub* | `CLuaPedDefs.cpp:305` | `ArgumentParser<SetPedExitVehicle>` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedFacialTalk` | Missing | `CLuaPedDefs.cpp:323` | `ArgumentParser<SetPedFacialTalk>` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedFightingStyle` | Missing | `CLuaPedDefs.cpp:294` | `ArgumentParser<SetPedFightingStyle>` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedFootBloodEnabled` | Missing | `CLuaPedDefs.cpp:298` | `SetPedFootBloodEnabled` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedFrozen` | Missing | `CLuaPedDefs.cpp:297` | `SetPedFrozen` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedGoTo` | *Stub* | `CLuaPedDefs.cpp:309` | `ArgumentParser<SetPedGoTo>` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedGoToOffset` | Missing | `CLuaPedDefs.cpp:317` | `ArgumentParser<SetPedGoToOffset>` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedHandsUp` | *Stub* | `CLuaPedDefs.cpp:315` | `ArgumentParser<SetPedHandsUp>` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedHeadless` | Missing | `CLuaPedDefs.cpp:296` | `SetPedHeadless` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedJump` | *Stub* | `CLuaPedDefs.cpp:321` | `ArgumentParser<SetPedJump>` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `setPedKillOnFoot` | *Stub* | `CLuaPedDefs.cpp:318` | `ArgumentParser<SetPedKillOnFoot>` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedLookAt` | Missing | `CLuaPedDefs.cpp:295` | `SetPedLookAt` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedMissionActor` | *Stub* | `CLuaPedDefs.cpp:337` | `ArgumentParser<SetPedMissionActor>` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedNavigateTo` | *Stub* | `CLuaPedDefs.cpp:310` | `ArgumentParser<SetPedNavigateTo>` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `setPedNeverTargeted` | *Stub* | `CLuaPedDefs.cpp:369` | `ArgumentParser<SetPedNeverTargeted>` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedOnlyDamagedByPlayer` | *Stub* | `CLuaPedDefs.cpp:367` | `ArgumentParser<SetPedOnlyDamagedByPlayer>` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedOxygenLevel` | Missing | `CLuaPedDefs.cpp:302` | `SetPedOxygenLevel` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedPhysicalProofs` | *Stub* | `CLuaPedDefs.cpp:370` | `ArgumentParser<SetPedPhysicalProofs>` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedRotation` | *Stub* | `CLuaPedDefs.cpp:283` | `SetPedRotation` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `setPedScriptedSpeechMuted` | Missing | `CLuaPedDefs.cpp:322` | `ArgumentParser<SetPedScriptedSpeechMuted>` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedShootAt` | Missing | `CLuaPedDefs.cpp:325` | `ArgumentParser<SetPedShootAt>` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedStandStill` | *Stub* | `CLuaPedDefs.cpp:314` | `ArgumentParser<SetPedStandStill>` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `setPedStat` | Missing | `CLuaPedDefs.cpp:301` | `SetPedStat` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedStayInSamePlace` | *Stub* | `CLuaPedDefs.cpp:368` | `ArgumentParser<SetPedStayInSamePlace>` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `setPedStoryProtected` | Missing | `CLuaPedDefs.cpp:364` | `ArgumentParser<SetPedStoryProtected>` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedSuffersCriticalHits` | *Stub* | `CLuaPedDefs.cpp:365` | `ArgumentParser<SetPedSuffersCriticalHits>` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedTaskSequence` | *Stub* | `CLuaPedDefs.cpp:326` | `SetPedTaskSequence` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedTurnToFace` | *Stub* | `CLuaPedDefs.cpp:316` | `ArgumentParser<SetPedTurnToFace>` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPedUseNativeWalkingStyle` | Missing | `CLuaPedDefs.cpp:290` | `SetPedUseNativeWalkingStyle` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedVoice` | Missing | `CLuaPedDefs.cpp:278` | `SetPedVoice` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedWalkingStyle` | Missing | `CLuaPedDefs.cpp:289` | `SetPedMoveAnim` | Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so. |
| `setPedWander` | **Implemented** | `CLuaPedDefs.cpp:319` | `ArgumentParser<SetPedWander>` | Operational on Android. (l_setPedWander) |
| `setPedWeaponAccuracy` | *Stub* | `CLuaPedDefs.cpp:307` | `ArgumentParser<SetPedWeaponAccuracy>` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `setPedWeaponShootingRate` | *Stub* | `CLuaPedDefs.cpp:306` | `ArgumentParser<SetPedWeaponShootingRate>` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `setPedWeaponSlot` | *Stub* | `CLuaPedDefs.cpp:284` | `SetPedWeaponSlot` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `setPlayerHudComponentProperty` | Missing | `CLuaPlayerDefs.cpp:124` | `ArgumentParser<SetPlayerHudComponentProperty>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setPlayerHudComponentVisible` | **Implemented** | `CLuaPlayerDefs.cpp:117` | `ShowPlayerHudComponent` | Operational on Android. (l_setPlayerHudComponentVisible) |
| `setPlayerMoney` | Missing | `CLuaPlayerDefs.cpp:118` | `SetPlayerMoney` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setPlayerNametagColor` | **Implemented** | `CLuaPlayerDefs.cpp:122` | `SetPlayerNametagColor` | Operational on Android. (SetPlayerNametagColor) |
| `setPlayerNametagShowing` | **Implemented** | `CLuaPlayerDefs.cpp:123` | `SetPlayerNametagShowing` | Operational on Android. (SetPlayerNametagShowing) |
| `setPlayerNametagText` | **Implemented** | `CLuaPlayerDefs.cpp:121` | `SetPlayerNametagText` | Operational on Android. (SetPlayerNametagText) |
| `showMissionBigText` | Missing | `CLuaPlayerDefs.cpp:143` | `ArgumentParser<ShowMissionBigText>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `showMissionHelp` | Missing | `CLuaPlayerDefs.cpp:142` | `ArgumentParser<ShowMissionHelp>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `showMissionText` | Missing | `CLuaPlayerDefs.cpp:141` | `ArgumentParser<ShowMissionText>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `showPlayerHudComponent` | **Implemented** | `CLuaPlayerDefs.cpp:116` | `ShowPlayerHudComponent` | Operational on Android. (l_setPlayerHudComponentVisible) |
| `stopPedFacialTalk` | Missing | `CLuaPedDefs.cpp:324` | `ArgumentParser<StopPedFacialTalk>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `takePlayerMoney` | Missing | `CLuaPlayerDefs.cpp:120` | `TakePlayerMoney` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `updateElementRpHAnim` | Missing | `CLuaPedDefs.cpp:271` | `ArgumentParser<UpdateElementRpHAnim>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `updatePedNativeCouplePresentation` | *Stub* | `CLuaPedDefs.cpp:351` | `UpdatePedNativeCouplePresentation` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `updatePedNativePointArm` | *Stub* | `CLuaPedDefs.cpp:355` | `UpdatePedNativePointArm` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `validatePedNativeCouple` | *Stub* | `CLuaPedDefs.cpp:345` | `ValidatePedNativeCouple` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `warpPedIntoVehicle` | Missing | `CLuaPedDefs.cpp:274` | `WarpPedIntoVehicle` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
