#include "LuaApiExtra.h"
#include "LuaVMStub.h"
#include "ServerWorld.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <string>

namespace WorldRPG::Script {

namespace {

// CVehicleNames.cpp:23-234. szName, then the compatibility alias (szName_replaced) when the desktop
// entry has one. Empty alias is nullptr. Indexed by model - 400.
struct VehicleName {
    const char* name;
    const char* alias;
};

constexpr VehicleName kNames[212] = {
    {"Landstalker", nullptr}, // 400
    {"Bravura", nullptr}, // 401
    {"Buffalo", nullptr}, // 402
    {"Linerunner", nullptr}, // 403
    {"Perennial", nullptr}, // 404
    {"Sentinel", nullptr}, // 405
    {"Dumper", nullptr}, // 406
    {"Fire Truck", nullptr}, // 407
    {"Trashmaster", nullptr}, // 408
    {"Stretch", nullptr}, // 409
    {"Manana", nullptr}, // 410
    {"Infernus", nullptr}, // 411
    {"Voodoo", nullptr}, // 412
    {"Pony", nullptr}, // 413
    {"Mule", nullptr}, // 414
    {"Cheetah", nullptr}, // 415
    {"Ambulance", nullptr}, // 416
    {"Leviathan", nullptr}, // 417
    {"Moonbeam", nullptr}, // 418
    {"Esperanto", nullptr}, // 419
    {"Taxi", nullptr}, // 420
    {"Washington", nullptr}, // 421
    {"Bobcat", nullptr}, // 422
    {"Mr. Whoopee", nullptr}, // 423
    {"BF Injection", nullptr}, // 424
    {"Hunter", nullptr}, // 425
    {"Premier", nullptr}, // 426
    {"Enforcer", nullptr}, // 427
    {"Securicar", nullptr}, // 428
    {"Banshee", nullptr}, // 429
    {"Predator", nullptr}, // 430
    {"Bus", nullptr}, // 431
    {"Rhino", nullptr}, // 432
    {"Barracks", nullptr}, // 433
    {"Hotknife", nullptr}, // 434
    {"Trailer 1", nullptr}, // 435
    {"Previon", nullptr}, // 436
    {"Coach", nullptr}, // 437
    {"Cabbie", nullptr}, // 438
    {"Stallion", nullptr}, // 439
    {"Rumpo", nullptr}, // 440
    {"RC Bandit", nullptr}, // 441
    {"Romero", nullptr}, // 442
    {"Packer", nullptr}, // 443
    {"Monster 1", "Monster"}, // 444
    {"Admiral", nullptr}, // 445
    {"Squalo", nullptr}, // 446
    {"Seasparrow", nullptr}, // 447
    {"Pizzaboy", nullptr}, // 448
    {"Tram", nullptr}, // 449
    {"Trailer 2", nullptr}, // 450
    {"Turismo", nullptr}, // 451
    {"Speeder", nullptr}, // 452
    {"Reefer", nullptr}, // 453
    {"Tropic", nullptr}, // 454
    {"Flatbed", nullptr}, // 455
    {"Yankee", nullptr}, // 456
    {"Caddy", nullptr}, // 457
    {"Solair", nullptr}, // 458
    {"Berkley's RC Van", nullptr}, // 459
    {"Skimmer", nullptr}, // 460
    {"PCJ-600", nullptr}, // 461
    {"Faggio", nullptr}, // 462
    {"Freeway", nullptr}, // 463
    {"RC Baron", nullptr}, // 464
    {"RC Raider", nullptr}, // 465
    {"Glendale", nullptr}, // 466
    {"Oceanic", nullptr}, // 467
    {"Sanchez", nullptr}, // 468
    {"Sparrow", nullptr}, // 469
    {"Patriot", nullptr}, // 470
    {"Quadbike", nullptr}, // 471
    {"Coastguard", nullptr}, // 472
    {"Dinghy", nullptr}, // 473
    {"Hermes", nullptr}, // 474
    {"Sabre", nullptr}, // 475
    {"Rustler", nullptr}, // 476
    {"ZR-350", nullptr}, // 477
    {"Walton", nullptr}, // 478
    {"Regina", nullptr}, // 479
    {"Comet", nullptr}, // 480
    {"BMX", nullptr}, // 481
    {"Burrito", nullptr}, // 482
    {"Camper", nullptr}, // 483
    {"Marquis", nullptr}, // 484
    {"Baggage", nullptr}, // 485
    {"Dozer", nullptr}, // 486
    {"Maverick", nullptr}, // 487
    {"News Chopper", nullptr}, // 488
    {"Rancher", nullptr}, // 489
    {"FBI Rancher", nullptr}, // 490
    {"Virgo", nullptr}, // 491
    {"Greenwood", nullptr}, // 492
    {"Jetmax", nullptr}, // 493
    {"Hotring Racer", nullptr}, // 494
    {"Sandking", nullptr}, // 495
    {"Blista Compact", nullptr}, // 496
    {"Police Maverick", nullptr}, // 497
    {"Boxville", nullptr}, // 498
    {"Benson", nullptr}, // 499
    {"Mesa", nullptr}, // 500
    {"RC Goblin", nullptr}, // 501
    {"Hotring Racer 2", nullptr}, // 502
    {"Hotring Racer 3", nullptr}, // 503
    {"Bloodring Banger", nullptr}, // 504
    {"Rancher Lure", nullptr}, // 505
    {"Super GT", nullptr}, // 506
    {"Elegant", nullptr}, // 507
    {"Journey", nullptr}, // 508
    {"Bike", nullptr}, // 509
    {"Mountain Bike", nullptr}, // 510
    {"Beagle", nullptr}, // 511
    {"Cropduster", nullptr}, // 512
    {"Stuntplane", nullptr}, // 513
    {"Tanker", nullptr}, // 514
    {"Roadtrain", nullptr}, // 515
    {"Nebula", nullptr}, // 516
    {"Majestic", nullptr}, // 517
    {"Buccaneer", nullptr}, // 518
    {"Shamal", nullptr}, // 519
    {"Hydra", nullptr}, // 520
    {"FCR-900", nullptr}, // 521
    {"NRG-500", nullptr}, // 522
    {"HPV1000", nullptr}, // 523
    {"Cement Truck", nullptr}, // 524
    {"Towtruck", nullptr}, // 525
    {"Fortune", nullptr}, // 526
    {"Cadrona", nullptr}, // 527
    {"FBI Truck", nullptr}, // 528
    {"Willard", nullptr}, // 529
    {"Forklift", nullptr}, // 530
    {"Tractor", nullptr}, // 531
    {"Combine Harvester", nullptr}, // 532
    {"Feltzer", nullptr}, // 533
    {"Remington", nullptr}, // 534
    {"Slamvan", nullptr}, // 535
    {"Blade", nullptr}, // 536
    {"Freight", nullptr}, // 537
    {"Streak", nullptr}, // 538
    {"Vortex", nullptr}, // 539
    {"Vincent", nullptr}, // 540
    {"Bullet", nullptr}, // 541
    {"Clover", nullptr}, // 542
    {"Sadler", nullptr}, // 543
    {"Fire Truck Ladder", nullptr}, // 544
    {"Hustler", nullptr}, // 545
    {"Intruder", nullptr}, // 546
    {"Primo", nullptr}, // 547
    {"Cargobob", nullptr}, // 548
    {"Tampa", nullptr}, // 549
    {"Sunrise", nullptr}, // 550
    {"Merit", nullptr}, // 551
    {"Utility Van", nullptr}, // 552
    {"Nevada", nullptr}, // 553
    {"Yosemite", nullptr}, // 554
    {"Windsor", nullptr}, // 555
    {"Monster 2", nullptr}, // 556
    {"Monster 3", nullptr}, // 557
    {"Uranus", nullptr}, // 558
    {"Jester", nullptr}, // 559
    {"Sultan", nullptr}, // 560
    {"Stratum", nullptr}, // 561
    {"Elegy", nullptr}, // 562
    {"Raindance", nullptr}, // 563
    {"RC Tiger", nullptr}, // 564
    {"Flash", nullptr}, // 565
    {"Tahoma", nullptr}, // 566
    {"Savanna", nullptr}, // 567
    {"Bandito", nullptr}, // 568
    {"Freight Train Flatbed", nullptr}, // 569
    {"Streak Train Trailer", nullptr}, // 570
    {"Kart", nullptr}, // 571
    {"Mower", nullptr}, // 572
    {"Dune", nullptr}, // 573
    {"Sweeper", nullptr}, // 574
    {"Broadway", nullptr}, // 575
    {"Tornado", nullptr}, // 576
    {"AT-400", nullptr}, // 577
    {"DFT-30", nullptr}, // 578
    {"Huntley", nullptr}, // 579
    {"Stafford", nullptr}, // 580
    {"BF-400", nullptr}, // 581
    {"Newsvan", nullptr}, // 582
    {"Tug", nullptr}, // 583
    {"Trailer (Tanker Commando)", nullptr}, // 584
    {"Emperor", nullptr}, // 585
    {"Wayfarer", nullptr}, // 586
    {"Euros", nullptr}, // 587
    {"Hotdog", nullptr}, // 588
    {"Club", nullptr}, // 589
    {"Box Freight", nullptr}, // 590
    {"Trailer 3", nullptr}, // 591
    {"Andromada", nullptr}, // 592
    {"Dodo", nullptr}, // 593
    {"RC Cam", nullptr}, // 594
    {"Launch", nullptr}, // 595
    {"Police LS", "Police"}, // 596
    {"Police SF", nullptr}, // 597
    {"Police LV", nullptr}, // 598
    {"Police Ranger", "Ranger"}, // 599
    {"Picador", nullptr}, // 600
    {"S.W.A.T.", nullptr}, // 601
    {"Alpha", nullptr}, // 602
    {"Phoenix", nullptr}, // 603
    {"Glendale Damaged", nullptr}, // 604
    {"Sadler Damaged", "Sadler"}, // 605
    {"Baggage Trailer (covered)", nullptr}, // 606
    {"Baggage Trailer (Uncovered)", nullptr}, // 607
    {"Trailer (Stairs)", nullptr}, // 608
    {"Boxville Mission", nullptr}, // 609
    {"Farm Trailer", nullptr}, // 610
    {"Street Clean Trailer", nullptr}, // 611
};

// CClientVehicleManager.cpp:19-32 (g_ucMaxPassengers). 255 means the model has no published passenger count.
constexpr unsigned char kMaxPassengers[212] = {
    3, 1, 1, 1, 3, 3, 0, 1, 1, 3, 1, 1, 1, 3, 1, 1,
    3, 1, 3, 1, 3, 3, 1, 1, 1, 0, 3, 3, 3, 1, 0, 8,
    0, 1, 1, 255, 1, 8, 3, 1, 3, 0, 1, 1, 1, 3, 0, 1,
    0, 1, 255, 1, 0, 0, 0, 1, 1, 1, 3, 3, 1, 1, 1, 1,
    1, 1, 3, 3, 1, 1, 3, 1, 0, 0, 1, 1, 0, 1, 1, 3,
    1, 0, 3, 3, 0, 0, 0, 3, 1, 1, 3, 1, 3, 0, 1, 1,
    1, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 0, 0, 1,
    0, 0, 1, 1, 3, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1,
    1, 3, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 3, 1, 1, 1,
    1, 1, 3, 3, 1, 1, 3, 3, 1, 0, 1, 1, 1, 1, 1, 1,
    3, 3, 1, 1, 0, 1, 3, 3, 0, 255, 1, 0, 0, 1, 0, 1,
    1, 1, 1, 3, 3, 1, 3, 0, 255, 3, 1, 1, 1, 1, 255, 255,
    1, 1, 1, 0, 3, 3, 3, 1, 1, 1, 1, 1, 3, 1, 255, 255,
    255, 3, 255, 255,
};

static_assert(sizeof(kNames) / sizeof(kNames[0]) == 212, "vehicle name table");
static_assert(sizeof(kMaxPassengers) == 212, "passenger table");

// CVehicleUpgrades.cpp:20-36. Index is the upgrade slot.
constexpr const char* kSlotNames[17] = {
    "Hood", "Vent", "Spoiler", "Sideskirt", "Front Bullbars", "Rear Bullbars", "Headlights", "Roof",
    "Nitro", "Hydraulics", "Stereo", "Unknown", "Wheels", "Exhaust", "Front Bumper", "Rear Bumper", "Misc",
};

// CServerModelDefinition.h. Ids at or above this are server-registry models, not GTA file ids.
constexpr uint32_t kServerModelIdMin = 42341;
constexpr uint32_t kServerModelIdMax = 65534;

constexpr bool IsStandardModel(int model) { return model >= 400 && model <= 611; }

// CVehicleNames.cpp:236-244. Index is ucVehicleTypes[model - 400].
constexpr const char* kVehicleTypes[10] = {
    "Automobile", "Plane", "Bike", "Helicopter", "Boat", "Train", "Trailer", "BMX", "Monster Truck", "Quad",
};
// CVehicleNames.cpp:239-244.
constexpr unsigned char kVehicleTypeIndex[212] = {
    0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 4, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0,
    0, 8, 0, 4, 3, 2, 5, 6, 0, 4, 4, 4, 0, 0, 0, 0, 0, 1, 2, 2, 2, 1, 3, 0, 0, 2, 3, 0, 9, 4, 4, 0, 0, 1, 0, 0, 0, 0, 7, 0, 0, 4, 0,
    0, 3, 3, 0, 0, 0, 0, 4, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 7, 7, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1, 0, 0, 8, 8, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 5, 5, 0,
    0, 8, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 6, 0, 2, 0, 0, 0, 5, 6, 1, 1, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 0, 6, 6,
};
static_assert(sizeof(kVehicleTypeIndex) == 212, "vehicle type table");

// stricmp of the desktop name lookup. Names are ASCII.
bool SameName(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) {
        unsigned char ca = static_cast<unsigned char>(*a);
        unsigned char cb = static_cast<unsigned char>(*b);
        if (ca >= 'A' && ca <= 'Z') ca = static_cast<unsigned char>(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z') cb = static_cast<unsigned char>(cb - 'A' + 'a');
        if (ca != cb) return false;
        ++a;
        ++b;
    }
    return *a == *b;
}

// CVehicleNames::GetVehicleTypeName (CVehicleNames.cpp 288-301) for a stock model.
// A non-stock id has no CModelInfo parent on the phone, so the type name is empty.
const char* VehicleTypeOf(int model) {
    if (!IsStandardModel(model)) return "";
    const unsigned char index = kVehicleTypeIndex[model - 400];
    return index < 10 ? kVehicleTypes[index] : "";
}

// CVehicleNames::GetVehicleName (CVehicleNames.cpp:258-268). Never null: unknown models are "".
const char* VehicleNameOf(int model) {
    if (!IsStandardModel(model)) return "";
    return kNames[model - 400].name;
}

// CVehicleNames::GetVehicleModel (CVehicleNames.cpp:270-286). 0 means no match.
unsigned ModelFromName(const char* name) {
    if (!name || name[0] == '\0') return 0;
    for (unsigned i = 0; i < 212; ++i) {
        if (SameName(name, kNames[i].name) || (kNames[i].alias && SameName(name, kNames[i].alias)))
            return i + 400;
    }
    return 0;
}

// CVehicleUpgrades::GetSlotFromUpgrade (CVehicleUpgrades.cpp:452-555). First matching list wins.
bool Listed(unsigned short us, std::initializer_list<unsigned short> ids) {
    for (unsigned short id : ids) {
        if (us == id) return true;
    }
    return false;
}

bool SlotFromUpgrade(unsigned short us, unsigned char& ucSlot) {
    if (Listed(us, {1011, 1012, 1111, 1112, 1142, 1143, 1144, 1145})) { ucSlot = 0; return true; }
    if (Listed(us, {1004, 1005, 1142, 1143, 1144, 1145})) { ucSlot = 1; return true; }
    if (Listed(us, {1000, 1001, 1002, 1003, 1014, 1015, 1016, 1023, 1049, 1050, 1058, 1060, 1138, 1139, 1146, 1147, 1158, 1162, 1163, 1164})) { ucSlot = 2; return true; }
    if (Listed(us, {1007, 1017, 1026, 1027, 1030, 1031, 1036, 1039, 1040, 1041, 1042, 1047, 1048, 1051, 1052, 1056, 1057, 1062, 1063, 1069, 1070, 1071, 1072, 1090, 1093, 1094, 1095, 1099, 1101, 1102, 1106, 1107, 1108, 1118, 1119, 1120, 1121, 1122, 1124, 1133, 1134, 1137})) { ucSlot = 3; return true; }
    if (Listed(us, {1115, 1116})) { ucSlot = 4; return true; }
    if (Listed(us, {1109, 1110})) { ucSlot = 5; return true; }
    if (Listed(us, {1013, 1024})) { ucSlot = 6; return true; }
    if (Listed(us, {1006, 1032, 1033, 1035, 1038, 1053, 1054, 1055, 1061, 1067, 1068, 1088, 1091, 1103, 1128, 1130, 1131})) { ucSlot = 7; return true; }
    if (Listed(us, {1008, 1009, 1010})) { ucSlot = 8; return true; }
    if (Listed(us, {1087})) { ucSlot = 9; return true; }
    if (Listed(us, {1086})) { ucSlot = 10; return true; }
    if (Listed(us, {1025, 1073, 1074, 1075, 1076, 1077, 1078, 1079, 1080, 1081, 1082, 1083, 1084, 1085, 1096, 1097, 1098})) { ucSlot = 12; return true; }
    if (Listed(us, {1018, 1019, 1020, 1021, 1022, 1028, 1029, 1034, 1037, 1043, 1044, 1045, 1046, 1059, 1064, 1065, 1066, 1089, 1092, 1104, 1105, 1113, 1114, 1126, 1127, 1129, 1132, 1135, 1136})) { ucSlot = 13; return true; }
    if (Listed(us, {1117, 1152, 1153, 1155, 1157, 1160, 1165, 1166, 1169, 1170, 1171, 1172, 1173, 1174, 1175, 1179, 1181, 1182, 1185, 1188, 1189, 1190, 1191})) { ucSlot = 14; return true; }
    if (Listed(us, {1140, 1141, 1148, 1149, 1150, 1151, 1154, 1156, 1159, 1161, 1167, 1168, 1176, 1177, 1178, 1180, 1183, 1184, 1186, 1187, 1192, 1193})) { ucSlot = 15; return true; }
    if (Listed(us, {1100, 1123, 1125})) { ucSlot = 16; return true; }
    return false;
}

// CVehicleUpgrades::GetSlotName (CVehicleUpgrades.cpp:764-770).
const char* SlotName(unsigned char slot) {
    return slot < 17 ? kSlotNames[slot] : "";
}

const char* LuaTypeName(int type) {
    switch (type) {
        case LUA_TNIL: return "nil";
        case LUA_TBOOLEAN: return "boolean";
        case LUA_TLIGHTUSERDATA: return "lightuserdata";
        case LUA_TNUMBER: return "number";
        case LUA_TSTRING: return "string";
        case LUA_TTABLE: return "table";
        case LUA_TFUNCTION: return "function";
        case LUA_TUSERDATA: return "userdata";
        case LUA_TTHREAD: return "thread";
        default: return "none";
    }
}

// Desktop CScriptDebugging::LogWarning prefix plus CScriptArgReader::GetFullErrorMessage.
void LogWarning(const char* text) { LUA_LOGI("WARNING: %s", text); }

void LogFull(const char* function, const char* category, const char* message) {
    char buf[512];
    std::snprintf(buf, sizeof(buf), "%s @ '%s' [%s]", category, function, message);
    LogWarning(buf);
}

// SetTypeError: "Expected <type> at argument N, got <lua type>" and the value when it is a string or number.
void LogTypeError(lua_State* L, const char* function, const char* expected, int index) {
    const int type = lua_type(L, index);
    const char* value = nullptr;
    if (type == LUA_TNUMBER || type == LUA_TSTRING) value = lua_tostring(L, index);
    char buf[512];
    if (value && value[0] != '\0') {
        std::snprintf(buf, sizeof(buf), "Bad argument @ '%s' [Expected %s at argument %d, got %s '%.64s']",
                      function, expected, index, LuaTypeName(type), value);
    } else {
        std::snprintf(buf, sizeof(buf), "Bad argument @ '%s' [Expected %s at argument %d, got %s]", function,
                      expected, index, LuaTypeName(type));
    }
    LogWarning(buf);
}

int Fail(lua_State* L) {
    lua_pushboolean(L, 0);
    return 1;
}

// Element tables {id, element = true} stand in for desktop userdata (see PushElement).
bool IsElementArg(lua_State* L, int index) {
    if (!lua_istable(L, index)) return false;
    lua_getfield(L, index, "element");
    const bool element = lua_toboolean(L, -1) != 0;
    lua_pop(L, 1);
    return element;
}

// CScriptArgReader::ReadUserData<CClientVehicle>. False logs the type error.
bool ReadVehicleModel(lua_State* L, const char* function, int& model) {
    if (!IsElementArg(L, 1) || std::strcmp(ServerWorld::ElementKindName(ElementIdOf(L, 1)), "vehicle") != 0) {
        LogTypeError(L, function, "vehicle", 1);
        return false;
    }
    model = ServerWorld::ModelOf(ElementIdOf(L, 1));
    return true;
}

// CScriptArgReader::ReadString. Numbers are accepted and converted, as on desktop.
bool ReadStringArg(lua_State* L, const char* function, std::string& out) {
    const int type = lua_type(L, 1);
    if (type == LUA_TSTRING || type == LUA_TNUMBER) {
        size_t length = 0;
        const char* text = lua_tolstring(L, 1, &length);
        out.assign(text ? text : "", text ? length : 0);
        return true;
    }
    LogTypeError(L, function, "string", 1);
    return false;
}

// CScriptArgReader::ReadNumber<lua_Number> then ReadNumberBounded(0, SERVER_MODEL_ID_MAX).
bool ReadModelArg(lua_State* L, const char* function, uint32_t& model) {
    const int type = lua_type(L, 1);
    if (type != LUA_TNUMBER && type != LUA_TSTRING) {
        LogTypeError(L, function, "number", 1);
        return false;
    }
    if (!lua_isnumber(L, 1)) {
        LogFull(function, "Bad argument", "Expected number, got non-convertible string");
        return false;
    }
    const lua_Number number = lua_tonumber(L, 1);
    if (std::isnan(number)) {
        LogFull(function, "Bad argument", "Expected number, got NaN");
        return false;
    }
    if (!std::isfinite(number) || number < 0.0 || number > static_cast<lua_Number>(kServerModelIdMax)) {
        LogFull(function, "Bad argument", "Expected number within the supported range");
        return false;
    }
    model = static_cast<uint32_t>(number);
    return true;
}

// CScriptArgReader::ReadNumber<unsigned short>. Narrows through int64, then keeps the low 16 bits.
bool ReadUnsignedShortArg(lua_State* L, const char* function, unsigned short& value) {
    const int type = lua_type(L, 1);
    if (type != LUA_TNUMBER && type != LUA_TSTRING) {
        LogTypeError(L, function, "number", 1);
        return false;
    }
    if (!lua_isnumber(L, 1)) {
        LogFull(function, "Bad argument", "Expected number, got non-convertible string");
        return false;
    }
    const lua_Number number = lua_tonumber(L, 1);
    if (std::isnan(number)) {
        LogFull(function, "Bad argument", "Expected number, got NaN");
        return false;
    }
    if (number < -static_cast<lua_Number>(FLT_EPSILON)) {
        LogFull(function, "Bad argument", "Expected positive value, got negative");
        return false;
    }
    if (!std::isfinite(number) || number > static_cast<lua_Number>(std::numeric_limits<int64_t>::max())) {
        LogFull(function, "Bad argument", "Expected number within the supported range");
        return false;
    }
    value = static_cast<unsigned short>(static_cast<int64_t>(number));
    return true;
}

// CClientModelManager::ResolveModelID (CClientModelManager.cpp:326-368) with an empty server-model
// map and no native-world pack. Stock GTA ids pass through. Registry ids fail ("invalid model ID").
// CGameSA::IsNativeWorldModelIdReserved is not called: nothing on the phone reserves a model id.
bool ResolveStockModel(uint32_t supplied, uint16_t& runtime) {
    if (supplied >= kServerModelIdMin) return false;
    runtime = static_cast<uint16_t>(supplied);
    return true;
}

// Parent remap (CLuaVehicleDefs.cpp GetVehicleName 1369-1372) reads g_pGame->GetModelInfo.
// That hook is not wired. Stock models have parent 0, so the name is the model's own.
// A non-stock model is left unchanged, which is the null model-info result.
void NoteMissingParent(const char* function, int model) {
    if (IsStandardModel(model)) return;
    LUA_LOGI("DIAG %s: CModelInfo::GetParentID not wired, model %d", function, model);
}

// getVehicleMaxPassengers(vehicle or model). CLuaVehicleDefs.cpp:652-690.
// Passenger counts: CClientVehicleManager::GetMaxPassengerCount (CClientVehicleManager.cpp:497-509).
int GetVehicleMaxPassengers(lua_State* L) {
    int model = -1;
    if (IsElementArg(L, 1)) {
        if (!ReadVehicleModel(L, "getVehicleMaxPassengers", model)) return Fail(L);
    } else {
        uint32_t supplied = 0;
        if (!ReadModelArg(L, "getVehicleMaxPassengers", supplied)) return Fail(L);
        uint16_t runtime = 0;
        if (!ResolveStockModel(supplied, runtime)) {
            LogFull("getVehicleMaxPassengers", "Bad usage", "invalid model ID");
            return Fail(L);
        }
        model = runtime;
    }

    // CClientVehicleManager::IsValidModel (CClientVehicleManager.cpp:442-453) asks CModelInfo::IsVehicle.
    // Every stock id 400-611 is a vehicle, so that check passes. A native id outside that range is
    // not a vehicle. A stored custom id has no model info on the phone, so it fails the same way.
    if (!IsStandardModel(model)) {
        if (model < 0 || model > 611)
            LUA_LOGI("DIAG getVehicleMaxPassengers: CModelInfo::IsVehicle not wired, model %d", model);
        return Fail(L);
    }
    const unsigned char count = kMaxPassengers[model - 400];
    if (count == 0xFF) return Fail(L);
    // Desktop pushes with lua_pushnumber. This VM is Lua 5.3, where a whole count is an integer.
    lua_pushinteger(L, count);
    return 1;
}

// getVehicleModelFromName(name). CLuaVehicleDefs.cpp:604-626 and
// CStaticFunctionDefinitions::GetVehicleModelFromName (CStaticFunctionDefinitions.cpp:2855-2867).
int GetVehicleModelFromName(lua_State* L) {
    std::string name;
    if (!ReadStringArg(L, "getVehicleModelFromName", name)) return Fail(L);
    const unsigned model = ModelFromName(name.c_str());
    if (model == 0) return Fail(L);
    // Desktop pushes with lua_pushnumber. This VM is Lua 5.3, where a model id is an integer.
    lua_pushinteger(L, model);
    return 1;
}

// getVehicleName(vehicle). CLuaVehicleDefs.cpp:1360-1386.
int GetVehicleName(lua_State* L) {
    int model = -1;
    if (!ReadVehicleModel(L, "getVehicleName", model)) return Fail(L);
    NoteMissingParent("getVehicleName", model);
    lua_pushstring(L, VehicleNameOf(model));
    return 1;
}

// getVehicleNameFromModel(model). CLuaVehicleDefs.cpp:1607-1642 and
// CStaticFunctionDefinitions::GetVehicleNameFromModel (CStaticFunctionDefinitions.cpp:2887-2891).
int GetVehicleNameFromModel(lua_State* L) {
    uint32_t supplied = 0;
    if (!ReadModelArg(L, "getVehicleNameFromModel", supplied)) return Fail(L);
    uint16_t runtime = 0;
    if (!ResolveStockModel(supplied, runtime)) {
        LogFull("getVehicleNameFromModel", "Bad usage", "invalid model ID");
        return Fail(L);
    }
    // Non-stock native ids are not vehicles, so the CModelInfo parent remap (lines 1620-1629)
    // does not change the model. Registry ids already failed ResolveStockModel above.
    const char* name = VehicleNameOf(runtime);
    if (name[0] == '\0') return Fail(L);
    lua_pushstring(L, name);
    return 1;
}

// getVehicleType(vehicle or model). CLuaVehicleDefs.cpp:455-487 and
// CVehicleNames::GetVehicleTypeName (CVehicleNames.cpp 288-301).
int GetVehicleType(lua_State* L) {
    int model = -1;
    if (IsElementArg(L, 1)) {
        if (!ReadVehicleModel(L, "getVehicleType", model)) return Fail(L);
    } else {
        uint32_t supplied = 0;
        if (!ReadModelArg(L, "getVehicleType", supplied)) return Fail(L);
        uint16_t runtime = 0;
        if (!ResolveStockModel(supplied, runtime)) {
            LogFull("getVehicleType", "Bad usage", "invalid model ID");
            return Fail(L);
        }
        model = runtime;
    }
    NoteMissingParent("getVehicleType", model);
    lua_pushstring(L, VehicleTypeOf(model));
    return 1;
}

// getVehicleUpgradeSlotName(slot or upgrade). CLuaVehicleDefs.cpp:953-988 and
// CStaticFunctionDefinitions::GetVehicleUpgradeSlotName (CStaticFunctionDefinitions.cpp:2869-2885).
int GetVehicleUpgradeSlotName(lua_State* L) {
    unsigned short number = 0;
    if (!ReadUnsignedShortArg(L, "getVehicleUpgradeSlotName", number)) return Fail(L);
    if (number < 17) {
        lua_pushstring(L, SlotName(static_cast<unsigned char>(number)));
        return 1;
    }
    if (number >= 1000 && number <= 1193) {
        unsigned char slot = 0;
        if (SlotFromUpgrade(number, slot)) {
            const char* name = SlotName(slot);
            if (name[0] != '\0') {
                lua_pushstring(L, name);
                return 1;
            }
        }
        return Fail(L);
    }
    // CScriptDebugging::LogBadPointer (CScriptDebugging.cpp:194-199), argument 1.
    char buf[128];
    std::snprintf(buf, sizeof(buf), "Bad 'slot/upgrade' pointer @ 'getVehicleUpgradeSlotName'(1)");
    LogWarning(buf);
    return Fail(L);
}

void Register(lua_State* L) {
    const struct {
        const char* name;
        lua_CFunction fn;
    } binds[] = {
        {"getVehicleMaxPassengers", GetVehicleMaxPassengers},
        {"getVehicleModelFromName", GetVehicleModelFromName},
        {"getVehicleName", GetVehicleName},
        {"getVehicleNameFromModel", GetVehicleNameFromModel},
        {"getVehicleType", GetVehicleType},
        {"getVehicleUpgradeSlotName", GetVehicleUpgradeSlotName},
    };
    for (const auto& bind : binds) {
        lua_pushcfunction(L, bind.fn);
        lua_setglobal(L, bind.name);
    }
}

}  // namespace

void BindVehicleLua(lua_State* L) { Register(L); }

}  // namespace WorldRPG::Script

// Strong definition. A weak InstallVehicleLua in another translation unit resolves to this one.
extern "C" void InstallVehicleLua(lua_State* L) { WorldRPG::Script::BindVehicleLua(L); }
