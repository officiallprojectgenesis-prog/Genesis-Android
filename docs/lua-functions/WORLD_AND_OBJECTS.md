# World, Objects & Environment

3D objects, world weather, game time, water levels, gravity, minute duration, heat haze, explosions, fires, garage doors, searchlights, and foliage.

## Parity Status

- **Total Functions:** 227
- **Implemented:** 124 (54.6%)
- **Stubs:** 11
- **Missing:** 92

> **Technical Requirement:** Hooks libGTASA.so `CObject`, `CWeather`, `CClock`, `CWaterLevel`, `CWorld`, and particle engine `FxSystem_c`.

## Function List

| Lua Function | Status | Desktop Source Ref | Desktop Handler | Conversion Requirement / Implementation Notes |
|---|---|---|---|---|
| `acquireGarageControl` | Missing | `CLuaWorldDefs.cpp:131` | `ArgumentParser<AcquireGarageControl>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `acquireObjectGangTag` | Missing | `CLuaObjectDefs.cpp:46` | `ArgumentParser<AcquireObjectGangTag>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `areTrafficLightsLocked` | **Implemented** | `CLuaWorldDefs.cpp:200` | `AreTrafficLightsLocked` | Operational on Android. (l_areTrafficLightsLocked) |
| `breakObject` | Missing | `CLuaObjectDefs.cpp:38` | `BreakObject` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `createBuilding` | Missing | `CLuaBuildingDefs.cpp:17` | `ArgumentParser<CreateBuilding>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `createEffect` | **Implemented** | `CLuaEffectDefs.cpp:32` | `CreateEffect` | Operational on Android. (l_createEffect) |
| `createExplosion` | Missing | `CLuaWorldDefs.cpp:175` | `CreateExplosion` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `createFire` | **Implemented** | `CLuaFireDefs.cpp:149` | `CreateFire` | Operational on Android. (l_createFire) |
| `createFoliage` | **Implemented** | `CLuaFoliageDefs.cpp:42` | `CreateFoliage` | Operational on Android. (l_createFoliage) |
| `createObject` | *Stub* | `CLuaObjectDefs.cpp:21` | `CreateObject` | Currently stubbed: `FalseResult: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `createSWATRope` | Missing | `CLuaWorldDefs.cpp:174` | `CreateSWATRope` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `createWater` | Missing | `CLuaWaterDefs.cpp:18` | `CreateWater` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `engineGetPedPathGraphRevision` | Missing | `CLuaWorldDefs.cpp:60` | `EngineGetPedPathGraphRevision` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `engineGetPedPathPatchRevision` | Missing | `CLuaWorldDefs.cpp:59` | `EngineGetPedPathPatchRevision` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `engineLoadPedPathPatch` | Missing | `CLuaWorldDefs.cpp:57` | `EngineLoadPedPathPatch` | Needs hooking WarDrum RenderWare streaming clump/texture loader. |
| `engineUnloadPedPathPatch` | Missing | `CLuaWorldDefs.cpp:58` | `EngineUnloadPedPathPatch` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `extinguishFire` | Missing | `CLuaFireDefs.cpp:150` | `ExtinguishFire` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `fxAddBlood` | *Stub* | `CLuaEffectDefs.cpp:18` | `fxAddBlood` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `fxAddBulletImpact` | Missing | `CLuaEffectDefs.cpp:22` | `fxAddBulletImpact` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `fxAddBulletSplash` | Missing | `CLuaEffectDefs.cpp:30` | `fxAddBulletSplash` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `fxAddDebris` | Missing | `CLuaEffectDefs.cpp:24` | `fxAddDebris` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `fxAddFootSplash` | Missing | `CLuaEffectDefs.cpp:31` | `fxAddFootSplash` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `fxAddGlass` | Missing | `CLuaEffectDefs.cpp:25` | `fxAddGlass` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `fxAddGunshot` | Missing | `CLuaEffectDefs.cpp:27` | `fxAddGunshot` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `fxAddPunchImpact` | Missing | `CLuaEffectDefs.cpp:23` | `fxAddPunchImpact` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `fxAddSparks` | Missing | `CLuaEffectDefs.cpp:20` | `fxAddSparks` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `fxAddTankFire` | Missing | `CLuaEffectDefs.cpp:28` | `fxAddTankFire` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `fxAddTyreBurst` | Missing | `CLuaEffectDefs.cpp:21` | `fxAddTyreBurst` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `fxAddWaterHydrant` | Missing | `CLuaEffectDefs.cpp:26` | `fxAddWaterHydrant` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `fxAddWaterSplash` | Missing | `CLuaEffectDefs.cpp:29` | `fxAddWaterSplash` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `fxAddWood` | Missing | `CLuaEffectDefs.cpp:19` | `fxAddWood` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `fxCreateParticle` | Missing | `CLuaEffectDefs.cpp:37` | `ArgumentParser<FxCreateParticle>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getAircraftMaxHeight` | **Implemented** | `CLuaWorldDefs.cpp:94` | `GetAircraftMaxHeight` | Operational on Android. (l_getAircraftMaxHeight) |
| `getAircraftMaxVelocity` | **Implemented** | `CLuaWorldDefs.cpp:95` | `GetAircraftMaxVelocity` | Operational on Android. (l_getAircraftMaxVelocity) |
| `getAmbientPedCivilianCoupleCandidate` | Missing | `CLuaWorldDefs.cpp:110` | `GetAmbientPedCivilianCoupleCandidate` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getAmbientPedGangGroupCandidate` | Missing | `CLuaWorldDefs.cpp:109` | `GetAmbientPedGangGroupCandidate` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getAmbientPedPopulationProfile` | Missing | `CLuaWorldDefs.cpp:106` | `GetAmbientPedPopulationProfile` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getAmbientPedSpawnCandidate` | Missing | `CLuaWorldDefs.cpp:108` | `GetAmbientPedSpawnCandidate` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getAmbientVehicleModelCandidate` | Missing | `CLuaWorldDefs.cpp:112` | `GetAmbientVehicleModelCandidate` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getAmbientVehicleOccupantModelCandidate` | Missing | `CLuaWorldDefs.cpp:113` | `GetAmbientVehicleOccupantModelCandidate` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getAmbientVehicleSpawnCandidate` | Missing | `CLuaWorldDefs.cpp:111` | `GetAmbientVehicleSpawnCandidate` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getBirdsEnabled` | **Implemented** | `CLuaWorldDefs.cpp:101` | `GetBirdsEnabled` | Operational on Android. (l_getBirdsEnabled) |
| `getBlurLevel` | **Implemented** | `CLuaWorldDefs.cpp:79` | `GetBlurLevel` | Operational on Android. (l_getBlurLevel) |
| `getCloudsEnabled` | **Implemented** | `CLuaWorldDefs.cpp:97` | `GetCloudsEnabled` | Operational on Android. (l_getCloudsEnabled) |
| `getColorFilter` | **Implemented** | `CLuaWorldDefs.cpp:63` | `ArgumentParser<GetColorFilter>` | Operational on Android. (l_getColorFilter) |
| `getCoronaReflectionsEnabled` | **Implemented** | `CLuaWorldDefs.cpp:114` | `ArgumentParser<GetCoronaReflectionsEnabled>` | Operational on Android. (l_getCoronaReflectionsEnabled) |
| `getEffectDensity` | **Implemented** | `CLuaEffectDefs.cpp:36` | `GetEffectDensity` | Operational on Android. (l_getEffectDensity) |
| `getEffectSpeed` | **Implemented** | `CLuaEffectDefs.cpp:34` | `GetEffectSpeed` | Operational on Android. (l_getEffectSpeed) |
| `getFPSLimit` | **Implemented** | `CLuaWorldDefs.cpp:100` | `GetFPSLimit` | Operational on Android. (l_getFPSLimit) |
| `getFarClipDistance` | **Implemented** | `CLuaWorldDefs.cpp:87` | `GetFarClipDistance` | Operational on Android. (l_getFarClipDistance) |
| `getFireDamageTargets` | Missing | `CLuaFireDefs.cpp:159` | `GetFireDamageTargets` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getFireDuration` | Missing | `CLuaFireDefs.cpp:151` | `GetFireDuration` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getFireMaxGenerations` | Missing | `CLuaFireDefs.cpp:163` | `GetFireMaxGenerations` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getFireRemainingTime` | Missing | `CLuaFireDefs.cpp:153` | `GetFireRemainingTime` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getFireSource` | Missing | `CLuaFireDefs.cpp:165` | `GetFireSource` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getFireStrength` | Missing | `CLuaFireDefs.cpp:155` | `GetFireStrength` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getFireTarget` | Missing | `CLuaFireDefs.cpp:167` | `GetFireTarget` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getFogDistance` | **Implemented** | `CLuaWorldDefs.cpp:91` | `GetFogDistance` | Operational on Android. (l_getFogDistance) |
| `getFoliageDensity` | **Implemented** | `CLuaFoliageDefs.cpp:44` | `GetFoliageDensity` | Operational on Android. (l_getFoliageDensity) |
| `getFoliageSurface` | **Implemented** | `CLuaFoliageDefs.cpp:42` | `GetFoliageSurface` | Operational on Android. (l_getFoliageSurface) |
| `getFoliageVertices` | **Implemented** | `CLuaFoliageDefs.cpp:43` | `GetFoliageVertices` | Operational on Android. (l_getFoliageVertices) |
| `getGameSpeed` | **Implemented** | `CLuaWorldDefs.cpp:73` | `GetGameSpeed` | Operational on Android. (l_getGameSpeed) |
| `getGarageBoundingBox` | Missing | `CLuaWorldDefs.cpp:78` | `GetGarageBoundingBox` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getGaragePosition` | Missing | `CLuaWorldDefs.cpp:76` | `GetGaragePosition` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getGarageSize` | Missing | `CLuaWorldDefs.cpp:77` | `GetGarageSize` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getGravity` | **Implemented** | `CLuaWorldDefs.cpp:72` | `GetGravity` | Operational on Android. (l_getGravity) |
| `getGroundPosition` | **Implemented** | `CLuaWorldDefs.cpp:65` | `GetGroundPosition` | Operational on Android. (l_getGroundPosition) |
| `getHeatHaze` | **Implemented** | `CLuaWorldDefs.cpp:82` | `GetHeatHaze` | Operational on Android. (l_getHeatHaze) |
| `getInteriorFurnitureEnabled` | **Implemented** | `CLuaWorldDefs.cpp:86` | `GetInteriorFurnitureEnabled` | Operational on Android. (l_getInteriorFurnitureEnabled) |
| `getInteriorSoundsEnabled` | **Implemented** | `CLuaWorldDefs.cpp:85` | `GetInteriorSoundsEnabled` | Operational on Android. (l_getInteriorSoundsEnabled) |
| `getJetpackMaxHeight` | **Implemented** | `CLuaWorldDefs.cpp:83` | `GetJetpackMaxHeight` | Operational on Android. (l_getJetpackMaxHeight) |
| `getMinuteDuration` | **Implemented** | `CLuaWorldDefs.cpp:74` | `GetMinuteDuration` | Operational on Android. (l_getMinuteDuration) |
| `getMoonSize` | **Implemented** | `CLuaWorldDefs.cpp:99` | `GetMoonSize` | Operational on Android. (l_getMoonSize) |
| `getNearClipDistance` | **Implemented** | `CLuaWorldDefs.cpp:88` | `GetNearClipDistance` | Operational on Android. (l_getNearClipDistance) |
| `getObjectGangTagProgress` | Missing | `CLuaObjectDefs.cpp:48` | `ArgumentParser<GetObjectGangTagProgress>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getObjectMass` | Missing | `CLuaObjectDefs.cpp:27` | `GetObjectMass` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getObjectProperty` | Missing | `CLuaObjectDefs.cpp:28` | `GetObjectProperty` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getObjectScale` | Missing | `CLuaObjectDefs.cpp:25` | `GetObjectScale` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getOcclusionsEnabled` | **Implemented** | `CLuaWorldDefs.cpp:96` | `GetOcclusionsEnabled` | Operational on Android. (l_getOcclusionsEnabled) |
| `getPedsLODDistance` | **Implemented** | `CLuaWorldDefs.cpp:90` | `GetPedsLODDistance` | Operational on Android. (l_getPedsLODDistance) |
| `getRainLevel` | **Implemented** | `CLuaWorldDefs.cpp:98` | `GetRainLevel` | Operational on Android. (l_getRainLevel) |
| `getRoofPosition` | Missing | `CLuaWorldDefs.cpp:64` | `GetRoofPosition` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getScreenFromWorldPosition` | *Stub* | `CLuaWorldDefs.cpp:69` | `GetScreenFromWorldPosition` | Currently stubbed: `DefaultValue(kNil)`. Needs native engine wiring. |
| `getSkyGradient` | **Implemented** | `CLuaWorldDefs.cpp:81` | `GetSkyGradient` | Operational on Android. (l_getSkyGradient) |
| `getSunColor` | **Implemented** | `CLuaWorldDefs.cpp:92` | `GetSunColor` | Operational on Android. (l_getSunColor) |
| `getSunSize` | **Implemented** | `CLuaWorldDefs.cpp:93` | `GetSunSize` | Operational on Android. (l_getSunSize) |
| `getTime` | **Implemented** | `CLuaWorldDefs.cpp:62` | `GetTime` | Operational on Android. (l_getTime) |
| `getTrafficLightState` | **Implemented** | `CLuaWorldDefs.cpp:80` | `GetTrafficLightState` | Operational on Android. (l_getTrafficLightState) |
| `getVehiclesLODDistance` | **Implemented** | `CLuaWorldDefs.cpp:89` | `GetVehiclesLODDistance` | Operational on Android. (l_getVehiclesLODDistance) |
| `getWaterColor` | **Implemented** | `CLuaWaterDefs.cpp:28` | `GetWaterColor` | Operational on Android. (l_getWaterColor) |
| `getWaterLevel` | *Stub* | `CLuaWaterDefs.cpp:29` | `GetWaterLevel` | Currently stubbed: `DefaultValue(kNil)`. Needs native engine wiring. |
| `getWaterVertexPosition` | Missing | `CLuaWaterDefs.cpp:30` | `GetWaterVertexPosition` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getWaveHeight` | **Implemented** | `CLuaWorldDefs.cpp:75` | `GetWaveHeight` | Operational on Android. (l_getWaveHeight) |
| `getWeather` | **Implemented** | `CLuaWorldDefs.cpp:70` | `GetWeather` | Operational on Android. (l_getWeather) |
| `getWindVelocity` | **Implemented** | `CLuaWorldDefs.cpp:84` | `GetWindVelocity` | Operational on Android. (l_getWindVelocity) |
| `getWorldFromScreenPosition` | Missing | `CLuaWorldDefs.cpp:68` | `GetWorldFromScreenPosition` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getWorldProperty` | **Implemented** | `CLuaWorldDefs.cpp:115` | `ArgumentParser<GetWorldProperty>` | Operational on Android. (l_getWorldProperty) |
| `getZoneName` | Missing | `CLuaWorldDefs.cpp:71` | `GetZoneName` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isAmbientPedSphereVisible` | Missing | `CLuaWorldDefs.cpp:107` | `ArgumentParser<IsAmbientPedSphereVisible>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isDynamicPedShadowsEnabled` | **Implemented** | `CLuaWorldDefs.cpp:207` | `ArgumentParser<IsDynamicPedShadowsEnabled>` | Operational on Android. (l_isDynamicPedShadowsEnabled) |
| `isFireDamageEnabled` | Missing | `CLuaFireDefs.cpp:157` | `IsFireDamageEnabled` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isFireSpreadEnabled` | Missing | `CLuaFireDefs.cpp:161` | `IsFireSpreadEnabled` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isGarageOpen` | **Implemented** | `CLuaWorldDefs.cpp:204` | `IsGarageOpen` | Operational on Android. (l_isGarageOpen) |
| `isLineOfSightClear` | *Stub* | `CLuaWorldDefs.cpp:202` | `IsLineOfSightClear` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `isObjectBreakable` | Missing | `CLuaObjectDefs.cpp:26` | `IsObjectBreakable` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isObjectMoving` | Missing | `CLuaObjectDefs.cpp:29` | `ArgumentParser<IsObjectMoving>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isObjectRespawnable` | Missing | `CLuaObjectDefs.cpp:30` | `ArgumentParser<IsObjectRespawnable>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isObjectStatic` | Missing | `CLuaObjectDefs.cpp:24` | `IsObjectStatic` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isPedTargetingMarkerEnabled` | **Implemented** | `CLuaWorldDefs.cpp:201` | `IsPedTargetingMarkerEnabled` | Operational on Android. (l_isPedTargetingMarkerEnabled) |
| `isTimeFrozen` | **Implemented** | `CLuaWorldDefs.cpp:205` | `ArgumentParser<IsTimeFrozen>` | Operational on Android. (l_isTimeFrozen) |
| `isVolumetricShadowsEnabled` | **Implemented** | `CLuaWorldDefs.cpp:206` | `ArgumentParser<IsVolumetricShadowsEnabled>` | Operational on Android. (l_isVolumetricShadowsEnabled) |
| `isWaterDrawnLast` | **Implemented** | `CLuaWaterDefs.cpp:31` | `IsWaterDrawnLast` | Operational on Android. (l_isWaterDrawnLast) |
| `isWorldSpecialPropertyEnabled` | **Implemented** | `CLuaWorldDefs.cpp:203` | `ArgumentParserWarn<false, IsWorldSpecialPropertyEnabled>` | Operational on Android. (l_isWorldSpecialPropertyEnabled) |
| `moveObject` | Missing | `CLuaObjectDefs.cpp:33` | `MoveObject` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `processLineAgainstMesh` | Missing | `CLuaWorldDefs.cpp:66` | `ArgumentParser<ProcessLineAgainstMesh>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `processLineOfSight` | *Stub* | `CLuaWorldDefs.cpp:67` | `ProcessLineOfSight` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `releaseGarageControl` | Missing | `CLuaWorldDefs.cpp:132` | `ArgumentParser<ReleaseGarageControl>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `releaseObjectGangTag` | Missing | `CLuaObjectDefs.cpp:49` | `ArgumentParser<ReleaseObjectGangTag>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `removeAllGameBuildings` | Missing | `CLuaBuildingDefs.cpp:18` | `ArgumentParser<RemoveAllGameBuildings>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `removeGameWorld` | Missing | `CLuaWorldDefs.cpp:166` | `ArgumentParser<RemoveGameWorld>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `removeObjectMaterial` | Missing | `CLuaObjectDefs.cpp:44` | `RemoveObjectMaterial` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `removeWorldModel` | *Stub* | `CLuaWorldDefs.cpp:163` | `RemoveWorldBuilding` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `resetAmbientPedPopulationModels` | *Stub* | `CLuaWorldDefs.cpp:103` | `ArgumentParser<ResetAmbientPedPopulationModels>` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `resetAmbientPedPopulationZonesToBootstrap` | Missing | `CLuaWorldDefs.cpp:104` | `ResetAmbientPedPopulationZonesToBootstrap` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `resetBlurLevel` | **Implemented** | `CLuaWorldDefs.cpp:192` | `ResetBlurLevel` | Operational on Android. (l_resetBlurLevel) |
| `resetColorFilter` | **Implemented** | `CLuaWorldDefs.cpp:178` | `ArgumentParser<ResetColorFilter>` | Operational on Android. (l_resetColorFilter) |
| `resetCoronaReflectionsEnabled` | **Implemented** | `CLuaWorldDefs.cpp:179` | `ArgumentParser<ResetCoronaReflectionsEnabled>` | Operational on Android. (l_resetCoronaReflectionsEnabled) |
| `resetDynamicPedShadows` | **Implemented** | `CLuaWorldDefs.cpp:197` | `ArgumentParser<ResetDynamicPedShadows>` | Operational on Android. (l_resetDynamicPedShadows) |
| `resetFarClipDistance` | **Implemented** | `CLuaWorldDefs.cpp:184` | `ResetFarClipDistance` | Operational on Android. (l_resetFarClipDistance) |
| `resetFogDistance` | **Implemented** | `CLuaWorldDefs.cpp:188` | `ResetFogDistance` | Operational on Android. (l_resetFogDistance) |
| `resetHeatHaze` | **Implemented** | `CLuaWorldDefs.cpp:181` | `ResetHeatHaze` | Operational on Android. (l_resetHeatHaze) |
| `resetMoonSize` | **Implemented** | `CLuaWorldDefs.cpp:191` | `ResetMoonSize` | Operational on Android. (l_resetMoonSize) |
| `resetNearClipDistance` | **Implemented** | `CLuaWorldDefs.cpp:185` | `ResetNearClipDistance` | Operational on Android. (l_resetNearClipDistance) |
| `resetOcclusionRegion` | **Implemented** | `CLuaWorldDefs.cpp:154` | `ResetOcclusionRegion` | Operational on Android. (l_resetOcclusionRegion) |
| `resetPedsLODDistance` | **Implemented** | `CLuaWorldDefs.cpp:187` | `ResetPedsLODDistance` | Operational on Android. (l_resetPedsLODDistance) |
| `resetRainLevel` | **Implemented** | `CLuaWorldDefs.cpp:183` | `ResetRainLevel` | Operational on Android. (l_resetRainLevel) |
| `resetSkyGradient` | **Implemented** | `CLuaWorldDefs.cpp:180` | `ResetSkyGradient` | Operational on Android. (l_resetSkyGradient) |
| `resetSunColor` | **Implemented** | `CLuaWorldDefs.cpp:189` | `ResetSunColor` | Operational on Android. (l_resetSunColor) |
| `resetSunSize` | **Implemented** | `CLuaWorldDefs.cpp:190` | `ResetSunSize` | Operational on Android. (l_resetSunSize) |
| `resetTimeFrozen` | **Implemented** | `CLuaWorldDefs.cpp:194` | `ArgumentParser<ResetTimeFrozen>` | Operational on Android. (l_resetTimeFrozen) |
| `resetVehiclesLODDistance` | **Implemented** | `CLuaWorldDefs.cpp:186` | `ResetVehiclesLODDistance` | Operational on Android. (l_resetVehiclesLODDistance) |
| `resetVolumetricShadows` | **Implemented** | `CLuaWorldDefs.cpp:195` | `ArgumentParser<ResetVolumetricShadows>` | Operational on Android. (l_resetVolumetricShadows) |
| `resetWaterColor` | **Implemented** | `CLuaWaterDefs.cpp:20` | `ResetWaterColor` | Operational on Android. (l_resetWaterColor) |
| `resetWaterLevel` | **Implemented** | `CLuaWaterDefs.cpp:21` | `ResetWaterLevel` | Operational on Android. (l_resetWaterLevel) |
| `resetWindVelocity` | **Implemented** | `CLuaWorldDefs.cpp:182` | `ResetWindVelocity` | Operational on Android. (l_resetWindVelocity) |
| `resetWorldProperties` | **Implemented** | `CLuaWorldDefs.cpp:196` | `ArgumentParser<ResetWorldProperties>` | Operational on Android. (l_resetWorldProperties) |
| `resetWorldProperty` | **Implemented** | `CLuaWorldDefs.cpp:193` | `ArgumentParserWarn<false, ResetWorldProperty>` | Operational on Android. (l_resetWorldProperty) |
| `respawnObject` | Missing | `CLuaObjectDefs.cpp:39` | `RespawnObject` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `restoreAllGameBuildings` | Missing | `CLuaBuildingDefs.cpp:19` | `ArgumentParser<RestoreGameBuildings>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `restoreAllWorldModels` | Missing | `CLuaWorldDefs.cpp:164` | `RestoreWorldBuildings` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `restoreGameWorld` | Missing | `CLuaWorldDefs.cpp:167` | `ArgumentParser<RestoreGameWorld>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `restoreWorldModel` | *Stub* | `CLuaWorldDefs.cpp:165` | `RestoreWorldBuilding` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setAircraftMaxHeight` | **Implemented** | `CLuaWorldDefs.cpp:150` | `SetAircraftMaxHeight` | Operational on Android. (l_setAircraftMaxHeight) |
| `setAircraftMaxVelocity` | **Implemented** | `CLuaWorldDefs.cpp:151` | `SetAircraftMaxVelocity` | Operational on Android. (l_setAircraftMaxVelocity) |
| `setAmbientPedPopulationZoneState` | Missing | `CLuaWorldDefs.cpp:105` | `SetAmbientPedPopulationZoneState` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setBirdsEnabled` | **Implemented** | `CLuaWorldDefs.cpp:155` | `SetBirdsEnabled` | Operational on Android. (l_setBirdsEnabled) |
| `setBlurLevel` | **Implemented** | `CLuaWorldDefs.cpp:134` | `SetBlurLevel` | Operational on Android. (l_setBlurLevel) |
| `setCloudsEnabled` | **Implemented** | `CLuaWorldDefs.cpp:136` | `SetCloudsEnabled` | Operational on Android. (l_setCloudsEnabled) |
| `setColorFilter` | **Implemented** | `CLuaWorldDefs.cpp:119` | `ArgumentParser<SetColorFilter>` | Operational on Android. (l_setColorFilter) |
| `setCoronaReflectionsEnabled` | **Implemented** | `CLuaWorldDefs.cpp:159` | `ArgumentParser<SetCoronaReflectionsEnabled>` | Operational on Android. (l_setCoronaReflectionsEnabled) |
| `setDynamicPedShadowsEnabled` | **Implemented** | `CLuaWorldDefs.cpp:171` | `ArgumentParser<SetDynamicPedShadowsEnabled>` | Operational on Android. (l_setDynamicPedShadowsEnabled) |
| `setEffectDensity` | **Implemented** | `CLuaEffectDefs.cpp:35` | `SetEffectDensity` | Operational on Android. (l_setEffectDensity) |
| `setEffectSpeed` | **Implemented** | `CLuaEffectDefs.cpp:33` | `SetEffectSpeed` | Operational on Android. (l_setEffectSpeed) |
| `setFPSLimit` | **Implemented** | `CLuaWorldDefs.cpp:158` | `SetFPSLimit` | Operational on Android. (l_setFPSLimit) |
| `setFarClipDistance` | **Implemented** | `CLuaWorldDefs.cpp:143` | `SetFarClipDistance` | Operational on Android. (l_setFarClipDistance) |
| `setFireDamageEnabled` | Missing | `CLuaFireDefs.cpp:158` | `SetFireDamageEnabled` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setFireDamageTargets` | Missing | `CLuaFireDefs.cpp:160` | `SetFireDamageTargets` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setFireDuration` | Missing | `CLuaFireDefs.cpp:152` | `SetFireDuration` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setFireMaxGenerations` | Missing | `CLuaFireDefs.cpp:164` | `SetFireMaxGenerations` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setFireRemainingTime` | Missing | `CLuaFireDefs.cpp:154` | `SetFireRemainingTime` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setFireSource` | Missing | `CLuaFireDefs.cpp:166` | `SetFireSource` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setFireSpreadEnabled` | Missing | `CLuaFireDefs.cpp:162` | `SetFireSpreadEnabled` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setFireStrength` | Missing | `CLuaFireDefs.cpp:156` | `SetFireStrength` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setFireTarget` | Missing | `CLuaFireDefs.cpp:168` | `SetFireTarget` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setFogDistance` | **Implemented** | `CLuaWorldDefs.cpp:147` | `SetFogDistance` | Operational on Android. (l_setFogDistance) |
| `setFoliageDensity` | **Implemented** | `CLuaFoliageDefs.cpp:45` | `SetFoliageDensity` | Operational on Android. (l_setFoliageDensity) |
| `setFoliageSurface` | **Implemented** | `CLuaFoliageDefs.cpp:43` | `SetFoliageSurface` | Operational on Android. (l_setFoliageSurface) |
| `setFoliageVertices` | **Implemented** | `CLuaFoliageDefs.cpp:44` | `SetFoliageVertices` | Operational on Android. (l_setFoliageVertices) |
| `setGameSpeed` | **Implemented** | `CLuaWorldDefs.cpp:127` | `SetGameSpeed` | Operational on Android. (l_setGameSpeed) |
| `setGarageOpen` | **Implemented** | `CLuaWorldDefs.cpp:130` | `SetGarageOpen` | Operational on Android. (l_setGarageOpen) |
| `setGrainLevel` | **Implemented** | `CLuaWorldDefs.cpp:121` | `ArgumentParser<SetGrainLevel>` | Operational on Android. (l_setGrainLevel) |
| `setGrainMultiplier` | **Implemented** | `CLuaWorldDefs.cpp:120` | `ArgumentParser<SetGrainMultiplier>` | Operational on Android. (l_setGrainMultiplier) |
| `setGravity` | **Implemented** | `CLuaWorldDefs.cpp:126` | `SetGravity` | Operational on Android. (l_setGravity) |
| `setHeatHaze` | **Implemented** | `CLuaWorldDefs.cpp:123` | `SetHeatHaze` | Operational on Android. (l_setHeatHaze) |
| `setInteriorFurnitureEnabled` | **Implemented** | `CLuaWorldDefs.cpp:141` | `SetInteriorFurnitureEnabled` | Operational on Android. (l_setInteriorFurnitureEnabled) |
| `setInteriorSoundsEnabled` | **Implemented** | `CLuaWorldDefs.cpp:140` | `SetInteriorSoundsEnabled` | Operational on Android. (l_setInteriorSoundsEnabled) |
| `setJetpackMaxHeight` | **Implemented** | `CLuaWorldDefs.cpp:135` | `SetJetpackMaxHeight` | Operational on Android. (l_setJetpackMaxHeight) |
| `setMinuteDuration` | **Implemented** | `CLuaWorldDefs.cpp:129` | `SetMinuteDuration` | Operational on Android. (l_setMinuteDuration) |
| `setMoonSize` | **Implemented** | `CLuaWorldDefs.cpp:157` | `SetMoonSize` | Operational on Android. (l_setMoonSize) |
| `setNearClipDistance` | **Implemented** | `CLuaWorldDefs.cpp:144` | `SetNearClipDistance` | Operational on Android. (l_setNearClipDistance) |
| `setObjectBreakable` | Missing | `CLuaObjectDefs.cpp:37` | `SetObjectBreakable` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setObjectGangTagAlpha` | Missing | `CLuaObjectDefs.cpp:45` | `ArgumentParser<SetObjectGangTagAlpha>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setObjectGangTagProgress` | Missing | `CLuaObjectDefs.cpp:47` | `ArgumentParser<SetObjectGangTagProgress>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setObjectMass` | Missing | `CLuaObjectDefs.cpp:41` | `SetObjectMass` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setObjectMaterial` | *Stub* | `CLuaObjectDefs.cpp:43` | `SetObjectMaterial` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setObjectProperty` | Missing | `CLuaObjectDefs.cpp:42` | `SetObjectProperty` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setObjectScale` | Missing | `CLuaObjectDefs.cpp:35` | `SetObjectScale` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setObjectStatic` | Missing | `CLuaObjectDefs.cpp:36` | `SetObjectStatic` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setOcclusionRegion` | **Implemented** | `CLuaWorldDefs.cpp:153` | `SetOcclusionRegion` | Operational on Android. (l_setOcclusionRegion) |
| `setOcclusionsEnabled` | **Implemented** | `CLuaWorldDefs.cpp:152` | `SetOcclusionsEnabled` | Operational on Android. (l_setOcclusionsEnabled) |
| `setPedTargetingMarkerEnabled` | **Implemented** | `CLuaWorldDefs.cpp:156` | `SetPedTargetingMarkerEnabled` | Operational on Android. (l_setPedTargetingMarkerEnabled) |
| `setPedsLODDistance` | **Implemented** | `CLuaWorldDefs.cpp:146` | `SetPedsLODDistance` | Operational on Android. (l_setPedsLODDistance) |
| `setRainLevel` | **Implemented** | `CLuaWorldDefs.cpp:142` | `SetRainLevel` | Operational on Android. (l_setRainLevel) |
| `setSkyGradient` | **Implemented** | `CLuaWorldDefs.cpp:122` | `SetSkyGradient` | Operational on Android. (l_setSkyGradient) |
| `setSunColor` | **Implemented** | `CLuaWorldDefs.cpp:148` | `SetSunColor` | Operational on Android. (l_setSunColor) |
| `setSunSize` | **Implemented** | `CLuaWorldDefs.cpp:149` | `SetSunSize` | Operational on Android. (l_setSunSize) |
| `setTime` | **Implemented** | `CLuaWorldDefs.cpp:118` | `SetTime` | Operational on Android. (l_setTime) |
| `setTimeFrozen` | **Implemented** | `CLuaWorldDefs.cpp:169` | `ArgumentParser<SetTimeFrozen>` | Operational on Android. (l_setTimeFrozen) |
| `setTrafficLightState` | **Implemented** | `CLuaWorldDefs.cpp:137` | `SetTrafficLightState` | Operational on Android. (l_setTrafficLightState) |
| `setTrafficLightsLocked` | **Implemented** | `CLuaWorldDefs.cpp:138` | `SetTrafficLightsLocked` | Operational on Android. (l_setTrafficLightsLocked) |
| `setVehiclesLODDistance` | **Implemented** | `CLuaWorldDefs.cpp:145` | `SetVehiclesLODDistance` | Operational on Android. (l_setVehiclesLODDistance) |
| `setVolumetricShadowsEnabled` | **Implemented** | `CLuaWorldDefs.cpp:170` | `ArgumentParser<SetVolumetricShadowsEnabled>` | Operational on Android. (l_setVolumetricShadowsEnabled) |
| `setWaterColor` | **Implemented** | `CLuaWaterDefs.cpp:23` | `SetWaterColor` | Operational on Android. (l_setWaterColor) |
| `setWaterDrawnLast` | **Implemented** | `CLuaWaterDefs.cpp:26` | `SetWaterDrawnLast` | Operational on Android. (l_setWaterDrawnLast) |
| `setWaterLevel` | **Implemented** | `CLuaWaterDefs.cpp:24` | `SetWaterLevel` | Operational on Android. (l_setWaterLevel) |
| `setWaterVertexPosition` | Missing | `CLuaWaterDefs.cpp:25` | `SetWaterVertexPosition` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setWaveHeight` | **Implemented** | `CLuaWorldDefs.cpp:128` | `SetWaveHeight` | Operational on Android. (l_setWaveHeight) |
| `setWeather` | **Implemented** | `CLuaWorldDefs.cpp:124` | `SetWeather` | Operational on Android. (l_setWeather) |
| `setWeatherBlended` | **Implemented** | `CLuaWorldDefs.cpp:125` | `SetWeatherBlended` | Operational on Android. (l_setWeatherBlended) |
| `setWindVelocity` | **Implemented** | `CLuaWorldDefs.cpp:139` | `SetWindVelocity` | Operational on Android. (l_setWindVelocity) |
| `setWorldProperty` | **Implemented** | `CLuaWorldDefs.cpp:160` | `ArgumentParser<SetWorldProperty>` | Operational on Android. (l_setWorldProperty) |
| `setWorldSpecialPropertyEnabled` | **Implemented** | `CLuaWorldDefs.cpp:133` | `ArgumentParser<SetWorldSpecialPropertyEnabled>` | Operational on Android. (l_setWorldSpecialPropertyEnabled) |
| `stopObject` | Missing | `CLuaObjectDefs.cpp:34` | `StopObject` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `testLineAgainstWater` | Missing | `CLuaWaterDefs.cpp:19` | `TestLineAgainstWater` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `testSphereAgainstWorld` | Missing | `CLuaWorldDefs.cpp:208` | `ArgumentParser<TestSphereAgainstWorld>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `toggleObjectRespawn` | Missing | `CLuaObjectDefs.cpp:40` | `ToggleObjectRespawn` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `updateAmbientPedPopulationModels` | *Stub* | `CLuaWorldDefs.cpp:102` | `ArgumentParser<UpdateAmbientPedPopulationModels>` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
