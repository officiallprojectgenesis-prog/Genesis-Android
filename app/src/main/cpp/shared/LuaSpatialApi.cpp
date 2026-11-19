#include "LuaSpatialApi.h"
#include "LuaApiExtra.h"
#include "LuaVMStub.h"
#include "LuaFieldApi.h"
#include "ResourceLoader.h"
#include "ServerWorld.h"
#include "../gtasa/GameEngine.h"
#include "lua.hpp"
#include <algorithm>
#include <cmath>
#include <map>

namespace WorldRPG::Script {
namespace {
struct LocalShape { Spatial::Element element; std::string owner; uint16_t netId=0; uint32_t generation=0; };
std::map<uint32_t,LocalShape> localShapes;
Spatial::World world;
bool pulsing=false;
int Fail(lua_State* L) { lua_pushboolean(L,0); return 1; }
bool Number(lua_State* L,int i,float& out) {
    if(!lua_isnumber(L,i)) return false;
    out=static_cast<float>(lua_tonumber(L,i)); return std::isfinite(out);
}
bool Vec(lua_State* L,int& i,Spatial::Vec& out) {
    if(lua_istable(L,i)) {
        const char* keys[]={"x","y","z"}; float* v[]={&out.x,&out.y,&out.z};
        for(unsigned k=0;k<3;++k) { lua_getfield(L,i,keys[k]); bool ok=Number(L,-1,*v[k]); lua_pop(L,1); if(!ok) return false; }
        ++i; return true;
    }
    if(!Number(L,i,out.x)||!Number(L,i+1,out.y)||!Number(L,i+2,out.z)) return false;
    i+=3; return true;
}
void Collect(bool native=true) {
    std::vector<Spatial::Element> scene;
    ServerWorld::CopySpatialElements(scene);
    for(auto& e:scene) {
        float x=0,y=0,z=0,h=0;
        if(e.id==ServerWorld::LocalPlayer()) { Game::CopyPose(x,y,z,h); e.position={x,y,z}; e.bounds.center=e.position; }
        else if(!e.shape && Game::CopyRemotePose(e.id,x,y,z)) { e.position={x,y,z}; e.bounds.center=e.position; }
    }
    if(native) Game::CopySpatialScene(scene);
    else for(auto& e:scene) if(!e.shape) {
        const auto* old=world.Find(e.id);
        if(old && old->generation==e.generation) { e.position=old->position; e.bounds=old->bounds; }
    }
    for(auto& e:scene) ApplySpatialArea(e.id,e.generation,e.interior,e.dimension);
    for(const auto& [id,shape]:localShapes) scene.push_back(shape.element);
    world.Sync(scene);
}
bool Current(uint32_t id,uint64_t generation) {
    if(auto it=localShapes.find(id);it!=localShapes.end()) return it->second.element.generation==generation;
    if(ServerWorld::ElementGeneration(id)==generation && generation) return true;
    return Game::ScriptPedAlive(id) && generation==id;
}
void Dispatch(const Spatial::Transition& t) {
    if(!Current(t.shape,t.shapeGeneration)||!Current(t.entity,t.entityGeneration)) return;
    if(t.hit) ActiveVmFireSpatialEvent("onClientColShapeHit",t.shape,t.entity,t.matchingDimension);
    else ActiveVmFireSpatialEvent("onClientColShapeLeave",t.shape,t.entity,t.matchingDimension);
    // A handler can destroy its source or stop the owning resource.
    if(!Current(t.shape,t.shapeGeneration)||!Current(t.entity,t.entityGeneration)) return;
    Collect(false);
    const auto* s=world.Find(t.shape); const auto* e=world.Find(t.entity);
    if(s&&e) {
        if(t.hit) ActiveVmFireSpatialEvent("onClientElementColShapeHit",t.entity,t.shape,s->dimension==e->dimension);
        else ActiveVmFireSpatialEvent("onClientElementColShapeLeave",t.entity,t.shape,s->dimension==e->dimension);
    }
}
int Create(lua_State* L,bool cuboid) {
    int i=1; Spatial::Element e; Spatial::Geometry g;
    if(!Vec(L,i,e.position)) return Fail(L);
    if(cuboid) {
        g.type=Spatial::ShapeType::Cuboid;
        if(!Vec(L,i,g.size)) return Fail(L);
        if(g.size.x<0) g.size.x=.1f;
        if(g.size.y<0) g.size.y=.1f; // desktop preserves negative Z
    } else {
        g.type=Spatial::ShapeType::Sphere;
        if(!Number(L,i,g.radius)) return Fail(L);
        if(g.radius<0) g.radius=.1f;
    }
    const std::string owner=ActiveVmOwner(); ResourceLoader::ResourceView caller;
    if(owner.empty()||!ResourceLoader::CopyCallerResource(ActiveVmOwnerNet(),ActiveVmOwnerGeneration(),owner,caller)||!caller.root) return Fail(L);
    e.id=CreateResourceElement("colshape"); if(!e.id) return Fail(L);
    e.generation=e.id; e.type="colshape"; e.shape=g; e.bounds=Spatial::ShapeBounds(e.position,g);
    localShapes.emplace(e.id,LocalShape{e,owner,ActiveVmOwnerNet(),ActiveVmOwnerGeneration()});
    // CStaticFunctionDefinitions.cpp:7598-7612 intentionally does not refresh on creation.
    Collect(false); PushElement(L,e.id); return 1;
}
int CreateColSphere(lua_State* L) { return Create(L,false); }
int CreateColCuboid(lua_State* L) { return Create(L,true); }
int IsElementWithinColShape(lua_State* L) {
    Spatial::Element shape;
    uint32_t e=ElementIdOf(L,1),s=ElementIdOf(L,2);
    if(!KnownClientElement(e)||!CopySpatialShape(s,shape)) { lua_pushnil(L); return 1; }
    Collect(false); lua_pushboolean(L,world.Within(e,s)); return 1;
}
int GetElementsWithinColShape(lua_State* L) {
    Spatial::Element shape; uint32_t id=ElementIdOf(L,1);
    if(!CopySpatialShape(id,shape)) return Fail(L);
    std::string type;
    if(!lua_isnoneornil(L,2)) { if(!lua_isstring(L,2)) return Fail(L); type=lua_tostring(L,2); }
    Collect(false); lua_newtable(L); int i=0;
    for(uint32_t e:world.Colliders(id,type)) { PushElement(L,e); lua_rawseti(L,-2,++i); }
    return 1;
}
int GetElementsWithinRange(lua_State* L) {
    int i=1; Spatial::Vec pos; float radius=0;
    if(!Vec(L,i,pos)||!Number(L,i++,radius)) return Fail(L);
    std::optional<std::string> type; std::optional<uint16_t> interior,dimension;
    if(!lua_isnoneornil(L,i)) { if(!lua_isstring(L,i)) return Fail(L); type=lua_tostring(L,i); } ++i;
    for(auto* value:{&interior,&dimension}) {
        if(!lua_isnoneornil(L,i)) { float n=0; if(!Number(L,i,n)||n<0||n>65535) return Fail(L); *value=static_cast<uint16_t>(n); }
        ++i;
    }
    Collect(); lua_newtable(L); i=0;
    for(uint32_t id:world.Range(pos,radius,type,interior,dimension)) { PushElement(L,id); lua_rawseti(L,-2,++i); }
    return 1;
}
int GetElementsByType(lua_State* L) {
    const char* type=lua_tostring(L,1);
    if(!type || std::string(type)!="colshape") {
        int n=lua_gettop(L); lua_pushvalue(L,lua_upvalueindex(1)); lua_insert(L,1); lua_call(L,n,LUA_MULTRET); return lua_gettop(L);
    }
    uint32_t root=ClientRootElement();
    if(lua_istable(L,2)) { root=ElementIdOf(L,2); if(!KnownClientElement(root)) return Fail(L); }
    Collect(false); std::vector<Spatial::Element> shapes;
    ServerWorld::CopySpatialElements(shapes);
    for(const auto& [id,shape]:localShapes) shapes.push_back(shape.element);
    lua_newtable(L); int i=0;
    for(const auto& e:shapes) if(e.shape && ClientDescendantOf(e.id,root)) { PushElement(L,e.id); lua_rawseti(L,-2,++i); }
    return 1;
}
// Adds the colshape branch to the existing position setter. Other element
// backends retain their previous behavior and are not counted as native parity.
int SetElementPosition(lua_State* L) {
    const uint32_t id=ElementIdOf(L,1); Spatial::Element shape;
    if(!KnownClientElement(id)) return Fail(L);
    if(!CopySpatialShape(id,shape)) {
        int n=lua_gettop(L); lua_pushvalue(L,lua_upvalueindex(1)); lua_insert(L,1); lua_call(L,n,LUA_MULTRET); return lua_gettop(L);
    }
    int i=2; Spatial::Vec position;
    if(!Vec(L,i,position)) return Fail(L);
    if(!lua_isnoneornil(L,i)&&!lua_isboolean(L,i)) return Fail(L);
    if(shape.position!=position) {
        if(auto local=localShapes.find(id);local!=localShapes.end()) {
            local->second.element.position=position;
            local->second.element.bounds=Spatial::ShapeBounds(position,*shape.shape);
        } else ServerWorld::NoteElementPosition(id,position.x,position.y,position.z);
        Collect(false); world.Refresh(id);
        PulseSpatial();
    }
    lua_pushboolean(L,1); return 1;
}
}
bool SpatialShapeAlive(uint32_t id) { Spatial::Element e; return CopySpatialShape(id,e); }
bool CopySpatialShape(uint32_t id,Spatial::Element& out) {
    if(auto it=localShapes.find(id);it!=localShapes.end()) { out=it->second.element; return true; }
    return ServerWorld::CopyColshape(id,out);
}
bool SpatialSetArea(uint32_t id,unsigned value,bool dimension) {
    Spatial::Element e; if(!CopySpatialShape(id,e)) return false;
    if(auto it=localShapes.find(id);it!=localShapes.end()) {
        if(dimension) it->second.element.dimension=value; else it->second.element.interior=value;
    } else if(dimension) ServerWorld::NoteElementDimension(id,value); else ServerWorld::NoteElementInterior(id,value);
    Collect(false); return true;
}
void SpatialRelease(uint32_t id) { localShapes.erase(id); world.Remove(id); }
void SpatialDropResource(const std::string& owner,uint16_t net,uint32_t generation) {
    std::vector<uint32_t> ids;
    for(const auto& [id,s]:localShapes) if(s.owner==owner&&(!net||s.netId==net)&&(!generation||s.generation==generation)) ids.push_back(id);
    for(uint32_t id:ids) DestroyClientElement(id);
}
void PulseSpatial() {
    const bool nested=pulsing;
    if(localShapes.empty()) {
        std::vector<Spatial::Element> scene; ServerWorld::CopySpatialElements(scene);
        if(std::none_of(scene.begin(),scene.end(),[](const auto& e){return e.shape.has_value();})) {
            world.Sync(scene); world.Pulse([](const auto&){}); return;
        }
    }
    Collect(!nested);
    pulsing=true; world.Pulse(Dispatch); pulsing=nested;
}
void ResetSpatial() { localShapes.clear(); world.Clear(); pulsing=false; }
void InstallSpatialLua(lua_State* L) {
    ResetSpatial();
    const struct { const char* name; lua_CFunction fn; } binds[]={
        {"getElementsWithinRange",GetElementsWithinRange},{"createColSphere",CreateColSphere},
        {"createColCuboid",CreateColCuboid},{"isElementWithinColShape",IsElementWithinColShape},
        {"getElementsWithinColShape",GetElementsWithinColShape},
    };
    for(const auto& b:binds) { lua_pushcfunction(L,b.fn); lua_setglobal(L,b.name); }
    lua_getglobal(L,"setElementPosition"); lua_pushcclosure(L,SetElementPosition,1); lua_setglobal(L,"setElementPosition");
    lua_getglobal(L,"getElementsByType"); lua_pushcclosure(L,GetElementsByType,1); lua_setglobal(L,"getElementsByType");
}
}
