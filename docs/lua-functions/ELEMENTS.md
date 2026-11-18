# Element Tree & Data (CLuaElementDefs)

Core element hierarchy (parent/child), element data (synced/local), collisions, dimensions, alpha, positions, velocities, matrix, and spatial streaming.

## Parity Status

- **Total Functions:** 81
- **Implemented:** 60 (74.1%)
- **Stubs:** 7
- **Missing:** 14

> **Technical Requirement:** Implemented via `LuaElementApi.cpp` and `ServerWorld.cpp`. Attachment queries read ENTITY_ADD metadata, resolve references after the batch and clear references on removal. Physical attachment, attachment RPCs, spatial octree lookup and complete element matrix math remain pending.

## Function List

| Lua Function | Status | Desktop Source Ref | Desktop Handler | Conversion Requirement / Implementation Notes |
|---|---|---|---|---|
| `acquireElementStreamingLease` | Missing | `CLuaElementDefs.cpp:96` | `AcquireElementStreamingLease` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `attachElements` | *Stub* | `CLuaElementDefs.cpp:89` | `AttachElements` | Returns false until native physical attachment and its lifecycle/events are ported. Server attachment queries are metadata-only; these calls do not mutate that snapshot. |
| `createElement` | **Implemented** | `CLuaElementDefs.cpp:76` | `CreateElement` | Operational on Android. (CreateElement) |
| `destroyElement` | **Implemented** | `CLuaElementDefs.cpp:77` | `DestroyElement` | Operational on Android. (DestroyElementDispatch) |
| `detachElements` | *Stub* | `CLuaElementDefs.cpp:90` | `DetachElements` | Returns false until native physical attachment and its lifecycle/events are ported. Server attachment queries are metadata-only; these calls do not mutate that snapshot. |
| `getAllElementData` | **Implemented** | `CLuaElementDefs.cpp:27` | `ArgumentParserWarn<false, GetAllElementData>` | Operational on Android. (GetAllElementData) |
| `getAttachedElements` | **Implemented** | `CLuaElementDefs.cpp:50` | `GetAttachedElements` | Reads server ENTITY_ADD attachment metadata, including forward references in the batch and offsets in degrees. Removal clears references before ID reuse. Physical attachment, local attach/detach and later attachment RPCs remain pending. Host-tested; not device-verified. |
| `getElementAlpha` | **Implemented** | `CLuaElementDefs.cpp:55` | `GetElementAlpha` | Operational on Android. (l_getElementAlpha) |
| `getElementAngularVelocity` | *Stub* | `CLuaElementDefs.cpp:32` | `GetElementTurnVelocity` | Currently stubbed: `DefaultValue(kZero3)`. Needs native engine wiring. |
| `getElementAttachedOffsets` | **Implemented** | `CLuaElementDefs.cpp:54` | `GetElementAttachedOffsets` | Reads server ENTITY_ADD attachment metadata, including forward references in the batch and offsets in degrees. Removal clears references before ID reuse. Physical attachment, local attach/detach and later attachment RPCs remain pending. Host-tested; not device-verified. |
| `getElementAttachedTo` | **Implemented** | `CLuaElementDefs.cpp:49` | `GetElementAttachedTo` | Reads server ENTITY_ADD attachment metadata, including forward references in the batch and offsets in degrees. Removal clears references before ID reuse. Physical attachment, local attach/detach and later attachment RPCs remain pending. Host-tested; not device-verified. |
| `getElementBoundingBox` | Missing | `CLuaElementDefs.cpp:46` | `GetElementBoundingBox` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getElementByID` | **Implemented** | `CLuaElementDefs.cpp:24` | `GetElementByID` | Operational on Android. (GetElementByID) |
| `getElementByIndex` | **Implemented** | `CLuaElementDefs.cpp:25` | `GetElementByIndex` | Operational on Android. (GetElementByIndex) |
| `getElementChild` | **Implemented** | `CLuaElementDefs.cpp:35` | `GetElementChild` | Operational on Android. (GetElementChild) |
| `getElementChildren` | **Implemented** | `CLuaElementDefs.cpp:34` | `GetElementChildren` | Operational on Android. (GetElementChildren) |
| `getElementChildrenCount` | **Implemented** | `CLuaElementDefs.cpp:36` | `GetElementChildrenCount` | Operational on Android. (GetElementChildrenCount) |
| `getElementColShape` | Missing | `CLuaElementDefs.cpp:62` | `GetElementColShape` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getElementCollisionsEnabled` | **Implemented** | `CLuaElementDefs.cpp:67` | `GetElementCollisionsEnabled` | Operational on Android. (l_getElementCollisionsEnabled) |
| `getElementData` | **Implemented** | `CLuaElementDefs.cpp:26` | `GetElementData` | Operational on Android. (GetElementData) |
| `getElementDimension` | **Implemented** | `CLuaElementDefs.cpp:45` | `GetElementDimension` | Operational on Android. (l_getElementDimension) |
| `getElementDistanceFromCentreOfMassToBaseOfModel` | *Stub* | `CLuaElementDefs.cpp:51` | `GetElementDistanceFromCentreOfMassToBaseOfModel` | Currently stubbed: `DefaultValue(kZero)`. Needs native engine wiring. |
| `getElementHealth` | **Implemented** | `CLuaElementDefs.cpp:58` | `GetElementHealth` | Operational on Android. (l_getElementHealth) |
| `getElementID` | **Implemented** | `CLuaElementDefs.cpp:37` | `GetElementID` | Operational on Android. (GetElementID) |
| `getElementInterior` | **Implemented** | `CLuaElementDefs.cpp:40` | `GetElementInterior` | Operational on Android. (l_getElementInterior) |
| `getElementLighting` | **Implemented** | `CLuaElementDefs.cpp:56` | `ArgumentParser<GetElementLighting>` | Operational on Android. (l_getElementLighting) |
| `getElementMatrix` | **Implemented** | `CLuaElementDefs.cpp:28` | `GetElementMatrix` | Operational on Android. (l_getElementMatrix) |
| `getElementModel` | **Implemented** | `CLuaElementDefs.cpp:59` | `GetElementModel` | Operational on Android. (l_getElementModel) |
| `getElementParent` | **Implemented** | `CLuaElementDefs.cpp:38` | `GetElementParent` | Operational on Android. (GetElementParent) |
| `getElementPosition` | **Implemented** | `CLuaElementDefs.cpp:29` | `GetElementPosition` | Operational on Android. (l_getElementPosition) |
| `getElementRadius` | Missing | `CLuaElementDefs.cpp:47` | `GetElementRadius` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getElementRotation` | **Implemented** | `CLuaElementDefs.cpp:30` | `GetElementRotation` | Operational on Android. (l_getElementRotation) |
| `getElementType` | **Implemented** | `CLuaElementDefs.cpp:33` | `GetElementType` | Operational on Android. (GetElementType) |
| `getElementVelocity` | **Implemented** | `CLuaElementDefs.cpp:31` | `GetElementVelocity` | Operational on Android. (l_getElementVelocity) |
| `getElementsByType` | **Implemented** | `CLuaElementDefs.cpp:39` | `GetElementsByType` | Operational on Android. (GetElementsByType) |
| `getElementsWithinColShape` | **Implemented** | `CLuaElementDefs.cpp:43` | `GetElementsWithinColShape` | Operational on Android. (GetElementsWithinColShape) |
| `getElementsWithinRange` | **Implemented** | `CLuaElementDefs.cpp:44` | `ArgumentParserWarn<false, GetElementsWithinRange>` | Operational on Android. (GetElementsWithinRange) |
| `getLowLODElement` | Missing | `CLuaElementDefs.cpp:69` | `GetLowLodElement` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
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
| `isElementLowLOD` | Missing | `CLuaElementDefs.cpp:70` | `IsElementLowLod` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isElementOnFire` | Missing | `CLuaElementDefs.cpp:73` | `ArgumentParser<IsElementOnFire>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isElementOnScreen` | *Stub* | `CLuaElementDefs.cpp:57` | `ArgumentParserWarn<nullptr, IsElementOnScreen>` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `isElementStreamable` | **Implemented** | `CLuaElementDefs.cpp:61` | `IsElementStreamable` | Operational on Android. (l_isElementStreamable) |
| `isElementStreamedIn` | **Implemented** | `CLuaElementDefs.cpp:60` | `IsElementStreamedIn` | Operational on Android. (l_isElement) |
| `isElementSyncer` | **Implemented** | `CLuaElementDefs.cpp:64` | `IsElementSyncer` | Operational on Android. (l_isElementSyncer) |
| `isElementWaitingForGroundToLoad` | Missing | `CLuaElementDefs.cpp:72` | `IsElementWaitingForGroundToLoad` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isElementWithinColShape` | **Implemented** | `CLuaElementDefs.cpp:41` | `IsElementWithinColShape` | Operational on Android. (IsElementWithinColShape) |
| `isElementWithinMarker` | Missing | `CLuaElementDefs.cpp:42` | `IsElementWithinMarker` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `releaseElementStreamingLease` | Missing | `CLuaElementDefs.cpp:97` | `ReleaseElementStreamingLease` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setElementAlpha` | **Implemented** | `CLuaElementDefs.cpp:92` | `SetElementAlpha` | Operational on Android. (l_setElementAlpha) |
| `setElementAngularVelocity` | *Stub* | `CLuaElementDefs.cpp:86` | `SetElementAngularVelocity` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setElementAttachedOffsets` | Missing | `CLuaElementDefs.cpp:91` | `SetElementAttachedOffsets` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
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
| `setElementMatrix` | Missing | `CLuaElementDefs.cpp:82` | `SetElementMatrix` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setElementModel` | **Implemented** | `CLuaElementDefs.cpp:94` | `SetElementModel` | Operational on Android. (SetElementModel) |
| `setElementOnFire` | Missing | `CLuaElementDefs.cpp:105` | `ArgumentParser<SetElementOnFire>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setElementParent` | **Implemented** | `CLuaElementDefs.cpp:79` | `SetElementParent` | Operational on Android. (SetElementParent) |
| `setElementPosition` | **Implemented** | `CLuaElementDefs.cpp:83` | `SetElementPosition` | Operational on Android. (SetElementPosition) |
| `setElementRotation` | **Implemented** | `CLuaElementDefs.cpp:84` | `SetElementRotation` | Operational on Android. (l_setElementRotation) |
| `setElementStreamable` | **Implemented** | `CLuaElementDefs.cpp:95` | `SetElementStreamable` | Operational on Android. (l_setElementStreamable) |
| `setElementVelocity` | *Stub* | `CLuaElementDefs.cpp:85` | `SetElementVelocity` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setLowLODElement` | Missing | `CLuaElementDefs.cpp:102` | `ArgumentParser<SetLowLodElement>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
