#!/usr/bin/env python3
"""
Generates detailed markdown files per category for ALL 1,581 desktop MTA Lua functions,
documenting their implementation status, desktop reference, and conversion requirements.
"""

import pathlib
import sys
from collections import OrderedDict

# Import parity_matrix extraction logic
sys.path.insert(0, str(pathlib.Path(__file__).parent))
import parity_matrix

client_root = pathlib.Path(parity_matrix.DEFAULT_DESKTOP)
lua_rows, lua_classes, lua_order, lua_unreached = parity_matrix.desktop_lua_functions(client_root)
helpers, fn_bodies = parity_matrix.port_stub_helpers(parity_matrix.PORT_SHARED)
eff = parity_matrix.port_lua_bindings(parity_matrix.PORT_SHARED, helpers, fn_bodies)

CATEGORIES = OrderedDict([
    ("VEHICLES", {
        "title": "Vehicles (CLuaVehicleDefs)",
        "file": "VEHICLES.md",
        "description": "Vehicle physics, handling, sirens, doors, colors, lights, engine state, trailers, nitro, gears, and occupant seats.",
        "groups": ["CLuaVehicleDefs"],
        "subsystem_notes": "Requires hooking libGTASA.so `CVehicle` vtable / offsets (`0x3A0000+` in ARM64), handling structures (`tHandlingData`), and vehicle damage packets (33, 56, 58)."
    }),
    ("PEDS_AND_PLAYERS", {
        "title": "Peds & Players (CLuaPedDefs, CLuaPlayerDefs)",
        "file": "PEDS_AND_PLAYERS.md",
        "description": "Ped animations, tasks, fighting styles, bone transforms, clothes, stats, voice, wanted levels, nametags, and player targeting.",
        "groups": ["CLuaPedDefs", "CLuaPlayerDefs"],
        "subsystem_notes": "Requires hooking libGTASA.so `CPed` / `CPlayerPed` (`_ZN10CPlayerPed...`), `CPedIK`, `CTask` hierarchy (wander, fight, duck), and clothing pools."
    }),
    ("WORLD_AND_OBJECTS", {
        "title": "World, Objects & Environment",
        "file": "WORLD_AND_OBJECTS.md",
        "description": "3D objects, world weather, game time, water levels, gravity, minute duration, heat haze, explosions, fires, garage doors, searchlights, and foliage.",
        "groups": ["CLuaWorldDefs", "CLuaObjectDefs", "CLuaBuildingDefs", "CLuaWaterDefs", "CLuaFoliageDefs", "CLuaFireDefs", "CLuaEffectDefs"],
        "subsystem_notes": "Hooks libGTASA.so `CObject`, `CWeather`, `CClock`, `CWaterLevel`, `CWorld`, and particle engine `FxSystem_c`."
    }),
    ("ELEMENTS", {
        "title": "Element Tree & Data (CLuaElementDefs)",
        "file": "ELEMENTS.md",
        "description": "Core element hierarchy (parent/child), element data (synced/local), collisions, dimensions, alpha, positions, velocities, matrix, and spatial streaming.",
        "groups": ["CLuaElementDefs"],
        "subsystem_notes": "Implemented via `LuaElementApi.cpp` and `ServerWorld.cpp`. Attachment queries read ENTITY_ADD metadata, resolve references after the batch and clear references on removal. Physical attachment, attachment RPCs, spatial octree lookup and complete element matrix math remain pending."
    }),
    ("GUI_AND_DX", {
        "title": "GUI & 2D/3D Drawing (DX & CEGUI)",
        "file": "GUI_AND_DX.md",
        "description": "2D screen drawing, fonts, text, rectangles, lines, textures, render targets, GUI windows, buttons, edits, labels, gridlists, and 3D line drawing.",
        "groups": ["CLuaGUIDefs", "CLuaDrawingDefs", "CLuaVectorGraphicDefs", "CLua2DFXDefs"],
        "subsystem_notes": "Desktop uses CEGUI and Direct3D9. Mobile uses `MtaDxView.java` (Android Canvas) + `LuaGuiApi.cpp`. Resource GUI fonts load through Android Typeface, are distinct from DX fonts, and detach from widgets on destruction/resource stop. GUI labels honor alignment, wrapping, clipping and logical lines; label metrics use the assigned font. Built-in GUI font names use Android system faces, so their appearance and CEGUI skin geometry still differ. Edit/memo input and caret count Unicode characters; JNI converts UTF-16/UTF-8. Memo Enter inserts a newline; logical lines render separately. The eight 2D draw calls honor postGUI, capture blend/font state and retain render-target order. DX text supports alignment, clipping, wrapping, independent numeric scales and rotation with a pivot; Canvas font metrics still differ from Direct3D. Primitive lines interpolate vertex colors. Grid sorting, selection, memo wrapping/automatic caret scrolling and HLSL/3D drawing remain pending. Host tests and Android compilation do not verify device rendering."
    }),
    ("ENGINE_AND_MODELS", {
        "title": "Engine & Model Replacement (DFF, TXD, COL)",
        "file": "ENGINE_AND_MODELS.md",
        "description": "Custom DFF mesh loading, TXD texture dictionaries, COL collision models, model handling overrides, texture filtering, and breakable objects.",
        "groups": ["CLuaEngineDefs", "CLuaPostfxDefs", "CLuaBreakEffectDefs"],
        "subsystem_notes": "Requires hooking War Drum's RenderWare stream loaders (`RwStreamOpen`, `RwTexDictionaryStreamRead`, `RpClumpStreamRead`) and `CModelInfo` array."
    }),
    ("AUDIO_AND_VOICE", {
        "title": "Audio & Voice System (CLuaAudioDefs)",
        "file": "AUDIO_AND_VOICE.md",
        "description": "2D audio playback, 3D spatial sounds, internet radio streams, sound effects, volume/pitch/pan, sound tags, and desktop Speex voice chat.",
        "groups": ["CLuaAudioDefs"],
        "subsystem_notes": "Desktop uses BASS for sound and Speex for voice. Mobile plays 2D sounds with MediaPlayer and supports explicit Speex/Opus codecs; the stock desktop handshake maps to Speex. Voice capture/playback and packet integration remain pending; 3D sound needs listener attenuation."
    }),
    ("CAMERA", {
        "title": "Camera & Viewport (CLuaCameraDefs)",
        "file": "CAMERA.md",
        "description": "Camera matrix (pos/lookAt), camera target, field of view (FOV), screen shake, fade transitions, clipping planes, and interior viewing.",
        "groups": ["CLuaCameraDefs"],
        "subsystem_notes": "Requires hooking libGTASA.so `TheCamera` (`CCamera`) at `0x71...` in ARM64, and matrix interpolation routines."
    }),
    ("WEAPONS_AND_COMBAT", {
        "title": "Weapons & Projectiles (CLuaWeaponDefs, CLuaProjectileDefs)",
        "file": "WEAPONS_AND_COMBAT.md",
        "description": "Custom weapons, weapon damage/range/accuracy stats, projectiles (rockets, grenades), bullet traces, and weapon firing flags.",
        "groups": ["CLuaWeaponDefs", "CLuaProjectileDefs"],
        "subsystem_notes": "Requires hooking `CWeaponInfo`, `CProjectileInfo`, bullet trace pool (`CBulletTraces`), and bullet sync packet (98)."
    }),
    ("CORE_AND_UTILITY", {
        "title": "Core System, Networking & Utilities",
        "file": "CORE_AND_UTILITY.md",
        "description": "Timers, event handling, network latent transfers, file I/O, XML parsing, JSON serialization, UTF strings, and cryptography.",
        "groups": [
            "CLuaManager (LoadCFunctions table)", "CLuaUtilDefs", "CLuaMain (lua_register)",
            "CLuaTimerDefs", "CLuaResourceDefs", "CLuaNetworkDefs", "CLuaFileDefs",
            "CLuaXMLDefs", "CLuaBitDefs", "CLuaCryptDefs", "CLuaUTFDefs", "CLuaPathDefs",
            "CLuaClientDefs"
        ],
        "subsystem_notes": "Pure CPU/platform logic. Mostly ported or needs standard library wrappers (e.g. tinyxml2, sha256, zlib)."
    }),
    ("MARKERS_AND_SPATIAL", {
        "title": "Markers, Blips, ColShapes & Spatial",
        "file": "MARKERS_AND_SPATIAL.md",
        "description": "Checkpoints, radar map blips, collision shapes (spheres, cubes, polygons), radar areas, pickups, and 3D searchlights.",
        "groups": [
            "CLuaColShapeDefs", "CLuaMarkerDefs", "CLuaBlipDefs", "CLuaRadarAreaDefs",
            "CLuaPickupDefs", "CLuaSearchLightDefs", "CLuaPointLightDefs", "CLuaRopeDefs",
            "CLuaBirdDefs", "CLuaNativeUIDefs", "CLuaTeamDefs"
        ],
        "subsystem_notes": "Requires spatial overlap checks (AABB / SAT collision algorithms) and radar texture rendering hooks."
    }),
    ("BROWSER_AND_COMPATIBILITY", {
        "title": "Web Browser & Compatibility (CEF / Discord)",
        "file": "BROWSER_AND_COMPATIBILITY.md",
        "description": "Embedded Chromium web pages, browser mouse/keyboard input, Discord Rich Presence, and backwards compatibility helpers.",
        "groups": ["CLuaBrowserDefs", "CLuaCompatibilityDefs", "CLuaDiscordDefs"],
        "subsystem_notes": "Desktop bundles CEF (Chromium Embedded Framework). On Android, this requires an off-screen Android `WebView` rendering to OpenGL texture or native popup."
    }),
])

