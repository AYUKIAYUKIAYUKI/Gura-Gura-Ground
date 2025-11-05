//============================================================================
// 
// プリコンパイルヘッダー [pch.h]
// Author : 福田歩希
// 
//============================================================================

#pragma once

//****************************************************
// マクロ定義
//****************************************************

// メモリリーク検出用
#define _CRTDBG_MAP_ALLOC

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

//****************************************************
// インクルードファイル
//****************************************************

// C++用
#include <array>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// ウィンドウズ用
#include <Windows.h>
#include <wrl/client.h>

// メモリリーク検出用
#include <crtdbg.h>
#include <stdlib.h>

// DX11用
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// assimp
//#include <assimp/Importer.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>

// B3
//#include "btBulletDynamicsCommon.h"

// ImGui用
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

// JSON用
#include <nlohmann/json.hpp>

// lua/sol2用
#include <sol/sol.hpp>

// 自作
#include "API.useful.h"

//****************************************************
// ライブラリのリンク
//****************************************************
#pragma comment(lib, "winmm.lib") // 実行時間制御のため
#pragma comment(lib, "d3d11.lib") // DirectX11使用のため

//****************************************************
// 型エイリアスを定義
//****************************************************
using JSON = nlohmann::json;

//****************************************************
// usingディレクティブ
//****************************************************
using namespace Microsoft::WRL;