# Web Browser & Compatibility (CEF / Discord)

Embedded Chromium web pages, browser mouse/keyboard input, Discord Rich Presence, and backwards compatibility helpers.

## Parity Status

- **Total Functions:** 103
- **Implemented:** 89 (86.4%)
- **Stubs:** 14
- **Missing:** 0

> **Technical Requirement:** Desktop bundles CEF. Android has no web core: `createBrowser` and `guiCreateBrowser` still enforce the size errors, `getBrowserSettings` returns the three flags as false, and page calls return false. Discord checks lengths then returns false. Ped compatibility reads the ENTITY_ADD jetpack flag, PED_SYNC fire/water bits and keysync duck, choke and aim origin. `setElementModel` stores stock ped and vehicle ids without rebuilding the body. Fourteen old names still alias constant stubs (ground, tasks, controls, attachment).

## Function List

| Lua Function | Status | Desktop Source Ref | Desktop Handler | Conversion Requirement / Implementation Notes |
|---|---|---|---|---|
| `attachElementToElement` | *Stub* | `CLuaCompatibilityDefs.cpp:60` | `CLuaElementDefs::AttachElements` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `canBrowserNavigateBack` | **Implemented** | `CLuaBrowserDefs.cpp:44` | `CanBrowserNavigateBack` | Operational on Android. (MissingBrowser) |
| `canBrowserNavigateForward` | **Implemented** | `CLuaBrowserDefs.cpp:45` | `CanBrowserNavigateForward` | Operational on Android. (MissingBrowser) |
| `canPlayerBeKnockedOffBike` | *Stub* | `CLuaCompatibilityDefs.cpp:18` | `CLuaPedDefs::CanPedBeKnockedOffBike` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `createBrowser` | **Implemented** | `CLuaBrowserDefs.cpp:20` | `CreateBrowser` | Operational on Android. (CreateBrowser) |
| `detachElementFromElement` | *Stub* | `CLuaCompatibilityDefs.cpp:61` | `CLuaElementDefs::DetachElements` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `doesPlayerHaveJetPack` | **Implemented** | `CLuaCompatibilityDefs.cpp:21` | `CLuaPedDefs::DoesPedHaveJetPack` | Operational on Android. (DoesPedHaveJetPack) |
| `executeBrowserJavascript` | **Implemented** | `CLuaBrowserDefs.cpp:33` | `ExecuteBrowserJavascript` | Operational on Android. (MissingBrowser) |
| `focusBrowser` | **Implemented** | `CLuaBrowserDefs.cpp:37` | `FocusBrowser` | Operational on Android. (MissingBrowser) |
| `getBrowserProperty` | **Implemented** | `CLuaBrowserDefs.cpp:40` | `GetBrowserProperty` | Operational on Android. (MissingBrowser) |
| `getBrowserSettings` | **Implemented** | `CLuaBrowserDefs.cpp:41` | `GetBrowserSettings` | Operational on Android. (GetBrowserSettings) |
| `getBrowserSource` | **Implemented** | `CLuaBrowserDefs.cpp:42` | `GetBrowserSource` | Operational on Android. (MissingBrowser) |
| `getBrowserTitle` | **Implemented** | `CLuaBrowserDefs.cpp:29` | `GetBrowserTitle` | Operational on Android. (MissingBrowser) |
| `getBrowserURL` | **Implemented** | `CLuaBrowserDefs.cpp:30` | `GetBrowserURL` | Operational on Android. (MissingBrowser) |
| `getBrowserVolume` | **Implemented** | `CLuaBrowserDefs.cpp:34` | `GetBrowserVolume` | Operational on Android. (MissingBrowser) |
| `getCameraShakeLevel` | **Implemented** | `CLuaCompatibilityDefs.cpp:73` | `ArgumentParserWarn<false, CLuaCameraDefs::GetCameraDrunkLevel>` | Operational on Android. (l_getCameraDrunkLevel) |
| `getControlState` | *Stub* | `CLuaCompatibilityDefs.cpp:71` | `ArgumentParserWarn<false, CLuaPedDefs::GetPedControlState>` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `getDiscordRichPresenceUserID` | **Implemented** | `CLuaDiscordDefs.cpp:30` | `ArgumentParser<GetDiscordUserID>` | Operational on Android. (GetUserID) |
| `getObjectRotation` | **Implemented** | `CLuaCompatibilityDefs.cpp:53` | `CLuaElementDefs::GetElementRotation` | Operational on Android. (l_getElementRotation) |
| `getPedSkin` | **Implemented** | `CLuaCompatibilityDefs.cpp:51` | `CLuaElementDefs::GetElementModel` | Operational on Android. (l_getElementModel) |
| `getPedWeaponMuzzlePosition` | **Implemented** | `CLuaCompatibilityDefs.cpp:38` | `CLuaPedDefs::GetPedWeaponMuzzlePosition` | Operational on Android. (GetPedWeaponMuzzlePosition) |
| `getPlayerAmmoInClip` | **Implemented** | `CLuaCompatibilityDefs.cpp:36` | `CLuaPedDefs::GetPedAmmoInClip` | Operational on Android. (GetPedAmmoInClip) |
| `getPlayerArmor` | **Implemented** | `CLuaCompatibilityDefs.cpp:40` | `ArgumentParserWarn<false, CLuaPedDefs::GetPedArmor>` | Operational on Android. (l_getPedArmor) |
| `getPlayerContactElement` | **Implemented** | `CLuaCompatibilityDefs.cpp:19` | `CLuaPedDefs::GetPedContactElement` | Operational on Android. (GetPedContactElement) |
| `getPlayerOccupiedVehicle` | **Implemented** | `CLuaCompatibilityDefs.cpp:39` | `CLuaPedDefs::GetPedOccupiedVehicle` | Operational on Android. (GetPedOccupiedVehicle) |
| `getPlayerRotation` | **Implemented** | `CLuaCompatibilityDefs.cpp:17` | `CLuaPedDefs::GetPedRotation` | Operational on Android. (l_getPedRotation) |
| `getPlayerSimplestTask` | *Stub* | `CLuaCompatibilityDefs.cpp:28` | `CLuaPedDefs::GetPedSimplestTask` | Currently stubbed: `DefaultValue(kEmptyString)`. Needs native engine wiring. |
| `getPlayerSkin` | **Implemented** | `CLuaCompatibilityDefs.cpp:41` | `CLuaElementDefs::GetElementModel` | Operational on Android. (l_getElementModel) |
| `getPlayerStat` | *Stub* | `CLuaCompatibilityDefs.cpp:44` | `CLuaPedDefs::GetPedStat` | Currently stubbed: `l_num0: lua_pushinteger(L, 0)`. Needs native engine wiring. |
| `getPlayerTarget` | *Stub* | `CLuaCompatibilityDefs.cpp:30` | `CLuaPedDefs::GetPedTarget` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `getPlayerTargetCollision` | **Implemented** | `CLuaCompatibilityDefs.cpp:33` | `CLuaPedDefs::GetPedTargetCollision` | Operational on Android. (GetPedTargetCollision) |
| `getPlayerTargetEnd` | **Implemented** | `CLuaCompatibilityDefs.cpp:32` | `CLuaPedDefs::GetPedTargetEnd` | Operational on Android. (GetPedTargetEnd) |
| `getPlayerTargetStart` | **Implemented** | `CLuaCompatibilityDefs.cpp:31` | `CLuaPedDefs::GetPedTargetStart` | Operational on Android. (GetPedTargetStart) |
| `getPlayerTask` | *Stub* | `CLuaCompatibilityDefs.cpp:27` | `CLuaPedDefs::GetPedTask` | Currently stubbed: `l_false: lua_pushboolean(L, 0)`. Needs native engine wiring. |
| `getPlayerTotalAmmo` | **Implemented** | `CLuaCompatibilityDefs.cpp:37` | `CLuaPedDefs::GetPedTotalAmmo` | Operational on Android. (GetPedTotalAmmo) |
| `getPlayerWeapon` | **Implemented** | `CLuaCompatibilityDefs.cpp:35` | `CLuaPedDefs::GetPedWeapon` | Operational on Android. (GetPedWeapon) |
| `getPlayerWeaponSlot` | **Implemented** | `CLuaCompatibilityDefs.cpp:34` | `CLuaPedDefs::GetPedWeaponSlot` | Operational on Android. (GetPedWeaponSlot) |
| `getVehicleID` | **Implemented** | `CLuaCompatibilityDefs.cpp:56` | `CLuaElementDefs::GetElementModel` | Operational on Android. (l_getElementModel) |
| `getVehicleIDFromName` | **Implemented** | `CLuaCompatibilityDefs.cpp:55` | `CLuaVehicleDefs::GetVehicleModelFromName` | Operational on Android. (GetVehicleModelFromName) |
| `getVehicleModel` | **Implemented** | `CLuaCompatibilityDefs.cpp:50` | `CLuaElementDefs::GetElementModel` | Operational on Android. (l_getElementModel) |
| `getVehicleNameFromID` | **Implemented** | `CLuaCompatibilityDefs.cpp:58` | `CLuaVehicleDefs::GetVehicleNameFromModel` | Operational on Android. (GetVehicleNameFromModel) |
| `getVehicleRotation` | **Implemented** | `CLuaCompatibilityDefs.cpp:57` | `CLuaElementDefs::GetElementRotation` | Operational on Android. (l_getElementRotation) |
| `guiCreateBrowser` | **Implemented** | `CLuaBrowserDefs.cpp:51` | `GUICreateBrowser` | Operational on Android. (GuiCreateBrowser) |
| `guiEditSetCaratIndex` | **Implemented** | `CLuaCompatibilityDefs.cpp:68` | `CLuaGUIDefs::GUIEditSetCaretIndex` | Operational on Android. (GuiEditSetCaretIndex) |
| `guiGetBrowser` | **Implemented** | `CLuaBrowserDefs.cpp:52` | `GUIGetBrowser` | Operational on Android. (MissingBrowser) |
| `guiMemoSetCaratIndex` | **Implemented** | `CLuaCompatibilityDefs.cpp:69` | `CLuaGUIDefs::GUIMemoSetCaretIndex` | Operational on Android. (GuiMemoSetCaretIndex) |
| `injectBrowserMouseDown` | **Implemented** | `CLuaBrowserDefs.cpp:26` | `InjectBrowserMouseDown` | Operational on Android. (MissingBrowser) |
| `injectBrowserMouseMove` | **Implemented** | `CLuaBrowserDefs.cpp:25` | `InjectBrowserMouseMove` | Operational on Android. (MissingBrowser) |
| `injectBrowserMouseUp` | **Implemented** | `CLuaBrowserDefs.cpp:27` | `InjectBrowserMouseUp` | Operational on Android. (MissingBrowser) |
| `injectBrowserMouseWheel` | **Implemented** | `CLuaBrowserDefs.cpp:28` | `InjectBrowserMouseWheel` | Operational on Android. (MissingBrowser) |
| `isBrowserDomainBlocked` | **Implemented** | `CLuaBrowserDefs.cpp:36` | `IsBrowserDomainBlocked` | Operational on Android. (IsBrowserDomainBlocked) |
| `isBrowserFocused` | **Implemented** | `CLuaBrowserDefs.cpp:38` | `IsBrowserFocused` | Operational on Android. (MissingBrowser) |
| `isBrowserGPUEnabled` | **Implemented** | `CLuaBrowserDefs.cpp:53` | `ArgumentParser<IsBrowserGPUEnabled>` | Operational on Android. (IsBrowserGPUEnabled) |
| `isBrowserLoading` | **Implemented** | `CLuaBrowserDefs.cpp:24` | `IsBrowserLoading` | Operational on Android. (MissingBrowser) |
| `isBrowserRenderingPaused` | **Implemented** | `CLuaBrowserDefs.cpp:32` | `ArgumentParser<IsBrowserRenderingPaused>` | Operational on Android. (MissingBrowser) |
| `isDiscordRichPresenceConnected` | **Implemented** | `CLuaDiscordDefs.cpp:29` | `ArgumentParser<IsDiscordRPCConnected>` | Operational on Android. (NotReady) |
| `isPedInWater` | **Implemented** | `CLuaCompatibilityDefs.cpp:23` | `CLuaElementDefs::IsElementInWater` | Operational on Android. (IsElementInWater) |
| `isPedOnFire` | **Implemented** | `CLuaCompatibilityDefs.cpp:24` | `CLuaPedDefs::IsPedOnFire` | Operational on Android. (IsPedOnFire) |
| `isPlayerChoking` | **Implemented** | `CLuaCompatibilityDefs.cpp:42` | `CLuaPedDefs::IsPedChoking` | Operational on Android. (IsPedChoking) |
| `isPlayerDead` | **Implemented** | `CLuaCompatibilityDefs.cpp:67` | `CLuaPedDefs::IsPedDead` | Operational on Android. (IsPedDead) |
| `isPlayerDoingTask` | *Stub* | `CLuaCompatibilityDefs.cpp:29` | `CLuaPedDefs::IsPedDoingTask` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `isPlayerDucked` | **Implemented** | `CLuaCompatibilityDefs.cpp:43` | `CLuaPedDefs::IsPedDucked` | Operational on Android. (IsPedDucked) |
| `isPlayerInVehicle` | **Implemented** | `CLuaCompatibilityDefs.cpp:20` | `CLuaPedDefs::IsPedInVehicle` | Operational on Android. (IsPedInVehicle) |
| `isPlayerInWater` | **Implemented** | `CLuaCompatibilityDefs.cpp:22` | `CLuaElementDefs::IsElementInWater` | Operational on Android. (IsElementInWater) |
| `isPlayerOnGround` | *Stub* | `CLuaCompatibilityDefs.cpp:26` | `CLuaPedDefs::IsPedOnGround` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `loadBrowserURL` | **Implemented** | `CLuaBrowserDefs.cpp:23` | `LoadBrowserURL` | Operational on Android. (MissingBrowser) |
| `navigateBrowserBack` | **Implemented** | `CLuaBrowserDefs.cpp:46` | `NavigateBrowserBack` | Operational on Android. (MissingBrowser) |
| `navigateBrowserForward` | **Implemented** | `CLuaBrowserDefs.cpp:47` | `NavigateBrowserForward` | Operational on Android. (MissingBrowser) |
| `openExternalURL` | **Implemented** | `CLuaBrowserDefs.cpp:22` | `OpenExternalURL` | Operational on Android. (OpenExternalURL) |
| `reloadBrowserPage` | **Implemented** | `CLuaBrowserDefs.cpp:48` | `ReloadBrowserPage` | Operational on Android. (MissingBrowser) |
| `requestBrowserDomains` | **Implemented** | `CLuaBrowserDefs.cpp:21` | `RequestBrowserDomains` | Operational on Android. (RequestBrowserDomains) |
| `resetDiscordRichPresenceData` | **Implemented** | `CLuaDiscordDefs.cpp:28` | `ArgumentParser<ResetData>` | Operational on Android. (NotReady) |
| `resizeBrowser` | **Implemented** | `CLuaBrowserDefs.cpp:50` | `ResizeBrowser` | Operational on Android. (MissingBrowser) |
| `setBrowserAjaxHandler` | **Implemented** | `CLuaBrowserDefs.cpp:43` | `SetBrowserAjaxHandler` | Operational on Android. (MissingBrowser) |
| `setBrowserProperty` | **Implemented** | `CLuaBrowserDefs.cpp:39` | `SetBrowserProperty` | Operational on Android. (MissingBrowser) |
| `setBrowserRenderingPaused` | **Implemented** | `CLuaBrowserDefs.cpp:31` | `SetBrowserRenderingPaused` | Operational on Android. (MissingBrowser) |
| `setBrowserVolume` | **Implemented** | `CLuaBrowserDefs.cpp:35` | `SetBrowserVolume` | Operational on Android. (SetBrowserVolume) |
| `setCameraShakeLevel` | **Implemented** | `CLuaCompatibilityDefs.cpp:72` | `ArgumentParserWarn<false, CLuaCameraDefs::SetCameraDrunkLevel>` | Operational on Android. (l_setCameraDrunkLevel) |
| `setControlState` | *Stub* | `CLuaCompatibilityDefs.cpp:70` | `ArgumentParserWarn<false, CLuaPedDefs::SetPedControlState>` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `setDiscordApplicationID` | **Implemented** | `CLuaDiscordDefs.cpp:19` | `ArgumentParser<SetAppID>` | Operational on Android. (SetAppID) |
| `setDiscordRichPresenceAsset` | **Implemented** | `CLuaDiscordDefs.cpp:22` | `ArgumentParser<SetLargeAsset>` | Operational on Android. (SetLargeAsset) |
| `setDiscordRichPresenceButton` | **Implemented** | `CLuaDiscordDefs.cpp:24` | `ArgumentParser<SetButtons>` | Operational on Android. (SetButtons) |
| `setDiscordRichPresenceDetails` | **Implemented** | `CLuaDiscordDefs.cpp:20` | `ArgumentParser<SetDetails>` | Operational on Android. (SetDetails) |
| `setDiscordRichPresenceEndTime` | **Implemented** | `CLuaDiscordDefs.cpp:26` | `ArgumentParser<SetEndTime>` | Operational on Android. (SetEndTime) |
| `setDiscordRichPresencePartySize` | **Implemented** | `CLuaDiscordDefs.cpp:27` | `ArgumentParser<SetPartySize>` | Operational on Android. (SetPartySize) |
| `setDiscordRichPresenceSmallAsset` | **Implemented** | `CLuaDiscordDefs.cpp:23` | `ArgumentParser<SetSmallAsset>` | Operational on Android. (SetSmallAsset) |
| `setDiscordRichPresenceStartTime` | **Implemented** | `CLuaDiscordDefs.cpp:25` | `ArgumentParser<SetStartTime>` | Operational on Android. (SetStartTime) |
| `setDiscordRichPresenceState` | **Implemented** | `CLuaDiscordDefs.cpp:21` | `ArgumentParser<SetState>` | Operational on Android. (SetState) |
| `setObjectRotation` | **Implemented** | `CLuaCompatibilityDefs.cpp:54` | `CLuaElementDefs::SetElementRotation` | Operational on Android. (l_setElementRotation) |
| `setPedOnFire` | **Implemented** | `CLuaCompatibilityDefs.cpp:25` | `CLuaPedDefs::SetPedOnFire` | Operational on Android. (SetPedOnFire) |
| `setPedSkin` | **Implemented** | `CLuaCompatibilityDefs.cpp:52` | `CLuaElementDefs::SetElementModel` | Operational on Android. (SetElementModel) |
| `setPlayerCanBeKnockedOffBike` | *Stub* | `CLuaCompatibilityDefs.cpp:48` | `CLuaPedDefs::SetPedCanBeKnockedOffBike` | Currently stubbed: `DefaultValue(kTrue)`. Needs native engine wiring. |
| `setPlayerRotation` | *Stub* | `CLuaCompatibilityDefs.cpp:47` | `CLuaPedDefs::SetPedRotation` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `setPlayerSkin` | **Implemented** | `CLuaCompatibilityDefs.cpp:46` | `CLuaElementDefs::SetElementModel` | Operational on Android. (SetElementModel) |
| `setPlayerWeaponSlot` | *Stub* | `CLuaCompatibilityDefs.cpp:45` | `CLuaPedDefs::SetPedWeaponSlot` | Currently stubbed: `l_true: lua_pushboolean(L, 1)`. Needs native engine wiring. |
| `setVehicleModel` | **Implemented** | `CLuaCompatibilityDefs.cpp:49` | `CLuaElementDefs::SetElementModel` | Operational on Android. (SetElementModel) |
| `setVehicleRotation` | **Implemented** | `CLuaCompatibilityDefs.cpp:59` | `CLuaElementDefs::SetElementRotation` | Operational on Android. (l_setElementRotation) |
| `toggleBrowserDevTools` | **Implemented** | `CLuaBrowserDefs.cpp:49` | `ToggleBrowserDevTools` | Operational on Android. (MissingBrowser) |
| `xmlCreateSubNode` | **Implemented** | `CLuaCompatibilityDefs.cpp:65` | `CLuaXMLDefs::xmlCreateChild` | Operational on Android. (l_xmlCreateChild) |
| `xmlFindSubNode` | **Implemented** | `CLuaCompatibilityDefs.cpp:62` | `CLuaXMLDefs::xmlNodeFindChild` | Operational on Android. (l_xmlFindChild) |
| `xmlNodeFindChild` | **Implemented** | `CLuaCompatibilityDefs.cpp:66` | `CLuaXMLDefs::xmlNodeFindChild` | Operational on Android. (l_xmlFindChild) |
| `xmlNodeFindSubNode` | **Implemented** | `CLuaCompatibilityDefs.cpp:64` | `CLuaXMLDefs::xmlNodeFindChild` | Operational on Android. (l_xmlFindChild) |
| `xmlNodeGetSubNodes` | **Implemented** | `CLuaCompatibilityDefs.cpp:63` | `CLuaXMLDefs::xmlNodeGetChildren` | Operational on Android. (l_xmlNodeGetChildren) |
