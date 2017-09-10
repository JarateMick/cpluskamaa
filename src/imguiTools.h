#pragma once 
// #define EXPORT extern "C" __declspec( dllexport )
// #define ArrayCount(array) ( sizeof(array) / sizeof((array)[0]))

// #include <cstring>
// #include <cstdlib>
// #include <cstdint>
// #include <vector>
// #include "TextureHolder.h"
// #include "gl3w/GL/gl3w.c" // used

// #define DLLCOMPILE 1
#ifdef DLLCOMPILE
#include "imgui/imgui_demo.cpp"
#include "imgui/imgui_draw.cpp"
#include "Imgui/imgui.cpp"
#include "Imgui/imgui_impl_a5.h"
#endif

#include "game.h"
// #include "utility.h"

#define internal static

struct ImGuiContext;
#define IMGUIFUNC(name) void name(EngineCore* core, ImGuiContext* context, void* tmpData, AssetFileInfo* assetInfo)
typedef IMGUIFUNC(Imgui_func);
EXPORT  IMGUIFUNC(Imgui);
