#include "shared/SpatialWorld.h"
#include "shared/LuaVMStub.h"
#include "shared/LuaSpatialApi.h"
#include "shared/LuaApiExtra.h"
#include "lua.hpp"
#include "shared/ResourceFiles.h"
#include "shared/ResourceLoader.h"
#include "shared/ServerWorld.h"
#include "shared/PacketHandler.h"
#include "shared/MtaIds.h"
#include "gtasa/GameEngine.h"
#include <zlib.h>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <string>
using namespace WorldRPG;
namespace S=WorldRPG::Spatial;
unsigned checks=0;
void Check(bool yes,const char* label) { ++checks; if(!yes) { std::fprintf(stderr,"FAIL: %s\n",label); std::exit(1); } }
void Lua(Script::LuaVMStub& vm,const std::string& script) {
    auto r=vm.executeBuffer(script.data(),script.size(),"spatial-test");
    if(!r.success) std::fprintf(stderr,"%s\n",r.errorMessage.c_str());
    Check(r.success,"Lua spatial behavior");
}
void Bits(Net::BitStreamWriter& w,uint32_t n,size_t count) {
    uint8_t raw[]={uint8_t(n),uint8_t(n>>8),uint8_t(n>>16),uint8_t(n>>24)}; w.WriteBits(raw,count);
}
void Position(Net::BitStreamWriter& w,float x,float y,float z) {
    Bits(w,uint32_t(int32_t(x*1024))&0xffffff,24); Bits(w,uint32_t(int32_t(y*1024))&0xffffff,24); w.Write(z);
}
void Header(Net::BitStreamWriter& w,uint32_t id) {
    w.WriteCompressed(1,4); Bits(w,id,17); w.Write(uint8_t(15)); Bits(w,0,17); w.Write(uint8_t(0)); w.WriteCompressed(0,2);
    Bits(w,0,1); Bits(w,1,1); w.WriteCompressed(0,2); w.WriteCompressed(0,2); w.Write(uint8_t(0));
}
void Shape(uint32_t id,unsigned type,bool enabled=true,bool autoEvents=true) {
    Net::BitStreamWriter w; Header(w,id); Bits(w,type,3); Position(w,100,100,10); Bits(w,enabled,1); Bits(w,autoEvents,1);
    switch(type) {
        case 0:case 2:w.Write(1.f);break;
        case 1:Position(w,2,2,2);break;
        case 3:Bits(w,2*1024,24);Bits(w,2*1024,24);break;
        case 5:w.Write(1.f);w.Write(2.f);break;
        case 4:
            w.WriteCompressed(4,4);
            for(auto p:{S::Vec{99,99,0},S::Vec{101,99,0},S::Vec{101,101,0},S::Vec{99,101,0}}) {
                Bits(w,uint32_t(p.x*1024),24); Bits(w,uint32_t(p.y*1024),24);
            }
            w.Write(9.f);w.Write(11.f);break;
    }
    ServerWorld::OnEntityAdd(w.GetData(),w.GetByteLength(),w.GetBitLength());
    S::Element shape; Check(Script::CopySpatialShape(id,shape),"ENTITY_ADD retains colshape");
    Check(unsigned(shape.shape->type)==type && shape.shape->enabled==enabled && shape.shape->autoEvents==autoEvents,"wire shape flags/type");
}
template<class Write> bool Rpc(Sync::PacketHandler& h,uint8_t fn,uint32_t id,Write write) {
    Net::BitStreamWriter w; w.Write(MtaIds::Packet::LuaElementRpc);w.Write(fn);Bits(w,id,17);write(w);
    return h.handlePacket(99,w.GetData(),w.GetByteLength(),w.GetBitLength());
}
std::vector<uint8_t> ScriptPacket(uint16_t net,const std::string& source) {
    uLongf size=compressBound(source.size()); std::vector<uint8_t> packed(size);
    compress2(packed.data(),&size,reinterpret_cast<const Bytef*>(source.data()),source.size(),Z_DEFAULT_COMPRESSION);packed.resize(size);
    std::vector<uint8_t> out={uint8_t(net),uint8_t(net>>8),1,0};
    uint32_t n=uint32_t(size+4); for(unsigned i=0;i<4;++i) out.push_back(uint8_t(n>>(i*8)));
    n=source.size(); for(int i=3;i>=0;--i) out.push_back(uint8_t(n>>(i*8)));
    out.insert(out.end(),packed.begin(),packed.end()); return out;
}
void Resource(uint16_t net,uint32_t root,const char* name,const std::string& script) {
    Net::ResourceStartInfo info; info.name=name; info.resourceId=net; info.entityId=root; info.dynamicEntityId=root+1;
    info.noClientCacheScripts=1; info.startCounter=1;
    Script::ActiveVmOnResourceStart(info);
    auto data=ScriptPacket(net,script);
    Check(Script::ActiveVmFeedScripts(data.data(),data.size(),data.size()*8),"resource scripts accepted");
    Script::ActiveVmPulseResources();
}
void GeometryChecks() {
    S::Geometry g; g.radius=1;
    Check(S::Hit({},g,{1,0,0})&&!S::Hit({},g,{1.001f,0,0}),"sphere inclusive boundary");
    g.type=S::ShapeType::Circle; Check(S::Hit({},g,{0,0,1000}),"circle ignores Z");
    g.type=S::ShapeType::Cuboid;g.size={2,3,4};
    Check(S::Hit({},g,{2,3,4})&&!S::Hit({},g,{2.001f,3,4}),"cuboid corner and point-only collision");
    g.size.z=-1;Check(!S::Hit({},g,{}),"negative cuboid height is empty");
    g.type=S::ShapeType::Rectangle;Check(S::Hit({},g,{1,1,1000}),"rectangle ignores height");
    g.type=S::ShapeType::Tube;g.height=2;
    Check(S::Hit({},g,{1,0,2})&&!S::Hit({},g,{0,0,2.001f}),"tube radial and height boundaries");
    g.type=S::ShapeType::Polygon;g.points={{-1,-1,0},{1,-1,0},{1,1,0},{-1,1,0}};g.floor=-1;g.ceiling=1;
    Check(S::Hit({},g,{0,0,1})&&!S::Hit({},g,{0,0,1.01f})&&!S::Hit({},g,{2,0,0}),"polygon points and absolute height");
    g.points.clear();Check(!S::Hit({},g,{}),"empty polygon");
    S::World world; S::Element e; e.id=1;e.generation=1;e.type="object";e.position={1,1,500};e.bounds={e.position,2};
    world.Sync({e});
    Check(world.Range({},.5f,{}, {}, {}).size()==1,"unfiltered range uses XY bounding boxes, including model radius");
    Check(world.Range({},.5f,std::string("object"),{},{}).empty(),"type argument adds exact 3D point filtering");
    Check(world.Range({},.5f,std::string(""),{},{}).size()==1,"empty type leaves broadphase query");
    Check(world.Range({},.5f,{},0,{}).empty(),"interior zero still enables exact 3D check");
    Check(world.Range({},-3,{}, {},{}).size()==1,"negative range radius uses absolute broadphase");
    Check(world.Range({},12001,{}, {},{}).empty(),"desktop spatial radius limit");
    Check(!S::ValidSphere({{12001,0,0},0})&&!S::ValidSphere({{},std::numeric_limits<float>::quiet_NaN()}),"invalid spatial bounds");
    S::Element shape;shape.id=2;shape.generation=1;shape.type="colshape";shape.shape=S::Geometry{};
    shape.shape->radius=10;shape.bounds=S::ShapeBounds({},*shape.shape);e.position={};e.bounds={e.position,0};e.type="player";
    world.Sync({shape,e}); unsigned events=0;world.Pulse([&](const auto& t){++events;Check(world.Within(t.entity,t.shape),"membership is committed before hit");});
    world.Pulse([&](auto){++events;});Check(events==1,"hit once while staying inside");
    world.Remove(1);Check(!world.Within(1,2),"destructor unlinks both sides without leave");
    ++e.generation;world.Sync({shape,e});world.Pulse([&](auto){++events;});Check(events==2,"reused ID can enter again");
    shape.shape->autoEvents=false;world.Clear();world.Sync({shape,e});events=0;world.Pulse([&](auto){++events;});
    Check(events==0&&world.Within(1,2),"auto-call false stores collision without hit event");
    e.position={20,0,0};world.Sync({shape,e});world.Pulse([&](const auto& t){++events;Check(!t.hit,"leave transition");});
    Check(events==1,"desktop leave still fires with auto-call false");
    world.Clear();e.type="object";e.position={};e.bounds={e.position,0};shape.shape->autoEvents=true;
    world.Sync({e});world.Pulse([](auto){});world.Sync({e,shape});world.Pulse([](auto){});
    Check(!world.Within(1,2),"new shape does not immediately refresh a stationary object");
    world.Refresh(2);world.Pulse([](auto){});Check(world.Within(1,2),"explicit shape refresh detects stationary objects");

}
int main() {
    GeometryChecks();ServerWorld::ResetSession();Script::ResourceLoader::Reset();ServerWorld::SetLocalPlayer(1);
    Game::ApplyServerSpawn(100.25f,100.25f,10); ServerWorld::NotePlayerSkin(2,7);ServerWorld::NoteElementPosition(2,100.5f,100,10);ServerWorld::NoteElementDimension(2,3);
    Script::LuaVMStub vm;Check(vm.initialize(),"VM initializes");
    Lua(vm,"assert(createColSphere(0,0,0,1)==false and isElementWithinColShape({}, {})==nil and getElementsWithinColShape({})==false)");
    Resource(41,4000,"spatial",R"LUA(
        _G.s=createColSphere(100,100,10,1)
        _G.b=createColCuboid(100,100,10,-1,-1,-1)
        assert(isElement(s) and getElementType(s)=='colshape')
        assert(getElementParent(s).id==4001 and #getElementsWithinColShape(s)==0)
        assert(isElementWithinColShape(localPlayer,s)==false)
        assert(setElementInterior(s,99) and setElementDimension(s,0))
        _G.hits=0;_G.leaves=0;_G.entityHits=0;_G.entityLeaves=0;_G.matching={}
        addEventHandler('onClientColShapeHit',s,function(e,matching)
            _G.hits=_G.hits+1;_G.matching[e.id]=matching
            _G.insideDuringHit=isElementWithinColShape(e,source)
        end)
        addEventHandler('onClientColShapeLeave',s,function(e,matching)
            _G.leaves=_G.leaves+1;_G.outsideDuringLeave=not isElementWithinColShape(e,source)
        end)
        addEventHandler('onClientElementColShapeHit',root,function(shape) if shape.id==s.id then _G.entityHits=_G.entityHits+1 end end)
        addEventHandler('onClientElementColShapeLeave',root,function(shape) if shape.id==s.id then _G.entityLeaves=_G.entityLeaves+1 end end)
        _G.started=true
    )LUA");
    Lua(vm,"assert(started and hits==0 and #getElementsByType('colshape')==2)");
    vm.pumpDx();
    Lua(vm,R"(
        assert(hits==2 and entityHits==2 and insideDuringHit and matching[1]==true and matching[2]==false)
        assert(#getElementsWithinColShape(s,'player')==2 and #getElementsWithinColShape(s,'ped')==0)
        assert(#getElementsWithinColShape(b)==0 and #getElementsByType('colshape',{id=4000},true)==2)
        assert(getElementInterior(s)==99 and getElementDimension(s)==0)
        assert(#getElementsWithinRange(100,100,10,1,'player',nil,3)==1)
        assert(#getElementsWithinRange(100,100,10,1,'player',nil,0)==1)
        assert(#getElementsWithinRange(100,100,1000,1)==4)
        assert(#getElementsWithinRange(100,100,1000,1,'player')==0)
        assert(getElementsWithinRange(0,0,0,0/0)==false and getElementsWithinRange(0,0,0,1,true)==false)
        assert(getElementsWithinRange(0,0,0,1,nil,-1)==false)
        assert(createColSphere(0,0,0,1/0)==false and createColCuboid(0,0,0,1,1,{})==false)
    )");
    vm.pumpDx();Lua(vm,"assert(hits==2 and leaves==0)");
    Game::ApplyServerSpawn(110,110,10);vm.pumpDx();Lua(vm,"assert(leaves==1 and entityLeaves==1 and outsideDuringLeave)");
    Lua(vm,"assert(setElementPosition(s,120,120,10))");Lua(vm,"assert(leaves==2 and entityLeaves==2 and #getElementsWithinColShape(s)==0)");
    Lua(vm,"local x,y,z=getElementPosition(s);assert(x==120 and y==120 and z==10)");
    for(unsigned type=0;type<6;++type) Shape(900+type,type);
    Game::ApplyServerSpawn(100.25f,100.25f,10);vm.pumpDx();
    Lua(vm,"for id=900,905 do assert(isElementWithinColShape(localPlayer,{id=id})==true,'shape '..id) end");
    Sync::PacketHandler handler;
    Check(Rpc(handler,MtaIds::Rpc::SetColshapeRadius,902,[](auto& w){w.Write(.1f);}),"radius RPC");
    vm.pumpDx();Lua(vm,"assert(isElementWithinColShape(localPlayer,{id=902})==false)");
    Check(!Rpc(handler,MtaIds::Rpc::SetColshapeRadius,902,[](auto&){}),"truncated radius RPC is rejected");
    Check(Rpc(handler,MtaIds::Rpc::SetColshapeSize,901,[](auto& w){w.Write(-1.f);w.Write(-1.f);w.Write(-1.f);}),"size RPC");
    Check(Rpc(handler,MtaIds::Rpc::SetColshapeSize,905,[](auto& w){w.Write(.1f);w.Write(999.f);w.Write(999.f);}),"tube size uses X for height");
    Check(Rpc(handler,MtaIds::Rpc::SetColPolygonHeight,904,[](auto& w){w.Write(20.f);w.Write(21.f);}),"polygon height RPC");
    vm.pumpDx();Lua(vm,"assert(not isElementWithinColShape(localPlayer,{id=901}) and not isElementWithinColShape(localPlayer,{id=904}))");
    Check(Rpc(handler,MtaIds::Rpc::SetElementPosition,900,[](auto& w){w.Write(120.f);w.Write(120.f);w.Write(0.f);w.Write(uint8_t(0));}),"colshape position RPC");
    vm.pumpDx();Lua(vm,"assert(not isElementWithinColShape(localPlayer,{id=900}))");
    Check(Rpc(handler,MtaIds::Rpc::RemoveColPolygonPoint,904,[](auto& w){w.Write(uint32_t(0));}),"remove polygon point RPC");
    S::Element polygon;Script::CopySpatialShape(904,polygon);Check(polygon.shape->points.size()==3,"polygon has three remaining points");
    auto revision=polygon.shapeRevision;
    Check(Rpc(handler,MtaIds::Rpc::RemoveColPolygonPoint,904,[](auto& w){w.Write(uint32_t(0));}),"minimum-size removal is consumed");
    Script::CopySpatialShape(904,polygon);Check(polygon.shape->points.size()==3 && polygon.shapeRevision==revision,"minimum three points and no false refresh");
    auto point=[](auto& w){Bits(w,98*1024,24);Bits(w,98*1024,24);};
    Check(Rpc(handler,MtaIds::Rpc::AddColPolygonPoint,904,point),"append polygon point RPC without index");
    Check(Rpc(handler,MtaIds::Rpc::AddColPolygonPoint,904,[&](auto& w){point(w);w.Write(uint32_t(1));}),"insert polygon point RPC with index");
    Check(Rpc(handler,MtaIds::Rpc::UpdateColPolygonPoint,904,[&](auto& w){point(w);w.Write(uint32_t(0));}),"update polygon point RPC");
    Script::CopySpatialShape(904,polygon);Check(polygon.shape->points.size()==5 && polygon.shape->points[0].x==98,"polygon mutation payload order");
    Check(!Rpc(handler,MtaIds::Rpc::UpdateColPolygonPoint,904,point),"update without index is malformed");
    Check(Rpc(handler,MtaIds::Rpc::SetElementInterior,903,[](auto& w){w.Write(uint8_t(7));w.Write(uint8_t(1));w.Write(200.f);w.Write(200.f);w.Write(0.f);}),"colshape interior and position RPC");
    Lua(vm,"assert(getElementInterior({id=903})==7 and getElementPosition({id=903})==200)");
    Lua(vm,"assert(setElementInterior(localPlayer,4) and #getElementsWithinRange(100,100,10,1,'player',4,0)==1)");
    Check(Rpc(handler,MtaIds::Rpc::SetElementInterior,1,[](auto& w){w.Write(uint8_t(7));w.Write(uint8_t(0));}),"player interior RPC supersedes Lua value");
    Lua(vm,"assert(getElementInterior(localPlayer)==7 and #getElementsWithinRange(100,100,10,1,'player',7,0)==1)");
    // Removal/reuse must not carry collision history into a fresh shape.
    Net::BitStreamWriter remove;Bits(remove,903,17);ServerWorld::OnEntityRemove(remove.GetData(),remove.GetByteLength(),remove.GetBitLength());
    Lua(vm,"assert(isElementWithinColShape(localPlayer,{id=903})==nil)");Shape(903,3);
    Lua(vm,"assert(isElementWithinColShape(localPlayer,{id=903})==false)");vm.pumpDx();Lua(vm,"assert(isElementWithinColShape(localPlayer,{id=903})==true)");
    Shape(910,2,false);vm.pumpDx();Lua(vm,"assert(isElementWithinColShape(localPlayer,{id=910})==false)");
    Resource(42,5000,"spatial-other",R"LUA(
        _G.otherShape=createColSphere(100,100,10,2)
        _G.movingShape=createColSphere(100,100,10,2);_G.order={}
        addEventHandler('onClientColShapeHit',movingShape,function()
            table.insert(_G.order,'hitShape');setElementPosition(source,300,300,10)
        end)
        addEventHandler('onClientColShapeLeave',movingShape,function() table.insert(_G.order,'leaveShape') end)
        addEventHandler('onClientElementColShapeHit',root,function(shape) if shape.id==movingShape.id then table.insert(_G.order,'hitElement') end end)
        addEventHandler('onClientElementColShapeLeave',root,function(shape) if shape.id==movingShape.id then table.insert(_G.order,'leaveElement') end end)
        _G.doomed=createColSphere(100,100,10,2);_G.doomedElementHits=0
        addEventHandler('onClientColShapeHit',doomed,function() destroyElement(doomed) end)
        addEventHandler('onClientElementColShapeHit',root,function(shape) if shape.id==doomed.id then _G.doomedElementHits=_G.doomedElementHits+1 end end)
    )LUA");
    vm.pumpDx();Lua(vm,"assert(not isElement(doomed) and doomedElementHits==0 and isElement(otherShape))");
    Lua(vm,"assert(table.concat(order,',')=='hitShape,leaveShape,leaveElement,hitElement')");
    Lua(vm,"assert(setElementParent(s,{id=5001}))");
    Net::BitStreamWriter stop;stop.Write(uint16_t(41));Script::ActiveVmOnResourceStop(stop.GetData(),stop.GetByteLength(),stop.GetBitLength());
    Lua(vm,"assert(not isElement(s) and not isElement(b) and isElement(otherShape) and leaves==2)");
    lua_State* L=Script::ActiveVmState();lua_getglobal(L,"otherShape");
    uint32_t localId=Script::ElementIdOf(L,-1);lua_pop(L,1);
    vm.shutdown();Check(!Script::SpatialShapeAlive(localId),"VM shutdown releases local colshapes");
    Check(Script::SpatialShapeAlive(903),"server geometry survives VM shutdown");
    ServerWorld::ResetSession();Check(!Script::SpatialShapeAlive(903),"new session forgets server geometry");
    std::printf("spatial tests: %u checks passed\n",checks);
}
