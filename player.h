/*==============================================================================

    プレイヤー制御 [player.h]
														 Author : Youhei Sato
														 Date   : 2025/10/31
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef PLAYER_H
#define PLAYER_H

#include <DirectXMath.h>
#include "collision.h"


void Player_Initialize(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& front);
void Player_Finalize();
void Player_Update(double elapsed_time);
void Player_Draw();
void Player_DepthDraw();

const DirectX::XMFLOAT3& Player_GetPosition();
const DirectX::XMFLOAT3& Player_GetFront();
void Player_SetPosition(const DirectX::XMFLOAT3& position);  // ★追加★
AABB Player_GetAABB();

AABB Player_ConvertPositionToAABB(const DirectX::XMVECTOR& position);

#endif // PLAYER_H