out_dir = pathlib.Path("docs/lua-functions")
out_dir.mkdir(parents=True, exist_ok=True)

# Process all functions
func_data = {}
for group, name, file, line, handler in lua_rows:
    status, detail = parity_matrix.classify_lua(name, eff, helpers)
    func_data[name] = {
        "group": group,
        "name": name,
        "status": status,
        "detail": detail,
        "file": file,
        "line": line,
        "handler": handler,
    }

def format_status(status):
    if status == "implemented":
        return "**Implemented**"
    elif status == "stub":
        return "*Stub*"
    else:
        return "Missing"

def guess_conversion_notes(name, group, status, detail):
    if status == "implemented" and name == "guiCreateFont":
        return "Creates a resource-owned gui-font from a resolved TTF/OTF/TTC file, including cross-resource paths. Android validates/preloads the Typeface; host checks only the sfnt signature. Size follows unsigned conversion and the desktop 5-150 point clamp at 96 DPI. Live GUI faces remain retained beyond LRU pressure; an ordered D command releases them after earlier draws on destruction/stop/reset. Host/Java lifecycle tests pass; not device-verified."
    if status == "implemented" and name in {"guiSetFont", "guiGetFont"}:
        return "Accepts a live GUI font element or a present GUI font name; DX fonts and unknown names are rejected without changing the widget. guiGetFont returns name and the explicitly attached font element (nil for a named assignment). Destruction/resource stop restores default-normal for users in other resources too. Built-in GUI names use Android system faces; CEGUI appearance/layout is not identical. Host/Java checks pass; not device-verified."
    if status == "implemented" and name == "guiLabelGetFontHeight":
        return "Returns assigned-font Android line spacing in pixels as an approximation. CEGUI returns cached glyph height, which can differ from baseline spacing; exact height parity remains pending. Host builds estimate metrics without Android. Host/Java checks pass; not device-verified."
    if status == "implemented" and name == "guiLabelGetTextExtent":
        return "Measures the longest logical line in pixels using the same Typeface source/size as the view. Host builds estimate metrics without Android. Host/Java checks pass; real glyph metrics require device validation."
    if status == "implemented" and name in {"guiLabelSetHorizontalAlign", "guiLabelSetVerticalAlign", "guiCreateLabel"}:
        return "GUI label layout honors horizontal/vertical alignment, optional word wrapping and clipping. Escaped literal text preserves newlines, backslashes and Unicode; labels have no three-line limit. The assigned GUI font is captured/reset around each widget. Canvas line breaking differs from CEGUI. Host/Java checks pass; not device-verified."
    if status == "implemented" and name in {"dxGetFontHeight", "dxGetTextWidth", "dxGetTextSize"}:
        return "DX metrics resolve the versioned native DX font source; a live GUI font is rejected instead of silently measuring the default. Height/width return false for that wrong type; dxGetTextSize raises its typed argument error before allocating C++ strings, even for empty text. Unknown/destroyed font fallback remains as before. Host type/lifecycle tests pass; real glyph metrics still need device validation."
    if status == "implemented" and name == "dxCreateFont":
        return "Custom fonts use a native instance identity in both the Typeface cache and measured-width cache; recreating a font at the same path reloads it. Java Typeface cache has an LRU bound. Host lifecycle checks pass; Android compiles; glyph metrics still need device validation."
    if status == "stub" and name in {"attachElements", "detachElements"}:
        return "Returns false until native physical attachment and its lifecycle/events are ported. Server attachment queries are metadata-only; these calls do not mutate that snapshot."
    if status == "implemented" and name in {"isElementAttached", "getElementAttachedTo", "getAttachedElements", "getElementAttachedOffsets"}:
        return "Reads server ENTITY_ADD attachment metadata, including forward references in the batch and offsets in degrees. Removal clears references before ID reuse. Physical attachment, local attach/detach and later attachment RPCs remain pending. Host-tested; not device-verified."
    if status == "implemented" and name in {"guiEditSetCaretIndex", "guiEditGetCaretIndex", "guiEditSetMaxLength", "guiEditGetMaxLength", "guiSetText"}:
        return "Edit/memo caret uses Unicode characters; insertion/backspace respect the caret and read-only flag. An explicit edit maximum of 0 accepts no text. Memo text preserves CEGUI's terminal newline. Selection editing remains pending. Host-tested; not device-verified."
    if status == "implemented" and name in {"guiCreateMemo", "guiMemoSetCaretIndex", "guiMemoGetCaretIndex", "guiMemoSetReadOnly", "guiMemoIsReadOnly"}:
        return "Memo accepts Unicode text, backspace and Enter without onClientGUIAccepted; emits onClientGUIChanged, participates in focus and requests multiline Android input. Terminal newline/caret follow CEGUI. Selection, wrapping and automatic caret scrolling remain pending. Host-tested; Android compiles; not device-verified."
    if status == "implemented" and name == "dxDrawText":
        return "Canvas text honors alignment, clip/wordBreak, two numeric scales, subpixel position, rotation/pivot and positive custom line height for color-coded text. Coded block height counts every logical line and each baseline advances by lineHeight, including the final line's occupied height. Escaped payload preserves line breaks and backslashes. Color-coded text ignores clip/wordBreak as desktop does. Font/blend state stays with queued text. Canvas metrics/wrapping differ from Direct3D; nonpositive scales draw nothing; layout width is capped at 32768 pixels. Host/Java regression checks pass; not device-verified."
    if status == "implemented" and name in {"dxDrawRectangle", "dxDrawImage", "dxDrawImageSection"}:
        return "Canvas 2D calls return true/false, honor postGUI and capture blend state. Render-target draws keep their original order. Existing texture elements work outside a resource owner; file paths still require one. Finite numeric arguments required. Host-tested; not device-verified."
    if status == "implemented" and name in {"dxDrawLine", "dxDrawCircle", "dxDrawPrimitive", "dxDrawMaterialPrimitive"}:
        return "2D Canvas bridge honors postGUI on screen and retains render-target order; queued calls capture blend mode. Primitive lines interpolate endpoint colors. Nonfinite/out-of-float-range input is rejected. Host-tested; Android compiles; device rendering not verified."
    if status == "implemented" and name in {"createProjectile", "getProjectileType", "getProjectileCreator", "getProjectileTarget", "getProjectileForce", "getProjectileCounter", "setProjectileCounter"}:
        return "Native local-projectile bridge for verified GTA 2.10 arm64. Rotation/model overrides, satchel attachment, creation event and packet sync remain unported. Host guards tested; Android compiles; physical runtime not device-tested."
    if status == "implemented" and name in {"getWeaponAmmo", "getWeaponClipAmmo", "getWeaponState", "getWeaponOwner", "getWeaponTarget"}:
        return f"Queries the last server ENTITY_ADD weapon snapshot; native firing and later weapon RPC updates are not ported. Host-tested, not device-verified. ({detail})"
    if status == "implemented" and name == "getSoundLength":
        return "2D MediaPlayer duration in seconds, reported asynchronously; 0 while unknown. Player voice overload is not ported. Host-tested bridge; not device-verified."
    if status == "implemented" and name == "getRadioChannelName":
        return "Desktop station-name lookup (IDs 0..12); does not tune the GTA radio. Host-tested."
    if status == "implemented" and name in {"setSoundPan", "getSoundPan"}:
        return "2D sound pan is applied to MediaPlayer left/right gains. Player voice overload is not ported. Android compiles; not device-verified."
    if status == "implemented" and name == "getValidPedModels":
        return "Stock models only (0..312 excluding 74, 149, 208); custom allocation is not ported. Host-tested; not device-verified. (GetValidPedModels)"
    if status == "implemented" and name == "getBoundKeys":
        return "Queries command bindings across resources; no physical GTA-control keyboard mappings on mobile yet. Host-tested; not device-verified. (GetBoundKeys)"
    if status == "implemented":
        return f"Operational on Android. ({detail})"
    if status == "stub":
        return f"Currently stubbed: `{detail}`. Needs native engine wiring."
    # Missing: provide technical requirements based on prefixes and groups
    if name.startswith("dxCreateShader") or "Shader" in name:
        return "Needs GLSL ES 3.0 shader pipeline (translating desktop D3DX9 HLSL)."
    if name.startswith("dx"):
        return "Needs Android Canvas / OpenGL ES draw line routing."
    if name.startswith("guiCreateGridList") or name.startswith("guiGridList"):
        return "Needs native Android ListView / RecyclerView or custom CEGUI widget port."
    if name.startswith("gui"):
        return "Needs Android widget state and input handler in LuaGuiApi."
    if name.startswith("sound") or name.startswith("getSound") or name.startswith("setSound") or name.startswith("play"):
        return "Needs AudioTrack / OpenSL ES audio playback mixer."
    if name.startswith("engineLoad") or name.startswith("engineReplace"):
        return "Needs hooking WarDrum RenderWare streaming clump/texture loader."
    if name.startswith("getVehicle") or name.startswith("setVehicle"):
        return "Needs reading/writing CVehicle struct fields in libGTASA.so."
    if name.startswith("getPed") or name.startswith("setPed"):
        return "Needs reading/writing CPed / CPlayerPed struct fields in libGTASA.so."
    if name.startswith("getCamera") or name.startswith("setCamera"):
        return "Needs reading/writing CCamera singleton in libGTASA.so."
    if name.startswith("xml"):
        return "Needs TinyXML2 client-side parser integration."
    if name.startswith("file"):
        return "Needs local sandbox file I/O operations."
    if "Browser" in group:
        return "Needs Android offscreen WebView render-to-texture bridge."
    return "Needs desktop C++ logic ported to app/src/main/cpp/shared."

