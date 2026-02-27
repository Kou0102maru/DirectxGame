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


static MapObject g_MapObjects[]{
	{FIELD,   { 0.0f, 0.0f,  0.0f}, {{-25.0f, -1.0f, -25.0f}, {25.0f, 0.0f, 25.0f}}},
};


static int g_CubeTexHoroId{ -1 };
static int g_CubeTexWoodBoxId{ -1 };
static MODEL* g_pRock01{};


void Map_Initialize()
{
	g_CubeTexWoodBoxId = Texture_Load(L"resource/texture/woodbox.png");
	g_CubeTexHoroId = Texture_Load(L"resource/texture/horo.png");

	g_pRock01 = ModelLoad("resource/model/Rock_01.fbx");


	for (MapObject& o : g_MapObjects) {
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

	for (const MapObject& o : g_MapObjects) {
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
		}
	}
}

int Map_GetObjectsCount()
{
	return sizeof(g_MapObjects)/sizeof(g_MapObjects[0]);
}

const MapObject* Map_GetObject(int index)
{
	return &g_MapObjects[index];
}
