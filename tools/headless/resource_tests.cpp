// Host checks for the resource lifecycle. Built as neon_resource_tests; needs no device or server.
//   - RESOURCE_STOP (83) body parsing and PLAYER_RESOURCE_START (108) bytes.
//   - RESOURCE_START descriptor sets the ServerWorld resource root.
//   - fromJSON / toJSON against the desktop rules; with a path argument, the real catalog.json.
//   - RESOURCE_STOP: onClientResourceStop runs for its own resource, its handlers and timers
//     are dropped, and another resource keeps running.
//   - GUI elements (desktop CLuaGUIDefs): a hidden window and its children are not drawn,
//     relative placement, alpha inheritance, destroyElement, and the DX font scale.
//   - fetchRemote options-table statusCode, abort, and https (no HttpDownloader stats change).
//   - NormVectorBytesMatchVehicleSync and one sent packet in getNetworkUsageData.
// Usage: neon_resource_tests [path/to/catalog.json]

#include "HttpDownloader.h"
#include "LuaEventRegistry.h"
#include "LuaKeyBindApi.h"
#include "LuaMtaWindowApi.h"
#include "LuaVMStub.h"
#include "MtaCipher.h"
#include "MtaOfflineCodec.h"
#include "MtaReliabilityCodec.h"
#include "RemoteVehicle.h"
#include "ResourceFiles.h"
#include "ResourceLoader.h"
#include "ResourcePackets.h"
#include "ServerWorld.h"
#include "entity/EntityAddTypes.h"
#include "net/raknet/RakNetWrapper.h"

#include <zlib.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace {

int g_checks = 0;
int g_failures = 0;

void Check(bool ok, const char* expr, int line) {
    ++g_checks;
    if (!ok) {
        ++g_failures;
        std::printf("FAIL line %d: %s\n", line, expr);
    }
}

#define CHECK(expr) Check((expr), #expr, __LINE__)

// Runs a chunk in the test VM. A failed assert reports its message here.
bool RunLua(WorldRPG::Script::LuaVMStub& vm, const char* name, const std::string& code) {
    const WorldRPG::Script::LuaExecResult result = vm.executeBuffer(code.data(), code.size(), name);
    ++g_checks;
    if (!result.success) {
        ++g_failures;
        std::printf("FAIL lua %s: %s\n", name, result.errorMessage.c_str());
    }
    return result.success;
}

// RESOURCE_CLIENT_SCRIPTS (packet 95) body after the game id, as the server writes it for a
// mobile peer: uint16 net id, uint16 count, then per item uint32 length and the item
// (uint32 big-endian original length + zlib data).
std::vector<uint8_t> NoCacheScriptBody(uint16_t netId, const std::string& source) {
    uLongf packedLength = compressBound(source.size());
    std::vector<uint8_t> packed(packedLength);
    compress2(packed.data(), &packedLength, reinterpret_cast<const Bytef*>(source.data()), source.size(),
              Z_DEFAULT_COMPRESSION);
    packed.resize(packedLength);

    const uint32_t original = static_cast<uint32_t>(source.size());
    std::vector<uint8_t> item = {static_cast<uint8_t>(original >> 24), static_cast<uint8_t>(original >> 16),
                                 static_cast<uint8_t>(original >> 8), static_cast<uint8_t>(original)};
    item.insert(item.end(), packed.begin(), packed.end());

    std::vector<uint8_t> body = {static_cast<uint8_t>(netId & 0xFF), static_cast<uint8_t>(netId >> 8), 1, 0};
    const uint32_t length = static_cast<uint32_t>(item.size());
    for (int i = 0; i < 4; ++i) body.push_back(static_cast<uint8_t>(length >> (8 * i)));
    body.insert(body.end(), item.begin(), item.end());
    return body;
}

void CheckPacketLayouts() {
    // CResourceStopPacket::Write writes BitStream.Write(m_usID).
    WorldRPG::Net::BitStreamWriter serverStop;
    serverStop.Write(static_cast<uint16_t>(0x0102));
    uint16_t netId = 0;
    CHECK(WorldRPG::Script::ResourcePackets::ParseResourceStop(serverStop.GetData(), serverStop.GetByteLength(),
                                                               serverStop.GetBitLength(), netId));
    CHECK(netId == 0x0102);
    CHECK(!WorldRPG::Script::ResourcePackets::ParseResourceStop(serverStop.GetData(), 1, 8, netId));

    // Game id 108, then uint16 net id and uint32 start counter, little-endian as the server reads them.
    WorldRPG::Net::BitStreamWriter playerStart;
    WorldRPG::Script::ResourcePackets::BuildPlayerResourceStart(playerStart, 0x0102, 0x03040506);
    const uint8_t expected[] = {108, 0x02, 0x01, 0x06, 0x05, 0x04, 0x03};
    CHECK(playerStart.GetByteLength() == sizeof(expected));
    CHECK(std::memcmp(playerStart.GetData(), expected, sizeof(expected)) == 0);
}

void CheckJson(WorldRPG::Script::LuaVMStub& vm, const char* catalogPath) {
    RunLua(vm, "json-arrays", R"LUA(
        local a, b, c, d = fromJSON('[ 1, "a", null, true ]')
        assert(a == 1 and b == "a" and c == nil and d == true, "array values")
        assert(select('#', fromJSON('[ 1, "a", null, true ]')) == 4, "one return value per element")
        assert(select('#', fromJSON('[]')) == 0, "empty array returns no values")
        assert(fromJSON('nope') == nil and fromJSON('') == nil and fromJSON('  "x"') == nil, "not an object or array")
        assert(fromJSON('{"a":1') == nil, "unterminated object")
        assert(fromJSON('[1,]') == nil, "trailing comma")
        assert(fromJSON('[1] x') == nil, "trailing text")
    )LUA");

    RunLua(vm, "json-objects", R"LUA(
        local o = fromJSON('{"format": 1, "list": [1, 2.5, "x"], "nested": {"k": false}, "big": 1e3}')
        assert(math.type(o.format) == "integer" and o.format == 1, "integral number")
        assert(o.list[2] == 2.5 and o.list[3] == "x", "array inside object")
        assert(o.nested.k == false, "false value")
        assert(o.big == 1000, "exponent")
        local u = fromJSON('["caf\\u00e9"]')
        assert(u == "caf\195\169", "unicode escape")
        local s = fromJSON('["\\ud83d\\ude00"]')
        assert(s == "\240\159\152\128", "surrogate pair")
    )LUA");

    RunLua(vm, "json-elements", R"LUA(
        local e = fromJSON('[ "^E^777" ]')
        assert(type(e) == "table" and e.element == true and e.id == 777, "known element id")
        assert(fromJSON('[ "^E^1" ]') == nil, "unknown element is nil")
        assert(fromJSON('[ "^Q^1" ]') == "^Q^1", "other ^x^ text stays a string")
    )LUA");

    RunLua(vm, "json-writer", R"LUA(
        assert(toJSON({1, 2}) == '[ [ 1, 2 ] ]', "spaced array, wrapped")
        assert(toJSON({1, 2}, true) == '[[1,2]]', "compact array, wrapped")
        assert(toJSON({a = 1}, true) == '[{"a":1}]', "compact object")
        assert(toJSON({}) == '[ [ ] ]', "empty table is an empty array")
        assert(toJSON({x = {}}) == '[ { "x": [ ] } ]', "nested empty array")
        assert(toJSON("x") == '[ "x" ]', "string")
        assert(toJSON(true, true) == '[true]', "boolean")
        assert(toJSON(nil) == nil, "nil argument gives nil")
        assert(toJSON(1, true, 5) == '[1]', "extra argument is ignored")
        assert(toJSON(1, "yes") == nil, "non-boolean compact flag gives nil")
        assert(toJSON(1.5, true) == '[1.5]', "fraction")
        assert(toJSON(50.0, true) == '[50]', "integral float is an integer")
        assert(toJSON("a/b", true) == '["a\\/b"]', "slash is escaped as json-c does")
        assert(toJSON("line\n", true) == '["line\\n"]', "newline escape")
        assert(toJSON("^E^5", true) == '["^E~5"]', "element-like text is escaped")
        assert(toJSON({id = 777, element = true}, true) == '[]', "element is not written by toJSON")
        assert(toJSON({1, 2, {x = 1}}, true) == '[[1,2,{"x":1}]]', "mixed nesting")
    )LUA");

    RunLua(vm, "json-round-trip", R"LUA(
        local c = {}
        c.self = c
        assert(toJSON(c, true) == '[{"self":"^T^0"}]', "cycle becomes a table reference")
        local back = fromJSON(toJSON(c, true))
        assert(back.self == back, "table reference reads back as the same table")
        local t = {name = "x", list = {1, 2, 3}}
        local r = fromJSON(toJSON(t, true))
        assert(r.name == "x" and r.list[3] == 3, "round trip")
    )LUA");

    if (catalogPath) {
        // samp-map-loader/models.lua: fromJSON(readFile("catalog.json")), then catalog.models[...].
        std::string code = std::string("local f = assert(io.open('") + catalogPath + "', 'rb'))\n" +
            R"LUA(
            local text = f:read('a')
            f:close()
            local cat = fromJSON(text)
            assert(type(cat) == 'table' and cat.format == 1 and type(cat.models) == 'table', 'catalog format')
            local count = 0
            for _ in pairs(cat.models) do count = count + 1 end
            assert(count == cat.modelCount, 'model count: ' .. count .. ' vs ' .. tostring(cat.modelCount))
            local m = cat.models['11682']
            assert(m.dff == 'CutsceneCouch1.dff' and m.txd == 'csmafcouch', 'model names')
            assert(m.animation == nil and m.supported == true and m.flags == 0, 'null, boolean and integer fields')
            assert(m.drawDistance == 50 and math.type(m.drawDistance) == 'integer', 'float literal 50.0')
            _G.catalogModels = count
        )LUA";
        RunLua(vm, "catalog", code);
    }
}

bool Has(const std::string& text, const char* needle) {
    return text.find(needle) != std::string::npos;
}

