// Resource queries. Desktop CLuaResourceDefs.cpp 20-30 and 146-413.
// A resource is a table {resource = true, name = "..."}, not an element.
// getResourceRootElement returns the element. The phone has one Lua state,
// so getThisResource is the resource whose code is running (ActiveVmOwner).

#include "LuaResourceApi.h"

#include "LuaApiExtra.h"
#include "LuaGuiApi.h"
#include "LuaVMStub.h"
#include "ResourceLoader.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include <string>

namespace WorldRPG::Script {
namespace {

constexpr const char* kResourceTable = "worldrpg.resources";

void PushResource(lua_State* L, const ResourceLoader::ResourceView& view) {
    // One table per net id and start, so getThisResource() == getResourceFromName(name)
    // for that instance. A later start of the same net id does not reuse the table.
    lua_getfield(L, LUA_REGISTRYINDEX, kResourceTable);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setfield(L, LUA_REGISTRYINDEX, kResourceTable);
    }
    lua_rawgeti(L, -1, view.netId);
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "gen");
        const bool same = lua_isinteger(L, -1) &&
            static_cast<uint32_t>(lua_tointeger(L, -1)) == view.generation;
        lua_pop(L, 1);
        if (same) {
            lua_remove(L, -2);
            return;
        }
    }
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushboolean(L, 1);
    lua_setfield(L, -2, "resource");
    lua_pushlstring(L, view.name.data(), view.name.size());
    lua_setfield(L, -2, "name");
    lua_pushinteger(L, view.netId);
    lua_setfield(L, -2, "netId");
    lua_pushinteger(L, view.startCounter);
    lua_setfield(L, -2, "start");
    lua_pushinteger(L, view.generation);
    lua_setfield(L, -2, "gen");
    lua_pushvalue(L, -1);
    lua_rawseti(L, -3, view.netId);
    lua_remove(L, -2);
}

bool ResourceAt(lua_State* L, int index, bool optional, ResourceLoader::ResourceView& view) {
    if (lua_isnoneornil(L, index)) {
        if (!optional) return false;
        return ResourceLoader::CopyCallerResource(ActiveVmOwnerNet(), ActiveVmOwnerGeneration(), ActiveVmOwner(), view);
    }
    if (!lua_istable(L, index)) return false;
    lua_getfield(L, index, "resource");
    const bool flag = lua_toboolean(L, -1) != 0;
    lua_pop(L, 1);
    if (!flag) return false;
    lua_getfield(L, index, "netId");
    const bool hasNet = lua_isinteger(L, -1);
    const uint16_t netId = hasNet ? static_cast<uint16_t>(lua_tointeger(L, -1)) : 0;
    lua_pop(L, 1);
    lua_getfield(L, index, "gen");
    const bool hasGen = lua_isinteger(L, -1);
    const uint32_t generation = hasGen ? static_cast<uint32_t>(lua_tointeger(L, -1)) : 0;
    lua_pop(L, 1);
    if (hasNet && hasGen) {
        if (!ResourceLoader::CopyResourceByGeneration(generation, view)) return false;
        return view.netId == netId;
    }
    lua_getfield(L, index, "name");
    size_t length = 0;
    const char* text = lua_tolstring(L, -1, &length);
    if (!text) {
        lua_pop(L, 1);
        return false;
    }
    const std::string name(text, length);
    lua_pop(L, 1);
    return ResourceLoader::CopyResource(name, view);
}

void PushFail(lua_State* L) { lua_pushboolean(L, 0); }

int l_getThisResource(lua_State* L) {
    // CLuaResourceDefs::GetThisResource (CLuaResourceDefs.cpp 146-165).
    ResourceLoader::ResourceView view;
    if (!ResourceLoader::CopyCallerResource(ActiveVmOwnerNet(), ActiveVmOwnerGeneration(), ActiveVmOwner(), view)) {
        PushFail(L);
        return 1;
    }
    PushResource(L, view);
    return 1;
}

int l_getResourceName(lua_State* L) {
    // GetResourceName (CLuaResourceDefs.cpp 226-232). No argument uses the running resource.
    ResourceLoader::ResourceView view;
    if (!ResourceAt(L, 1, true, view)) {
        PushFail(L);
        return 1;
    }
    lua_pushlstring(L, view.name.data(), view.name.size());
    return 1;
}

int l_getResourceFromName(lua_State* L) {
    // CLuaResourceDefs::GetResourceFromName (234-257).
    size_t length = 0;
    const char* text = lua_tolstring(L, 1, &length);
    ResourceLoader::ResourceView view;
    if (!text || !ResourceLoader::CopyResource(std::string(text, length), view)) {
        PushFail(L);
        return 1;
    }
    PushResource(L, view);
    return 1;
}

int l_getResourceRootElement(lua_State* L) {
    // CLuaResourceDefs::GetResourceRootElement (259-297). The argument is optional.
    ResourceLoader::ResourceView view;
    if (!ResourceAt(L, 1, true, view) || view.root == 0) {
        PushFail(L);
        return 1;
    }
    EnsureResourceNode(view.root, view.name);
    PushElement(L, view.root);
    return 1;
}

