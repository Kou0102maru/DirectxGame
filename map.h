/*==============================================================================

    マップの管理 [map.h]
														 Author : Youhei Sato
														 Date   : 2025/10/10
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef MAP_H
#define MAP_H

#include <DirectXMath.h>
#include "collision.h"


void Map_Initialize();
void Map_Finalize();

// void Map_Update(double elapsed_time);
void Map_Draw();

int Map_GetObjectsCount();

enum ObjectKind
{
	FIELD,
	BLOCK,
	WOODBOX,
	ROCK01,
	WALL,		// 迷路の壁（AABBで直接サイズ指定、スケーリング描画）
	CEILING,	// 天井（ミニマップ描画時はスキップ）
};

struct MapObject
{
	int KindId;
	DirectX::XMFLOAT3 Position;
	AABB Aabb;
};

const MapObject* Map_GetObject(int index);
void Map_SetMinimapMode(bool minimap);
void Map_SetStage(int stage);
void Map_CollideWithWalls(DirectX::XMFLOAT3& position, float radius);


#endif // MAP__H