// Element, player, bit and vehicle helpers that do not need a GTA native.
void CheckHelpers(WorldRPG::Script::LuaVMStub& vm) {
    RunLua(vm, "helpers", R"LUA(
        local me = {id = 777, element = true}
        assert(isElement(me) == true, "local player is an element")
        assert(getPlayerName(me) == "Sam", "nick from the join")
        assert(getPlayerFromName("sam") == me or getPlayerFromName("sam").id == 777, "case-insensitive nick")
        assert(getPlayerFromNick("Sam").id == 777, "getPlayerFromNick")
        assert(getPlayerNametagText(me) == "Sam", "nametag starts as the nick")
        assert(setPlayerNametagText(me, "Tag") == true, "set nametag")
        assert(getPlayerNametagText(me) == "Tag", "nametag override")
        assert(setPlayerNametagText(me, "") == false, "empty nametag rejected")
        assert(getPlayerNametagText(me) == "Tag", "rejected text is not stored")
        assert(getPlayerTeam(me) == false, "no team is stored")
        assert(type(getPlayerPing(me)) == "number", "ping is a number")
        assert(destroyElement(me) == false and isElement(me) == true, "local player is not deleted")
        assert(destroyElement(false) == false and destroyElement({}) == false, "non-element")
        local root = getRootElement()
        assert(isElement(root) == true and destroyElement(root) == false and isElement(root) == true, "root stays")
        assert(destroyElement(root) == false, "root is still there")
        setElementData(me, "coin", 3)
        setElementData(me, "label", "x")
        local data = getAllElementData(me)
        assert(type(data) == "table" and data.coin == 3 and data.label == "x", "own element data")
        assert(getAllElementData(false) == false, "getAllElementData needs an element")
        assert(bitAnd(7, 3) == 3 and bitOr(1, 2, 4) == 7 and bitXor(7, 1) == 6, "bit and/or/xor")
        assert(bitNot(0) == 4294967295, "bitNot")
        assert(bitTest(1, 1) == true and bitTest(1, 2) == false, "bitTest")
        assert(bitLShift(1, 4) == 16 and bitRShift(16, 2) == 4, "shifts")
        assert(bitLRotate(1, 1) == 2 and bitRRotate(1, 1) == 2147483648, "rotates")
        assert(bitExtract(255, 0, 4) == 15 and bitReplace(255, 0, 0, 4) == 240, "extract and replace")
        assert(bitAnd(true) == false and bitTest(true) == nil, "bad bit arguments")
        assert(getVehicleType(400) == "Automobile" and getVehicleType(460) == "Plane", "vehicle type")
        assert(getVehicleType(481) == "BMX" and getVehicleType(520) == "Plane", "more types")
        assert(getVehicleType(700) == "" and getVehicleType("nope") == false, "bad vehicle type")
        local sound = playSound("http://example/login.mp3", false)
        assert(type(sound) == "table" and isElement(sound) == true, "playSound stays an element")
        assert(getElementType(sound) == "sound", "sound type")
        assert(setSoundVolume(sound, 0.65) == true, "setSoundVolume")
        local volume = getSoundVolume(sound)
        assert(type(volume) == "number" and math.abs(volume - 0.65) < 0.01, "getSoundVolume")
        assert(stopSound(sound) == true and isElement(sound) == false, "stopSound removes it")
        assert(stopSound(sound) == false, "stopSound on a stopped sound")
        local px, py, pz, lx, ly, lz, roll, fov = getCameraMatrix()
        assert(setCameraMatrix(10, 20, 30, 11, 21, 31, 0, 80) == true, "setCameraMatrix")
        px, py, pz, lx, ly, lz, roll, fov = getCameraMatrix()
        assert(px == 10 and ly == 21 and fov == 80, "matrix stored")
        assert(setCameraMatrix(1, 2, 3, 4, 5, 6, 0, 0) == true, "fov 0 becomes 70")
        px, py, pz, lx, ly, lz, roll, fov = getCameraMatrix()
        assert(fov == 70, "clamped fov")
        local objects, vehicles = getCameraClip()
        assert(objects == true and vehicles == true, "clip defaults")
        assert(setCameraClip(false, true) == true, "setCameraClip")
        objects, vehicles = getCameraClip()
        assert(objects == false and vehicles == true, "clip stored")
        assert(setCameraViewMode(2, 1) == true, "view mode")
        local vehicleMode, pedMode = getCameraViewMode()
        assert(vehicleMode == 2 and pedMode == 1, "view mode stored")
        assert(fadeCamera(true, 0) == true, "fadeCamera")
        assert(setCameraDrunkLevel(40) == true and getCameraDrunkLevel() == 40, "drunk level")
        assert(setCameraDrunkLevel(300) == false, "drunk level range")
        assert(setCameraGoggleEffect("nightvision") == true, "goggles")
        assert(getCameraGoggleEffect() == "nightvision", "goggle read")
        assert(setCameraGoggleEffect("nope") == false, "bad goggles")
        assert(shakeCamera(1.5) == true and resetShakeCamera() == true, "shake")
        assert(getElementType(getCamera()) == "camera" and isElement(getCamera()) == true, "camera element")
        assert(acquireScriptCamera() == false, "no resource cannot lease the camera")
        local timer = setTimer(function() end, 1000, 1)
        assert(isTimer(timer) == true, "live timer")
        assert(killTimer(timer) == true and isTimer(timer) == false, "killed timer")
        assert(isTimer("no") == false and killTimer(false) == false, "bad timer")
        assert(setElementAlpha(me, 40) == true and getElementAlpha(me) == 40, "alpha")
        assert(setElementAlpha(me, 300) == false, "alpha range")
        assert(setElementInterior(me, 3) == true and getElementInterior(me) == 3, "interior")
        assert(setElementInterior(me, 300) == false, "interior range")
        assert(setElementDimension(me, 9) == true and getElementDimension(me) == 9, "dimension")
        assert(setElementDimension(me, -1) == false and setElementDimension(me, 70000) == false, "dimension range")
        assert(isElementFrozen(me) == false, "not frozen")
        assert(setElementHealth(me, 50) == true and getElementHealth(me) == 50, "health")
        assert(type(getElementModel(me)) == "number", "model")
        local rx, ry, rz = getElementRotation(me)
        assert(type(rz) == "number" and rx == 0 and ry == 0, "rotation")
        assert(setGravity(0.01) == true and math.abs(getGravity() - 0.01) < 0.0001, "gravity")
        assert(setGravity(2) == false, "gravity range")
        assert(setGameSpeed(2) == true and getGameSpeed() == 2, "game speed")
        assert(setGameSpeed(11) == false, "game speed range")
        assert(setMinuteDuration(2000) == true and getMinuteDuration() == 2000, "minute")
        assert(setMinuteDuration(0) == false, "minute range")
        assert(setFarClipDistance(500) == true and getFarClipDistance() == 500, "far clip")
        assert(resetFarClipDistance() == true and getFarClipDistance() == 800, "far clip reset")
        assert(setFogDistance(80) == true and getFogDistance() == 80, "fog")
        assert(resetFogDistance() == true and getFogDistance() == 150, "fog reset")
        assert(setWeather(7) == true, "weather")
        local weather, blend = getWeather()
        assert(weather == 7 and blend == nil, "weather read")
        assert(setSoundLooped(sound, true) == false, "stopped sound does not loop")
        local live = playSound("http://example/loop.mp3", false)
        assert(isSoundLooped(live) == false, "not looped")
        assert(setSoundLooped(live, true) == true and isSoundLooped(live) == true, "loop")
        assert(setSoundPaused(live, true) == true and isSoundPaused(live) == true, "pause")
        assert(setSoundPaused(live, false) == true and isSoundPaused(live) == false, "resume")
        assert(setSoundPosition(live, 1.5) == true, "sound position")
        assert(getSoundPosition(live) >= 1.5, "sound position read")
        assert(playSoundFrontEnd(40) == true and playSoundFrontEnd(200) == false, "frontend sound")
        local held = setTimer(function() end, 5000, 2)
        assert(setTimerPaused(held, true) == true and isTimerPaused(held) == true, "timer pause")
        local left, repeats, delay = getTimerDetails(held)
        assert(type(left) == "number" and repeats == 2 and delay == 5000, "timer details")
        assert(resetTimer(held) == true, "timer reset")
        assert(type(getTimers(0)) == "table", "timer list")
        assert(getEasingValue(0.5, "Linear") == 0.5, "easing")
        local ix, iy, iz = interpolateBetween(0, 0, 0, 10, 0, 0, 0.5, "Linear")
        assert(ix == 5 and iy == 0 and iz == 0, "interpolate")
        assert(getRealTime().year >= 126, "real time")
        local parts = split("a,b", ",")
        assert(parts[1] == "a" and parts[2] == "b", "split")
        local cr, cg, cb = getColorFromString("#ff0000")
        assert(cr == 255 and cg == 0 and cb == 0, "color")
        assert(utfLen("é") == 1 and utfSub("éx", 2) == "x", "utf")
        assert(utfChar(65) == "A" and utfCode("A") == 65 and utfSeek("A", 1) == 1, "utf code")
        assert(getVersion().os == "Android", "version")
        assert(getWeaponNameFromID(22) == "Colt 45", "weapon name")
        assert(getWeaponIDFromName("AK-47") == 30 and getSlotFromWeapon(31) == 5, "weapon id")
        assert(setElementCollisionsEnabled(me, false) == true and getElementCollisionsEnabled(me) == false, "collisions")
        assert(setNearClipDistance(1) == true and getNearClipDistance() == 1, "near clip")
        assert(setCloudsEnabled(false) == true and getCloudsEnabled() == false, "clouds")
        assert(setBirdsEnabled(false) == true and getBirdsEnabled() == false, "birds")
        assert(setRainLevel(0.5) == true and getRainLevel() == 0.5, "rain")
        assert(setBlurLevel(10) == true and getBlurLevel() == 10, "blur")
        assert(setMoonSize(4) == true and getMoonSize() == 4, "moon")
        assert(setSunSize(2) == true and getSunSize() == 2, "sun size")
        assert(setSunColor(1, 2, 3) == true, "sun color")
        assert(setSkyGradient(1, 2, 3, 4, 5, 6, 7, 8) == true and resetSkyGradient() == true, "sky")
        assert(setWindVelocity(1, 0, 0) == true, "wind")
        assert(setWaveHeight(1) == true and getWaveHeight() == 1, "waves")
        assert(setFPSLimit(60) == true and getFPSLimit() == 60, "fps")
        assert(setFPSLimit(10) == true and getFPSLimit() == 10, "fps low")
        assert(setFPSLimit(-1) == false, "fps range")
        assert(md5("") == "d41d8cd98f00b204e9800998ecf8427e", "md5")
        assert(sha256("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "sha256")
        assert(hash("md5", "a") == md5("a"), "hash")
        assert(base64Decode(base64Encode("hello")) == "hello", "base64")
        assert(teaDecode(teaEncode("1234", "key"), "key") == "1234", "tea")
        assert(gettok("a,b,c", 2, ",") == "b", "gettok")
        assert(pregFind("abc", "b") == true, "preg find")
        assert(pregReplace("a1b", "\\d", "x") == "axb", "preg replace")
        local matched = pregMatch("a1b2", "\\d")
        assert(matched[1] == "1" and matched[2] == "2", "preg match")
        local refId = ref("kept")
        assert(deref(refId) == "kept", "ref")
        local compiled, loadError = loadstring("return 7")
        assert(loadError == nil and compiled() == 7, "loadstring")
        assert(dofile("x") == false, "unsafe dofile")
        assert(getBodyPartName(3) == "Torso" and getClothesTypeName(1) == "Hair", "names")
        assert(isOOPEnabled() == false and getLocalization().code == "en_US", "locale")
        assert(setWorldSpecialPropertyEnabled("hovercars", true) == true and isWorldSpecialPropertyEnabled("hovercars") == true, "world prop")
        assert(resetRainLevel() == true and resetBlurLevel() == true, "world reset")
        assert(setJetpackMaxHeight(40) == true and getJetpackMaxHeight() == 40, "jetpack height")
        assert(setJetpackMaxHeight(-21) == false, "jetpack range")
        assert(setElementDoubleSided(me, true) == true and isElementDoubleSided(me) == true, "double sided")
        assert(setSoundMaxDistance(live, 30) == true and getSoundMaxDistance(live) == 30, "sound distance")
        local created = fileCreate("note.txt")
        assert(type(created) == "number" and fileWrite(created, "hi") == 2 and fileGetPos(created) == 2, "file write")
        assert(fileSetPos(created, 0) == 0 and fileRead(created, 2) == "hi", "file read back")
        assert(fileClose(created) == true and fileGetContents("note.txt") == "hi", "file read")
        assert(fileExists("note.txt") == true and pathIsFile("note.txt") == true, "file exists")
        assert(fileDelete("note.txt") == true and fileExists("note.txt") == false, "file delete")
        assert(fileOpen("../secret") == false, "file sandbox")
        local dotted = fileCreate("a..b.txt")
        assert(type(dotted) == "number" and fileClose(dotted) == true and fileDelete("a..b.txt") == true, "dots in a file name")
        local matrix = getElementMatrix(me)
        assert(type(matrix) == "table" and type(matrix[4][1]) == "number" and matrix[4][4] == 1, "matrix")
        assert(setHeatHaze(10) == true and getHeatHaze() == 10, "heat haze")
        assert(resetHeatHaze() == true, "heat haze reset")
        assert(setColorFilter(1, 2, 3, 4, 5, 6, 7, 8) == true, "color filter")
        local fr = getColorFilter()
        assert(fr == 1, "color filter read")
        assert(setTrafficLightState(2) == true and getTrafficLightState() == 2, "lights")
        assert(setTrafficLightState("green", "red") == true and getTrafficLightState() == 0 and areTrafficLightsLocked() == true, "light colors")
        assert(setTrafficLightState("auto") == true and areTrafficLightsLocked() == false, "light auto")
        assert(setTimeFrozen(true) == true and isTimeFrozen() == true and resetTimeFrozen() == true, "frozen time")
        assert(setAircraftMaxVelocity(50) == true and getAircraftMaxVelocity() == 50, "aircraft speed")
        assert(setAircraftMaxVelocity(-1) == false, "aircraft range")
        assert(setCoronaReflectionsEnabled(1) == true and getCoronaReflectionsEnabled() == 1, "corona")
        assert(setCoronaReflectionsEnabled(3) == false, "corona range")
        assert(setWorldProperty("SpriteSize", 2) == true and getWorldProperty("SpriteSize") == 2, "world property")
        local ar, ag, ab = getWorldProperty("AmbientColor")
        assert(setWorldProperty("AmbientColor", 1, 2, 3) == true and ar ~= nil, "ambient before")
        ar, ag, ab = getWorldProperty("AmbientColor")
        assert(ar == 1 and ag == 2 and ab == 3, "ambient")
        assert(setWorldProperty("nope", 1) == false, "bad property")
        assert(setWeatherBlended(8) == true, "blend")
        local current, blend = getWeather()
        assert(blend == 8 and type(current) == "number", "blend read")
        assert(setGarageOpen(1, true) == true and isGarageOpen(1) == true, "garage")
        assert(setDynamicPedShadowsEnabled(false) == true and isDynamicPedShadowsEnabled() == false, "shadows")
        local xml = xmlLoadString('<meta><info name="x">hi</info></meta>')
        assert(xmlNodeGetName(xml) == "meta", "xml name")
        local info = xmlFindChild(xml, "info", 0)
        assert(xmlNodeGetAttribute(info, "name") == "x" and xmlNodeGetValue(info) == "hi", "xml child")
        local script = xmlCreateChild(xml, "script")
        assert(xmlNodeSetAttribute(script, "src", "a.lua") == true, "xml attr")
        assert(xmlNodeGetName(xmlNodeGetParent(script)) == "meta", "xml parent")
        assert(xmlSaveFile(xml, "meta.xml") == true and xmlLoadFile("meta.xml") ~= false, "xml file")
        assert(xmlUnloadFile(xml) == true and xmlNodeGetName(xml) == false, "xml unload")
        assert(setWaterLevel(5) == true and resetWaterLevel() == true, "water level")
        assert(setOcclusionRegion(0, 0, 10, 10) == 1 and resetOcclusionRegion() == true, "occlusion")
        assert(setDevelopmentMode(true) == true and getDevelopmentMode() == true, "dev mode")
        assert(setTransferBoxVisible(true) == true and isTransferBoxVisible() == true, "transfer")
        assert(setNeonClientSetting("a", "b") == true and getNeonClientSetting("a") == "b", "neon setting")
        assert(resetNeonClientSettings() == true and getNeonClientSetting("a") == false, "neon reset")
        assert(setClipboard("hi") == true, "clipboard")
        assert(type(getProcessMemoryStats()) == "table", "memory")
        assert(getKeyboardLayout() == "ltr" and isVoiceEnabled() == false, "client flags")
        assert(getUserdataType(me) == false, "userdata type")
        assert(setWeaponRenderEnabled(false) == true and isWeaponRenderEnabled() == false, "weapon render")
        local ran = false
        assert(addCommandHandler("hi", function() ran = true end) == true, "command add")
        assert(executeCommandHandler("hi", "there") == true and ran == true, "command run")
        assert(getCommandHandlers()[1][1] == "hi", "command list")
        assert(removeCommandHandler("hi") == true and executeCommandHandler("hi") == false, "command remove")
        local hashed = fileCreate("hash.txt")
        assert(fileWrite(hashed, "abc") == 3 and fileFlush(hashed) == true, "hash write")
        assert(fileGetHash(hashed, "md5") == md5("abc"), "file hash")
        assert(fileClose(hashed) == true and fileDelete("hash.txt") == true, "hash cleanup")
        local foliage = createFoliage(0, 0, 0, 1, 0, 0, 0, 1, 0, 3, 0.5)
        assert(isElement(foliage) == true and getElementType(foliage) == "foliage", "foliage element")
        assert(getFoliageSurface(foliage) == 3 and getFoliageDensity(foliage) == 0.5, "foliage values")
        assert(setFoliageSurface(foliage, 300) == false, "foliage surface range")
        assert(setFoliageDensity(foliage, 2) == true and getFoliageDensity(foliage) == 2, "foliage density")
        local first = getFoliageVertices(foliage)
        assert(first.x == 0 and first.y == 0 and first.z == 0, "foliage vertex")
        local effect = createEffect("prt_blood", 1, 2, 3)
        assert(getElementType(effect) == "effect", "effect element")
        assert(setEffectSpeed(effect, 2) == true and getEffectSpeed(effect) == 2, "effect speed")
        assert(setEffectDensity(effect, 0.25) == true and getEffectDensity(effect) == 0.25, "effect density")
        assert(destroyElement(foliage) == true and isElement(foliage) == false, "foliage destroy")
        assert(createFire(0, 0, 1, 2) == true and createFire("no") == false, "fire")
        local texture, model = getClothesByTypeIndex(0, 0)
        assert(texture == "player_torso" and model == "torso", "clothes index")
        local clothesType, clothesIndex = getTypeIndexFromClothes("foot", "feet")
        assert(clothesType == 3 and clothesIndex == 0, "clothes lookup")
        assert(getClothesByTypeIndex(0, 999) == false, "clothes range")
        clothesType, clothesIndex = getTypeIndexFromClothes("player_torso")
        assert(clothesType == 0 and clothesIndex == 0, "clothes texture")
        assert(getTypeIndexFromClothes("Player_Torso", "torso") == false, "clothes case")
        assert(getWeaponProperty(22, "std", "damage") == 25, "colt damage")
        assert(getWeaponProperty(22, "std", "weapon_range") == 35, "colt range")
        assert(getOriginalWeaponProperty(23, "poor", "damage") == 40, "silenced poor")
        assert(getWeaponProperty(32, "pro", "damage") == 20, "tec9 pro")
        assert(setWeaponProperty(22, "std", "damage", 99) == true, "weapon set")
        assert(getWeaponProperty(22, "std", "damage") == 99, "weapon override")
        assert(getOriginalWeaponProperty(22, "std", "damage") == 25, "weapon original")
        assert(getWeaponProperty(22, "std", "flag_aim_1st_person") == false, "colt aim flag")
        assert(setWeaponProperty(22, "std", "flag_aim_1st_person", true) == true, "colt flag set")
        assert(getWeaponProperty(22, "std", "flag_aim_1st_person") == true, "colt flag read")
        assert(getWeaponProperty(22, "std", "flags") == 12343, "colt flags word")
        assert(setWeaponProperty(22, "std", "flag_aim_1st_person", true) == false, "colt flag already set")
        assert(getOriginalWeaponProperty(22, "std", "flags") == 12339, "colt flags original")
        assert(getWeaponProperty(22, "std", "nope") == false and getWeaponProperty(0, "std", "damage") == false, "weapon range")
        assert(addClothingModel("customtex", "custommod", 0) == true, "add clothes")
        local addedType, addedIndex = getTypeIndexFromClothes("customtex", "custommod")
        assert(addedType == 0 and addedIndex == 68, "added clothes index")
        local addedTexture, addedModel = getClothesByTypeIndex(0, 68)
        assert(addedTexture == "customtex" and addedModel == "custommod", "added clothes read")
        assert(removeClothingModel("customtex", "custommod", 0) == true, "remove clothes")
        assert(getTypeIndexFromClothes("customtex", "custommod") == false, "removed clothes")
        local function onRender() end
        assert(addEventHandler("onClientRender", me, onRender) == true, "add handler")
        assert(addEventHandler("onClientRender", me, onRender) == false, "duplicate handler")
        local handlers = getEventHandlers("onClientRender", me)
        local found = false
        if type(handlers) == "table" then
            for _, fn in ipairs(handlers) do if fn == onRender then found = true end end
        end
        assert(found, "event handlers")
        local rootHandlers = getEventHandlers("onClientRender", root)
        local onRoot = false
        if type(rootHandlers) == "table" then
            for _, fn in ipairs(rootHandlers) do if fn == onRender then onRoot = true end end
        end
        assert(onRoot == false, "handler stays on the player")
        assert(getEventHandlers("onClientRender") == false, "event handlers need an element")
        local hit = false
        -- CEvents: a custom event exists only after addEvent (CStaticFunctionDefinitions.cpp:115-174).
        assert(addEventHandler("onTest", me, function() end) == false, "handler needs an added event")
        assert(triggerEvent("onTest", me) == nil, "trigger needs an added event")
        assert(addEvent() == false and addEvent("") == false and addEvent("onTest", 1) == false, "addEvent arguments")
        assert(addEvent("onTest") == true and addEvent("onTest", false) == true, "addEvent twice")
        assert(addEvent("onTest", true) == false, "remote flag cannot change")
        assert(addEvent("onRemoteTest", true) == true and addEvent("onRemoteTest") == false, "remote event")
        assert(addEvent("onClientRender") == true and addEvent("onClientRender", true) == false, "built-in event")
        addEventHandler("onTest", me, function(n) hit = n == 5 end)
        assert(triggerEvent("onTest", me, 5) == true and hit == true, "triggerEvent")
        assert(addEvent("onStop") and addEvent("onOuter") and addEvent("onBubble") and addEvent("onPlayerOnly"), "test events")
        addEventHandler("onStop", me, function() cancelEvent() end)
        assert(triggerEvent("onStop", me) == false and wasEventCancelled() == true, "trigger cancel")
        local nested = true
        addEventHandler("onOuter", me, function() nested = triggerEvent("onStop", me) end)
        assert(triggerEvent("onOuter", me) == true and nested == false, "nested cancel")
        local playerOnly = false
        local function onlyMe() playerOnly = true end
        assert(addEventHandler("onPlayerOnly", me, onlyMe) == true, "player handler")
        assert(triggerEvent("onPlayerOnly", root) == true and playerOnly == false, "element filter")
        assert(removeEventHandler("onPlayerOnly", root, onlyMe) == false, "remove is per element")
        assert(removeEventHandler("onPlayerOnly", me, onlyMe) == true, "remove on the element")
        local bubbled = false
        addEventHandler("onBubble", root, function() bubbled = true end)
        assert(triggerEvent("onBubble", me) == true and bubbled == true, "bubble to root")
        assert(triggerEvent("onTest") == nil, "trigger needs an element")
    )LUA");
}

// Screen is the default 1920x1080 (no surface size is set in the host build).
void CheckGui(WorldRPG::Script::LuaVMStub& vm) {
    RunLua(vm, "gui-create", R"LUA(
        win = guiCreateWindow(10, 20, 300, 200, "Hello Window", false)
        lab = guiCreateLabel(5, 40, 100, 20, "Child label", false, win)
        btn = guiCreateButton(0.5, 0.5, 0.25, 0.1, "OK", true)
        assert(getElementType(win) == "gui-window" and getElementType(lab) == "gui-label", 'types')
        assert(getElementType(btn) == "gui-button", 'button type')
        assert(isElement(win) and isElement(lab) and isElement(btn), 'gui elements are elements')
        assert(guiGetVisible(win) == true and guiGetVisible(lab) == true, 'visible on create')
        assert(guiGetText(win) == "Hello Window" and guiGetText(lab) == "Child label", 'text')
        assert(guiGetEnabled(btn) == true and guiGetAlpha(win) == 1, 'enabled and opaque on create')
        local x, y = guiGetPosition(btn, true)
        assert(x == 0.5 and y == 0.5, 'relative position kept')
        local px, py = guiGetPosition(btn, false)
        assert(px == 960 and py == 540, 'absolute position of relative element: ' .. px .. ' ' .. py)
        local w, h = guiGetSize(btn, false)
        assert(w == 480 and h == 108, 'absolute size of relative element')
        assert(showCursor(true) == true and isCursorShowing() == true, 'cursor flag')
        local bar = guiCreateProgressBar(0, 0, 100, 20, false)
        assert(guiProgressBarSetProgress(bar, 40) == true and guiProgressBarGetProgress(bar) == 40, 'progress')
        local a = guiCreateRadioButton(0, 0, 80, 20, 'A', false)
        local b = guiCreateRadioButton(0, 0, 80, 20, 'B', false)
        assert(guiRadioButtonSetSelected(a, true) == true and guiRadioButtonSetSelected(b, true) == true, 'radios')
        assert(guiRadioButtonGetSelected(a) == false and guiRadioButtonGetSelected(b) == true, 'one radio')
        local scroll = guiCreateScrollBar(0, 0, 100, 16, true, false)
        assert(guiScrollBarSetScrollPosition(scroll, 150) == true and guiScrollBarGetScrollPosition(scroll) == 100, 'scrollbar')
        local edit = guiCreateEdit(0, 0, 80, 20, 'ab', false)
        assert(guiEditSetMaxLength(edit, 2) == true and guiSetText(edit, 'abcd') == true and guiGetText(edit) == 'ab', 'edit length')
        assert(guiEditSetCaretIndex(edit, 1) == true and guiEditGetCaretIndex(edit) == 1, 'caret')
        assert(guiProgressBarSetProgress(bar, 40.9) == true and guiProgressBarGetProgress(bar) == 40, 'progress truncates')
        local dotted = guiCreateStaticImage(0, 0, 8, 8, 'a..b.png', false)
        assert(isElement(dotted) == true and guiCreateStaticImage(0, 0, 8, 8, '../x.png', false) == false, 'image path')
        assert(guiEditSetReadOnly(edit, true) == true and guiEditIsReadOnly(edit) == true, 'readonly')
        local memo = guiCreateMemo(0, 0, 80, 40, 'note', false)
        assert(guiMemoSetVerticalScrollPosition(memo, 10) == true and guiMemoGetVerticalScrollPosition(memo) == 10, 'memo')
        assert(guiWindowSetMovable(win, false) == true and guiWindowIsMovable(win) == false, 'window')
        assert(guiLabelSetColor(lab, 1, 2, 3) == true, 'label color')
        local cr, cg, cb = guiLabelGetColor(lab)
        assert(cr == 1 and cg == 2 and cb == 3, 'label color read')
        guiLabelSetColor(lab, 255, 255, 255)
        local panel = guiCreateTabPanel(0, 0, 100, 80, false)
        local tab = guiCreateTab('One', panel)
        assert(guiGetSelectedTab(panel).id == tab.id, 'first tab')
        local tab2 = guiCreateTab('Two', panel)
        assert(guiSetSelectedTab(panel, tab2) == true and guiGetSelectedTab(panel).id == tab2.id, 'select tab')
        assert(guiDeleteTab(tab, panel) == true and isElement(tab) == false, 'delete tab')
        assert(guiSetProperty(btn, 'AlwaysOnTop', 'True') == true and guiGetProperty(btn, 'AlwaysOnTop') == 'True', 'property')
        assert(guiMoveToBack(btn) == true, 'move back')
        local image = guiCreateStaticImage(0, 0, 10, 10, 'face.png', false)
        assert(isElement(image) == true and guiStaticImageGetNativeSize(image) == false, 'static image')
        assert(guiSetFont(lab, 'default-bold') == true and guiGetFont(lab) == 'default-bold', 'font')
        assert(guiGetCursorType() == 'arrow', 'cursor')
        assert(guiCreateFont('../x.ttf') == false and guiCreateFont('face.ttf', 12) == false, 'font file')
        local grid = guiCreateGridList(0, 0, 200, 100, false)
        assert(getElementType(grid) == 'gui-gridlist', 'grid type')
        assert(guiGridListAddColumn(grid, 'Name', 0.5) == 1 and guiGridListAddColumn(grid, 'Score', 0.3) == 2, 'columns')
        assert(guiGridListGetColumnCount(grid) == 2 and guiGridListGetColumnTitle(grid, 1) == 'Name', 'column title')
        assert(guiGridListGetColumnWidth(grid, 1, true) == 0.5 and guiGridListGetColumnWidth(grid, 1, false) == 100, 'column width')
        assert(guiGridListSetColumnWidth(grid, 1, 40, false) == true and guiGridListGetColumnWidth(grid, 1, false) == 40, 'absolute width')
        local row = guiGridListAddRow(grid, nil, 'B')
        assert(row == 0 and guiGridListGetItemText(grid, 0, 1) == 'B' and guiGridListGetItemText(grid, 9, 1) == '', 'skipped value')
        assert(guiGridListAddRow(grid, 'Sam', 3) == 1 and guiGridListGetItemText(grid, 1, 2) == '3', 'row text')
        assert(guiGridListSetItemData(grid, 1, 1, 'id') == true and guiGridListGetItemData(grid, 1, 1) == 'id', 'item data')
        assert(guiGridListGetItemData(grid, 0, 1) == nil, 'empty data')
        assert(guiGridListSetItemColor(grid, 1, 1, 1, 2, 3, 4) == true, 'item color')
        local cr, cg, cb, ca = guiGridListGetItemColor(grid, 1, 1)
        assert(cr == 1 and cg == 2 and cb == 3 and ca == 4, 'item color read')
        local sr, sc = guiGridListGetSelectedItem(grid)
        assert(sr == -1 and sc == 0 and guiGridListGetSelectionMode(grid) == 0, 'no selection')
        assert(guiGridListSetSelectedItem(grid, 1, 2) == true, 'select')
        sr, sc = guiGridListGetSelectedItem(grid)
        assert(sr == 1 and sc == 1 and guiGridListGetSelectedCount(grid) == 2, 'selected row')
        local picked = guiGridListGetSelectedItems(grid)
        assert(picked[1].row == 1 and picked[1].column == 0 and picked[2].column == 1, 'selected table')
        assert(guiGridListInsertRowAfter(grid, -1, 'Top', 1) == 0 and guiGridListGetItemText(grid, 0, 1) == 'Top', 'insert')
        assert(guiGridListGetItemText(grid, 2, 1) == 'Sam' and guiGridListGetRowCount(grid) == 3, 'shifted row')
        assert(guiGridListSetSelectionMode(grid, 2) == true and guiGridListGetSelectionMode(grid) == 2, 'cell mode')
        assert(guiGridListSetSelectedItem(grid, 2, 2, true) == true, 'select cell')
        sr, sc = guiGridListGetSelectedItem(grid)
        assert(sr == 2 and sc == 2 and guiGridListGetSelectedCount(grid) == 1, 'one cell')
        assert(guiGridListSetSelectionMode(grid, 1) == true, 'multi row')
        assert(guiGridListSetSelectedItem(grid, 0, 1, false) == true and guiGridListGetSelectedCount(grid) == 3, 'extra selection')
        assert(guiGridListSetSelectionMode(grid, 0) == true and guiGridListSetSelectedItem(grid, 1, 1, false) == true, 'single mode')
        assert(guiGridListGetSelectedCount(grid) == 2, 'single replaces')
        assert(guiGridListIsSortingEnabled(grid) == true and guiGridListSetSortingEnabled(grid, false) == true, 'sort flag')
        assert(guiGridListIsSortingEnabled(grid) == false and guiGridListGetItemText(grid, 0, 1) == 'Top', 'sort keeps order')
        assert(guiGridListSetVerticalScrollPosition(grid, 150) == true and guiGridListGetVerticalScrollPosition(grid) == 100, 'grid scroll')
        assert(guiGridListSetScrollBars(grid, true, false) == true and guiGridListAutoSizeColumn(grid, 1) == true, 'bars and autosize')
        assert(guiGridListRemoveRow(grid, 0) == true and guiGridListGetItemText(grid, 0, 1) == 'B', 'remove row')
        assert(guiGridListRemoveColumn(grid, 2) == true and guiGridListGetColumnCount(grid) == 1, 'remove column')
        assert(guiGridListSetItemText(grid, 9, 1, 'no', false, false) == true, 'missing row')
        assert(guiGridListRemoveRow(grid, 50) == true and guiGridListGetRowCount(grid) == 2, 'missing remove')
        assert(guiGridListClear(grid) == true and guiGridListGetRowCount(grid) == 0, 'clear')
    )LUA");

    std::string frame = WorldRPG::Script::ActiveVmPump();
    CHECK(Has(frame, "R 10.0 20.0 300.0 200.0 "));
    CHECK(Has(frame, "Hello Window"));
    CHECK(Has(frame, "T 15.0 60.0 115.0 80.0 1.00 4294967295 0 Child label"));
    CHECK(Has(frame, "R 960.0 540.0 480.0 108.0 "));

    // A window hidden by the script is no longer drawn, and neither is its child.
    RunLua(vm, "gui-hide", "guiSetVisible(win, false)\nassert(guiGetVisible(win) == false)");
    frame = WorldRPG::Script::ActiveVmPump();
    CHECK(!Has(frame, "Hello Window"));
    CHECK(!Has(frame, "Child label"));
    CHECK(Has(frame, "R 960.0 540.0 480.0 108.0 "));  // the button is a separate element

    // Shown again: the window and its child come back, with the window alpha inherited.
    RunLua(vm, "gui-show", "guiSetVisible(win, true)\nguiSetAlpha(win, 0.5)");
    frame = WorldRPG::Script::ActiveVmPump();
    CHECK(Has(frame, "Hello Window"));
    CHECK(Has(frame, "Child label"));
    CHECK(Has(frame, "2147483647 0 Child label"));  // white text at alpha 0.5 (127 of 255)
    RunLua(vm, "gui-alpha-reset", "guiSetAlpha(win, 1)");

    // Font scale: desktop pricedown is 30 px, so the draw line carries scale 2.
    RunLua(vm, "dx-font", R"LUA(
        assert(dxGetFontHeight(1, "default") == 15 and dxGetFontHeight(1, "pricedown") == 30)
        assert(dxGetFontHeight(2) == 30, 'default height at scale 2')
        -- dx calls are only valid in onClientRender: draw once from a render handler.
        local function once()
            dxDrawText("BIG", 0, 0, 10, 10, tocolor(255, 255, 255, 255), 1, "pricedown")
            dxDrawText("SMALL", 0, 0, 10, 10, tocolor(255, 255, 255, 255), 1, "default")
            removeEventHandler("onClientRender", root, once)
        end
        addEventHandler("onClientRender", root, once)
    )LUA");
    frame = WorldRPG::Script::ActiveVmPump();
    CHECK(Has(frame, "T 0.0 0.0 10.0 10.0 2.00 "));
    CHECK(Has(frame, "T 0.0 0.0 10.0 10.0 1.00 "));

    // destroyElement removes the element and its children; the element is no longer an element.
    RunLua(vm, "gui-destroy", "destroyElement(win)\nassert(not isElement(win) and not isElement(lab), 'children destroyed')");
    frame = WorldRPG::Script::ActiveVmPump();
    CHECK(!Has(frame, "Hello Window"));
    CHECK(!Has(frame, "Child label"));
    CHECK(Has(frame, "R 960.0 540.0 480.0 108.0 "));
    RunLua(vm, "gui-cleanup", "destroyElement(btn)\nassert(not isElement(btn), 'button destroyed')");
    frame = WorldRPG::Script::ActiveVmPump();
    CHECK(!Has(frame, "R 960.0 540.0"));

    // A tap on a gui-edit focuses it, the frame asks for the soft keyboard ("K"), and
    // typed text stays in the edit (onClientCharacter does not also see it).
    RunLua(vm, "gui-edit", R"LUA(
        edit = guiCreateEdit(10, 10, 200, 40, "", false)
        assert(getElementType(edit) == "gui-edit" and guiGetText(edit) == "", "edit")
        assert(guiSetInputMode("no_binds") == true and guiGetInputMode() == "no_binds", "no_binds")
        assert(guiGetInputEnabled() == true, "capture on")
        assert(guiSetInputMode("nope") == false, "bad mode")
        assert(guiSetInputMode("no_binds_when_editing") == true, "restore mode")
        assert(guiGetInputEnabled() == false, "capture off without a focused edit")
        _G.changed, _G.accepted, _G.focused, _G.blurred = 0, 0, 0, 0
        _G.chars, _G.keys = "", ""
        addEventHandler("onClientGUIChanged", root, function() _G.changed = _G.changed + 1 end)
        addEventHandler("onClientGUIAccepted", root, function() _G.accepted = _G.accepted + 1 end)
        addEventHandler("onClientGUIFocus", root, function() _G.focused = _G.focused + 1 end)
        addEventHandler("onClientGUIBlur", root, function() _G.blurred = _G.blurred + 1 end)
        addEventHandler("onClientCharacter", root, function(c) _G.chars = _G.chars .. c end)
        addEventHandler("onClientKey", root, function(button, press)
            if press then _G.keys = _G.keys .. button .. "," end
        end)
        showCursor(true)
    )LUA");
    WorldRPG::Script::ActiveVmQueueCursor(0, 20.f, 25.f);
    frame = WorldRPG::Script::ActiveVmPump();
    CHECK(Has(frame, "\nK\n"));
    CHECK(Has(frame, "|"));
    RunLua(vm, "gui-edit-focus", R"LUA(
        assert(_G.focused == 1 and guiGetInputEnabled() == true, "tap focused the edit")
        assert(guiFocus(edit) == true, "guiFocus")
    )LUA");
    WorldRPG::Script::ActiveVmQueueScriptInput("ab", 0, "");
    WorldRPG::Script::ActiveVmPump();
    RunLua(vm, "gui-edit-type", R"LUA(
        assert(guiGetText(edit) == "ab" and _G.changed == 1, "typed into the edit")
        assert(_G.chars == "", "a focused edit does not emit onClientCharacter")
    )LUA");
    WorldRPG::Script::ActiveVmQueueScriptInput("", 1, "enter");
    WorldRPG::Script::ActiveVmPump();
    RunLua(vm, "gui-edit-enter", R"LUA(
        assert(guiGetText(edit) == "a" and _G.accepted == 1, "backspace and enter")
        assert(guiEditSetMasked(edit, true) == true and guiEditIsMasked(edit) == true, "masked")
    )LUA");
    frame = WorldRPG::Script::ActiveVmPump();
    CHECK(Has(frame, "**|") == false);
    CHECK(Has(frame, "*|"));

    // DX edits (the login boxes) are not widgets. guiSetInputEnabled(true) opens the
    // keyboard and the keystrokes are onClientCharacter / onClientKey.
    RunLua(vm, "dx-capture", "guiBlur(edit)\nguiSetInputEnabled(true)");
    WorldRPG::Script::ActiveVmQueueScriptInput("Z", 0, "");
    WorldRPG::Script::ActiveVmQueueScriptInput("", 1, "");
    WorldRPG::Script::ActiveVmQueueScriptInput("", 0, "enter");
    frame = WorldRPG::Script::ActiveVmPump();
    CHECK(Has(frame, "\nK\n"));
    RunLua(vm, "dx-keys", R"LUA(
        assert(_G.chars == "Z", "dx character: " .. _G.chars)
        assert(_G.keys == "backspace,enter,", "dx keys: " .. _G.keys)
        assert(_G.blurred == 1, "blur")
        guiSetInputEnabled(false)
        destroyElement(edit)
    )LUA");
    frame = WorldRPG::Script::ActiveVmPump();
    CHECK(!Has(frame, "\nK\n"));
}

// Client window state calls (desktop CLuaGUIDefs.cpp:642-700, 3631-3748, 4066-4086).
void CheckMtaWindow(WorldRPG::Script::LuaVMStub& vm) {
    RunLua(vm, "mta-window-state", R"LUA(
        assert(isChatBoxInputActive() == false and isConsoleActive() == false, 'chat input and console closed')
        assert(isMainMenuActive() == false and isTransferBoxActive() == false, 'menu and transfer box closed')
        assert(isMTAWindowActive() == false, 'no MTA window open')
        assert(isDebugViewActive() == false, 'debug view starts hidden')
        assert(setDebugViewActive() == false and setDebugViewActive(1) == false, 'debug view needs a boolean')
        assert(setDebugViewActive(true) == true and isDebugViewActive() == true, 'debug view on')
        assert(setDebugViewActive(false) == true and isDebugViewActive() == false, 'debug view off')
    )LUA");
    WorldRPG::Script::MtaWindowSetChatInputActive(true);
    RunLua(vm, "mta-window-chat-open",
           "assert(isChatBoxInputActive() == true and isMTAWindowActive() == true, 'chat input open')");
    WorldRPG::Script::MtaWindowSetChatInputActive(false);
    RunLua(vm, "mta-window-chat-closed",
           "assert(isChatBoxInputActive() == false and isMTAWindowActive() == false, 'chat input closed')");
    RunLua(vm, "chatbox-character-limit", R"LUA(
        assert(getChatboxCharacterLimit() == 96, 'default limit')
        assert(setChatboxCharacterLimit(120) == true and getChatboxCharacterLimit() == 120, 'limit set')
        assert(setChatboxCharacterLimit('64') == true and getChatboxCharacterLimit() == 64, 'numeric string')
        assert(setChatboxCharacterLimit(0) == true and getChatboxCharacterLimit() == 0, 'limit 0')
        assert(setChatboxCharacterLimit(255) == true and getChatboxCharacterLimit() == 255, 'limit max')
        local ok, err = pcall(setChatboxCharacterLimit, 256)
        assert(not ok and err:find('less than, or equal to 255 %(got: 256%)'), 'over max: ' .. tostring(err))
        ok, err = pcall(setChatboxCharacterLimit, -2)
        assert(not ok and err:find('greater than, or equal to 0 %(got: %-2%)'), 'negative: ' .. tostring(err))
        assert(not pcall(setChatboxCharacterLimit) and not pcall(setChatboxCharacterLimit, 'abc'), 'bad argument')
        assert(getChatboxCharacterLimit() == 255, 'rejected values keep the limit')
        assert(setChatboxCharacterLimit(-1) == true and getChatboxCharacterLimit() == 96, '-1 resets')
    )LUA");
    RunLua(vm, "chatbox-layout", R"LUA(
        local all = getChatboxLayout()
        local count = 0
        for _ in pairs(all) do count = count + 1 end
        assert(count == 21, 'layout holds 21 values, got ' .. count)
        assert(all.chat_lines == 10 and all.chat_font == 2 and all.chat_width == 1.5, 'numbers')
        assert(all.chat_use_cegui == false and all.chat_text_outline == 0 and all.chat_css_style_text == 0, 'only cegui is boolean')
        local c = all.chat_text_color
        assert(#c == 4 and c[1] == 172 and c[2] == 213 and c[3] == 254 and c[4] == 255, 'text colour')
        assert(#all.chat_scale == 2 and all.chat_scale[1] == 1 and all.chat_scale[2] == 1, 'scale')
        assert(getChatboxLayout('chat_lines') == 10 and getChatboxLayout('CHAT_LINES') == 10, 'one value, any case')
        assert(#getChatboxLayout('chat_input_color') == 4, 'one colour')
        assert(getChatboxLayout('text_scale') == 1 and getChatboxLayout('nope') == false, 'unknown name')
        assert(getChatboxLayout({}) == false and getChatboxLayout(nil) == false, 'bad argument')
    )LUA");
    // Occupancy (CLuaPedDefs.cpp:834-880, 1214-1238; CLuaVehicleDefs.cpp:692-777). The host has no
    // vehicle element, so only the on-foot and bad-argument results are checked here.
    {
        // A player or a vehicle that leaves takes its seats with it, so a reused id starts on foot.
        uint32_t vehicle = 0;
        uint8_t seat = 0;
        WorldRPG::RemoteVehicle::SetOccupant(50, 60, 1);
        WorldRPG::RemoteVehicle::SetOccupant(51, 60, 2);
        WorldRPG::RemoteVehicle::ForgetElement(50);
        CHECK(!WorldRPG::RemoteVehicle::FindOccupant(50, vehicle, seat));
        CHECK(WorldRPG::RemoteVehicle::FindOccupant(51, vehicle, seat) && vehicle == 60 && seat == 2);
        WorldRPG::RemoteVehicle::ForgetElement(60);
        CHECK(!WorldRPG::RemoteVehicle::FindOccupant(51, vehicle, seat));
    }
    RunLua(vm, "occupancy-on-foot", R"LUA(
        local me = {id = 777, element = true}
        assert(getPedOccupiedVehicle(me) == false, 'local player on foot')
        assert(getPedOccupiedVehicleSeat(me) == false, 'no seat on foot')
        assert(isPedInVehicle(me) == false, 'not in a vehicle')
        assert(isPedInVehicle() == nil and isPedInVehicle({}) == nil, 'bad ped is nil')
        assert(getPedOccupiedVehicle() == false and getPedOccupiedVehicleSeat(5) == false, 'bad ped is false')
        assert(getVehicleOccupant(me) == false and getVehicleOccupants() == false, 'not a vehicle')
        assert(getVehicleController({}) == false, 'not a vehicle')
    )LUA");
    // Key binds (CLuaFunctionDefs.Input.cpp 148-276, 420-655; CScriptKeyBinds.cpp).
    RunLua(vm, "bind-key", R"LUA(
        _G.keyLog = {}
        _G.onE = function(key, state, extra) keyLog[#keyLog + 1] = key .. ':' .. state .. ':' .. tostring(extra) end
        _G.onBoth = function(key, state) keyLog[#keyLog + 1] = 'both:' .. key .. ':' .. state end
        assert(bindKey() == false and bindKey('e') == false and bindKey('e', 'down') == false, 'bad arguments')
        assert(bindKey('nokey', 'down', onE) == false, 'unknown key')
        assert(bindKey('e', 'sideways', onE) == false, 'unknown state')
        assert(bindKey('E', 'down', onE, 'x') == true, 'key name is case-insensitive')
        assert(bindKey('F2', 'both', onBoth) == true, 'both states')
        assert(bindKey('fire', 'down', onBoth) == true, 'a control name binds')
        assert(#getFunctionsBoundToKey('e') == 1 and getFunctionsBoundToKey('e')[1] == onE, 'functions of e')
        assert(#getFunctionsBoundToKey('E') == 0, 'getter uses the table spelling')
        assert(#getFunctionsBoundToKey('F2') == 2 and #getFunctionsBoundToKey('F2', 'up') == 1, 'state filter')
        assert(getKeyBoundToFunction(onE) == 'e' and getKeyBoundToFunction(print) == false, 'key of function')
        assert(getKeyBoundToFunction('x') == false, 'needs a function')
        assert(getKeyState('e') == false and getKeyState('nokey') == false, 'nothing pressed')
        assert(bindKey('p', 'down', 'panel', 'open') == true and bindKey('p', 'down', 'panel', 'open') == true, 'command bind')
        assert(getCommandsBoundToKey('p').panel == 'open' and getKeyBoundToCommand('panel') == 'p', 'command getters')
        assert(getKeyBoundToCommand('nothing') == false, 'unbound command')
        addCommandHandler('panel', function(name, args) keyLog[#keyLog + 1] = 'cmd:' .. name .. ':' .. tostring(args) end)
        addEventHandler('onClientKey', root, function(key, press)
            _G.keySource = source
            if key == 'F3' then cancelEvent() end
        end)
        local cmd = function(name, a, b, c) _G.lastCmd = name .. '|' .. tostring(a) .. '|' .. tostring(b) .. '|' .. tostring(c) end
        assert(addCommandHandler('Greet', cmd, false) == true, 'command added')
        assert(addCommandHandler('greet', cmd, false) == false, 'same function is not added twice')
        addEventHandler('onClientConsole', root, function(text) _G.consoleText = text; _G.consoleSource = source end)
        assert(bindKey('F3', 'down', onBoth) == true, 'F3 bound')
        -- A bind removed by an earlier bind of the same key press is not called.
        _G.second = function() keyLog[#keyLog + 1] = 'second' end
        assert(bindKey('F4', 'down', function() unbindKey('F4', 'down', second) end) == true, 'F4 first')
        assert(bindKey('F4', 'down', second) == true, 'F4 second')
    )LUA");
    // The on-screen key list shows key binds only (not the "fire" control), in table order.
    CHECK(WorldRPG::Script::KeyBindsBoundKeys() == "e|p|F2|F3|F4");
    WorldRPG::Script::KeyBindsQueueKey("E", true);
    WorldRPG::Script::KeyBindsQueueKey("f2", true);
    WorldRPG::Script::KeyBindsQueueKey("f2", false);
    WorldRPG::Script::KeyBindsQueueKey("p", true);
    WorldRPG::Script::KeyBindsQueueKey("F3", true);
    WorldRPG::Script::KeyBindsQueueKey("F8", true);
    WorldRPG::Script::KeyBindsQueueKey("F4", true);
    // A chat command reaches the client handlers, one argument per word, then onClientConsole.
    WorldRPG::Script::ActiveVmQueueCommand("greet  one two");
    WorldRPG::Script::ActiveVmPump();
    {
        // The key-bound command is handed back for the server, as a typed command is sent.
        const std::vector<std::string> toServer = WorldRPG::Script::ActiveVmTakeServerCommands();
        CHECK(toServer.size() == 1 && toServer[0] == "panel open");
        CHECK(WorldRPG::Script::ActiveVmTakeServerCommands().empty());
    }
    RunLua(vm, "chat-command", R"LUA(
        assert(lastCmd == 'Greet|one|two|nil', 'arguments are split on spaces: ' .. tostring(lastCmd))
        assert(consoleText == 'greet  one two', 'onClientConsole text: ' .. tostring(consoleText))
        assert(consoleSource and consoleSource.id == 777, 'onClientConsole source is the local player')
        assert(keySource and keySource.id == root.id, 'onClientKey source is root')
    )LUA");
    WorldRPG::Script::ActiveVmQueueCommand("login secret");
    WorldRPG::Script::ActiveVmPump();
    RunLua(vm, "chat-command-login", "assert(consoleText == 'login ***', 'login is censored: ' .. tostring(consoleText))");
    RunLua(vm, "bind-key-fired", R"LUA(
        local got = table.concat(keyLog, ' ')
        assert(got == 'e:down:x both:F2:down both:F2:up cmd:panel:open', 'binds fired: ' .. got)
        assert(getKeyState('e') == true and getKeyState('F2') == false, 'key state')
        assert(unbindKey('e', 'up') == false and unbindKey('e', 'down', print) == false, 'no such bind')
        assert(unbindKey('e', 'down', onE) == true and #getFunctionsBoundToKey('e') == 0, 'unbind function')
        assert(unbindKey('F2') == true and #getFunctionsBoundToKey('F2') == 0, 'unbind every state')
        assert(unbindKey('F2') == false and unbindKey('nokey') == false, 'nothing left')
        assert(unbindKey('p', 'down', 'panel') == true and getKeyBoundToCommand('panel') == false, 'unbind command')
        _G.keyLog = {}
    )LUA");
    WorldRPG::Script::KeyBindsQueueKey("e", false);
    WorldRPG::Script::KeyBindsQueueKey("f2", true);
    WorldRPG::Script::ActiveVmPump();
    CHECK(WorldRPG::Script::KeyBindsBoundKeys() == "F3|F4");
    RunLua(vm, "bind-key-removed", "assert(#keyLog == 0 and getKeyState('e') == false, 'removed binds do not fire')");
    RunLua(vm, "chatbox-limit-set", "assert(setChatboxCharacterLimit(40) == true)");
    WorldRPG::Script::MtaWindowResetSession();
    RunLua(vm, "chatbox-limit-session", "assert(getChatboxCharacterLimit() == 96, 'a new session resets the limit')");
}

void CheckStopLifecycle(WorldRPG::Script::LuaVMStub& vm) {
    // Resource "demo" (net id 5, root 0x1234) and "other" (net id 6, root 0x2222). Both start
    // from one no-cache script each (packet 95).
    WorldRPG::Net::ResourceStartInfo demo;
    demo.name = "demo";
    demo.resourceId = 5;
    demo.entityId = 0x1234;
    demo.dynamicEntityId = 0x1235;
    demo.noClientCacheScripts = 1;
    demo.startCounter = 3;
    demo.exportedFunctions = {"ping"};
    WorldRPG::Net::ResourceStartInfo other = demo;
    other.name = "other";
    other.resourceId = 6;
    other.entityId = 0x2222;
    other.dynamicEntityId = 0x2223;
    other.startCounter = 1;

    WorldRPG::Script::ActiveVmOnResourceStart(demo);
    WorldRPG::Script::ActiveVmOnResourceStart(other);

    const std::string demoScript = R"LUA(
        _G.demoStarts = (_G.demoStarts or 0) + 1
        addEventHandler("onClientResourceStart", root, function(started)
            _G.demoStarted = true
            if getResourceName(started) == "demo" then
                _G.demoStartArg = started
                _G.demoStartSource = source
            end
        end)
        addEventHandler("onClientRender", root, function()
            _G.demoRenders = (_G.demoRenders or 0) + 1
            _G.demoSelf = getThisResource()
        end)
        addEventHandler("onClientResourceStop", root, function(res)
            _G.demoStopped = res
            _G.demoStopSource = source
            _G.demoStopState = getResourceState(getThisResource())
        end)
        local created = createElement("coin")
        _G.demoCreated = created
        _G.demoParent = getElementParent(created)
        showCursor(true)
        local res = getThisResource()
        _G.demoRes = res
        _G.demoResName = getResourceName(res)
        _G.demoResRoot = getResourceRootElement(res)
        _G.demoDyn = getResourceDynamicElementRoot(res)
        local maps = 0
        for _, child in ipairs(getElementChildren(_G.demoResRoot)) do
            if getElementType(child) == "map" then maps = maps + 1 end
        end
        _G.demoMaps = maps
        _G.demoGui = getResourceGUIElement()
        _G.demoChild = guiCreateWindow(0, 0, 40, 40, "child", false, _G.demoGui)
        _G.demoExports = getResourceExportedFunctions(res)
        _G.demoState = getResourceState(res)
        assert(addEvent("onMine") == true, "resource event")
        addCommandHandler("democmd", function(name, args) _G.demoCmd = (getThisResource() == _G.demoRes) and args end)
        addEventHandler("onMine", root, function() _G.demoSource = sourceResource == getThisResource() end)
        triggerEvent("onMine", root)
        setTimer(function() _G.demoTicks = (_G.demoTicks or 0) + 1 end, 1, 0)
        _G.demoWin = guiCreateWindow(0, 0, 50, 50, "demo window", false)
    )LUA";
    const std::string otherScript = R"LUA(
        addEventHandler("onClientRender", root, function() _G.otherRenders = (_G.otherRenders or 0) + 1 end)
    )LUA";
    const std::vector<uint8_t> demoBody = NoCacheScriptBody(5, demoScript);
    const std::vector<uint8_t> otherBody = NoCacheScriptBody(6, otherScript);
    CHECK(WorldRPG::Script::ActiveVmFeedScripts(demoBody.data(), demoBody.size(), demoBody.size() * 8));
    CHECK(WorldRPG::Script::ActiveVmFeedScripts(otherBody.data(), otherBody.size(), otherBody.size() * 8));

    WorldRPG::Script::ActiveVmPulseResources();
    RunLua(vm, "after-start", R"LUA(
        assert(_G.demoStarted == true and _G.demoStarts == 1, "demo started once")
        assert(_G.demoStartArg == _G.demoRes, "start passes the resource")
        assert(_G.demoStartSource.id == 0x1234, "start source is the root")
        assert(_G.demoResName == "demo" and _G.demoState == "running", "this resource")
        assert(_G.demoRes == getResourceFromName("demo"), "same resource handle")
        assert(isElement(_G.demoResRoot) and _G.demoResRoot.id == 0x1234, "resource root")
        assert(getElementType(_G.demoResRoot) == "resource", "resource type")
        assert(isElement(_G.demoDyn) and _G.demoDyn.id == 0x1235 and getElementType(_G.demoDyn) == "map", "dynamic root")
        assert(_G.demoParent.id == _G.demoDyn.id and _G.demoMaps == 1, "one dynamic map")
        assert(isElement(_G.demoGui) and getElementType(_G.demoGui) == "guiroot", "gui root")
        assert(isElement(_G.demoChild), "window parented to gui root")
        assert(_G.demoSource == true, "sourceResource")
        assert(_G.demoExports[1] == "ping", "exports")
        assert(getResourceName(getResourceFromName("other")) == "other", "resource by name")
        assert(getResourceFromName("missing") == false, "unknown resource")
        local names = {}
        for _, res in ipairs(getResources()) do names[getResourceName(res)] = true end
        assert(names.demo and names.other, "resource list")
        assert(getThisResource() == false, "core code has no resource")
    )LUA");

    WorldRPG::Net::ResourceStartInfo again = demo;
    again.resourceId = 7;
    again.entityId = 0x1236;
    again.dynamicEntityId = 0x1237;
    again.noClientCacheScripts = 1;
    again.exportedFunctions = {"pong"};
    WorldRPG::Script::ActiveVmOnResourceStart(again);
    const std::vector<uint8_t> againBody = NoCacheScriptBody(7, "showCursor(true)\n");
    CHECK(WorldRPG::Script::ActiveVmFeedScripts(againBody.data(), againBody.size(), againBody.size() * 8));
    WorldRPG::Script::ActiveVmPulseResources();

    WorldRPG::Script::ActiveVmPump();
    WorldRPG::Script::ActiveVmPump();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    WorldRPG::Script::ActiveVmPump();
    RunLua(vm, "before-stop",
           "assert(_G.demoRenders == 3 and _G.otherRenders == 3, 'both resources render')\n"
           "assert(_G.demoTicks and _G.demoTicks >= 1, 'demo timer fired')\n"
           "assert(_G.demoSelf == _G.demoRes, 'handler stays on the old resource')\n"
           "assert(getResourceFromName('demo') ~= _G.demoRes, 'name finds the new start')\n"
           "assert(getResourceExportedFunctions(getResourceFromName('demo'))[1] == 'pong', 'new exports')\n"
           "assert(getResourceState(_G.demoRes) == 'running', 'old one still running')\n"
           "assert(getResourceRootElement(_G.demoRes).id == 0x1234, 'old root')\n"
           "_G.snapTicks, _G.snapRenders = _G.demoTicks, _G.demoRenders");

    // A command handler runs as the resource that registered it, and leaves with it.
    CHECK(WorldRPG::Script::ActiveVmRunCommand("democmd", "hello"));
    RunLua(vm, "command-owner", "assert(_G.demoCmd == 'hello', 'command ran as its resource: ' .. tostring(_G.demoCmd))");

    // RESOURCE_STOP for net id 5: CResourceStopPacket writes the id as a uint16.
    WorldRPG::Net::BitStreamWriter stop;
    stop.Write(static_cast<uint16_t>(5));
    WorldRPG::Script::ActiveVmOnResourceStop(stop.GetData(), stop.GetByteLength(), stop.GetBitLength());
    RunLua(vm, "stop-event",
           "assert(_G.demoStopped == _G.demoRes, 'stop passes the resource')\n"
           "assert(_G.demoStopSource.id == 0x1234, 'stop source is the root')\n"
           "assert(_G.demoStopState == 'stopping', 'stop state')\n"
           "assert(getResourceFromName('demo') ~= _G.demoRes, 'old handle is not the current demo')\n"
           "assert(not isElement(_G.demoResRoot), 'old root is gone')\n"
           "assert(not isElement(_G.demoCreated), 'client elements are destroyed')\n"
           "assert(not isElement(_G.demoGui) and not isElement(_G.demoChild), 'old gui is gone')\n"
           "assert(isCursorShowing() == true, 'the other start still holds the cursor')");
    RunLua(vm, "stop-gui", "assert(not isElement(_G.demoWin), 'stopped resource GUI elements are destroyed')");

    // The stopped resource's handlers and timers are gone; the other resource still runs.
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    WorldRPG::Script::ActiveVmPump();
    WorldRPG::Script::ActiveVmPump();
    RunLua(vm, "after-stop",
           "assert(_G.demoRenders == _G.snapRenders, 'demo render handler dropped')\n"
           "assert(_G.demoTicks == _G.snapTicks, 'demo timer dropped')\n"
           "assert(_G.otherRenders == 5, 'other resource keeps rendering')");
    CHECK(!WorldRPG::Script::ActiveVmRunCommand("democmd", "again"));
    // CEvents::RemoveAllEvents(CLuaMain*): the stopped resource's event is gone, a core one stays.
    CHECK(!WorldRPG::Script::Events::Exists("onMine"));
    CHECK(WorldRPG::Script::Events::Exists("onTest"));
    CHECK(WorldRPG::Script::Events::RemoteState("onMine") == WorldRPG::Script::Events::Remote::NotAdded);
    CHECK(WorldRPG::Script::Events::RemoteState("onTest") == WorldRPG::Script::Events::Remote::NotRemote);
    CHECK(WorldRPG::Script::Events::RemoteState("onRemoteTest") == WorldRPG::Script::Events::Remote::Allowed);
    CHECK(WorldRPG::Script::Events::RemoteState("onClientRender") == WorldRPG::Script::Events::Remote::NotRemote);

    // A second RESOURCE_STOP for the same id is unknown and does nothing.
    WorldRPG::Script::ActiveVmOnResourceStop(stop.GetData(), stop.GetByteLength(), stop.GetBitLength());
    RunLua(vm, "unknown-stop", "assert(_G.otherRenders == 5, 'unknown stop changes nothing')");

    // Too short a body is rejected without touching the VM.
    const uint8_t oneByte[] = {0x05};
    WorldRPG::Script::ActiveVmOnResourceStop(oneByte, sizeof(oneByte), 8);
    RunLua(vm, "short-stop", "assert(_G.otherRenders == 5, 'short body changes nothing')");
}

// dxDrawImage writes one "I" line per call. With no resource running, the call draws nothing
// and rectangles still do. A resource resolves the PNG through the file cache.
void CheckDxImage(WorldRPG::Script::LuaVMStub& vm) {
    WorldRPG::Net::HttpDownloader::SetResourceCacheRoot("/tmp/neon-dx-cache");

    RunLua(vm, "dx-image-noscript", R"LUA(
        local function once()
            dxDrawRectangle(1, 2, 3, 4, tocolor(255, 255, 255, 255))
            dxDrawText("plain", 0, 0, 10, 10)
            _G.imageWithoutScript = dxDrawImage(1, 2, 3, 4, "radar/radar00.png")
            _G.sectionWithoutScript = dxDrawImageSection(1, 2, 3, 4, 0, 0, 8, 8, "radar/radar00.png")
            removeEventHandler("onClientRender", root, once)
        end
        addEventHandler("onClientRender", root, once)
    )LUA");
    std::string frame = WorldRPG::Script::ActiveVmPump();
    CHECK(Has(frame, "R 1.0 2.0 3.0 4.0 "));
    CHECK(Has(frame, "plain"));
    CHECK(frame.find("\nI ") == std::string::npos);
    CHECK(frame.rfind("I ", 0) != 0);
    RunLua(vm, "dx-image-noscript-result",
           "assert(_G.imageWithoutScript == false and _G.sectionWithoutScript == false, 'no resource draws no image')");

    WorldRPG::Net::ResourceStartInfo info;
    info.name = "radarres";
    info.resourceId = 9;
    info.entityId = 0x4444;
    info.dynamicEntityId = 0x4445;
    info.noClientCacheScripts = 1;
    info.startCounter = 1;
    WorldRPG::Script::ActiveVmOnResourceStart(info);
    const std::string script = R"LUA(
        addEventHandler("onClientRender", root, function()
            if _G.radarDrew then return end
            _G.radarDrew = true
            dxDrawRectangle(8, 9, 10, 11, tocolor(0, 0, 0, 255))
            dxDrawText("keep", 0, 0, 10, 10)
            _G.fullImage = dxDrawImage(1, 2, 3, 4, "radar/radar00.png", 0, 0, 0, tocolor(255, 255, 255, 255))
            _G.sectionImage = dxDrawImageSection(5, 6, 7, 8, 9, 10, 11, 12, "radar/radar01.png", 90, 1, 2, tocolor(255, 0, 0, 255))
            _G.escapedImage = dxDrawImage(1, 2, 3, 4, "../secret.png")
            _G.pedRotation = getPedRotation(localPlayer)
            _G.velX, _G.velY, _G.velZ = getElementVelocity(localPlayer)
            _G.pedArmor = getPedArmor(localPlayer)
        end)
    )LUA";
    const std::vector<uint8_t> body = NoCacheScriptBody(9, script);
    CHECK(WorldRPG::Script::ActiveVmFeedScripts(body.data(), body.size(), body.size() * 8));
    WorldRPG::Script::ActiveVmPulseResources();
    frame = WorldRPG::Script::ActiveVmPump();
    CHECK(Has(frame, "R 8.0 9.0 10.0 11.0 "));
    CHECK(Has(frame, "keep"));
    CHECK(Has(frame, "I 1.0 2.0 3.0 4.0 0.0 0.0 -1.0 -1.0 4294967295 0.0 0.0 0.0 /tmp/neon-dx-cache/radarres/radar/radar00.png"));
    CHECK(Has(frame, "I 5.0 6.0 7.0 8.0 9.0 10.0 11.0 12.0 4294901760 90.0 1.0 2.0 /tmp/neon-dx-cache/radarres/radar/radar01.png"));
    CHECK(!Has(frame, "secret"));
    RunLua(vm, "dx-image-resource", R"LUA(
        assert(_G.fullImage == true and _G.sectionImage == true, 'resource image calls draw')
        assert(_G.escapedImage == false, 'parent path is rejected')
        assert(type(_G.pedRotation) == 'number' and _G.pedRotation >= 0 and _G.pedRotation < 360, 'heading in degrees')
        assert(_G.velX == 0 and _G.velY == 0 and _G.velZ == 0, 'local velocity')
        assert(_G.pedArmor == 0, 'armor sample')
    )LUA");
}

// Polls the game pulse that delivers fetchRemote completions. A failed probe is not a
// recorded check; the final RunLua reports the assertion that is still false.
bool WaitRemote(WorldRPG::Script::LuaVMStub& vm, const char* name, const std::string& assertCode, int timeoutMs) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    // One beat so a localhost callback can land before the first probe. executeBuffer logs
    // every failed probe, and the completion is usually ready within this window.
    WorldRPG::Script::ActiveVmPulseResources();
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    while (std::chrono::steady_clock::now() < deadline) {
        WorldRPG::Script::ActiveVmPulseResources();
        const WorldRPG::Script::LuaExecResult result = vm.executeBuffer(assertCode.data(), assertCode.size(), name);
        if (result.success) {
            ++g_checks;
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    WorldRPG::Script::ActiveVmPulseResources();
    return RunLua(vm, name, assertCode);
}

bool SameHttpStats(const WorldRPG::Net::HttpDownloader::Stats& before, const WorldRPG::Net::HttpDownloader::Stats& after) {
    return before.queued == after.queued && before.skippedDuplicate == after.skippedDuplicate &&
           before.cached == after.cached && before.downloaded == after.downloaded &&
           before.md5Verified == after.md5Verified && before.crcVerified == after.crcVerified &&
           before.mismatches == after.mismatches && before.failed == after.failed && before.bytes == after.bytes &&
           before.dropped == after.dropped && before.failures.size() == after.failures.size() &&
           before.samples.size() == after.samples.size();
}

// Local HTTP peer for the fetchRemote checks. Hold waits until Release before the response,
// so abort can run while the client is blocked in the read.
class LoopbackHttp {
public:
    enum class Mode { Respond, Hold };

    explicit LoopbackHttp(Mode mode) : mode_(mode) {
        listen_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (listen_ < 0) return;
        int yes = 1;
        ::setsockopt(listen_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = 0;
        if (::bind(listen_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0 || ::listen(listen_, 4) != 0) {
            ::close(listen_);
            listen_ = -1;
            return;
        }
        socklen_t len = sizeof(addr);
        if (::getsockname(listen_, reinterpret_cast<sockaddr*>(&addr), &len) != 0) {
            ::close(listen_);
            listen_ = -1;
            return;
        }
        port_ = ntohs(addr.sin_port);
        thread_ = std::thread([this] { Serve(); });
    }

    ~LoopbackHttp() {
        stop_.store(true);
        Release();
        if (listen_ >= 0) ::shutdown(listen_, SHUT_RDWR);
        if (thread_.joinable()) thread_.join();
        if (listen_ >= 0) {
            ::close(listen_);
            listen_ = -1;
        }
    }

    uint16_t port() const { return port_; }
    bool ok() const { return port_ != 0; }

    bool WaitAccepted(int timeoutMs) {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
        while (std::chrono::steady_clock::now() < deadline) {
            if (accepted_.load() > 0) return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return accepted_.load() > 0;
    }

    void Release() {
        release_.store(true);
        releaseCv_.notify_all();
    }

private:
    void Serve() {
        while (!stop_.load()) {
            pollfd pfd{};
            pfd.fd = listen_;
            pfd.events = POLLIN;
            if (::poll(&pfd, 1, 50) <= 0) continue;
            const int client = ::accept(listen_, nullptr, nullptr);
            if (client < 0) continue;
            std::string request;
            char buf[1024];
            while (request.find("\r\n\r\n") == std::string::npos && request.size() < 8192 && !stop_.load()) {
                pollfd in{};
                in.fd = client;
                in.events = POLLIN;
                if (::poll(&in, 1, 200) <= 0) continue;
                const ssize_t n = ::recv(client, buf, sizeof(buf), 0);
                if (n <= 0) break;
                request.append(buf, static_cast<size_t>(n));
            }
            if (mode_ == Mode::Hold) {
                accepted_.fetch_add(1);
                std::unique_lock<std::mutex> lock(releaseMu_);
                releaseCv_.wait(lock, [this] { return release_.load() || stop_.load(); });
            }
            if (!stop_.load()) {
                const char response[] = "HTTP/1.1 200 OK\r\nContent-Length: 7\r\nConnection: close\r\n\r\nok-body";
                ::send(client, response, sizeof(response) - 1, MSG_NOSIGNAL);
            }
            ::close(client);
        }
    }

    Mode mode_;
    int listen_ = -1;
    uint16_t port_ = 0;
    std::atomic<bool> stop_{false};
    std::atomic<bool> release_{false};
    std::atomic<int> accepted_{0};
    std::mutex releaseMu_;
    std::condition_variable releaseCv_;
    std::thread thread_;
};

uint16_t ClosedTcpPort() {
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return 0;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        ::close(fd);
        return 0;
    }
    socklen_t len = sizeof(addr);
    if (::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len) != 0) {
        ::close(fd);
        return 0;
    }
    const uint16_t port = ntohs(addr.sin_port);
    ::close(fd);
    return port;
}

// Enough of an MTA peer for RakNetWrapper::SendPacket to pass isConnected().
// The handshake bytes are unchanged; this only answers OPEN and the connection request.
class LoopbackMta {
public:
    LoopbackMta() {
        fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (fd_ < 0) return;
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = 0;
        if (::bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
            ::close(fd_);
            fd_ = -1;
            return;
        }
        socklen_t len = sizeof(addr);
        if (::getsockname(fd_, reinterpret_cast<sockaddr*>(&addr), &len) != 0) {
            ::close(fd_);
            fd_ = -1;
            return;
        }
        port_ = ntohs(addr.sin_port);
        key_ = {1, 2, 3, 4, 5, 6, 7, 8};
        thread_ = std::thread([this] { Serve(); });
    }

    ~LoopbackMta() {
        stop_.store(true);
        if (thread_.joinable()) thread_.join();
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
    }

    uint16_t port() const { return port_; }
    bool ok() const { return port_ != 0; }
    int datagrams() const { return datagrams_.load(); }
    int opened() const { return opened_.load(); }
    int frames() const { return frames_.load(); }
    int accepts() const { return accepts_.load(); }

private:
    static std::vector<uint8_t> OpenReply(const std::vector<uint8_t>& key) {
        using WorldRPG::Net::Mta::OfflineMagic;
        std::vector<uint8_t> reply;
        reply.push_back(0x0a);
        reply.insert(reply.end(), OfflineMagic.begin(), OfflineMagic.end());
        reply.insert(reply.end(), 16, 0);
        reply.push_back(0xed);
        reply.push_back(0x80);
        reply.push_back(1);
        reply.push_back(1);
        reply.push_back(0x6e);
        reply.push_back(static_cast<uint8_t>(key.size()));
        reply.insert(reply.end(), key.begin(), key.end());
        reply.push_back(0x1a);
        reply.push_back(0);
        reply.push_back(0xde);
        return reply;
    }

    void Serve() {
        using namespace WorldRPG::Net::Mta;
        std::optional<Cipher> cipher;
        uint8_t selector = 1;
        uint32_t nextNumber = 0;
        bool sentAccept = false;
        const std::vector<uint8_t> reply = OpenReply(key_);
        while (!stop_.load()) {
            pollfd pfd{};
            pfd.fd = fd_;
            pfd.events = POLLIN;
            if (::poll(&pfd, 1, 20) <= 0) continue;
            uint8_t buf[2048];
            sockaddr_in from{};
            socklen_t fromLen = sizeof(from);
            const ssize_t n = ::recvfrom(fd_, buf, sizeof(buf), 0, reinterpret_cast<sockaddr*>(&from), &fromLen);
            if (n <= 0) continue;
            datagrams_.fetch_add(1);
            if (buf[0] == 0x09) {
                ::sendto(fd_, reply.data(), reply.size(), 0, reinterpret_cast<sockaddr*>(&from), fromLen);
                opened_.fetch_add(1);
                continue;
            }
            if (!cipher) cipher = Cipher::Create(key_);
            if (!cipher) continue;
            const auto plain = cipher->Open(std::span<const uint8_t>(buf, static_cast<size_t>(n)));
            const auto* bytes = std::get_if<Bytes>(&plain);
            if (!bytes) continue;
            const auto decoded = DecodeFrame(*bytes);
            const auto* frame = std::get_if<ReliableFrame>(&decoded);
            if (!frame) continue;
            frames_.fetch_add(1);
            ReliableFrame out;
            out.remoteTime = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                                        std::chrono::steady_clock::now().time_since_epoch())
                                                        .count());
            for (const ReliableMessage& message : frame->messages) {
                if (message.reliability >= 2) out.acknowledgments.push_back(AckRange{message.number, message.number});
            }
            // EncodeFrame rejects acknowledgments without ackTime. The client stamps remoteTime
            // on the connection request; that value is the ack time it expects.
            if (!out.acknowledgments.empty()) out.ackTime = frame->remoteTime.value_or(0);
            const bool includeAccept = !sentAccept;
            if (includeAccept) {
                ReliableMessage accept;
                accept.number = nextNumber++;
                accept.reliability = 2;
                accept.bitLength = 15 * 8;
                accept.payload.assign(15, 0);
                accept.payload[0] = 14;
                out.messages.push_back(std::move(accept));
            }
            const auto encoded = EncodeFrame(out);
            const auto* wire = std::get_if<Bytes>(&encoded);
            if (!wire) continue;
            const Bytes sealed = cipher->Seal(*wire, selector++);
            if (::sendto(fd_, sealed.data(), sealed.size(), 0, reinterpret_cast<sockaddr*>(&from), fromLen) < 0) continue;
            if (includeAccept) {
                sentAccept = true;
                accepts_.fetch_add(1);
            }
        }
    }

    int fd_ = -1;
    uint16_t port_ = 0;
    std::vector<uint8_t> key_;
    std::atomic<bool> stop_{false};
    std::atomic<int> datagrams_{0};
    std::atomic<int> opened_{0};
    std::atomic<int> frames_{0};
    std::atomic<int> accepts_{0};
    std::thread thread_;
};

void CheckNetworkCalls(WorldRPG::Script::LuaVMStub& vm) {
    CHECK(WorldRPG::Net::NormVectorBytesMatchVehicleSync());

    {
        LoopbackHttp server(LoopbackHttp::Mode::Respond);
        CHECK(server.ok());
        if (server.ok()) {
            const std::string url = "http://127.0.0.1:" + std::to_string(server.port()) + "/ok";
            RunLua(vm, "fetch-ok-start",
                   "_G.okCalls = 0\n"
                   "local req = fetchRemote(\"" +
                       url +
                       "\", {queueName = \"ok\", connectionAttempts = 1, connectTimeout = 2000}, "
                       "function(body, info)\n"
                       "  _G.okCalls = _G.okCalls + 1\n"
                       "  _G.okBody = body\n"
                       "  _G.okCode = info.statusCode\n"
                       "  _G.okSuccess = info.success\n"
                       "end)\n"
                       "assert(type(req) == \"table\" and req.remoteRequest == true and type(req.id) == \"number\", \"handle\")");
            WaitRemote(vm, "fetch-ok",
                       "assert(_G.okCalls == 1 and _G.okCode == 200 and _G.okSuccess == true and _G.okBody == \"ok-body\", "
                       "\"status \" .. tostring(_G.okCode) .. \" body \" .. tostring(_G.okBody))",
                       3000);
        }
    }

    {
        const uint16_t closed = ClosedTcpPort();
        CHECK(closed != 0);
        if (closed != 0) {
            const std::string url = "http://127.0.0.1:" + std::to_string(closed) + "/no";
            RunLua(vm, "fetch-refused-start",
                   "_G.refusedCalls = 0\n"
                   "fetchRemote(\"" +
                       url +
                       "\", {queueName = \"refused\", connectionAttempts = 1, connectTimeout = 1000}, "
                       "function(body, info)\n"
                       "  _G.refusedCalls = _G.refusedCalls + 1\n"
                       "  _G.refusedCode = info.statusCode\n"
                       "  _G.refusedSuccess = info.success\n"
                       "end)");
            WaitRemote(vm, "fetch-refused",
                       "assert(_G.refusedCalls == 1 and _G.refusedSuccess == false and _G.refusedCode ~= 200, "
                       "\"refused statusCode \" .. tostring(_G.refusedCode))",
                       3000);
        }
    }

    {
        LoopbackHttp server(LoopbackHttp::Mode::Hold);
        CHECK(server.ok());
        if (server.ok()) {
            const std::string url = "http://127.0.0.1:" + std::to_string(server.port()) + "/slow";
            RunLua(vm, "fetch-abort-start",
                   "_G.abortCalls = 0\n"
                   "_G.abortReq = fetchRemote(\"" +
                       url +
                       "\", {queueName = \"abortq\", connectionAttempts = 1, connectTimeout = 4000}, "
                       "function()\n"
                       "  _G.abortCalls = _G.abortCalls + 1\n"
                       "end)\n"
                       "assert(type(_G.abortReq) == \"table\" and _G.abortReq.remoteRequest == true, \"abort handle\")\n"
                       "local info = getRemoteRequestInfo(_G.abortReq)\n"
                       "assert(type(info) == \"table\" and info.resource == false, \"resource is false\")\n"
                       "local listed = false\n"
                       "for _, item in ipairs(getRemoteRequests()) do\n"
                       "  if item.id == _G.abortReq.id then listed = true end\n"
                       "end\n"
                       "assert(listed, \"live request is listed\")");
            CHECK(server.WaitAccepted(3000));
            RunLua(vm, "fetch-abort",
                   "assert(abortRemoteRequest(_G.abortReq) == true, \"abort\")\n"
                   "local still = false\n"
                   "for _, item in ipairs(getRemoteRequests()) do\n"
                   "  if item.id == _G.abortReq.id then still = true end\n"
                   "end\n"
                   "assert(still == false, \"aborted request leaves the list\")");
            server.Release();
            for (int i = 0; i < 25; ++i) {
                WorldRPG::Script::ActiveVmPulseResources();
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
            RunLua(vm, "fetch-abort-quiet", "assert(_G.abortCalls == 0, \"late completion ran the callback\")");
        }
    }

    {
        const auto before = WorldRPG::Net::HttpDownloader::GetStats();
        RunLua(vm, "fetch-https-start",
               "_G.httpsCalls = 0\n"
               "fetchRemote(\"https://127.0.0.1/no\", {queueName = \"httpsq\", connectionAttempts = 1, connectTimeout = 1000}, "
               "function(body, info)\n"
               "  _G.httpsCalls = _G.httpsCalls + 1\n"
               "  _G.httpsCode = info.statusCode\n"
               "  _G.httpsSuccess = info.success\n"
               "end)");
        WaitRemote(vm, "fetch-https",
                   "assert(_G.httpsCalls == 1 and _G.httpsSuccess == false and _G.httpsCode == 1, "
                   "\"https statusCode \" .. tostring(_G.httpsCode))",
                   1000);
        const auto after = WorldRPG::Net::HttpDownloader::GetStats();
        CHECK(SameHttpStats(before, after));
    }

    RunLua(vm, "get-server-ip",
           "assert(type(getServerIp()) == \"string\", \"getServerIp\")\n"
           "assert(type(getServerIp(true)) == \"string\", \"getServerIp with port\")");

    {
        LoopbackMta peer;
        CHECK(peer.ok());
        if (peer.ok()) {
            WorldRPG::Net::RakNetWrapper net;
            const bool started = net.StartNetwork("127.0.0.1", peer.port());
            CHECK(started);
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
            while (started && !net.IsConnected() && std::chrono::steady_clock::now() < deadline) {
                net.DoPulse();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            if (!net.IsConnected()) {
                std::printf("FAIL mta loopback not connected (datagrams=%d opened=%d frames=%d accepts=%d)\n",
                            peer.datagrams(), peer.opened(), peer.frames(), peer.accepts());
            }
            CHECK(net.IsConnected());
            if (net.IsConnected()) {
                RunLua(vm, "usage-before",
                       "_G.usageBefore = {}\n"
                       "local count = getNetworkUsageData().out.count\n"
                       "for id = 0, 255 do _G.usageBefore[id] = count[id] end");
                // sendBuf[0] is the wire id. A packet id other than USER_PACKET_ENUM (99)
                // is stored as that id, so one send of 42 increments out.count[42].
                WorldRPG::Net::BitStreamWriter stream;
                stream.Write(static_cast<uint8_t>(0x11));
                CHECK(net.SendPacket(42, stream, WorldRPG::Net::PacketPriority::HIGH,
                                     WorldRPG::Net::PacketReliability::UNRELIABLE));
                RunLua(vm, "usage-after",
                       "local count = getNetworkUsageData().out.count\n"
                       "local raised = -1\n"
                       "for id = 0, 255 do\n"
                       "  local delta = count[id] - _G.usageBefore[id]\n"
                       "  if delta == 1 then\n"
                       "    assert(raised < 0, \"two wire ids increased\")\n"
                       "    raised = id\n"
                       "  else\n"
                       "    assert(delta == 0, \"id \" .. id .. \" changed by \" .. tostring(delta))\n"
                       "  end\n"
                       "end\n"
                       "assert(raised == 42, \"wire id \" .. tostring(raised))");
            }
            net.Shutdown();
        }
    }

    WorldRPG::Script::ResourceLoader::Reset();
}

} // namespace

// ROADMAP_PEDS_SYNC phase 1: every field of the ENTITY_ADD ped body is kept, in the order of
// CPacketHandler::Packet_EntityAdd case PED (CPacketHandler.cpp 3997-4187), at the 0x0030 widths.
void CheckPedEntityBody() {
    using namespace WorldRPG;
    Net::BitStreamWriter w;
    auto bits = [&](uint32_t value, size_t count) {
        uint8_t raw[4] = {static_cast<uint8_t>(value), static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value >> 16),
                          static_cast<uint8_t>(value >> 24)};
        w.WriteBits(raw, count);
    };
    bits(static_cast<uint32_t>(100 * 1024), 24);  // SPositionSync x = 100
    bits(static_cast<uint32_t>(-50 * 1024) & 0xFFFFFFu, 24);  // y = -50
    w.Write(12.5f);                                // z
    w.WriteCompressed(61u, 2);                     // model
    bits(32768, 16);                               // SPedRotationSync, ~0 rad
    w.Write(static_cast<uint8_t>(255));            // SPlayerHealthSync -> 255
    w.Write(static_cast<uint8_t>(102));            // SPlayerArmorSync -> 51
    bits(1, 1);                                    // in vehicle
    bits(321, 17);                                 // vehicle id
    bits(2, 4);                                    // seat
    bits(0, 1); bits(1, 1); bits(1, 1); bits(1, 1);  // jetpack, synced, headless, frozen
    w.WriteCompressed(55u, 1);                     // alpha raw -> 200
    w.Write(static_cast<uint8_t>(54));             // move anim
    w.Write(static_cast<uint8_t>(1));              // one clothes item
    w.Write(static_cast<uint8_t>(5)); w.WriteBytes(reinterpret_cast<const uint8_t*>("vestw"), 5);
    w.Write(static_cast<uint8_t>(4)); w.WriteBytes(reinterpret_cast<const uint8_t*>("vest"), 4);
    w.Write(static_cast<uint8_t>(0));              // clothes type
    w.Write(static_cast<uint8_t>(2)); w.Write(static_cast<uint8_t>(22)); w.Write(static_cast<uint16_t>(170));  // slot 2: colt, 170
    w.Write(static_cast<uint8_t>(0xFF));           // end of weapons
    w.Write(static_cast<uint8_t>(2));              // current slot
    bits(1, 1);                                    // animation present
    w.Write(static_cast<uint16_t>(3)); w.WriteBytes(reinterpret_cast<const uint8_t*>("ped"), 3);
    w.Write(static_cast<uint16_t>(9)); w.WriteBytes(reinterpret_cast<const uint8_t*>("WALK_civi"), 9);
    w.Write(static_cast<int32_t>(-1));
    bits(1, 1); bits(1, 1); bits(0, 1); bits(0, 1);  // looped, updatePosition, interruptable, freezeLastFrame
    w.Write(static_cast<int32_t>(250));
    bits(1, 1);                                    // taskRestore
    w.Write(400.f); w.Write(1.5f);                 // elapsed, speed

    Net::BitStreamReader r(w.GetData(), w.GetByteLength(), w.GetBitLength());
    Entity::EntityCommon common;
    common.id = 900;
    common.type = Entity::kTypePed;
    common.collisions = false;
    common.dimension = 7;
    Entity::Visual visual;
    CHECK(Entity::ReadEntityBodyPed(r, common, visual));
    CHECK(r.GetRemainingBits() < 8);
    CHECK(visual.model == 61 && visual.x == 100.f && visual.y == -50.f && visual.z == 12.5f);
    CHECK(visual.ped != nullptr);
    if (!visual.ped) return;
    const Entity::PedSpawn& p = *visual.ped;
    CHECK(p.health == 255.f && p.armor == 51.f && p.synced);
    CHECK(p.vehicleId == 321 && p.seat == 2);
    CHECK(!p.hasJetpack && p.headless && p.frozen);
    CHECK(p.alpha == 200 && p.moveAnim == 54);
    CHECK(p.clothes.size() == 1 && p.clothes[0].texture == "vestw" && p.clothes[0].model == "vest" && p.clothes[0].type == 0);
    CHECK(p.weapons.size() == 1 && p.weapons[0].slot == 2 && p.weapons[0].type == 22 && p.weapons[0].totalAmmo == 170);
    CHECK(p.currentSlot == 2 && !p.collisions && p.dimension == 7);
    CHECK(p.animation.present && p.animation.block == "ped" && p.animation.name == "WALK_civi");
    CHECK(p.animation.time == -1 && p.animation.looped && p.animation.updatePosition && !p.animation.interruptable);
    CHECK(p.animation.blendTime == 250 && p.animation.taskRestore && p.animation.elapsedTime == 400.f && p.animation.speed == 1.5f);
}

// PLAYER_LIST as the mobile peer receives it (device capture 2026-11-14): an 8-byte prefix before the records.
void CheckPlayerListCapture() {
    const char* hex = "030002002d310000b906804454d85b5d595b17d85cdcdd5b98d85bdc18cf402f40000012000030000000000000000000000040601f00000000";
    std::vector<uint8_t> body;
    for (size_t i = 0; hex[i] && hex[i + 1]; i += 2) body.push_back(static_cast<uint8_t>(std::stoi(std::string(hex + i, 2), nullptr, 16)));
    WorldRPG::ServerWorld::OnPlayerList(body.data(), body.size(), 456);
    uint32_t id = 0;
    CHECK(WorldRPG::ServerWorld::FindPlayerByNick("Samuel_assuncaopc", id));
    CHECK(id != 0 && WorldRPG::ServerWorld::ModelOf(id) >= 0 && WorldRPG::ServerWorld::ModelOf(id) <= 312);
    std::printf("player list capture: id=%u model=%d\n", id, WorldRPG::ServerWorld::ModelOf(id));
}

int main(int argc, char** argv) {
    CheckPacketLayouts();
    CheckPedEntityBody();
    CheckPlayerListCapture();

    // RESOURCE_START sets the root the phone reports for the resource (shim layout included).
    WorldRPG::Net::ResourceStartInfo root;
    root.name = "rootcheck";
    root.entityId = 0x0ABC;
    root.dynamicEntityId = 0x0ABD;
    WorldRPG::ServerWorld::OnResourceStart(root);
    CHECK(WorldRPG::ServerWorld::LastResourceRoot() == 0x0ABC);

    WorldRPG::Script::LuaVMStub vm;
    CHECK(vm.initialize());
    // Host has no libGTASA. Bad arguments return false (desktop CLuaWorldDefs::GetGroundPosition).
    // A valid triple also returns false here, because FindGroundZFor3DPosition cannot run.
    RunLua(vm, "get-ground-position",
           "assert(getGroundPosition() == false, \"no args\")\n"
           "assert(getGroundPosition({}) == false, \"table\")\n"
           "assert(getGroundPosition(1, 2) == false, \"short\")\n"
           "assert(getGroundPosition(1, 2, \"zz\") == false, \"bad z\")\n"
           "assert(getGroundPosition(0/0, 1, 2) == false, \"nan\")\n"
           "assert(getGroundPosition(1, \"2\", 3) == false, \"host has no world\")\n"
           "assert(getGroundPosition(10, 20, 30) == false, \"host triple\")");
    WorldRPG::ServerWorld::SetLocalPlayer(777);
    WorldRPG::ServerWorld::NotePlayerNick(777, "Sam");

    CheckJson(vm, argc > 1 ? argv[1] : nullptr);
    CheckHelpers(vm);
    CheckGui(vm);
    CheckMtaWindow(vm);
    CheckStopLifecycle(vm);
    CheckDxImage(vm);
    CheckNetworkCalls(vm);

    if (argc > 1) {
        RunLua(vm, "catalog-count", "assert(_G.catalogModels == 1435, 'catalog holds 1435 models')");
    }
    std::printf("neon_resource_tests: %d checks, %d failure(s)\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