# Generate category files
for cat_id, cat_info in CATEGORIES.items():
    cat_funcs = []
    for g in cat_info["groups"]:
        for name, data in func_data.items():
            if data["group"] == g:
                cat_funcs.append(data)
    
    # Sort functions by name
    cat_funcs.sort(key=lambda x: x["name"])
    
    total = len(cat_funcs)
    impl_cnt = sum(1 for x in cat_funcs if x["status"] == "implemented")
    stub_cnt = sum(1 for x in cat_funcs if x["status"] == "stub")
    miss_cnt = sum(1 for x in cat_funcs if x["status"] == "missing")
    pct = (impl_cnt / total * 100) if total > 0 else 0.0

    filepath = out_dir / cat_info["file"]
    with open(filepath, "w", encoding="utf-8") as f:
        f.write(f"# {cat_info['title']}\n\n")
        f.write(f"{cat_info['description']}\n\n")
        f.write("## Parity Status\n\n")
        f.write(f"- **Total Functions:** {total}\n")
        f.write(f"- **Implemented:** {impl_cnt} ({pct:.1f}%)\n")
        f.write(f"- **Stubs:** {stub_cnt}\n")
        f.write(f"- **Missing:** {miss_cnt}\n\n")
        f.write(f"> **Technical Requirement:** {cat_info['subsystem_notes']}\n\n")
        f.write("## Function List\n\n")
        f.write("| Lua Function | Status | Desktop Source Ref | Desktop Handler | Conversion Requirement / Implementation Notes |\n")
        f.write("|---|---|---|---|---|\n")
        for x in cat_funcs:
            req = guess_conversion_notes(x["name"], x["group"], x["status"], x["detail"])
            ref = f"`{x['file'].split('/')[-1]}:{x['line']}`"
            handler = f"`{x['handler']}`" if x['handler'] else "—"
            f.write(f"| `{x['name']}` | {format_status(x['status'])} | {ref} | {handler} | {req} |\n")
    print(f"Generated {filepath} ({total} functions)")

