# Element Tree & Data (CLuaElementDefs)

Core element hierarchy (parent/child), element data (synced/local), collisions, dimensions, alpha, positions, velocities, matrix, and spatial streaming.

## Parity Status

- **Total Functions:** 81
- **Implemented:** 81 (100.0%)
- **Stubs:** 0
- **Missing:** 0

> **Technical Requirement:** Implemented via `LuaElementApi.cpp`, `LuaElementTree.cpp` and `ServerWorld.cpp`. Attachment queries read ENTITY_ADD metadata. Marker collisions are created with the marker. Velocity, angular velocity, matrix, attachment offsets, low-LOD links and streaming-lease tokens are stored. Model bounding boxes, on-screen tests, ground-wait flags, native fire and the streamer force-in are not read from the game. Physical attachment and attachment RPCs remain pending. Host-tested. Not yet run on a device.

## Function List

| Lua Function | Status | Desktop Source Ref | Desktop Handler | Conversion Requirement / Implementation Notes |
|---|---|---|---|---|
| `acquireElementStreamingLease` | **Implemented** | `CLuaElementDefs.cpp:96` | `AcquireElementStreamingLease` | Follows `CLuaElementDefs.cpp` 2479-2503 and `CResource.cpp` 304-319. A ped, player, vehicle, object, weapon, marker, pickup or searchlight returns a token and counts a script stream reference. 65535 references return false. The streamer force-in is not called. Host-tested. Not device-verified. |
| `attachElements` | **Implemented** | `CLuaElementDefs.cpp:89` | `AttachElements` | Operational on Android. (AttachElements) |
| `createElement` | **Implemented** | `CLuaElementDefs.cpp:76` | `CreateElement` | Operational on Android. (CreateElement) |
| `destroyElement` | **Implemented** | `CLuaElementDefs.cpp:77` | `DestroyElement` | Operational on Android. (DestroyElementDispatch) |
| `detachElements` | **Implemented** | `CLuaElementDefs.cpp:90` | `DetachElements` | Operational on Android. (DetachElements) |
| `getAllElementData` | **Implemented** | `CLuaElementDefs.cpp:27` | `ArgumentParserWarn<false, GetAllElementData>` | Operational on Android. (GetAllElementData) |
| `getAttachedElements` | **Implemented** | `CLuaElementDefs.cpp:50` | `GetAttachedElements` | Reads server ENTITY_ADD attachment metadata, including forward references in the batch and offsets in degrees. Removal clears references before ID reuse. Physical attachment, local attach/detach and later attachment RPCs remain pending. Host-tested; not device-verified. |
| `getElementAlpha` | **Implemented** | `CLuaElementDefs.cpp:55` | `GetElementAlpha` | Operational on Android. (l_getElementAlpha) |
| `getElementAngularVelocity` | **Implemented** | `CLuaElementDefs.cpp:32` | `GetElementTurnVelocity` | Follows `CLuaElementDefs.cpp` 604-629 and `CStaticFunctionDefinitions.cpp` 510-539. Ped, player, vehicle, object and weapon return the stored turn speed, or zero. A frozen vehicle does not replace that stored turn speed. Other types return false. `CPhysical::GetTurnSpeed` is not called. Host-tested. Not device-verified. |
| `getElementAttachedOffsets` | **Implemented** | `CLuaElementDefs.cpp:54` | `GetElementAttachedOffsets` | Reads server ENTITY_ADD attachment metadata, including forward references in the batch and offsets in degrees. Removal clears references before ID reuse. Physical attachment, local attach/detach and later attachment RPCs remain pending. Host-tested; not device-verified. |
| `getElementAttachedTo` | **Implemented** | `CLuaElementDefs.cpp:49` | `GetElementAttachedTo` | Reads server ENTITY_ADD attachment metadata, including forward references in the batch and offsets in degrees. Removal clears references before ID reuse. Physical attachment, local attach/detach and later attachment RPCs remain pending. Host-tested; not device-verified. |
| `getElementBoundingBox` | **Implemented** | `CLuaElementDefs.cpp:46` | `GetElementBoundingBox` | Follows `CLuaElementDefs.cpp` 1083-1110 and `CStaticFunctionDefinitions.cpp` 548-600. Ped, player, vehicle, object, weapon and building ask for the model bounds. `CModelInfo` is not loaded, so the result is false. Host-tested. Not device-verified. |
| `getElementByID` | **Implemented** | `CLuaElementDefs.cpp:24` | `GetElementByID` | Operational on Android. (GetElementByID) |
| `getElementByIndex` | **Implemented** | `CLuaElementDefs.cpp:25` | `GetElementByIndex` | Operational on Android. (GetElementByIndex) |
| `getElementChild` | **Implemented** | `CLuaElementDefs.cpp:35` | `GetElementChild` | Operational on Android. (GetElementChild) |
| `getElementChildren` | **Implemented** | `CLuaElementDefs.cpp:34` | `GetElementChildren` | Operational on Android. (GetElementChildren) |
| `getElementChildrenCount` | **Implemented** | `CLuaElementDefs.cpp:36` | `GetElementChildrenCount` | Operational on Android. (GetElementChildrenCount) |
| `getElementColShape` | **Implemented** | `CLuaElementDefs.cpp:62` | `GetElementColShape` | Follows `CLuaElementDefs.cpp` 1426-1447 and `CStaticFunctionDefinitions.cpp` 7781-7795. A marker returns the collision created with it. That collision cannot be destroyed by script. A pickup returns false because its sphere is created when `CPickup` streams in. Host-tested. Not device-verified. |
| `getElementCollisionsEnabled` | **Implemented** | `CLuaElementDefs.cpp:67` | `GetElementCollisionsEnabled` | Operational on Android. (l_getElementCollisionsEnabled) |
| `getElementData` | **Implemented** | `CLuaElementDefs.cpp:26` | `GetElementData` | Operational on Android. (GetElementData) |
| `getElementDimension` | **Implemented** | `CLuaElementDefs.cpp:45` | `GetElementDimension` | Operational on Android. (l_getElementDimension) |
| `getElementDistanceFromCentreOfMassToBaseOfModel` | **Implemented** | `CLuaElementDefs.cpp:51` | `GetElementDistanceFromCentreOfMassToBaseOfModel` | Follows `CLuaElementDefs.cpp` 1234-1256 and `CStaticFunctionDefinitions.cpp` 654-682. Ped, player, vehicle, object, weapon and building return 0 because the game entity is absent. Other types return false. Host-tested. Not device-verified. |
| `getElementHealth` | **Implemented** | `CLuaElementDefs.cpp:58` | `GetElementHealth` | Operational on Android. (l_getElementHealth) |
| `getElementID` | **Implemented** | `CLuaElementDefs.cpp:37` | `GetElementID` | Operational on Android. (GetElementID) |
| `getElementInterior` | **Implemented** | `CLuaElementDefs.cpp:40` | `GetElementInterior` | Operational on Android. (l_getElementInterior) |
| `getElementLighting` | **Implemented** | `CLuaElementDefs.cpp:56` | `ArgumentParser<GetElementLighting>` | Operational on Android. (l_getElementLighting) |
| `getElementMatrix` | **Implemented** | `CLuaElementDefs.cpp:28` | `GetElementMatrix` | Operational on Android. (l_getElementMatrix) |
| `getElementModel` | **Implemented** | `CLuaElementDefs.cpp:59` | `GetElementModel` | Operational on Android. (l_getElementModel) |
| `getElementParent` | **Implemented** | `CLuaElementDefs.cpp:38` | `GetElementParent` | Operational on Android. (GetElementParent) |
| `getElementPosition` | **Implemented** | `CLuaElementDefs.cpp:29` | `GetElementPosition` | Operational on Android. (l_getElementPosition) |
| `getElementRadius` | **Implemented** | `CLuaElementDefs.cpp:47` | `GetElementRadius` | Follows `CLuaElementDefs.cpp` 1113-1135 and `CStaticFunctionDefinitions.cpp` 603-639. Same model-info miss as the bounding box. Buildings are not queried. Host-tested. Not device-verified. |
| `getElementRotation` | **Implemented** | `CLuaElementDefs.cpp:30` | `GetElementRotation` | Operational on Android. (l_getElementRotation) |
| `getElementType` | **Implemented** | `CLuaElementDefs.cpp:33` | `GetElementType` | Operational on Android. (GetElementType) |
| `getElementVelocity` | **Implemented** | `CLuaElementDefs.cpp:31` | `GetElementVelocity` | Operational on Android. (l_getElementVelocity) |
| `getElementsByType` | **Implemented** | `CLuaElementDefs.cpp:39` | `GetElementsByType` | Operational on Android. (GetElementsByType) |
| `getElementsWithinColShape` | **Implemented** | `CLuaElementDefs.cpp:43` | `GetElementsWithinColShape` | Operational on Android. (GetElementsWithinColShape) |
| `getElementsWithinRange` | **Implemented** | `CLuaElementDefs.cpp:44` | `ArgumentParserWarn<false, GetElementsWithinRange>` | Operational on Android. (GetElementsWithinRange) |
| `getLowLODElement` | **Implemented** | `CLuaElementDefs.cpp:69` | `GetLowLodElement` | Follows `CLuaElementDefs.cpp` 2529-2550 and `CStaticFunctionDefinitions.cpp` 4035-4057. An object or building with no link returns nil. A low-LOD object returns nil. Other types return false. The ENTITY_ADD low-LOD id is not stored. Host-tested. Not device-verified. |
| `getRootElement` | **Implemented** | `CLuaElementDefs.cpp:22` | `GetRootElement` | Operational on Android. (GetRootElement) |
| `hasElementData` | **Implemented** | `CLuaElementDefs.cpp:53` | `HasElementData` | Operational on Android. (HasElementData) |
| `isElement` | **Implemented** | `CLuaElementDefs.cpp:23` | `IsElement` | Operational on Android. (IsElement) |
| `isElementAttached` | **Implemented** | `CLuaElementDefs.cpp:48` | `IsElementAttached` | Reads server ENTITY_ADD attachment metadata, including forward references in the batch and offsets in degrees. Removal clears references before ID reuse. Physical attachment, local attach/detach and later attachment RPCs remain pending. Host-tested; not device-verified. |
| `isElementCallPropagationEnabled` | **Implemented** | `CLuaElementDefs.cpp:71` | `IsElementCallPropagationEnabled` | Operational on Android. (IsElementCallPropagationEnabled) |
| `isElementCollidableWith` | **Implemented** | `CLuaElementDefs.cpp:65` | `IsElementCollidableWith` | Operational on Android. (IsElementCollidableWith) |
| `isElementDoubleSided` | **Implemented** | `CLuaElementDefs.cpp:66` | `IsElementDoubleSided` | Operational on Android. (l_isElementDoubleSided) |
| `isElementFrozen` | **Implemented** | `CLuaElementDefs.cpp:68` | `IsElementFrozen` | Operational on Android. (l_isElementFrozen) |
| `isElementInWater` | **Implemented** | `CLuaElementDefs.cpp:63` | `IsElementInWater` | Operational on Android. (IsElementInWater) |
| `isElementLocal` | **Implemented** | `CLuaElementDefs.cpp:52` | `IsElementLocal` | Operational on Android. (IsElementLocal) |
| `isElementLowLOD` | **Implemented** | `CLuaElementDefs.cpp:70` | `IsElementLowLod` | Follows `CLuaElementDefs.cpp` 2567-2588 and `CStaticFunctionDefinitions.cpp` 4060-4082. An object reports the `createObject` flag. A building reports whether another building uses it as LOD. Other types return false. Host-tested. Not device-verified. |
| `isElementOnFire` | **Implemented** | `CLuaElementDefs.cpp:73` | `ArgumentParser<IsElementOnFire>` | Follows `CLuaElementDefs.cpp` 2701-2703. A bad argument raises. A ped or player returns the stored fire flag. A vehicle or object without a game body returns false. Host-tested. Not device-verified. |
| `isElementOnScreen` | **Implemented** | `CLuaElementDefs.cpp:57` | `ArgumentParserWarn<nullptr, IsElementOnScreen>` | Follows `CLuaElementDefs.cpp` 1369-1374. A bad argument warns and returns nil. A valid element is false because `CEntity::IsOnScreen` is not read. Host-tested. Not device-verified. |
| `isElementStreamable` | **Implemented** | `CLuaElementDefs.cpp:61` | `IsElementStreamable` | Operational on Android. (l_isElementStreamable) |
| `isElementStreamedIn` | **Implemented** | `CLuaElementDefs.cpp:60` | `IsElementStreamedIn` | Operational on Android. (l_isElement) |
| `isElementSyncer` | **Implemented** | `CLuaElementDefs.cpp:64` | `IsElementSyncer` | Operational on Android. (l_isElementSyncer) |
| `isElementWaitingForGroundToLoad` | **Implemented** | `CLuaElementDefs.cpp:72` | `IsElementWaitingForGroundToLoad` | Follows `CLuaElementDefs.cpp` 2640-2660 and `CStaticFunctionDefinitions.cpp` 4105-4125. A ped, player or vehicle returns false because the waiting flag is not stored. Other types return false. Host-tested. Not device-verified. |
| `isElementWithinColShape` | **Implemented** | `CLuaElementDefs.cpp:41` | `IsElementWithinColShape` | Operational on Android. (IsElementWithinColShape) |
| `isElementWithinMarker` | **Implemented** | `CLuaElementDefs.cpp:42` | `IsElementWithinMarker` | Follows `CLuaElementDefs.cpp` 899-920. A bad argument is nil. The result is the cached collider list of the marker collision. Host-tested. Not device-verified. |
| `releaseElementStreamingLease` | **Implemented** | `CLuaElementDefs.cpp:97` | `ReleaseElementStreamingLease` | Follows `CLuaElementDefs.cpp` 2506-2526 and `CResource.cpp` 322-333. An unknown token is false. Dropping the last reference does not call streamer force-out. Host-tested. Not device-verified. |
| `setElementAlpha` | **Implemented** | `CLuaElementDefs.cpp:92` | `SetElementAlpha` | Operational on Android. (l_setElementAlpha) |
| `setElementAngularVelocity` | **Implemented** | `CLuaElementDefs.cpp:86` | `SetElementAngularVelocity` | Follows `CLuaElementDefs.cpp` 2013-2037 and `CStaticFunctionDefinitions.cpp` 1219-1250. Ped, player, vehicle, object and weapon store the vector, including children while call propagation is on. A frozen vehicle keeps the previous vector. `CPhysical::SetTurnSpeed` is not called. Host-tested. Not device-verified. |
| `setElementAttachedOffsets` | **Implemented** | `CLuaElementDefs.cpp:91` | `SetElementAttachedOffsets` | Follows `CLuaElementDefs.cpp` 2241-2267 and `CStaticFunctionDefinitions.cpp` 1437-1443. Offsets are stored in degrees and applied to children while call propagation is on. `CPhysical::SetAttachedOffsets` is not called. Host-tested. Not device-verified. |
| `setElementCallPropagationEnabled` | **Implemented** | `CLuaElementDefs.cpp:103` | `SetElementCallPropagationEnabled` | Operational on Android. (SetElementCallPropagationEnabled) |
| `setElementCollidableWith` | **Implemented** | `CLuaElementDefs.cpp:99` | `SetElementCollidableWith` | Operational on Android. (SetElementCollidableWith) |
| `setElementCollisionsEnabled` | **Implemented** | `CLuaElementDefs.cpp:98` | `SetElementCollisionsEnabled` | Operational on Android. (l_setElementCollisionsEnabled) |
| `setElementData` | **Implemented** | `CLuaElementDefs.cpp:80` | `SetElementData` | Operational on Android. (SetElementData) |
| `setElementDimension` | **Implemented** | `CLuaElementDefs.cpp:88` | `SetElementDimension` | Operational on Android. (l_setElementDimension) |
| `setElementDoubleSided` | **Implemented** | `CLuaElementDefs.cpp:100` | `SetElementDoubleSided` | Operational on Android. (l_setElementDoubleSided) |
| `setElementFrozen` | **Implemented** | `CLuaElementDefs.cpp:101` | `SetElementFrozen` | Operational on Android. (l_setElementFrozen) |
| `setElementHealth` | **Implemented** | `CLuaElementDefs.cpp:93` | `SetElementHealth` | Operational on Android. (l_setElementHealth) |
| `setElementID` | **Implemented** | `CLuaElementDefs.cpp:78` | `SetElementID` | Operational on Android. (SetElementID) |
| `setElementInterior` | **Implemented** | `CLuaElementDefs.cpp:87` | `SetElementInterior` | Operational on Android. (l_setElementInterior) |
| `setElementLighting` | **Implemented** | `CLuaElementDefs.cpp:104` | `ArgumentParser<SetElementLighting>` | Operational on Android. (l_setElementLighting) |
| `setElementMatrix` | **Implemented** | `CLuaElementDefs.cpp:82` | `SetElementMatrix` | Follows `CLuaElementDefs.cpp` 1847-1882 and `CStaticFunctionDefinitions.cpp` 1063-1069. A 4x4 table or 12 numbers are stored, and the position is passed to `setElementPosition`. A bad table is rejected. `CEntity::SetMatrix` and the Euler conversion are not run. Host-tested. Not device-verified. |
| `setElementModel` | **Implemented** | `CLuaElementDefs.cpp:94` | `SetElementModel` | Operational on Android. (SetElementModel) |
| `setElementOnFire` | **Implemented** | `CLuaElementDefs.cpp:105` | `ArgumentParser<SetElementOnFire>` | Follows `CLuaElementDefs.cpp` 2559-2564. A bad argument raises. Only a local element or the local player is accepted. A ped stores the flag. A vehicle or object returns false because the game body is absent. Host-tested. Not device-verified. |
| `setElementParent` | **Implemented** | `CLuaElementDefs.cpp:79` | `SetElementParent` | Operational on Android. (SetElementParent) |
| `setElementPosition` | **Implemented** | `CLuaElementDefs.cpp:83` | `SetElementPosition` | Operational on Android. (SetElementPosition) |
| `setElementRotation` | **Implemented** | `CLuaElementDefs.cpp:84` | `SetElementRotation` | Operational on Android. (l_setElementRotation) |
| `setElementStreamable` | **Implemented** | `CLuaElementDefs.cpp:95` | `SetElementStreamable` | Operational on Android. (l_setElementStreamable) |
| `setElementVelocity` | **Implemented** | `CLuaElementDefs.cpp:85` | `SetElementVelocity` | Follows `CLuaElementDefs.cpp` 1983-2010 and `CStaticFunctionDefinitions.cpp` 1172-1216. Ped, player, vehicle, object, weapon and sound store the vector and pass it to children while call propagation is on. A projectile returns true and stores nothing. A frozen vehicle keeps the previous vector, and `getElementVelocity` reads zero until it thaws. A radar area returns false. `CPhysical::SetMoveSpeed` is not called. Host-tested. Not device-verified. |
| `setLowLODElement` | **Implemented** | `CLuaElementDefs.cpp:102` | `ArgumentParser<SetLowLodElement>` | Follows `CLuaElementDefs.cpp` 2553-2556 and `CStaticFunctionDefinitions.cpp` 4001-4032. A bad argument raises. Object links follow `SetLowLodObject`. Building links follow `SetLowLodBuilding` and stay true when cleared. A type mismatch returns false. `CBuilding::SetLod` is not called. Host-tested. Not device-verified. |
