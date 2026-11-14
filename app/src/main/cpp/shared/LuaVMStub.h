#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#ifdef __ANDROID__
#include <android/log.h>
#define LUA_LOG_TAG "WorldRPGNeon"
#define LUA_LOGI(...) __android_log_print(ANDROID_LOG_INFO, LUA_LOG_TAG, __VA_ARGS__)
#define LUA_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LUA_LOG_TAG, __VA_ARGS__)
#else
#include <cstdio>
#define LUA_LOGI(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#define LUA_LOGE(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while(0)
#endif

namespace WorldRPG::Net {
struct ResourceStartInfo;
}

namespace WorldRPG::Script {

/// Lua script execution result
struct LuaExecResult {
    bool success = false;
    std::string errorMessage;
    int returnCode = 0;
};

/// Callback for Lua print/output redirection
using LuaOutputCallback = std::function<void(const char* message)>;

/// Stub Lua VM interface for server-side resource script execution.
/// Provides the API surface that will be backed by an actual Lua 5.1/5.3
/// interpreter in a future phase. Currently logs all calls for protocol
/// validation and integration testing without executing scripts.
class LuaVMStub {
public:
    LuaVMStub();
    ~LuaVMStub();

    LuaVMStub(const LuaVMStub&) = delete;
    LuaVMStub& operator=(const LuaVMStub&) = delete;

    /// Initialize the Lua VM (stub: logs initialization)
    bool initialize();

    /// Check if VM is initialized
    bool isReady() const;

    /// Load and execute a Lua script from memory buffer
    LuaExecResult executeBuffer(const char* code, size_t length, const char* chunkName = "script");

    /// Load and execute a Lua script from file path
    LuaExecResult executeFile(const char* filePath);

    /// Register a C function callable from Lua scripts
    void registerFunction(const char* name, std::function<int()> handler);

    /// Set output callback for Lua print() redirection
    void setOutputCallback(LuaOutputCallback callback);

    /// Get list of registered MTA client-side functions (stub)
    std::vector<std::string> getRegisteredFunctions() const;

    /// Reset the VM state (clear globals, loaded scripts)
    void reset();

    /// Shutdown and release resources
    void shutdown();

    /// Physical pixels the client scripts see from guiGetScreenSize.
    void setScreenSize(int width, int height);

    /// Delivers a CHAT_ECHO into onClientChatMessage when a resource
    /// registered one. Returns false when the Java chat should show it.
    bool onChatMessage(const char* text, int red, int green, int blue);

    /// Runs timers and onClientRender. The result is one frame of
    /// "R x y w h argb" and "T x y size argb text" commands.
    std::string pumpDx();

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;
};

// RESOURCE_START (82): records the resource and queues its client files and scripts.
void ActiveVmOnResourceStart(const WorldRPG::Net::ResourceStartInfo& info);
// RESOURCE_CLIENT_SCRIPTS (95): holds the no-cache scripts until the resource can start.
bool ActiveVmFeedScripts(const uint8_t* body, size_t length, size_t bits);
// Runs resources that became ready (downloads finished, scripts received). Call on the
// VM thread after each network pulse. Returns true when the chat hook became active.
bool ActiveVmPulseResources();
// New session: drops every resource and restarts the VM with a clean state.
void ActiveVmResetSession();
bool ActiveVmOnChat(const char* text, int red, int green, int blue);
std::string ActiveVmPump();
void ActiveVmSetScreen(int width, int height);
// Touch on the DX overlay while the cursor is showing. Any thread. action: 0 down, 1 up, 2 move;
// x and y are in the pixels guiGetScreenSize reports.
void ActiveVmQueueCursor(int action, float x, float y);
// Soft-keyboard edit from the UI thread. text is the characters inserted since the
// previous value, backspaces is how many characters were removed before that, and
// key is "enter" or empty. Applied on the VM thread in pumpDx.
void ActiveVmQueueScriptInput(const char* text, int backspaces, const char* key);
// MediaPlayer finished a playSound id. Any thread.
void ActiveVmNoteSoundEnded(uint32_t id);
// True while that playSound id still has a player. isElement uses this, because the
// element-tree isElement binding runs after the sound binding and would otherwise
// say the login track is already gone.
bool SoundElementAlive(uint32_t id);
// Screen size scripts see (guiGetScreenSize), and the resource whose code is running.
void ActiveVmScreenSize(int& width, int& height);
const std::string& ActiveVmOwner();
// Net id of the resource whose code is running, or 0 outside a resource.
uint16_t ActiveVmOwnerNet();
uint32_t ActiveVmOwnerGeneration();
bool ActiveVmChatHooked();
// Client root id, or 0 before the element bindings are installed.
uint32_t ClientRootElement();
// The element an event is triggered on, then each parent while call propagation
// is enabled. An element that is not in the tree still reaches the root.
// CClientEntity::CallParentEvent (CClientEntity.cpp 811-823).
void CollectEventChain(uint32_t base, std::vector<uint32_t>& out);
// Resource element under the client root. The dynamic map is a separate call so a
// root read does not invent the map before the resource is running.
void EnsureResourceNode(uint32_t resourceRoot, const std::string& name);
void EnsureDynamicNode(uint32_t resourceRoot, uint32_t dynamicRoot);
// Drops the nodes this client added for that resource. Server placeholders of kind 0
// are forgotten too, so isElement is false after the stop.
void ReleaseResourceElements(uint32_t resourceRoot, uint32_t dynamicRoot);
bool ActiveVmOnServerEvent(const uint8_t* body, size_t length, size_t bits);
// Runs the client command handlers of this command, each as the resource that registered it.
// False when none is registered. VM thread.
bool ActiveVmRunCommand(const char* name, const char* args);
// A command line the player entered in the chat box ("say hello", "panel open"), already sent to
// the server by the caller. The client handlers and onClientConsole run on the next pump. Any thread.
void ActiveVmQueueCommand(const char* line);
// Command lines that a key bind ran and that still have to go to the server (desktop
// COMMAND_Executed sends every command). Drained by the caller that owns the connection.
std::vector<std::string> ActiveVmTakeServerCommands();
// RESOURCE_STOP (83): removes the resource and stops its scripts (desktop Packet_ResourceStop).
void ActiveVmOnResourceStop(const uint8_t* body, size_t length, size_t bits);

} // namespace WorldRPG::Script