#pragma once 

// #define DLLCOMPILE 1
#ifdef DLLCOMPILE
#include "imgui/imgui_demo.cpp"
#include "imgui/imgui_draw.cpp"
#include "Imgui/imgui.cpp"
// #include "Imgui/imgui_impl_a5.h"
#endif

#include "game.h"
#define internal2 static

struct ImGuiContext;
#define IMGUIFUNC(name) void name(EngineCore* core, ImGuiContext* context, void* tmpData, AssetFileInfo* assetInfo)
typedef IMGUIFUNC(Imgui_func);
EXPORT  IMGUIFUNC(Imgui);
