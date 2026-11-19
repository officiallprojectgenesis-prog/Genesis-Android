# Markers, Blips, ColShapes & Spatial

Checkpoints, radar map blips, collision shapes (spheres, cubes, polygons), radar areas, pickups, and 3D searchlights.

## Parity Status

- **Total Functions:** 145
- **Implemented:** 145 (100%)
- **Stubs:** 0
- **Missing:** 0

> **Technical Requirement:** Requires spatial overlap checks (AABB / SAT collision algorithms) and radar texture rendering hooks.

## Function List

| Lua Function | Status | Desktop Source Ref | Desktop Handler | Conversion Requirement / Implementation Notes |
|---|---|---|---|---|
| `addColPolygonPoint` | **Implemented** | `CLuaColShapeDefs.cpp:31` | `AddColPolygonPoint` | Ported. Appends when the index is omitted. 1-based insert. `CLuaColShapeDefs.cpp` 747-782. |
| `attachElementToRope` | **Implemented** | `CLuaRopeDefs.cpp:230` | `AttachElementToRope` | Ported. The element must be an object or a vehicle. `CLuaRopeDefs.cpp` 461-465. |
| `clearNativeText` | **Implemented** | `CLuaNativeUIDefs.cpp:261` | `ClearNativeText` | Ported. Channels are all, objective, dialogue, help, and big. Nothing is drawn. `CLuaNativeUIDefs.cpp` 363-372. |
| `countPlayersInTeam` | **Implemented** | `CLuaTeamDefs.cpp:18` | `ArgumentParserWarn<false, CountPlayersInTeam>` | Ported. Counts the players from the last ENTITY_ADD. `CLuaTeamDefs.cpp` 80-83. |
| `createBird` | **Implemented** | `CLuaBirdDefs.cpp:149` | `CreateBird` | Ported. Defaults are size 0.55, speed 5.3, and gray. `water` and `desert` presets apply first. The mesh is not drawn and the bird does not move. `CLuaBirdDefs.cpp` 183-277. |
| `createBlip` | **Implemented** | `CLuaBlipDefs.cpp:141` | `ArgumentParserWarn<false, CreateBlip>` | Ported. Defaults are icon 0, size 2, red, ordering 0 and distance 16383. Icons above 63 fail. Size above 25 is clamped. The radar sprite is not drawn. `CLuaBlipDefs.cpp` 17-44. |
| `createBlipAttachedTo` | **Implemented** | `CLuaBlipDefs.cpp:142` | `ArgumentParserWarn<false, CreateBlipAttachedTo>` | Ported. The blip follows the element position. `CLuaBlipDefs.cpp` 46-74. |
| `createColCircle` | **Implemented** | `CLuaColShapeDefs.cpp:17` | `CreateColCircle` | Ported. Negative radius becomes 0.1. Z is 0. `CLuaColShapeDefs.cpp` 109-150. |
| `createColCuboid` | **Implemented** | `CLuaColShapeDefs.cpp:18` | `CreateColCuboid` | Operational on Android. (CreateColCuboid) |
| `createColPolygon` | **Implemented** | `CLuaColShapeDefs.cpp:21` | `CreateColPolygon` | Ported. At least three world-XY points. Open height until set. `CLuaColShapeDefs.cpp` 288-337. |
| `createColRectangle` | **Implemented** | `CLuaColShapeDefs.cpp:20` | `CreateColRectangle` | Ported. Negative width or depth becomes 0.1. Z is 0. `CLuaColShapeDefs.cpp` 241-286. |
| `createColSphere` | **Implemented** | `CLuaColShapeDefs.cpp:19` | `CreateColSphere` | Operational on Android. (CreateColSphere) |
| `createColTube` | **Implemented** | `CLuaColShapeDefs.cpp:22` | `CreateColTube` | Ported. Negative radius or height becomes 0.1. Argument errors use the short bad-argument warning. `CLuaColShapeDefs.cpp` 339-386. |
| `createLight` | **Implemented** | `CLuaPointLightDefs.cpp:19` | `CreateLight` | Ported. Radius defaults to 3, color to red, direction to 0. `createsShadow` is checked and not stored. The light is not drawn. `CLuaPointLightDefs.cpp` 56-102. |
| `createMarker` | **Implemented** | `CLuaMarkerDefs.cpp:20` | `CreateMarker` | Ported. Stores type, size, color and the alpha-limit flag. The drawable and the marker colshape are not created. `CLuaMarkerDefs.cpp` 107-152. |
| `createNativeUI` | **Implemented** | `CLuaNativeUIDefs.cpp:260` | `CreateNativeUI` | Ported. Stores the widget for the resource. It is not drawn. `CLuaNativeUIDefs.cpp` 266-283. |
| `createPickup` | **Implemented** | `CLuaPickupDefs.cpp:17` | `CreatePickup` | Ported. Health and armor take 0..100. Weapon ammo above 9999 becomes 9999. A custom model must be a stock object. `CLuaPickupDefs.cpp` 48-92. |
| `createRadarArea` | **Implemented** | `CLuaRadarAreaDefs.cpp:18` | `CreateRadarArea` | Ported. Defaults are red and not flashing. A negative size still contains its rectangle. The area is not drawn. `CLuaRadarAreaDefs.cpp` 49-95. |
| `createRope` | **Implemented** | `CLuaRopeDefs.cpp:218` | `CreateRope` | Ported. Default type is swat. Remaining time starts at the duration and does not tick. `CLuaRopeDefs.cpp` 237-319. |
| `createSearchLight` | **Implemented** | `CLuaSearchLightDefs.cpp:17` | `CreateSearchLight` | Ported. Color starts at 200, 200, 255, 0. `renderSpot` defaults to true. The beam is not drawn. `CLuaSearchLightDefs.cpp` 59-98. |
| `destroyNativeUI` | **Implemented** | `CLuaNativeUIDefs.cpp:260` | `DestroyNativeUI` | Ported. `CLuaNativeUIDefs.cpp` 297-307. |
| `detachElementFromRope` | **Implemented** | `CLuaRopeDefs.cpp:230` | `DetachElementFromRope` | Ported. Clears the carried element. `CLuaRopeDefs.cpp` 467-470. |
| `getBirdColors` | **Implemented** | `CLuaBirdDefs.cpp:151` | `GetBirdColors` | Ported. Six channels, body then wings. `CLuaBirdDefs.cpp` 316-322. |
| `getBirdRenderDistance` | **Implemented** | `CLuaBirdDefs.cpp:153` | `GetBirdRenderDistance` | Ported. `CLuaBirdDefs.cpp` 342-346. |
| `getBirdSize` | **Implemented** | `CLuaBirdDefs.cpp:151` | `GetBirdSize` | Ported. `CLuaBirdDefs.cpp` 305-308. |
| `getBirdTargetVelocity` | **Implemented** | `CLuaBirdDefs.cpp:150` | `GetBirdTargetVelocity` | Ported. Returns a Vector3. `CLuaBirdDefs.cpp` 292-297. |
| `getBirdVelocity` | **Implemented** | `CLuaBirdDefs.cpp:149` | `GetBirdVelocity` | Ported. Returns a Vector3. `CLuaBirdDefs.cpp` 279-284. |
| `getBirdWingBeatTime` | **Implemented** | `CLuaBirdDefs.cpp:152` | `GetBirdWingBeatTime` | Ported. `CLuaBirdDefs.cpp` 331-334. |
| `getBlipColor` | **Implemented** | `CLuaBlipDefs.cpp:147` | `ArgumentParserWarn<false, GetBlipColor>` | Ported. `CLuaBlipDefs.cpp` 86-90. |
| `getBlipIcon` | **Implemented** | `CLuaBlipDefs.cpp:145` | `ArgumentParserWarn<false, GetBlipIcon>` | Ported. `CLuaBlipDefs.cpp` 76-79. |
| `getBlipOrdering` | **Implemented** | `CLuaBlipDefs.cpp:148` | `ArgumentParserWarn<false, GetBlipOrdering>` | Ported. `CLuaBlipDefs.cpp` 92-95. |
| `getBlipSize` | **Implemented** | `CLuaBlipDefs.cpp:146` | `ArgumentParserWarn<false, GetBlipSize>` | Ported. `CLuaBlipDefs.cpp` 81-84. |
| `getBlipVisibleDistance` | **Implemented** | `CLuaBlipDefs.cpp:149` | `ArgumentParserWarn<false, GetBlipVisibleDistance>` | Ported. `CLuaBlipDefs.cpp` 97-100. |
| `getColPolygonHeight` | **Implemented** | `CLuaColShapeDefs.cpp:37` | `ArgumentParser<GetColPolygonHeight>` | Ported. A non-polygon raises. `CLuaColShapeDefs.cpp` 820-825. |
| `getColPolygonPointPosition` | **Implemented** | `CLuaColShapeDefs.cpp:29` | `GetColPolygonPointPosition` | Ported. Index 0 or past the end warns and returns false. `CLuaColShapeDefs.cpp` 650-680. |
| `getColPolygonPoints` | **Implemented** | `CLuaColShapeDefs.cpp:28` | `GetColPolygonPoints` | Ported. Each point is `{x, y}`. `CLuaColShapeDefs.cpp` 580-618. |
| `getColShapeRadius` | **Implemented** | `CLuaColShapeDefs.cpp:24` | `GetColShapeRadius` | Ported. Circle, sphere and tube only. `CLuaColShapeDefs.cpp` 414-433. |
| `getColShapeSize` | **Implemented** | `CLuaColShapeDefs.cpp:26` | `GetColShapeSize` | Ported. Rectangle returns two numbers, cuboid three, tube the height. `CLuaColShapeDefs.cpp` 457-494. |
| `getColShapeType` | **Implemented** | `CLuaColShapeDefs.cpp:35` | `GetColShapeType` | Ported. Returns the desktop enum number. `CLuaColShapeDefs.cpp` 84-107. |
| `getLightColor` | **Implemented** | `CLuaPointLightDefs.cpp:20` | `GetLightColor` | Ported. Three channels. `CLuaPointLightDefs.cpp` 152-176. |
| `getLightDirection` | **Implemented** | `CLuaPointLightDefs.cpp:20` | `GetLightDirection` | Ported. `CLuaPointLightDefs.cpp` 178-202. |
| `getLightRadius` | **Implemented** | `CLuaPointLightDefs.cpp:19` | `GetLightRadius` | Ported. `CLuaPointLightDefs.cpp` 128-150. |
| `getLightType` | **Implemented** | `CLuaPointLightDefs.cpp:19` | `GetLightType` | Ported. `CLuaPointLightDefs.cpp` 104-126. |
| `getMarkerColor` | **Implemented** | `CLuaMarkerDefs.cpp:27` | `GetMarkerColor` | Ported. Checkpoint and arrow alpha reads as 255 unless the limit is ignored. `CLuaMarkerDefs.cpp` 228-255. |
| `getMarkerCount` | **Implemented** | `CLuaMarkerDefs.cpp:23` | `GetMarkerCount` | Ported. Local markers plus ENTITY_ADD markers. `CLuaMarkerDefs.cpp` 154-159. |
| `getMarkerIcon` | **Implemented** | `CLuaMarkerDefs.cpp:29` | `GetMarkerIcon` | Ported. Checkpoint and ring only. `CLuaMarkerDefs.cpp` 308-331. |
| `getMarkerLimitStats` | **Implemented** | `CLuaMarkerDefs.cpp:24` | `GetMarkerLimitStats` | Ported. Capacities are 4096. Live 3D and checkpoint counts stay 0. `CLuaMarkerDefs.cpp` 161-183. |
| `getMarkerSize` | **Implemented** | `CLuaMarkerDefs.cpp:26` | `GetMarkerSize` | Ported. `CLuaMarkerDefs.cpp` 208-226. |
| `getMarkerTarget` | **Implemented** | `CLuaMarkerDefs.cpp:28` | `GetMarkerTarget` | Ported. Checkpoint and ring with a target. `CLuaMarkerDefs.cpp` 257-280. |
| `getMarkerTargetArrowProperties` | **Implemented** | `CLuaMarkerDefs.cpp:30` | `ArgumentParser<GetMarkerTargetArrowProperties>` | Ported. Checkpoint with a target only. `CLuaMarkerDefs.cpp` 500-512. |
| `getMarkerType` | **Implemented** | `CLuaMarkerDefs.cpp:25` | `GetMarkerType` | Ported. `CLuaMarkerDefs.cpp` 185-206. |
| `getNativeUIState` | **Implemented** | `CLuaNativeUIDefs.cpp:260` | `GetNativeUIState` | Ported. Selection does not change because input is not captured. `CLuaNativeUIDefs.cpp` 308-327. |
| `getPickupAmmo` | **Implemented** | `CLuaPickupDefs.cpp:19` | `GetPickupAmmo` | Ported. `CLuaPickupDefs.cpp` 154-172. |
| `getPickupAmount` | **Implemented** | `CLuaPickupDefs.cpp:19` | `GetPickupAmount` | Ported. `CLuaPickupDefs.cpp` 134-152. |
| `getPickupType` | **Implemented** | `CLuaPickupDefs.cpp:19` | `GetPickupType` | Ported. 0 health, 1 armor, 2 weapon, 3 custom. `CLuaPickupDefs.cpp` 94-112. |
| `getPickupWeapon` | **Implemented** | `CLuaPickupDefs.cpp:19` | `GetPickupWeapon` | Ported. `CLuaPickupDefs.cpp` 114-132. |
| `getPlayersInTeam` | **Implemented** | `CLuaTeamDefs.cpp:18` | `ArgumentParserWarn<false, GetPlayersInTeam>` | Ported. Returns those player elements in packet order. `CLuaTeamDefs.cpp` 75-78. |
| `getRadarAreaColor` | **Implemented** | `CLuaRadarAreaDefs.cpp:18` | `GetRadarAreaColor` | Ported. `CLuaRadarAreaDefs.cpp` 97-122. |
| `getRadarAreaSize` | **Implemented** | `CLuaRadarAreaDefs.cpp:18` | `GetRadarAreaSize` | Ported. `CLuaRadarAreaDefs.cpp` 124-147. |
| `getRopeCarriedElement` | **Implemented** | `CLuaRopeDefs.cpp:230` | `GetRopeCarriedElement` | Ported. `CLuaRopeDefs.cpp` 455-459. |
| `getRopeDuration` | **Implemented** | `CLuaRopeDefs.cpp:220` | `GetRopeDuration` | Ported. `CLuaRopeDefs.cpp` 344. |
| `getRopeFixedNode` | **Implemented** | `CLuaRopeDefs.cpp:227` | `GetRopeFixedNode` | Ported. `CLuaRopeDefs.cpp` 348. |
| `getRopeHolder` | **Implemented** | `CLuaRopeDefs.cpp:222` | `GetRopeHolder` | Ported. `CLuaRopeDefs.cpp` 390-394. |
| `getRopeHolderOffset` | **Implemented** | `CLuaRopeDefs.cpp:223` | `GetRopeHolderOffset` | Ported. Returns a Vector3. `CLuaRopeDefs.cpp` 405-409. |
| `getRopeLength` | **Implemented** | `CLuaRopeDefs.cpp:226` | `GetRopeLength` | Ported. `CLuaRopeDefs.cpp` 347. |
| `getRopePositionAt` | **Implemented** | `CLuaRopeDefs.cpp:231` | `GetRopePositionAt` | Ported. Returns false because no native rope slot is leased. `CLuaRopeDefs.cpp` 472-481. |
| `getRopeRemainingTime` | **Implemented** | `CLuaRopeDefs.cpp:221` | `GetRopeRemainingTime` | Ported. Does not tick. `CLuaRopeDefs.cpp` 345. |
| `getRopeTopVelocity` | **Implemented** | `CLuaRopeDefs.cpp:224` | `GetRopeTopVelocity` | Ported. Returns a Vector3. `CLuaRopeDefs.cpp` 419-423. |
| `getRopeType` | **Implemented** | `CLuaRopeDefs.cpp:219` | `GetRopeType` | Ported. `CLuaRopeDefs.cpp` 321-328. |
| `getRopeWinchHeight` | **Implemented** | `CLuaRopeDefs.cpp:225` | `GetRopeWinchHeight` | Ported. `CLuaRopeDefs.cpp` 346. |
| `getSearchLightColor` | **Implemented** | `CLuaSearchLightDefs.cpp:23` | `ArgumentParser<GetSearchLightColor>` | Ported. Four channels. `CLuaSearchLightDefs.cpp` 318-322. |
| `getSearchLightEndPosition` | **Implemented** | `CLuaSearchLightDefs.cpp:20` | `GetSearchLightEndPosition` | Ported. `CLuaSearchLightDefs.cpp` 123-144. |
| `getSearchLightEndRadius` | **Implemented** | `CLuaSearchLightDefs.cpp:22` | `GetSearchLightEndRadius` | Ported. `CLuaSearchLightDefs.cpp` 166-184. |
| `getSearchLightStartPosition` | **Implemented** | `CLuaSearchLightDefs.cpp:19` | `GetSearchLightStartPosition` | Ported. Also what `getElementPosition` returns. `CLuaSearchLightDefs.cpp` 100-121. |
| `getSearchLightStartRadius` | **Implemented** | `CLuaSearchLightDefs.cpp:21` | `GetSearchLightStartRadius` | Ported. `CLuaSearchLightDefs.cpp` 146-164. |
| `getTeamColor` | **Implemented** | `CLuaTeamDefs.cpp:17` | `ArgumentParserWarn<false, GetTeamColor>` | Ported. `CLuaTeamDefs.cpp` 59-68. |
| `getTeamFriendlyFire` | **Implemented** | `CLuaTeamDefs.cpp:17` | `ArgumentParserWarn<false, GetTeamFriendlyFire>` | Ported. `CLuaTeamDefs.cpp` 70-73. |
| `getTeamFromName` | **Implemented** | `CLuaTeamDefs.cpp:16` | `ArgumentParserWarn<false, GetTeamFromName>` | Ported. The name match is case sensitive. `CClientTeamManager.cpp` 72-81. |
| `getTeamName` | **Implemented** | `CLuaTeamDefs.cpp:16` | `ArgumentParserWarn<false, GetTeamName>` | Ported. Reads the team-body name, not the element name. `CLuaTeamDefs.cpp` 54-57. |
| `isBirdCurvedFlightEnabled` | **Implemented** | `CLuaBirdDefs.cpp:154` | `IsBirdCurvedFlightEnabled` | Ported. `CLuaBirdDefs.cpp` 353-356. |
| `isBirdMovementEnabled` | **Implemented** | `CLuaBirdDefs.cpp:156` | `IsBirdMovementEnabled` | Ported. Stored only. DoPulse does not run. `CLuaBirdDefs.cpp` 375-378. |
| `isBirdShootable` | **Implemented** | `CLuaBirdDefs.cpp:155` | `IsBirdShootable` | Ported. `CLuaBirdDefs.cpp` 364-367. |
| `isCoronaReflectionEnabled` | **Implemented** | `CLuaMarkerDefs.cpp:40` | `ArgumentParser<IsCoronaReflectionEnabled>` | Ported. False unless the marker is a corona. `CLuaMarkerDefs.cpp` 478-486. |
| `isInsideColShape` | **Implemented** | `CLuaColShapeDefs.cpp:34` | `IsInsideColShape` | Ported. Uses the same hit test as the spatial world. `CLuaColShapeDefs.cpp` 388-412. |
| `isInsideRadarArea` | **Implemented** | `CLuaRadarAreaDefs.cpp:20` | `IsInsideRadarArea` | Ported. Bounds are sorted, so a negative size still hits. `CStaticFunctionDefinitions.cpp` 4544-4568. |
| `isRadarAreaFlashing` | **Implemented** | `CLuaRadarAreaDefs.cpp:19` | `IsRadarAreaFlashing` | Ported. The flag is stored and does not pulse the alpha. `CLuaRadarAreaDefs.cpp` 173-193. |
| `isRopeActive` | **Implemented** | `CLuaRopeDefs.cpp:231` | `IsRopeActive` | Ported. False because no native rope slot is leased. `CLuaRopeDefs.cpp` 483-486. |
| `isRopeOnGround` | **Implemented** | `CLuaRopeDefs.cpp:228` | `IsRopeOnGround` | Ported. `CLuaRopeDefs.cpp` 433-436. |
| `isRopePhysicsEnabled` | **Implemented** | `CLuaRopeDefs.cpp:229` | `IsRopePhysicsEnabled` | Ported. `CLuaRopeDefs.cpp` 444-447. |
| `isShowCollisionsEnabled` | **Implemented** | `CLuaColShapeDefs.cpp:40` | `ArgumentParser<IsShowCollisionsEnabled>` | Ported. Reads the client flag. `CLuaColShapeDefs.cpp` 863-866. |
| `processBirdGunShot` | **Implemented** | `CLuaBirdDefs.cpp:157` | `ProcessBirdGunShot` | Ported. A hit within 0.5 fires `onClientBirdShot` and destroys the bird unless cancelled. `CClientBirdManager.cpp` 321-373. |
| `releaseNativeUI` | **Implemented** | `CLuaNativeUIDefs.cpp:261` | `ReleaseNativeUI` | Ported. Drops the resource widgets and messages. `CLuaNativeUIDefs.cpp` 328-335. |
| `removeColPolygonPoint` | **Implemented** | `CLuaColShapeDefs.cpp:32` | `RemoveColPolygonPoint` | Ported. The last three points stay. `CLuaColShapeDefs.cpp` 784-818. |
| `renderScriptImportantArea` | **Implemented** | `CLuaMarkerDefs.cpp:21` | `ArgumentParser<RenderScriptImportantArea>` | Ported. A valid call returns false because `C3DMarkers` is not hooked. `CLuaMarkerDefs.cpp` 48-75. |
| `setBirdColors` | **Implemented** | `CLuaBirdDefs.cpp:151` | `SetBirdColors` | Ported. Channels are clamped to 0..255. `CLuaBirdDefs.cpp` 324-329. |
| `setBirdCurvedFlightEnabled` | **Implemented** | `CLuaBirdDefs.cpp:154` | `SetBirdCurvedFlightEnabled` | Ported. `CLuaBirdDefs.cpp` 358-362. |
| `setBirdMovementEnabled` | **Implemented** | `CLuaBirdDefs.cpp:156` | `SetBirdMovementEnabled` | Ported. `CLuaBirdDefs.cpp` 380-384. |
| `setBirdRenderDistance` | **Implemented** | `CLuaBirdDefs.cpp:153` | `SetBirdRenderDistance` | Ported. Must be above 0 and at most 5000. `CLuaBirdDefs.cpp` 348-351. |
| `setBirdShootable` | **Implemented** | `CLuaBirdDefs.cpp:155` | `SetBirdShootable` | Ported. `CLuaBirdDefs.cpp` 369-373. |
| `setBirdSize` | **Implemented** | `CLuaBirdDefs.cpp:151` | `SetBirdSize` | Ported. Must be above 0 and at most 20. `CLuaBirdDefs.cpp` 310-314. |
| `setBirdTargetVelocity` | **Implemented** | `CLuaBirdDefs.cpp:150` | `SetBirdTargetVelocity` | Ported. `CLuaBirdDefs.cpp` 299-303. |
| `setBirdVelocity` | **Implemented** | `CLuaBirdDefs.cpp:149` | `SetBirdVelocity` | Ported. `CLuaBirdDefs.cpp` 286-290. |
| `setBirdWingBeatTime` | **Implemented** | `CLuaBirdDefs.cpp:152` | `SetBirdWingBeatTime` | Ported. Must be 1 through 600000. `CLuaBirdDefs.cpp` 336-340. |
| `setBlipColor` | **Implemented** | `CLuaBlipDefs.cpp:154` | `ArgumentParserWarn<false, SetBlipColor>` | Ported. Four channels, 0 through 255. `CLuaBlipDefs.cpp` 122-125. |
| `setBlipIcon` | **Implemented** | `CLuaBlipDefs.cpp:152` | `ArgumentParserWarn<false, SetBlipIcon>` | Ported. Icons above 63 fail. `CLuaBlipDefs.cpp` 102-108. |
| `setBlipOrdering` | **Implemented** | `CLuaBlipDefs.cpp:155` | `ArgumentParserWarn<false, SetBlipOrdering>` | Ported. Signed 16-bit. `CLuaBlipDefs.cpp` 127-130. |
| `setBlipSize` | **Implemented** | `CLuaBlipDefs.cpp:153` | `ArgumentParserWarn<false, SetBlipSize>` | Ported. Values above 25 are clamped to 25. `CLuaBlipDefs.cpp` 110-120. |
| `setBlipVisibleDistance` | **Implemented** | `CLuaBlipDefs.cpp:156` | `ArgumentParserWarn<false, SetBlipVisibleDistance>` | Ported. Unsigned 16-bit. `CLuaBlipDefs.cpp` 132-135. |
| `setColPolygonHeight` | **Implemented** | `CLuaColShapeDefs.cpp:36` | `ArgumentParser<SetColPolygonHeight>` | Ported. A boolean restores the open end. An unchanged span returns false. `CLuaColShapeDefs.cpp` 827-852. |
| `setColPolygonPointPosition` | **Implemented** | `CLuaColShapeDefs.cpp:30` | `SetColPolygonPointPosition` | Ported. Refreshes colliders. `CLuaColShapeDefs.cpp` 714-745. |
| `setColShapeRadius` | **Implemented** | `CLuaColShapeDefs.cpp:25` | `SetColShapeRadius` | Ported. Negative becomes 0 and colliders refresh. `CLuaColShapeDefs.cpp` 435-455. |
| `setColShapeSize` | **Implemented** | `CLuaColShapeDefs.cpp:27` | `SetColShapeSize` | Ported. Negative components become 0. Tube uses the first number as height. `CLuaColShapeDefs.cpp` 532-578. |
| `setCoronaReflectionEnabled` | **Implemented** | `CLuaMarkerDefs.cpp:39` | `ArgumentParser<SetCoronaReflectionEnabled>` | Ported. Stored on corona markers. `CLuaMarkerDefs.cpp` 469-476. |
| `setLightColor` | **Implemented** | `CLuaPointLightDefs.cpp:20` | `SetLightColor` | Ported. Three channels, 0 through 255. `CLuaPointLightDefs.cpp` 229-257. |
| `setLightDirection` | **Implemented** | `CLuaPointLightDefs.cpp:20` | `SetLightDirection` | Ported. `CLuaPointLightDefs.cpp` 259-281. |
| `setLightRadius` | **Implemented** | `CLuaPointLightDefs.cpp:19` | `SetLightRadius` | Ported. `CLuaPointLightDefs.cpp` 204-227. |
| `setMarkerColor` | **Implemented** | `CLuaMarkerDefs.cpp:34` | `SetMarkerColor` | Ported. Channels must be 0 through 255. `CLuaMarkerDefs.cpp` 379-418. |
| `setMarkerIcon` | **Implemented** | `CLuaMarkerDefs.cpp:36` | `SetMarkerIcon` | Ported. Checkpoint and ring. `none`, `arrow`, `finish`. `CLuaMarkerDefs.cpp` 445-467. |
| `setMarkerSize` | **Implemented** | `CLuaMarkerDefs.cpp:33` | `SetMarkerSize` | Ported. A checkpoint size change resets the arrow size. `CLuaMarkerDefs.cpp` 355-377. |
| `setMarkerTarget` | **Implemented** | `CLuaMarkerDefs.cpp:35` | `SetMarkerTarget` | Ported. Sets the icon to arrow. `CLuaMarkerDefs.cpp` 420-443. |
| `setMarkerTargetArrowProperties` | **Implemented** | `CLuaMarkerDefs.cpp:37` | `ArgumentParser<SetMarkerTargetArrowProperties>` | Ported. Only an arrow icon. Defaults 255, 64, 64, 255 and 0.625 of the size. `CLuaMarkerDefs.cpp` 488-498. |
| `setMarkerType` | **Implemented** | `CLuaMarkerDefs.cpp:32` | `SetMarkerType` | Ported. Checkpoint/ring and cylinder/arrow keep their extra state. A valid name on a non-marker returns true. `CLuaMarkerDefs.cpp` 333-353. |
| `setPickupType` | **Implemented** | `CLuaPickupDefs.cpp:21` | `SetPickupType` | Ported. A non-pickup returns false. Changing to health keeps the previous weapon id. `CLuaPickupDefs.cpp` 174-201. |
| `setRadarAreaColor` | **Implemented** | `CLuaRadarAreaDefs.cpp:19` | `SetRadarAreaColor` | Ported. Alpha defaults to 255. Channels wrap like `SColor`. `CLuaRadarAreaDefs.cpp` 195-225. |
| `setRadarAreaFlashing` | **Implemented** | `CLuaRadarAreaDefs.cpp:19` | `SetRadarAreaFlashing` | Ported. `CLuaRadarAreaDefs.cpp` 252-275. |
| `setRadarAreaSize` | **Implemented** | `CLuaRadarAreaDefs.cpp:20` | `SetRadarAreaSize` | Ported. `CLuaRadarAreaDefs.cpp` 227-250. |
| `setRopeDuration` | **Implemented** | `CLuaRopeDefs.cpp:220` | `SetRopeDuration` | Ported. Also resets the remaining time. `CLuaRopeDefs.cpp` 350-356. |
| `setRopeFixedNode` | **Implemented** | `CLuaRopeDefs.cpp:227` | `SetRopeFixedNode` | Ported. The node must be 0 through 30. `CLuaRopeDefs.cpp` 382-388. |
| `setRopeHolder` | **Implemented** | `CLuaRopeDefs.cpp:222` | `SetRopeHolder` | Ported. Nil clears it. The holder must be an object or a vehicle. `CLuaRopeDefs.cpp` 396-403. |
| `setRopeHolderOffset` | **Implemented** | `CLuaRopeDefs.cpp:223` | `SetRopeHolderOffset` | Ported. `CLuaRopeDefs.cpp` 411-417. |
| `setRopeLength` | **Implemented** | `CLuaRopeDefs.cpp:226` | `SetRopeLength` | Ported. `CLuaRopeDefs.cpp` 374-380. |
| `setRopeOnGround` | **Implemented** | `CLuaRopeDefs.cpp:228` | `SetRopeOnGround` | Ported. `CLuaRopeDefs.cpp` 438-442. |
| `setRopePhysicsEnabled` | **Implemented** | `CLuaRopeDefs.cpp:229` | `SetRopePhysicsEnabled` | Ported. `CLuaRopeDefs.cpp` 449-453. |
| `setRopeRemainingTime` | **Implemented** | `CLuaRopeDefs.cpp:221` | `SetRopeRemainingTime` | Ported. `CLuaRopeDefs.cpp` 358-364. |
| `setRopeTopVelocity` | **Implemented** | `CLuaRopeDefs.cpp:224` | `SetRopeTopVelocity` | Ported. `CLuaRopeDefs.cpp` 425-431. |
| `setRopeType` | **Implemented** | `CLuaRopeDefs.cpp:219` | `SetRopeType` | Ported. Unknown names return false. `CLuaRopeDefs.cpp` 330-339. |
| `setRopeWinchHeight` | **Implemented** | `CLuaRopeDefs.cpp:225` | `SetRopeWinchHeight` | Ported. Must be at least 0.01. `CLuaRopeDefs.cpp` 366-372. |
| `setSearchLightColor` | **Implemented** | `CLuaSearchLightDefs.cpp:29` | `ArgumentParser<SetSearchLightColor>` | Ported. One tocolor number, `0xAARRGGBB`. `CLuaSearchLightDefs.cpp` 324-327. |
| `setSearchLightEndPosition` | **Implemented** | `CLuaSearchLightDefs.cpp:26` | `SetSearchLightEndPosition` | Ported. `CLuaSearchLightDefs.cpp` 209-231. |
| `setSearchLightEndRadius` | **Implemented** | `CLuaSearchLightDefs.cpp:28` | `SetSearchLightEndRadius` | Ported. `CLuaSearchLightDefs.cpp` 255-276. |
| `setSearchLightStartPosition` | **Implemented** | `CLuaSearchLightDefs.cpp:25` | `SetSearchLightStartPosition` | Ported. `CLuaSearchLightDefs.cpp` 186-207. |
| `setSearchLightStartRadius` | **Implemented** | `CLuaSearchLightDefs.cpp:27` | `SetSearchLightStartRadius` | Ported. `CLuaSearchLightDefs.cpp` 232-253. |
| `showCol` | **Implemented** | `CLuaColShapeDefs.cpp:39` | `ArgumentParser<SetShowCollision>` | Ported. Writes the flag only in development mode. `CLuaColShapeDefs.cpp` 854-861. |
| `showNativeText` | **Implemented** | `CLuaNativeUIDefs.cpp:261` | `ShowNativeText` | Ported. The text handle, channel, and style are checked. The message is not drawn. `CLuaNativeUIDefs.cpp` 336-362. |
| `updateNativeUI` | **Implemented** | `CLuaNativeUIDefs.cpp:260` | `UpdateNativeUI` | Ported. `CLuaNativeUIDefs.cpp` 284-296. |
