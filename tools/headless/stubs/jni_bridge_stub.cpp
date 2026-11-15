// Host replacement for WorldRPG::JniBridge (jni/JNI_OnLoad.cpp is Android-only).
// Game lines that the Android client would show in the GameActivity are printed
// to stdout instead, so chat/progress text from the server stays visible.
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace WorldRPG::JniBridge {
void PostGameLine(int kind, const char* text) {
    if (text && *text) std::printf("[GAMELINE kind=%d] %s\n", kind, text);
}
// No Activity on the host: the caller falls back to its fixed-width estimate.
float MeasureText(const std::string&, float, const std::string&, bool) { return -1.f; }
// No Android bitmap codecs on the host.
bool LoadPixels(const std::string&, std::vector<uint32_t>&, unsigned&, unsigned&) { return false; }
bool DecodePixels(const char*, size_t, std::vector<uint32_t>&, unsigned&, unsigned&) { return false; }
bool EncodePixels(const std::vector<uint32_t>&, unsigned, unsigned, bool, int, std::string&) { return false; }
bool StorePixels(const std::string&, const std::vector<uint32_t>&, unsigned, unsigned) { return false; }
void CopyScreen(const std::string&, unsigned, unsigned) {}
// No SVG renderer on the host: the document is kept without a picture.
int RenderSvg(const std::string&, const std::string&, unsigned, unsigned) { return -1; }
} // namespace WorldRPG::JniBridge
