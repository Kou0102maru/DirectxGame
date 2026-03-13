/*==============================================================================

   プレイヤー用カメラ処理 [player_camera.cpp]
														 Author : Youhei Sato
														 Date   : 2025/10/31
--------------------------------------------------------------------------------

==============================================================================*/
#include "player_camera.h"

#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "player.h"
#include "key_logger.h"
#include "pad_logger.h"



static XMFLOAT3 g_CameraFront{ 0.0f, 0.0f, 1.0f };
static XMFLOAT3 g_CameraPosition{ 0.0f, 0.0f, 0.0f };
static XMFLOAT4X4 g_CameraMatrix{};
static XMFLOAT4X4 g_CameraPerspectiveMatrix{};

// カメラ回転角度（ラジアン）
static float g_CameraYaw = 0.0f;    // 水平回転
static float g_CameraPitch = 0.43f; // 垂直角度（元カメラ: atan2(3,6.5)≒0.43）
static const float CAMERA_DISTANCE = 7.2f;  // プレイヤーからの距離（元: sqrt(6.52+32)≒7.16）
static const float CAMERA_ROT_SPEED = 2.0f; // 回転速度（ラジアン/秒）
static const float PITCH_MIN = 0.1f;
static const float PITCH_MAX = 1.2f;


void PlayerCamera_Initialize()
{
	g_CameraYaw = 0.0f;
	g_CameraPitch = 0.43f;
}

void PlayerCamera_Finalize()
{
}

void PlayerCamera_Update(double elapsed_time)
{
	float dt = (float)elapsed_time;

	// --- カメラ回転入力 ---
	float yawInput = 0.0f;
	float pitchInput = 0.0f;

	// 矢印キー
	if (KeyLogger_IsPressed(KK_LEFT))  yawInput += 1.0f;
	if (KeyLogger_IsPressed(KK_RIGHT)) yawInput -= 1.0f;
	if (KeyLogger_IsPressed(KK_UP))    pitchInput += 1.0f;
	if (KeyLogger_IsPressed(KK_DOWN))  pitchInput -= 1.0f;

	// 右スティック
	XMFLOAT2 rs = PadLogger_GetRightThumbStick(0);
	yawInput -= rs.x;
	pitchInput -= rs.y; // スティック上でピッチ増加（見上げ）

	g_CameraYaw += yawInput * CAMERA_ROT_SPEED * dt;
	g_CameraPitch += pitchInput * CAMERA_ROT_SPEED * dt;
	if (g_CameraPitch < PITCH_MIN) g_CameraPitch = PITCH_MIN;
	if (g_CameraPitch > PITCH_MAX) g_CameraPitch = PITCH_MAX;

	// --- 注視点（元カメラと同じくプレイヤーのXZ座標、Y=0） ---
	XMVECTOR playerPos = XMLoadFloat3(&Player_GetPosition());
	XMVECTOR target = playerPos * XMVECTOR{1.0f, 0.0f, 1.0f, 0.0f};

	// --- 球面座標でカメラ位置を計算 ---
	float camX = CAMERA_DISTANCE * cosf(g_CameraPitch) * sinf(g_CameraYaw);
	float camY = CAMERA_DISTANCE * sinf(g_CameraPitch);
	float camZ = -CAMERA_DISTANCE * cosf(g_CameraPitch) * cosf(g_CameraYaw);

	XMVECTOR position = target + XMVECTOR{camX, camY, camZ, 0.0f};

	XMVECTOR front = XMVector3Normalize(target - position);
	XMStoreFloat3(&g_CameraPosition, position);
	XMStoreFloat3(&g_CameraFront, front);

	// ビュー座標変換行列作成
	XMMATRIX mtxView = XMMatrixLookAtLH(
		position,
		target,
		{ 0.0f, 1.0f, 0.0f }); // 上方向

	// カメラ行列保存
	XMStoreFloat4x4(&g_CameraMatrix, mtxView);

	// パースペクティブ行列作成
	float aspectRatio = (float)Direct3D_GetBackBufferWidth() / Direct3D_GetBackBufferHeight();
	float nearz = 0.1f;
	float farz = 400.0f;
	XMMATRIX mtxPerspective = XMMatrixPerspectiveFovLH(1.0f, aspectRatio, nearz, farz);

	// パースペクティブ行列保存
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
