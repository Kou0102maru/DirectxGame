/*==============================================================================

    É}ÉbÉvÇÃä«óù [map.h]
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
};

struct MapObject
{
	int KindId;
	DirectX::XMFLOAT3 Position;
	AABB Aabb;
};

const MapObject* Map_GetObject(int index);


#endif // MAP__H