# Generate README index
with open(out_dir / "README.md", "w", encoding="utf-8") as f:
    f.write("# MTA Neon Client Lua API: Complete Conversion Catalog\n\n")
    f.write("This directory contains a complete, exhaustive breakdown of **all 1,581 desktop MTA client Lua functions**, organized into 12 functional categories.\n\n")
    f.write("Each document lists every single function, its current Android implementation state (`Implemented`, `Stub`, or `Missing`), the exact desktop reference line in `mta_neon_source`, and what engine hooks or network modules are required for conversion.\n\n")
    f.write("## Summary by Category\n\n")
    f.write("| Category | Documentation File | Total | Implemented | Stub | Missing | Parity % |\n")
    f.write("|---|---|---|---|---|---|---|\n")
    for cat_id, cat_info in CATEGORIES.items():
        cat_funcs = []
        for g in cat_info["groups"]:
            for name, data in func_data.items():
                if data["group"] == g:
                    cat_funcs.append(data)
        total = len(cat_funcs)
        impl_cnt = sum(1 for x in cat_funcs if x["status"] == "implemented")
        stub_cnt = sum(1 for x in cat_funcs if x["status"] == "stub")
        miss_cnt = sum(1 for x in cat_funcs if x["status"] == "missing")
        pct = (impl_cnt / total * 100) if total > 0 else 0.0
        f.write(f"| **{cat_info['title']}** | [{cat_info['file']}]({cat_info['file']}) | {total} | {impl_cnt} | {stub_cnt} | {miss_cnt} | {pct:.1f}% |\n")
    
    total_all = len(func_data)
    total_impl = sum(1 for x in func_data.values() if x["status"] == "implemented")
    total_stub = sum(1 for x in func_data.values() if x["status"] == "stub")
    total_miss = sum(1 for x in func_data.values() if x["status"] == "missing")
    total_pct = (total_impl / total_all * 100) if total_all > 0 else 0.0
    f.write(f"| **TOTAL** | **All Categories** | **{total_all}** | **{total_impl}** | **{total_stub}** | **{total_miss}** | **{total_pct:.1f}%** |\n\n")
    f.write("## Implementation Priority Tiers\n\n")
    f.write("1. **Tier 1 (Core Gameplay & UI - Needed Now)**: `ELEMENTS.md`, `GUI_AND_DX.md`, `PEDS_AND_PLAYERS.md`, `VEHICLES.md`.\n")
    f.write("2. **Tier 2 (World & Audio - Needed for Ambience)**: `WORLD_AND_OBJECTS.md`, `AUDIO_AND_VOICE.md`, `CAMERA.md`, `WEAPONS_AND_COMBAT.md`.\n")
    f.write("3. **Tier 3 (Modding & Advanced UI - Long Term)**: `ENGINE_AND_MODELS.md`, `MARKERS_AND_SPATIAL.md`, `BROWSER_AND_COMPATIBILITY.md`.\n")

print("Generated docs/lua-functions/README.md")
