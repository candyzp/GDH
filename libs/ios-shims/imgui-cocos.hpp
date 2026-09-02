#pragma once

// Shared GDH hack files historically include imgui-cocos.hpp for ImGui types.
// iOS uses GDH's native Cocos mobile interface instead of gd-imgui-cocos, so
// expose only the plain ImGui API here and avoid unsupported input/render hooks.
#include "imgui.h"
