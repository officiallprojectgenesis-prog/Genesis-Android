#pragma once

// Client window state calls of desktop CLuaGUIDefs.cpp: chat input, console, debug view,
// main menu, transfer box, chatbox character limit and chatbox layout.

struct lua_State;

namespace WorldRPG::Script {

// Installs isChatBoxInputActive, isConsoleActive, isDebugViewActive, setDebugViewActive,
// isMainMenuActive, isMTAWindowActive, isTransferBoxActive, setChatboxCharacterLimit,
// getChatboxCharacterLimit and getChatboxLayout. Call after the default stubs.
void InstallMtaWindowLua(lua_State* L);

// The Activity reports the chat input bar here (UI thread). Desktop CLocalGUI::IsChatBoxInputEnabled.
void MtaWindowSetChatInputActive(bool active);
bool MtaWindowChatInputActive();

// Desktop CCore::OnModUnload (CCore.cpp:2964-2965): a new session starts with the default limit.
void MtaWindowResetSession();

}  // namespace WorldRPG::Script
