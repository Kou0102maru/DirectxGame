/*==============================================================================

	マップの管理 [map.h]
														 Author : Youhei Sato
														 Date   : 2025/10/10
--------------------------------------------------------------------------------

==============================================================================*/
#include "map.h"
using namespace DirectX;
#include "cube.h"
#include "texture.h"
#include "meshfield.h"
#include "light.h"
#include "player_camera.h"
#include "model.h"


// ステージ1のマップデータ
static MapObject g_MapObjectsStage1[]{
	// 地面
	{FIELD,   { 0.0f, 0.0f,  0.0f}, {{-32.0f, -1.0f, -42.0f}, {32.0f, 0.0f, 60.0f}}},

	//=======================================================================
	// 遺跡迷路の壁（ジグザグ構造）
	// エリア: X=-32~+32, Z=-42~+60
	// 壁の高さ: Y=0~12
	//=======================================================================

	// --- 外壁 ---
	// 南壁（入口なし・完全に閉鎖）
	{WALL, {0,0,0}, {{-32.0f, 0.0f, -42.0f}, {32.0f, 12.0f, -40.0f}}},
	// 北壁
	{WALL, {0,0,0}, {{-32.0f, 0.0f, 58.0f}, {32.0f, 12.0f, 60.0f}}},
	// 西壁
	{WALL, {0,0,0}, {{-32.0f, 0.0f, -42.0f}, {-30.0f, 12.0f, 60.0f}}},
	// 東壁
	{WALL, {0,0,0}, {{ 30.0f, 0.0f, -42.0f}, {32.0f, 12.0f, 60.0f}}},

	// --- 内壁（ジグザグ通路を作る） ---
	// 内壁1 (Z=-15): 西壁~X=+10、右側に通路（X=+10~+30）
	{WALL, {0,0,0}, {{-30.0f, 0.0f, -16.0f}, {10.0f, 12.0f, -14.0f}}},
	// 内壁2 (Z=+15): X=-10~東壁、左側に通路（X=-30~-10）
	{WALL, {0,0,0}, {{-10.0f, 0.0f, 14.0f}, {30.0f, 12.0f, 16.0f}}},
	// 内壁3 (Z=+40): 西壁~X=+10、右側に通路（X=+10~+30）
	{WALL, {0,0,0}, {{-30.0f, 0.0f, 39.0f}, {10.0f, 12.0f, 41.0f}}},

	// --- 天井（ミニマップ描画時はスキップ） ---
	{CEILING, {0,0,0}, {{-32.0f, 12.0f, -42.0f}, {32.0f, 12.5f, 60.0f}}},
};

// ステージ2のマップデータ（内壁が逆ジグザグ）
static MapObject g_MapObjectsStage2[]{
	// 地面
	{FIELD,   { 0.0f, 0.0f,  0.0f}, {{-32.0f, -1.0f, -42.0f}, {32.0f, 0.0f, 60.0f}}},

	// --- 外壁（ステージ1と同じ・入口なし） ---
	{WALL, {0,0,0}, {{-32.0f, 0.0f, -42.0f}, {32.0f, 12.0f, -40.0f}}},
	{WALL, {0,0,0}, {{-32.0f, 0.0f, 58.0f}, {32.0f, 12.0f, 60.0f}}},
	{WALL, {0,0,0}, {{-32.0f, 0.0f, -42.0f}, {-30.0f, 12.0f, 60.0f}}},
	{WALL, {0,0,0}, {{ 30.0f, 0.0f, -42.0f}, {32.0f, 12.0f, 60.0f}}},

	// --- 内壁（逆ジグザグ：通路が左右反転） ---
	// 内壁1 (Z=-15): X=-10~東壁、左側に通路（X=-30~-10）
	{WALL, {0,0,0}, {{-10.0f, 0.0f, -16.0f}, {30.0f, 12.0f, -14.0f}}},
	// 内壁2 (Z=+15): 西壁~X=+10、右側に通路（X=+10~+30）
	{WALL, {0,0,0}, {{-30.0f, 0.0f, 14.0f}, {10.0f, 12.0f, 16.0f}}},
	// 内壁3 (Z=+40): X=-10~東壁、左側に通路（X=-30~-10）
	{WALL, {0,0,0}, {{-10.0f, 0.0f, 39.0f}, {30.0f, 12.0f, 41.0f}}},

	// --- 天井 ---
	{CEILING, {0,0,0}, {{-32.0f, 12.0f, -42.0f}, {32.0f, 12.5f, 60.0f}}},
};

// 現在のマップデータへのポインタ
static MapObject* g_pCurrentMap = g_MapObjectsStage1;
static int g_CurrentMapCount = sizeof(g_MapObjectsStage1) / sizeof(g_MapObjectsStage1[0]);


static int g_CubeTexHoroId{ -1 };
static int g_CubeTexWoodBoxId{ -1 };
static int g_CubeTexWallId{ -1 };   // 壁テクスチャ（kabe.png）
static MODEL* g_pRock01{};
static bool g_IsMinimapMode = false; // ミニマップ描画中フラグ


void Map_Initialize()
{
	g_CubeTexWoodBoxId = Texture_Load(L"resource/texture/woodbox.png");
	g_CubeTexHoroId = Texture_Load(L"resource/texture/horo.png");
	g_CubeTexWallId = Texture_Load(L"resource/texture/kabe.png");

	g_pRock01 = ModelLoad("resource/model/Rock_01.fbx");


	for (int i = 0; i < g_CurrentMapCount; i++) {
		MapObject& o = g_pCurrentMap[i];
		if (o.KindId == BLOCK || o.KindId == WOODBOX) {
			o.Aabb = Cube_GetAABB(o.Position);
		}
		else if (o.KindId == ROCK01) {
			o.Aabb = Model_GetAABB(g_pRock01, o.Position);
		}
	}
}

