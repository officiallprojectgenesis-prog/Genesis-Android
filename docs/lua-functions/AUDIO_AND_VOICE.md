# Audio & Voice System (CLuaAudioDefs)

2D audio playback, 3D spatial sounds, internet radio streams, sound effects, volume/pitch/pan, sound tags, and Opus voice chat.

## Parity Status

- **Total Functions:** 57
- **Implemented:** 19 (33.3%)
- **Stubs:** 1
- **Missing:** 37

> **Technical Requirement:** Desktop uses BASS library. Mobile has Opus 1.4 compiled; requires OpenSL ES / AudioTrack output mixer, URL streaming, and 3D listener attenuation math.

## Function List

| Lua Function | Status | Desktop Source Ref | Desktop Handler | Conversion Requirement / Implementation Notes |
|---|---|---|---|---|
| `getRadioChannel` | Missing | `CLuaAudioDefs.cpp:227` | `GetRadioChannel` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getRadioChannelName` | Missing | `CLuaAudioDefs.cpp:228` | `GetRadioChannelName` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getSFXStatus` | Missing | `CLuaAudioDefs.cpp:180` | `GetSFXStatus` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `getSoundBPM` | Missing | `CLuaAudioDefs.cpp:210` | `GetSoundBPM` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `getSoundBufferLength` | Missing | `CLuaAudioDefs.cpp:196` | `GetSoundBufferLength` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `getSoundEffectParameters` | Missing | `CLuaAudioDefs.cpp:221` | `GetSoundEffectParameters` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `getSoundEffects` | Missing | `CLuaAudioDefs.cpp:219` | `GetSoundEffects` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `getSoundFFTData` | Missing | `CLuaAudioDefs.cpp:207` | `GetSoundFFTData` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `getSoundLength` | Missing | `CLuaAudioDefs.cpp:195` | `GetSoundLength` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `getSoundLevelData` | Missing | `CLuaAudioDefs.cpp:209` | `GetSoundLevelData` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `getSoundMaxDistance` | **Implemented** | `CLuaAudioDefs.cpp:216` | `GetSoundMaxDistance` | Operational on Android. (l_getSoundMaxDistance) |
| `getSoundMetaTags` | Missing | `CLuaAudioDefs.cpp:217` | `GetSoundMetaTags` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `getSoundMinDistance` | **Implemented** | `CLuaAudioDefs.cpp:214` | `GetSoundMinDistance` | Operational on Android. (l_getSoundMinDistance) |
| `getSoundPan` | **Implemented** | `CLuaAudioDefs.cpp:223` | `GetSoundPan` | Operational on Android. (l_getSoundPan) |
| `getSoundPosition` | **Implemented** | `CLuaAudioDefs.cpp:194` | `GetSoundPosition` | Operational on Android. (l_getSoundPosition) |
| `getSoundProperties` | Missing | `CLuaAudioDefs.cpp:206` | `GetSoundProperties` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `getSoundSpeed` | **Implemented** | `CLuaAudioDefs.cpp:204` | `GetSoundSpeed` | Operational on Android. (l_getSoundSpeed) |
| `getSoundVolume` | **Implemented** | `CLuaAudioDefs.cpp:202` | `GetSoundVolume` | Operational on Android. (l_getSoundVolume) |
| `getSoundWaveData` | Missing | `CLuaAudioDefs.cpp:208` | `GetSoundWaveData` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `isAmbientSoundEnabled` | Missing | `CLuaAudioDefs.cpp:173` | `IsAmbientSoundEnabled` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isMissionAudioFinished` | Missing | `CLuaAudioDefs.cpp:184` | `ArgumentParser<IsMissionAudioFinished>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isMissionAudioLoaded` | Missing | `CLuaAudioDefs.cpp:182` | `ArgumentParser<IsMissionAudioLoaded>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isShowSoundEnabled` | Missing | `CLuaAudioDefs.cpp:232` | `ArgumentParser<IsShowSoundEnabled>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isSoundLooped` | **Implemented** | `CLuaAudioDefs.cpp:198` | `ArgumentParser<IsSoundLooped>` | Operational on Android. (l_isSoundLooped) |
| `isSoundPanningEnabled` | Missing | `CLuaAudioDefs.cpp:212` | `IsSoundPanEnabled` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `isSoundPaused` | **Implemented** | `CLuaAudioDefs.cpp:200` | `IsSoundPaused` | Operational on Android. (l_isSoundPaused) |
| `isWorldSoundEnabled` | Missing | `CLuaAudioDefs.cpp:176` | `IsWorldSoundEnabled` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `playMissionAudio` | Missing | `CLuaAudioDefs.cpp:183` | `ArgumentParser<PlayMissionAudio>` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `playMissionPassedTune` | Missing | `CLuaAudioDefs.cpp:171` | `ArgumentParser<PlayMissionPassedTune>` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `playSFX` | Missing | `CLuaAudioDefs.cpp:178` | `PlaySFX` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `playSFX3D` | Missing | `CLuaAudioDefs.cpp:179` | `PlaySFX3D` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `playSound` | **Implemented** | `CLuaAudioDefs.cpp:190` | `PlaySound` | Operational on Android. (l_playSound) |
| `playSound3D` | *Stub* | `CLuaAudioDefs.cpp:191` | `PlaySound3D` | Currently stubbed: `DefaultValue(kFalse)`. Needs native engine wiring. |
| `playSoundFrontEnd` | **Implemented** | `CLuaAudioDefs.cpp:170` | `PlaySoundFrontEnd` | Operational on Android. (l_playSoundFrontEnd) |
| `releaseMissionAudio` | Missing | `CLuaAudioDefs.cpp:185` | `ArgumentParser<ReleaseMissionAudio>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `reportPositionMissionAudioEvent` | Missing | `CLuaAudioDefs.cpp:186` | `ArgumentParser<ReportPositionMissionAudioEvent>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `reportVehicleMissionAudioEvent` | Missing | `CLuaAudioDefs.cpp:187` | `ArgumentParser<ReportVehicleMissionAudioEvent>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `requestMissionAudio` | Missing | `CLuaAudioDefs.cpp:181` | `ArgumentParser<RequestMissionAudio>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `resetAmbientSounds` | Missing | `CLuaAudioDefs.cpp:174` | `ResetAmbientSounds` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `resetWorldSounds` | Missing | `CLuaAudioDefs.cpp:177` | `ResetWorldSounds` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setAmbientSoundEnabled` | Missing | `CLuaAudioDefs.cpp:172` | `SetAmbientSoundEnabled` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setRadioChannel` | Missing | `CLuaAudioDefs.cpp:226` | `SetRadioChannel` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `setSoundEffectEnabled` | Missing | `CLuaAudioDefs.cpp:218` | `SetSoundEffectEnabled` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `setSoundEffectParameter` | Missing | `CLuaAudioDefs.cpp:220` | `SetSoundEffectParameter` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `setSoundLooped` | **Implemented** | `CLuaAudioDefs.cpp:197` | `ArgumentParser<SetSoundLooped>` | Operational on Android. (l_setSoundLooped) |
| `setSoundMaxDistance` | **Implemented** | `CLuaAudioDefs.cpp:215` | `SetSoundMaxDistance` | Operational on Android. (l_setSoundMaxDistance) |
| `setSoundMinDistance` | **Implemented** | `CLuaAudioDefs.cpp:213` | `SetSoundMinDistance` | Operational on Android. (l_setSoundMinDistance) |
| `setSoundPan` | **Implemented** | `CLuaAudioDefs.cpp:222` | `SetSoundPan` | Operational on Android. (l_setSoundPan) |
| `setSoundPanningEnabled` | Missing | `CLuaAudioDefs.cpp:211` | `SetSoundPanEnabled` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `setSoundPaused` | **Implemented** | `CLuaAudioDefs.cpp:199` | `SetSoundPaused` | Operational on Android. (l_setSoundPaused) |
| `setSoundPosition` | **Implemented** | `CLuaAudioDefs.cpp:193` | `SetSoundPosition` | Operational on Android. (l_setSoundPosition) |
| `setSoundProperties` | Missing | `CLuaAudioDefs.cpp:205` | `SetSoundProperties` | Needs AudioTrack / OpenSL ES audio playback mixer. |
| `setSoundSpeed` | **Implemented** | `CLuaAudioDefs.cpp:203` | `SetSoundSpeed` | Operational on Android. (l_setSoundSpeed) |
| `setSoundVolume` | **Implemented** | `CLuaAudioDefs.cpp:201` | `SetSoundVolume` | Operational on Android. (l_setSoundVolume) |
| `setWorldSoundEnabled` | Missing | `CLuaAudioDefs.cpp:175` | `SetWorldSoundEnabled` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `showSound` | Missing | `CLuaAudioDefs.cpp:231` | `ArgumentParser<ShowSound>` | Needs desktop C++ logic ported to app/src/main/cpp/shared. |
| `stopSound` | **Implemented** | `CLuaAudioDefs.cpp:192` | `StopSound` | Operational on Android. (l_stopSound) |
