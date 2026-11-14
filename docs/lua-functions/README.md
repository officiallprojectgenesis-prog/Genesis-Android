# MTA Neon Client Lua API: Complete Conversion Catalog

This directory contains a complete, exhaustive breakdown of **all 1,581 desktop MTA client Lua functions**, organized into 12 functional categories.

Each document lists every single function, its current Android implementation state (`Implemented`, `Stub`, or `Missing`), the exact desktop reference line in `mta_neon_source`, and what engine hooks or network modules are required for conversion.

## Summary by Category

| Category | Documentation File | Total | Implemented | Stub | Missing | Parity % |
|---|---|---|---|---|---|---|
| **Vehicles (CLuaVehicleDefs)** | [VEHICLES.md](VEHICLES.md) | 165 | 6 | 22 | 137 | 3.6% |
| **Peds & Players (CLuaPedDefs, CLuaPlayerDefs)** | [PEDS_AND_PLAYERS.md](PEDS_AND_PLAYERS.md) | 206 | 24 | 80 | 102 | 11.7% |
| **World, Objects & Environment** | [WORLD_AND_OBJECTS.md](WORLD_AND_OBJECTS.md) | 227 | 4 | 11 | 212 | 1.8% |
| **Element Tree & Data (CLuaElementDefs)** | [ELEMENTS.md](ELEMENTS.md) | 81 | 31 | 21 | 29 | 38.3% |
| **GUI & 2D/3D Drawing (DX & CEGUI)** | [GUI_AND_DX.md](GUI_AND_DX.md) | 199 | 49 | 5 | 145 | 24.6% |
| **Engine & Model Replacement (DFF, TXD, COL)** | [ENGINE_AND_MODELS.md](ENGINE_AND_MODELS.md) | 125 | 1 | 15 | 109 | 0.8% |
| **Audio & Voice System (CLuaAudioDefs)** | [AUDIO_AND_VOICE.md](AUDIO_AND_VOICE.md) | 57 | 4 | 5 | 48 | 7.0% |
| **Camera & Viewport (CLuaCameraDefs)** | [CAMERA.md](CAMERA.md) | 47 | 3 | 3 | 41 | 6.4% |
| **Weapons & Projectiles (CLuaWeaponDefs, CLuaProjectileDefs)** | [WEAPONS_AND_COMBAT.md](WEAPONS_AND_COMBAT.md) | 31 | 0 | 0 | 31 | 0.0% |
| **Core System, Networking & Utilities** | [CORE_AND_UTILITY.md](CORE_AND_UTILITY.md) | 195 | 39 | 13 | 143 | 20.0% |
| **Markers, Blips, ColShapes & Spatial** | [MARKERS_AND_SPATIAL.md](MARKERS_AND_SPATIAL.md) | 145 | 0 | 3 | 142 | 0.0% |
| **Web Browser & Compatibility (CEF / Discord)** | [BROWSER_AND_COMPATIBILITY.md](BROWSER_AND_COMPATIBILITY.md) | 103 | 0 | 0 | 103 | 0.0% |
| **TOTAL** | **All Categories** | **1581** | **161** | **178** | **1242** | **10.2%** |

## Implementation Priority Tiers

1. **Tier 1 (Core Gameplay & UI - Needed Now)**: `ELEMENTS.md`, `GUI_AND_DX.md`, `PEDS_AND_PLAYERS.md`, `VEHICLES.md`.
2. **Tier 2 (World & Audio - Needed for Ambience)**: `WORLD_AND_OBJECTS.md`, `AUDIO_AND_VOICE.md`, `CAMERA.md`, `WEAPONS_AND_COMBAT.md`.
3. **Tier 3 (Modding & Advanced UI - Long Term)**: `ENGINE_AND_MODELS.md`, `MARKERS_AND_SPATIAL.md`, `BROWSER_AND_COMPATIBILITY.md`.