void Map_Finalize()
{
	ModelRelease(g_pRock01);
}

void Map_Draw()
{
	XMMATRIX mtxWorld;

	for (int i = 0; i < g_CurrentMapCount; i++) {
		const MapObject& o = g_pCurrentMap[i];
		switch (o.KindId)
		{
		case FIELD:
			Light_SetSpecularWorld(PlayerCamera_GetPosition(), 50.0f, { 0.3f, 0.3f, 0.3f, 1.0f });
			MeshField_Draw();
			break;

		case BLOCK:
			mtxWorld = XMMatrixTranslation(o.Position.x, o.Position.y, o.Position.z);
			Light_SetSpecularWorld(PlayerCamera_GetPosition(), 50.0f, { 0.3f, 0.1f, 0.1f, 1.0f });
			Cube_Draw(g_CubeTexHoroId, mtxWorld);
			break;
		
		case WOODBOX:
			mtxWorld = XMMatrixTranslation(o.Position.x, o.Position.y, o.Position.z);
			Light_SetSpecularWorld(PlayerCamera_GetPosition(), 4.0f, { 0.1f, 0.1f, 0.1f, 1.0f });
			Cube_Draw(g_CubeTexWoodBoxId, mtxWorld);
			break;

		case ROCK01:
			mtxWorld = XMMatrixTranslation(o.Position.x, o.Position.y, o.Position.z);
			Light_SetSpecularWorld(PlayerCamera_GetPosition(), 1.0f, { 0.0f, 0.0f, 0.0f, 1.0f });
			ModelDraw(g_pRock01, mtxWorld);
			break;

		case WALL:
		{
			// AABBの中心とサイズを計算してキューブをタイリング描画
			float cx = (o.Aabb.min.x + o.Aabb.max.x) * 0.5f;
			float cy = (o.Aabb.min.y + o.Aabb.max.y) * 0.5f;
			float cz = (o.Aabb.min.z + o.Aabb.max.z) * 0.5f;
			float sx = o.Aabb.max.x - o.Aabb.min.x;
			float sy = o.Aabb.max.y - o.Aabb.min.y;
			float sz = o.Aabb.max.z - o.Aabb.min.z;
			mtxWorld = XMMatrixScaling(sx, sy, sz) * XMMatrixTranslation(cx, cy, cz);
			Light_SetSpecularWorld(PlayerCamera_GetPosition(), 10.0f, { 0.15f, 0.1f, 0.05f, 1.0f });
			Cube_DrawTiled(g_CubeTexWallId, mtxWorld, sx, sy, sz);
			break;
		}

		case CEILING:
		{
			// ミニマップ描画時は天井をスキップ（上から見えるようにする）
			if (g_IsMinimapMode) break;

			float cx = (o.Aabb.min.x + o.Aabb.max.x) * 0.5f;
			float cy = (o.Aabb.min.y + o.Aabb.max.y) * 0.5f;
			float cz = (o.Aabb.min.z + o.Aabb.max.z) * 0.5f;
			float sx = o.Aabb.max.x - o.Aabb.min.x;
			float sy = o.Aabb.max.y - o.Aabb.min.y;
			float sz = o.Aabb.max.z - o.Aabb.min.z;
			mtxWorld = XMMatrixScaling(sx, sy, sz) * XMMatrixTranslation(cx, cy, cz);
			Light_SetSpecularWorld(PlayerCamera_GetPosition(), 10.0f, { 0.15f, 0.1f, 0.05f, 1.0f });
			Cube_DrawTiled(g_CubeTexWallId, mtxWorld, sx, sy, sz);
			break;
		}
		}
	}
}

void Map_SetMinimapMode(bool minimap)
{
	g_IsMinimapMode = minimap;
}

int Map_GetObjectsCount()
{
	return g_CurrentMapCount;
}

const MapObject* Map_GetObject(int index)
{
	return &g_pCurrentMap[index];
}

void Map_SetStage(int stage)
{
	if (stage == 2) {
		g_pCurrentMap = g_MapObjectsStage2;
		g_CurrentMapCount = sizeof(g_MapObjectsStage2) / sizeof(g_MapObjectsStage2[0]);
	}
	else {
		g_pCurrentMap = g_MapObjectsStage1;
		g_CurrentMapCount = sizeof(g_MapObjectsStage1) / sizeof(g_MapObjectsStage1[0]);
	}
}

void Map_CollideWithWalls(XMFLOAT3& position, float radius)
{
	for (int i = 0; i < g_CurrentMapCount; i++) {
		const MapObject& obj = g_pCurrentMap[i];
		if (obj.KindId != WALL) continue;

		AABB monster_aabb;
		monster_aabb.min = { position.x - radius, position.y - 0.5f, position.z - radius };
		monster_aabb.max = { position.x + radius, position.y + 0.5f, position.z + radius };

		Hit hit = Collision_IsHitAABB(obj.Aabb, monster_aabb);
		if (hit.isHit) {
			if (hit.noraml.x > 0.0f)
				position.x = obj.Aabb.max.x + radius;
			else if (hit.noraml.x < 0.0f)
				position.x = obj.Aabb.min.x - radius;
			else if (hit.noraml.z > 0.0f)
				position.z = obj.Aabb.max.z + radius;
			else if (hit.noraml.z < 0.0f)
				position.z = obj.Aabb.min.z - radius;
		}
	}
}
