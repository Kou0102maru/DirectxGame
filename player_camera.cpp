/*==============================================================================

   プレイヤー用のカメラ制御 [player_camera.cpp]
														 Author : Youhei Sato
														 Date   : 2025/10/31
--------------------------------------------------------------------------------

==============================================================================*/
#include "player_camera.h"

#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "player.h"


static XMFLOAT3 g_CameraFront{ 0.0f, 0.0f, 1.0f };
static XMFLOAT3 g_CameraPosition{ 0.0f, 0.0f, 0.0f };
static XMFLOAT4X4 g_CameraMatrix{};
static XMFLOAT4X4 g_CameraPerspectiveMatrix{};


void PlayerCamera_Initialize()
{

}

void PlayerCamera_Finalize()
{
}

void PlayerCamera_Update(double elapsed_time)
{
	// XMVECTOR position = XMLoadFloat3(&Player_GetPosition()) - XMLoadFloat3(&Player_GetFront()) * 5.0f;
	XMVECTOR position = XMLoadFloat3(&Player_GetPosition());
	position *= {1.0f, 0.0f, 1.0f};

	XMVECTOR target = position;

	position += {2.5f, 3.0f, -6.0f};

	XMVECTOR front = XMVector3Normalize(target - position);
	XMStoreFloat3(&g_CameraPosition, position);
	XMStoreFloat3(&g_CameraFront, front);

	// ビュー座標変換行列の作成
	XMMATRIX mtxView = XMMatrixLookAtLH(
		position,
		target,
		{ 0.0f, 1.0f, 0.0f }); // 上向き

	// カメラ行列を保存
	XMStoreFloat4x4(&g_CameraMatrix, mtxView);

	// パースペクティブ行列の作成
	float aspectRatio = (float)Direct3D_GetBackBufferWidth() / Direct3D_GetBackBufferHeight();
	float nearz = 0.1f;
	float farz = 400.0f;
	XMMATRIX mtxPerspective = XMMatrixPerspectiveFovLH(1.0f, aspectRatio, nearz, farz);

	// パースペクティブ行列の保存
	XMStoreFloat4x4(&g_CameraPerspectiveMatrix, mtxPerspective);
}

const DirectX::XMFLOAT3& PlayerCamera_GetFront()
{
	return g_CameraFront;
}

const DirectX::XMFLOAT3& PlayerCamera_GetPosition()
{
	return g_CameraPosition;
}

const DirectX::XMFLOAT4X4& PlayerCamera_GetViewMatrix()
{
	return g_CameraMatrix;
}

const DirectX::XMFLOAT4X4& PlayerCamera_GetPerspectiveMatrix()
{
	return g_CameraPerspectiveMatrix;
}