int l_getResourceDynamicElementRoot(lua_State* L) {
    // CLuaResourceDefs::GetResourceDynamicElementRoot (337-358).
    ResourceLoader::ResourceView view;
    if (!ResourceAt(L, 1, false, view)) {
        PushFail(L);
        return 1;
    }
    if ((view.state != "running" && view.state != "stopping") || view.dynamic == 0) {
        LUA_LOGE("getResourceDynamicElementRoot: Resource %s Is Not Currently Running", view.name.c_str());
        PushFail(L);
        return 1;
    }
    EnsureResourceNode(view.root, view.name);
    EnsureDynamicNode(view.root, view.dynamic);
    PushElement(L, view.dynamic);
    return 1;
}

int l_getResourceGUIElement(lua_State* L) {
    // CLuaResourceDefs::GetResourceGUIElement (299-334). The argument is optional.
    ResourceLoader::ResourceView view;
    if (!ResourceAt(L, 1, true, view)) {
        PushFail(L);
        return 1;
    }
    const uint32_t gui = GuiEnsureResourceRoot(view.name, view.netId, view.generation);
    if (!gui) {
        PushFail(L);
        return 1;
    }
    PushElement(L, gui);
    return 1;
}

int l_getResourceExportedFunctions(lua_State* L) {
    // CLuaResourceDefs::GetResourceExportedFunctions (360-393).
    ResourceLoader::ResourceView view;
    if (!ResourceAt(L, 1, true, view)) {
        PushFail(L);
        return 1;
    }
    lua_createtable(L, static_cast<int>(view.exports.size()), 0);
    int index = 0;
    for (const std::string& name : view.exports) {
        lua_pushlstring(L, name.data(), name.size());
        lua_rawseti(L, -2, ++index);
    }
    return 1;
}

int l_getResources(lua_State* L) {
    // CLuaResourceDefs::GetResources (415-418), in RESOURCE_START order.
    const std::vector<ResourceLoader::ResourceView> list = ResourceLoader::CopyResources();
    lua_createtable(L, static_cast<int>(list.size()), 0);
    int index = 0;
    for (const ResourceLoader::ResourceView& view : list) {
        PushResource(L, view);
        lua_rawseti(L, -2, ++index);
    }
    return 1;
}

int l_getResourceState(lua_State* L) {
    // CLuaResourceDefs::GetResourceState (395-413).
    ResourceLoader::ResourceView view;
    if (!ResourceAt(L, 1, false, view)) {
        PushFail(L);
        return 1;
    }
    lua_pushlstring(L, view.state.data(), view.state.size());
    return 1;
}

}  // namespace

bool PushResourceInstance(lua_State* L, uint16_t netId, uint32_t generation) {
    ResourceLoader::ResourceView view;
    if (generation != 0) {
        if (!ResourceLoader::CopyResourceByGeneration(generation, view) || view.netId != netId) return false;
    } else if (!ResourceLoader::CopyResourceByNet(netId, view)) {
        return false;
    }
    PushResource(L, view);
    return true;
}

void ApplyEventSource(lua_State* L, uint16_t netId, uint32_t generation) {
    // CMapEventManager.cpp 227-244: sourceResource is the resource that triggered
    // the event. netId 0 is an engine call, and both globals become nil.
    ResourceLoader::ResourceView view;
    const bool found = generation != 0
        ? ResourceLoader::CopyResourceByGeneration(generation, view) && view.netId == netId
        : ResourceLoader::CopyResourceByNet(netId, view);
    if (netId != 0 && found) {
        PushResource(L, view);
        lua_setglobal(L, "sourceResource");
        EnsureResourceNode(view.root, view.name);
        if (view.root != 0) PushElement(L, view.root);
        else lua_pushnil(L);
        lua_setglobal(L, "sourceResourceRoot");
        return;
    }
    lua_pushnil(L);
    lua_setglobal(L, "sourceResource");
    lua_pushnil(L);
    lua_setglobal(L, "sourceResourceRoot");
}

void InstallResourceLua(lua_State* L) {
    lua_pushcfunction(L, l_getThisResource);
    lua_setglobal(L, "getThisResource");
    lua_pushcfunction(L, l_getResourceName);
    lua_setglobal(L, "getResourceName");
    lua_pushcfunction(L, l_getResourceFromName);
    lua_setglobal(L, "getResourceFromName");
    lua_pushcfunction(L, l_getResourceRootElement);
    lua_setglobal(L, "getResourceRootElement");
    lua_pushcfunction(L, l_getResourceDynamicElementRoot);
    lua_setglobal(L, "getResourceDynamicElementRoot");
    lua_pushcfunction(L, l_getResourceGUIElement);
    lua_setglobal(L, "getResourceGUIElement");
    lua_pushcfunction(L, l_getResourceExportedFunctions);
    lua_setglobal(L, "getResourceExportedFunctions");
    lua_pushcfunction(L, l_getResources);
    lua_setglobal(L, "getResources");
    lua_pushcfunction(L, l_getResourceState);
    lua_setglobal(L, "getResourceState");
}

}  // namespace WorldRPG::Script
